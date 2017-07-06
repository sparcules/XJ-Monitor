/*********************************************************************
 * @file          XJ_Cab_Display.ino
 * @authors       Kevin Carlborg
 * @version       V1.0
 * @date          14-May-2017
 * @targetDevice  Teensy 3.2
 * @description   Drives the SSD1306 128x64 OLED display in my Jeep Cherokee
 *                Display time, date, voltage, engine temp, air temp
 *                Uses two push button switches as inputs to navigate through menus
 *                Provides an interface to change the DR44 alternator voltage
 *                
 * @howTo         Press the mode button to enter the menu. Press the mode button again
 *                to advance through the menu options. Press the select button to select
 *                an item.
 *                Alternator menu Options:
 *                Turbo mode - Bumps the voltage up to 14.8V this value is set in the DR44 Alternator sketch
 *                SetVoltage - Sets a new voltage set point in the DR44 Arduino. First read the current 
 *                             value. Then open an adjustment diaglog with that current value. Hit the select 
 *                             button to cycle through values  on the highlighted digit. Hit the mode button to 
 *                             advance to the next digit. Advancing after the decimal digit exits the adjustment 
 *                             dialog and sends the updated value to the DR44 Arduino. Then waits for a confirmation.
 *                Save V Set - Tell the DR44 Arduino to save this new set point in EEPROM
 *                Target V   - Read the current target voltage from the DR44 Arduino
 *                Target PWM - Read the current PWM target from the DR44 Arduino
 *                
 * @libraries     Adafruit_GFX.h/Adafruit_SSD1306.h to drive the display
 *                RTClib.h to interface with DS3231 RTC via I2C
 *                qMenuSystem.h/qMenuDisplay.h source: https://github.com/dasaki/qMenuSystem
 *                   I heavily modified the menu library to suit my needs.
 *                EasyTransfer.h to communicate serially to the arduino under the hood
 *
*********************************************************************/
 

/**
 * Menu Library
 * source: https://github.com/dasaki/qMenuSystem
 * 
 */

#include <SPI.h>
#include <Wire.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EasyTransfer.h>
#include <qMenuDisplay.h>
#include <qMenuSystem.h>
#include <EEPROM.h>

#include "RTClib.h"
#include "XJ_Cab_Display.h"

/*********  OLED Library Defines  *********/
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
  //#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


/*********  RTC Library Defines  *********/
RTC_DS3231 rtc;
DateTime now;

/*********  Menu Library Defines  *********/
qMenuSystem menu = qMenuSystem(&display);


/*********  2-Way Serial Library Defines  *********/
EasyTransfer ETin, ETout;

struct RECEIVE_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int16_t dataName;
  int16_t dataIntValue;
  int16_t dataDecValue;
};

struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int16_t dataName;
  int16_t dataIntValue;
  int16_t dataDecValue;
};

//give a name to the group of data
RECEIVE_DATA_STRUCTURE rxdata;
SEND_DATA_STRUCTURE txdata;


/*********  Pin Defines  *********/
#define PB_MODE         2
#define PB_SELECT       3
#define ENGINE_STARTED  6
#define DASH_LIGHTS_ON  7
#define LED_PIN         13

/*********  EEPROM Defines  *********/
#define EEPROM_WRITTEN_VALUE   0x5A
#define EEPROM_WRITTEN_ADDR    0
#define MODE_ENABLE_ADDR       1

/*********  Time Defines  *********/
#define DELAYTIME             20
#define SIGNAL_DEBOUNCE_TIME  50
#define SW_DEBOUNCE_TIME      400
#define DISPLAY_INTERVAL      4000
#define SERIAL_TIMEOUT        3000  //was 1500
#define MSG_BOX_TIMEOUT       3000
#define TURBO_DIALOG_TIMEOUT  3000
#define MENU_TIMEOUT          10000
#define CLOCK_ADJ_TIMEOUT     20000

/*********  Text Size Defines  *********/
#define TEXT_SIZE_1   1
#define TEXT_SIZE_2   2
#define TEXT_SIZE_3   3
#define TEXT_SIZE_4   4

/*********  Global Defines  *********/
//#define NUM_MODES         4
#define SHOW_MENU         true
#define DONT_SHOW_MENU    false
#define TEXT_SIZE_2_MAX_CHAR_LENGTH 10
#define MIN_V_ADJUST      11
#define MAX_V_ADJUST      15
#define HIGHLIGHT_SINGLE  1
#define HIGHLIGHT_DOUBLE  2

/*********  Global Constants  *********/
//const unsigned long SW_DEBOUNCE_TIME = 600;
//const unsigned long MENU_TIMEOUT = 10000;

/*********  Global Variables  *********/
#define NUM_MODES 5 //The number of modes to cycle through in main view
enum displayMode {
  mode_time           = 0,
  mode_date           = 1,
  mode_voltage        = 2,
  mode_engine_temp    = 3,
  mode_air_temp       = 4,
  mode_target_voltage = 5,
  mode_alternator_pwm = 6,
  mode_set_voltage    = 7,
  mode_save_v_set     = 8,
  mode_turbo          = 9
};

static bool showMenuFlag;
static bool modeBtnPressed;
static bool selectBtnPressed;
static bool modeBtnState;
static bool selectBtnState;
static bool engineStartedFlag;
static bool turboMode;
uint8_t currentMode;
uint8_t numModes;
int8_t voltageIntVal = 0;
uint8_t voltageDecimal = 0; 
unsigned long last_millis_time;
static unsigned long last_interrupt_time;
float tempEngine;
float tempAir;
char strnBuf[20];
ModeEnableObject modeEnableVal;

/*********   Required Prototypes  *********/
int8_t displayMessageBox(const char msg[], int8_t textSize);
int8_t getAlternatorValue(int8_t dataName);
int8_t processVoltageAdjust(void);
int8_t processTurboMode(void);
int8_t processClockAdjust(void);
int8_t processDateAdjust(void);

void initEEPROM(void);
void requestAlternatorData(void);
void radiusLineSweep(int16_t xCenter, int16_t yCenter, int16_t degStart, int16_t degEnd, int16_t length, int8_t step, int8_t dir);
void setNumModes(void);
void showEngineStartedViz(void);
void wipeScreen(void);
void writeEepromModeEnables(void) ;
void writeScreen(uint8_t lineNum, uint8_t textSize, String value);

/*********   ISR Prototypes  *********/
void modeButtonPushedISR(void);
void selectButtonPushedISR(void);
//void engineStartedSignalISR(void);


/*********   ISR Prototypes  *********/
void modeButtonPushedISR(void) {
  //debounce switch
  if (millis() - last_interrupt_time > SW_DEBOUNCE_TIME) {
    last_interrupt_time = millis();
    modeBtnPressed = showMenuFlag = true;
  } 
}

void selectButtonPushedISR(void) {
  //debounce switch
  if (millis() - last_interrupt_time > SW_DEBOUNCE_TIME) {
    last_interrupt_time = millis();
    //if(showMenuFlag)
      selectBtnPressed = true;
  } 
}

/*
void engineStartedSignalISR(void) {
  //debounce signal
  //if (millis() - last_interrupt_time > SIGNAL_DEBOUNCE_TIME) {
    //last_interrupt_time = millis();
    engineStartedFlag = true;
 // } 
}
*/
void setup()   {       
  pinMode(PB_MODE, INPUT_PULLUP);
  pinMode(PB_SELECT, INPUT_PULLUP);
  pinMode(ENGINE_STARTED,INPUT_PULLDOWN);
  pinMode(DASH_LIGHTS_ON,INPUT_PULLDOWN);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);
  Serial1.begin(9600);
  ETin.begin(details(rxdata), &Serial1);
  ETout.begin(details(txdata), &Serial1);
  menu.InitMenu((const char ** )mnuRoot,cntRoot,1,TEXT_SIZE_2,DONT_SHOW_MENU); //init the menu

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000); 
  wipeScreen();
  // delay(2000);
  //showEngineStartedViz(); //test the viz

  // Init Variables
  currentMode = 0;
  last_millis_time = last_interrupt_time = 0;
  turboMode = showMenuFlag = modeBtnPressed = selectBtnPressed = false;
  engineStartedFlag =  false;
  modeBtnState = selectBtnState = LOW;
  txdata.dataName = rxdata.dataName = 0;
  txdata.dataIntValue = rxdata.dataIntValue = 0;
  tempEngine = tempAir = 0.0;

  initEEPROM();
  setNumModes();
  
  attachInterrupt(digitalPinToInterrupt(PB_MODE), modeButtonPushedISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(PB_SELECT), selectButtonPushedISR, FALLING);
  //attachInterrupt(digitalPinToInterrupt(ENGINE_STARTED), engineStartedSignalISR, RISING);

  /*** Init Real Time Clock ***/
  if (! rtc.begin()) {
    //Serial.println("Couldn't find RTC");
    writeScreen(0, TEXT_SIZE_2, "Error");  // Write Title
    writeScreen(1, TEXT_SIZE_2, "Couldn't\nfind RTC");
    delay(2000); 
  }
  else
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  if (rtc.lostPower()) {
    writeScreen(0, TEXT_SIZE_2, "Error");  // Write Title
    writeScreen(1, TEXT_SIZE_2, "RTC lost power");
    writeScreen(2, TEXT_SIZE_2, "lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    delay(2000);
  }
}

void loop() {
  bool advanceMode = false;
  if(showMenuFlag) {
    currentMode = 0;
    last_millis_time = 0;
    showMenu();
    setNumModes();
    advanceMode = true;
  }
  else {
    
    if(selectBtnPressed) {
        selectBtnPressed = false;
        advanceMode = true;
     }
     unsigned long currentMillis = millis();
     if (currentMillis - last_millis_time >= DISPLAY_INTERVAL || advanceMode) {
        if(numModes > 1 && !advanceMode)
          wipeScreen();
        last_millis_time = currentMillis;
        strcpy_P(strnBuf, (char*)pgm_read_word(&(modeNames[currentMode])));
        if(modeEnabled[currentMode] || advanceMode) {
          display.clearDisplay();
          now = rtc.now();
          switch(currentMode) {
            case mode_time:
              //writeScreen(0, 2, strnBuf);  // Write Title
              display.drawBitmap(0, 0,  icon16_clock, 16, 16, 1);
              char minutesText[3];
              factorMinutes(minutesText, now.minute());
              sprintf (strnBuf, "%2i:%s", now.hour()==0 ? 12 : now.hour()>12 ? now.hour()-12 : now.hour(),minutesText);
              writeScreen(1, TEXT_SIZE_4, strnBuf);   // Write Value1
              sprintf(strnBuf,"        %s",now.hour()<12 ? "am" : "pm");
              writeScreen(3, TEXT_SIZE_2, strnBuf);   // Write Value1
              break;
            case mode_date:
              //writeScreen(0, 2, strnBuf);  // Write Title
              display.drawBitmap(0, 0,  icon16_calendar, 16, 16, 1);
              strcpy_P(strnBuf, (char*)pgm_read_word(&(daysOfTheWeek[now.dayOfTheWeek()])));
              writeScreen(1, TEXT_SIZE_4, strnBuf);   // Write Value1
              //Crude way to right justify the date
              if(now.month()<10 && now.day()<10)
                sprintf (strnBuf, "  %i/%i/%i",now.month(),now.day(),now.year());
              else if(now.month()<10 || now.day()<10)
                sprintf (strnBuf, " %i/%i/%i",now.month(),now.day(),now.year());
              else
                sprintf (strnBuf, "%i/%i/%i",now.month(),now.day(),now.year());
              writeScreen(3, TEXT_SIZE_2, strnBuf);   // Write Value2
              break;
            case mode_voltage:
              //writeScreen(0, 2, strnBuf);  // Write Title
              display.drawBitmap(0, 0,  icon16_battery, 16, 16, 1);
              sprintf (strnBuf, "%i.%i", voltageIntVal,voltageDecimal);
              writeScreen(1, TEXT_SIZE_4, strnBuf);   // Write Value1
              sprintf (strnBuf, "     Volts");
              writeScreen(3, TEXT_SIZE_2, strnBuf);   // Write Value2
              break;
            case mode_engine_temp:
              display.drawBitmap(0, 0,  icon16_engine_temp, 16, 16, 1);
              break;
            case mode_air_temp:
              display.drawBitmap(0, 0,  icon16_air_temp, 16, 16, 1);
              sprintf (strnBuf, "%2.1f", tempAir);
              writeScreen(1, TEXT_SIZE_4, strnBuf);   // Write Value1
              sprintf (strnBuf, "         F");
              writeScreen(3, TEXT_SIZE_2, strnBuf);   // Write Value2
            //case default:
              break;
          }//end switch
        }//end if current mode is true

        display.display();
 
         //currentMode = (currentMode+1)%NUM_MODES;
        //currentMode++;

        if(advanceMode) {
          currentMode++;
        }
        else {
          //Lets skip the modes that aren't enabled
          while((!modeEnabled[++currentMode] && currentMode<NUM_MODES));
        }
        
        //Reset currentMode to 0
        if(currentMode>=NUM_MODES) {currentMode=0;}
        

        //Request next mode value from Alternator Micro
        if(currentMode > 1) {
          txdata.dataName = currentMode;
          txdata.dataIntValue = 0;
          ETout.sendData();
        }
        advanceMode = false;
     }//end if last_millis_time
     
     //We received serial data
     if(ETin.receiveData()) {
       switch(rxdata.dataName) {
        case mode_voltage:
          voltageIntVal = (int8_t)rxdata.dataIntValue; 
          voltageDecimal = (uint8_t)rxdata.dataDecValue;
          break;
        case mode_engine_temp:
          tempEngine = (float)rxdata.dataIntValue+((float)rxdata.dataDecValue/10.0);
          break;
        case mode_air_temp:
          tempAir = (float)rxdata.dataIntValue+((float)rxdata.dataDecValue/10.0);
          break;
       }
     }
     
   }//End if Show Menu

  if(!engineStartedFlag) {
    //We'll only do this once per system reset/power-up
    if(digitalRead(ENGINE_STARTED)) {
      engineStartedFlag=true;
      showEngineStartedViz();
    }
  }

  if(digitalRead(DASH_LIGHTS_ON))
   display.dim(true);
  else
    display.dim(false);

    
   //Delay for good measure
   //delay(DELAYTIME);
}

void initEEPROM(void) {
  uint8_t val = EEPROM.read(EEPROM_WRITTEN_ADDR);
  if(val != EEPROM_WRITTEN_VALUE) {
    EEPROM.write(EEPROM_WRITTEN_ADDR, EEPROM_WRITTEN_VALUE);
    writeEepromModeEnables();
  } 
  else if (val == EEPROM_WRITTEN_VALUE){
    EEPROM.get(MODE_ENABLE_ADDR, modeEnableVal);
    modeEnabled[0] = modeEnableVal.en_time;
    modeEnabled[1] = modeEnableVal.en_date;
    modeEnabled[2] = modeEnableVal.en_voltage;
    modeEnabled[3] = modeEnableVal.en_engine_temp;
    modeEnabled[4] = modeEnableVal.en_air_temp;
  }
}

/**
 * Find out how many modes are enabled
 * We need this when we wipe the view
 * If there is only one mode enabled, we 
 * won't wipe the view
 */
void setNumModes(void) {
  numModes = 0;
  for(int i=0;i<NUM_MODES;i++) {
    if(modeEnabled[i] == 1)
      numModes++;   
  }
}

void writeEepromModeEnables(void) {
  modeEnableVal.en_time       = modeEnabled[0],
  modeEnableVal.en_date       = modeEnabled[1],
  modeEnableVal.en_voltage    = modeEnabled[2],
  modeEnableVal.en_engine_temp= modeEnabled[3],
  modeEnableVal.en_air_temp   = modeEnabled[4],
  EEPROM.put(MODE_ENABLE_ADDR, modeEnableVal);
}

void writeScreen(uint8_t lineNum, uint8_t textSize, String value) {
  display.setCursor(0,lineNum==0 ? 0 : lineNum==1 ? 16 : lineNum==2 ? 32 : 48  );
  display.setTextSize(textSize);
  display.println(value);
  //display.display();
}

// Add a zero before digits that are less then 10
void factorMinutes(char* minutesText, int minutesInt) {
  if(minutesInt < 10)
    sprintf(minutesText, "0%i", minutesInt);
  else
    sprintf(minutesText, "%i", minutesInt);
}

  
//Clears the screen using a randomly chosen wipe Option
void wipeScreen(void) {
  enum wipeOption {
    LEFT2RIGHT          = 0,
    RIGHT2LEFT          = 1,
    BOTTOM2TOP          = 2,
    TOP2BOTTOM          = 3,
    CIRCULAR_OUT        = 4,
    RECTANGULAR_IN      = 5,
  };
  uint8_t wipeMethod = random(0,6);
  uint8_t distance;
  uint8_t lines2Clear;
  switch(wipeMethod){
    case LEFT2RIGHT:
    case RIGHT2LEFT:
      distance = display.width()-1;
      lines2Clear = 16;
      break;
    case BOTTOM2TOP:
    case TOP2BOTTOM:
      distance = 64;  //was 48
      lines2Clear = 6;
      break;
    case CIRCULAR_OUT:
      distance = 64;
      lines2Clear = 8;
      break;
    case RECTANGULAR_IN:
      distance = display.width()/2;
      lines2Clear = 8;
      break;
  }
  for(int j=0;j<=distance;j+=lines2Clear) {
    for(int i=0;i<lines2Clear;i++) 
    switch(wipeMethod){
      case LEFT2RIGHT:
        display.drawLine(distance-j-i, 0, distance-j-i, 63, BLACK);
        break;
      case RIGHT2LEFT:
        display.drawLine(j+i, 0, j+i, 63, BLACK);
        break;
      case BOTTOM2TOP:
      display.drawLine(0, display.height()-1-j-i, 123, display.height()-1-j-i,  BLACK);
        break;
      case TOP2BOTTOM:
        display.drawLine(0, j+i, 123, j+i,  BLACK); //was 16+j+i
        break;
      case CIRCULAR_OUT:
        display.fillCircle(62, 39, j+i, BLACK);
        break;
      case RECTANGULAR_IN:
        display.drawRect(j+i, j+i, display.width()-(j+i)*2, 64-(j+i)*2, BLACK);//was 16+j+i, 48-(j+i)*2
        break;
    }
    display.display();
  }
}


void showMenu(void) {
  uint8_t clickedItem = 0;
  //bool isFirstClick = true; //Used to show the current value of the display item on first click
  unsigned long menuStartTime = millis();
  display.setTextSize(1);
  menu.InitMenu((const char ** )mnuRoot,cntRoot,0,TEXT_SIZE_2,DONT_SHOW_MENU); //init the menu
  
  while(millis() - menuStartTime < MENU_TIMEOUT) {
 
    //Define Button Pressed Action
    if(modeBtnPressed) {
      modeBtnPressed = false;
      menuStartTime = millis();
      menu.ProcessMenu(ACTION_DOWN);
    }
    else if(selectBtnPressed) {
      selectBtnPressed = false;
      menuStartTime = millis();
      clickedItem = menu.ProcessMenu(ACTION_SELECT);
    }
  
    //Select Button has been pressed
    if (clickedItem>0)
    {
      // Logic for Root menu
      if (menu.CurrentMenu==mnuRoot) {
        switch (clickedItem) {
          case 1:
            menu.InitMenu((const char ** )mnuAlternatorSubmenu,cntAlternatorSubmenu,1,TEXT_SIZE_2,SHOW_MENU);
            break;
          case 2:
            menu.InitMenu((const char ** )mnuDisplaySubmenu,cntDisplaySubmenu,1,TEXT_SIZE_2,SHOW_MENU);
            break;
          case 3:
            menu.InitMenu((const char ** )mnuDateClockSubmenu,cntDateClockSubmenu,1,TEXT_SIZE_2,SHOW_MENU);
            break; 
          case 4:
            menu.InitMenu((const char ** )mnuExit,cntExit,1,TEXT_SIZE_2,DONT_SHOW_MENU);
            showMenuFlag = false;
            return;
        }
      }
      // Logic for Alternator Submenu
      else if (menu.CurrentMenu==mnuAlternatorSubmenu) {
          switch (clickedItem) {
            case 1:
              if(1 == processTurboMode()) {
                requestAlternatorData(mode_turbo, clickedItem);
                //menu.InitMenu((const char ** )mnuAlternatorSubmenu,cntAlternatorSubmenu,clickedItem,TEXT_SIZE_2,SHOW_MENU);
                showMenuFlag = false;
                return;
              }
              break;
            case 2:
              if(1 == processVoltageAdjust()) {
                menu.InitMenu((const char ** )mnuAlternatorSubmenu,cntAlternatorSubmenu,clickedItem,TEXT_SIZE_2,SHOW_MENU);
                requestAlternatorData(mode_set_voltage, 4);
              }
              menu.InitMenu((const char ** )mnuAlternatorSubmenu,cntAlternatorSubmenu,clickedItem,TEXT_SIZE_2,SHOW_MENU);
              break;
            case 3:
              requestAlternatorData(mode_save_v_set, clickedItem);
              break;
            case 4:
              requestAlternatorData(mode_target_voltage, clickedItem);
              //menu.InitMenu((const char ** )mnuAlternatorSubmenu,cntAlternatorSubmenu,clickedItem,TEXT_SIZE_2,SHOW_MENU);
              break;
            case 5:
              requestAlternatorData(mode_alternator_pwm, clickedItem);
              //menu.InitMenu((const char ** )mnuAlternatorSubmenu,cntAlternatorSubmenu,clickedItem,TEXT_SIZE_2,SHOW_MENU);
              break;
            case 6:
              //menu.InitMenu((const char ** )mnuRoot,cntRoot,1,TEXT_SIZE_2,DONT_SHOW_MENU);
              showMenuFlag = false;
              return;
          }
          menuStartTime = millis();
      }
      // Logic for Display Submenu
      else if (menu.CurrentMenu==mnuDisplaySubmenu) { 
        switch (clickedItem) {
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
            //if(!isFirstClick) {
              modeEnabled[clickedItem-1] = !modeEnabled[clickedItem-1];
            //}
            //isFirstClick = false;
            strcpy_P(strnBuf, (char*)pgm_read_word(&(modeNames[clickedItem-1])));
            strncat(strnBuf, " is ",4);
            strncat(strnBuf, modeEnabled[clickedItem-1] ? "ON " : "OFF",3);
            menu.MessageBox(strnBuf, 1);
            break;
          case 6: //Save Settings
            writeEepromModeEnables();
            displayMessageBox("Settings Saved", TEXT_SIZE_1);
            menu.InitMenu((const char ** )mnuDisplaySubmenu,cntDisplaySubmenu,clickedItem,TEXT_SIZE_2,SHOW_MENU);
            break;
          case 7:
            //isFirstClick = true;
            showMenuFlag = false;
            return;
        }
      }
      else if (menu.CurrentMenu==mnuDateClockSubmenu) {
          switch (clickedItem) {
            case 1:
              processClockAdjust();
              break;
            case 2:
              processDateAdjust();
              break;
            case 3:
              menu.InitMenu((const char ** )mnuRoot,cntRoot,3,TEXT_SIZE_2,SHOW_MENU);
              break;
          }
          menuStartTime = millis();
      }
      clickedItem = 0;
    }//end if clickedItem
  }//end while
  showMenuFlag = false;
}

/**
 * Request the passed dataName from the Alternator Micro
 * Then display the returned value
 * If no data has been received from the Alt Mirco, print error message
 */
void requestAlternatorData(uint8_t dataName, uint8_t clickedItem) {
  char cBuff[20];
  if(getAlternatorValue(dataName) == 1){
    if(rxdata.dataName == dataName) {
      strcpy_P(strnBuf, (char*)pgm_read_word(&(mnuAlternatorSubmenu[clickedItem])));
      switch (dataName) {
        case mode_target_voltage:
          sprintf(cBuff, "%s %i.%i",strnBuf,rxdata.dataIntValue, rxdata.dataDecValue);
          break;
        case mode_alternator_pwm:
          sprintf(cBuff, "%s %i",strnBuf,rxdata.dataIntValue);
          break;
        case mode_set_voltage:
          sprintf(cBuff, "%s %i.%i",strnBuf,rxdata.dataIntValue, rxdata.dataDecValue);
          break;
        case mode_save_v_set:
          sprintf(cBuff, "V Set %s", rxdata.dataIntValue==1?"Saved":"Not Saved");
          break;
        case mode_turbo:
          sprintf(cBuff, "Target set to %i.%i", rxdata.dataIntValue, rxdata.dataDecValue);
          break;
      }
    } else {
       sprintf(cBuff, "Error wrong data");
       displayMessageBox(cBuff, TEXT_SIZE_1);
       sprintf(cBuff, "Rtn %i:%s", rxdata.dataName,rxdata.dataName<10?modeNames[rxdata.dataName]:"");
       displayMessageBox(cBuff, TEXT_SIZE_1);
       return;
    }
  } else {
    sprintf(cBuff, "%s Timeout",mnuAlternatorSubmenu[clickedItem]);
  }
  //menu.MessageBox(cBuff, TEXT_SIZE_1);
  displayMessageBox(cBuff, TEXT_SIZE_1);
}

/**
 * Pings the Alternator Micro with the passed dataName
 * Return 1 on successful receive data from Alt Micro within the given timeout period
 * Return 0 when no data has been received within the given timeout period
 */
int8_t getAlternatorValue(int8_t dataName) {
  //reset receive data
  rxdata.dataName = rxdata.dataIntValue = rxdata.dataDecValue = 0;
  //load up the data we want to get
  txdata.dataName = dataName;
  ETout.sendData();
  unsigned long comStartTime = millis();
  while(millis() - comStartTime < SERIAL_TIMEOUT) {
    if(ETin.receiveData())
      return 1;
  }
  return 0;
}

/**
 * Display Message Box for the given amount of time
 * Then return back to current menu
 */
 int8_t displayMessageBox(const char msg[], int8_t textSize) {
  unsigned long comStartTime = millis();
  menu.MessageBox(msg, textSize);
  while(millis() - comStartTime < MSG_BOX_TIMEOUT) {
    if(modeBtnPressed) {
        modeBtnPressed = false;
        return 1;
    }
    // Had to delay to give enough time for Button Interrupt Routine to execute, weird!
    delay(10); 
  }
  return 0;
}

  
/**
 * Get current Target Voltage from the Alternator micro
 * Then pop up an adjustment dialog box
 * Select Button cycles through integer values
 * Mode Button advances from Integer digit to Decimal digit to Exit
 * Return 1 happens when the Mode Button advances to Exit
 * Return 0 happens on a timeout
 * Return -1 when a communication error happens
 */
int8_t processVoltageAdjust(void) {
  if(getAlternatorValue(mode_target_voltage) == 1) {
    unsigned long menuStartTime = millis();
    int8_t selectedTextIndex=0;
    char voltageText[5];
    char cBuff[20];
    
    // Error catching
    if(rxdata.dataName != mode_target_voltage) {
      displayMessageBox("Error wrong data", TEXT_SIZE_1);
      sprintf(cBuff, "Rtn %i:%s", rxdata.dataName,rxdata.dataName<10?modeNames[rxdata.dataName]:"");
      displayMessageBox(cBuff, TEXT_SIZE_1);
      return -1;
    }
    if(rxdata.dataIntValue < 10) {
      char cBuff[20];
      sprintf(cBuff, "Error: Read %sV",voltageText);
      displayMessageBox(cBuff, TEXT_SIZE_1);
      return -1;
    }
    txdata.dataName = mode_set_voltage;
    txdata.dataIntValue = rxdata.dataIntValue;
    txdata.dataDecValue = rxdata.dataDecValue;
    sprintf (voltageText, "%i.%i", txdata.dataIntValue,txdata.dataDecValue);
    menu.adjustMsgBox(voltageText, 3, selectedTextIndex, HIGHLIGHT_DOUBLE);
    while(selectedTextIndex < 4) {
      if(millis() - menuStartTime > MENU_TIMEOUT)
        return 0;
      if(modeBtnPressed) {
        modeBtnPressed = false;
        menuStartTime = millis();
        selectedTextIndex+=3;
        if(selectedTextIndex < 4)
          menu.adjustMsgBox(voltageText, TEXT_SIZE_3, selectedTextIndex,HIGHLIGHT_SINGLE);
      }
      else if(selectBtnPressed) {
        selectBtnPressed = false;
        menuStartTime = millis();
        if(selectedTextIndex==0) {
          txdata.dataIntValue++;
          if(txdata.dataIntValue > MAX_V_ADJUST)
            txdata.dataIntValue = MIN_V_ADJUST;
        }
        else {
          txdata.dataDecValue = (txdata.dataDecValue+1)%10; 
        }
        sprintf (voltageText, "%i.%i", txdata.dataIntValue,txdata.dataDecValue);
        menu.adjustMsgBox(voltageText, TEXT_SIZE_3, selectedTextIndex,selectedTextIndex==0?HIGHLIGHT_DOUBLE:HIGHLIGHT_SINGLE);
      }
      delay(10);
    }
  }
  else {
    //menu.MessageBox("Error getting V Setpoint", TEXT_SIZE_1);
    displayMessageBox("Error getting V Setpoint", TEXT_SIZE_1);
    return -1;
  }
  /*
  ETout.sendData();
  //Wait for dummy return value
  unsigned long comStartTime = millis();
  while(millis() - comStartTime < SERIAL_TIMEOUT) {
    if(ETin.receiveData())
      return 1;
  }
  menu.InitMenu((const char ** )mnuAlternatorSubmenu,cntAlternatorSubmenu,2,TEXT_SIZE_2,SHOW_MENU);
  */
  return 1;
}

/**
 * It is required to press the mode button to push the Turbo status to the
 * Alternator Micro. If the Turbo selection has been pressed by accident, simply
 * let this function timeout to not send the Alt Mirco the update. The user can toggle the
 * current status (on or off) several times before the Alt Mirco sees the next update.
 * Return 1 happens when the Mode Button advances to Exit
 * Return 0 happens on a timeout
 */
int8_t processTurboMode(void) {
  unsigned long turboToggleTime = millis();
  turboMode = !turboMode;
  //strcpy_P(strnBuf, (char*)pgm_read_word(&(itmTurboMode)));
  sprintf(strnBuf, "%s is %s",itmTurboMode,turboMode ? "ON " : "OFF");
  menu.MessageBox(strnBuf, TEXT_SIZE_1);
  while(millis() - turboToggleTime < TURBO_DIALOG_TIMEOUT) {
    if(selectBtnPressed) {
      selectBtnPressed = false;
      turboToggleTime = millis();
      turboMode = !turboMode;
      sprintf(strnBuf, "%s is %s",itmTurboMode,turboMode ? "ON " : "OFF");
      menu.MessageBox(strnBuf, TEXT_SIZE_1);
    }
    if(modeBtnPressed) {
      modeBtnPressed = false;
      txdata.dataName = mode_turbo;
      txdata.dataIntValue = turboMode;
      return 1;
    }
  }//end while
  return 0;
}

int8_t processClockAdjust(void) {
  int8_t highlightWidth = HIGHLIGHT_DOUBLE;
  unsigned long menuStartTime = millis();
  int8_t selectedTextIndex=0;
  uint8_t am_pmIdx = 1;
  char am_pmText[][3] = {"am", "pm"};
  char clockText[6];
  now = rtc.now();
  int8_t hourVal = now.hour()==0 ? 12 : now.hour()>12 ? now.hour()-12 : now.hour();
  int8_t minuteTensVal = now.minute()/10;
  int8_t minuteOnesVal = now.minute() - (minuteTensVal*10);

  if(now.hour()<12) am_pmIdx = 0;
  sprintf (clockText, "%2i:%i%i%s", hourVal,minuteTensVal,minuteOnesVal,am_pmText[am_pmIdx]);
  menu.adjustMsgBox(clockText, TEXT_SIZE_2, selectedTextIndex, highlightWidth);
  while(selectedTextIndex < 6) {
    if(millis() - menuStartTime > CLOCK_ADJ_TIMEOUT)
      return 0;
    if(modeBtnPressed) {
      modeBtnPressed = false;
      menuStartTime = millis();
      selectedTextIndex++;
      if(selectedTextIndex == 1) {selectedTextIndex=3;}  //Move to first minutes digit
      if(selectedTextIndex == 5) {highlightWidth=HIGHLIGHT_DOUBLE;}
      else {highlightWidth=HIGHLIGHT_SINGLE;}
      if(selectedTextIndex < 6)
        menu.adjustMsgBox(clockText, TEXT_SIZE_2, selectedTextIndex, highlightWidth);
    }
    else if(selectBtnPressed) {
      selectBtnPressed = false;
      menuStartTime = millis();
      switch(selectedTextIndex) {
        case 0:
          hourVal++;
          if(hourVal > 12) 
            hourVal = 1;
          highlightWidth = HIGHLIGHT_DOUBLE;
          break;
        case 3:
          minuteTensVal = (minuteTensVal+1)%6; 
          highlightWidth = HIGHLIGHT_SINGLE;
          break;
        case 4:
          minuteOnesVal = (minuteOnesVal+1)%10; 
          highlightWidth = HIGHLIGHT_SINGLE;
          break;
        case 5:
          am_pmIdx = (am_pmIdx+1)%2; 
          highlightWidth = HIGHLIGHT_DOUBLE;
          break;
      }
      sprintf (clockText, "%2i:%i%i%s", hourVal,minuteTensVal,minuteOnesVal,am_pmText[am_pmIdx]);
      menu.adjustMsgBox(clockText, TEXT_SIZE_2, selectedTextIndex, highlightWidth);
    }
    delay(10);
  }
  if(am_pmIdx == 0 && hourVal == 12)
      hourVal = 0;
  else if(am_pmIdx == 1)
    hourVal += 12;
  rtc.adjust(DateTime(now.year(), now.month(), now.day(), hourVal, (minuteTensVal*10)+minuteOnesVal, 0));
  menu.InitMenu((const char ** )mnuDateClockSubmenu,cntDateClockSubmenu,2,2,SHOW_MENU);
  return 1;
}


int8_t processDateAdjust(void) {
  bool doubleDigitHighlight = true;
  unsigned long menuStartTime = millis();
  int8_t selectedTextIndex=0;
  char dateText[9];
  now = rtc.now();
  int8_t yearThousandsVal = now.year()/1000;
  int8_t yearHundredsVal = (now.year() - (yearThousandsVal*1000))/100;
  int8_t yearTensVal = (now.year() - (yearThousandsVal*1000) - (yearHundredsVal*100))/10;
  int8_t yearOnesVal = now.year() - (yearThousandsVal*1000) - (yearHundredsVal*100) - (yearTensVal*10);
  int8_t monthVal = now.month();
  int8_t dayTensVal = now.day()/10;
  int8_t dayOnesVal = now.day() - (((int8_t)(now.day()/10))*10);

  sprintf (dateText, "%2i/%i%i/%i%i", monthVal,dayTensVal,dayOnesVal,yearTensVal,yearOnesVal);
  menu.adjustMsgBox(dateText, TEXT_SIZE_2, selectedTextIndex, doubleDigitHighlight);
  while(selectedTextIndex < 8) {
    if(millis() - menuStartTime > CLOCK_ADJ_TIMEOUT)
      return 0;
    if(modeBtnPressed) {
      modeBtnPressed = false;
      menuStartTime = millis();
      selectedTextIndex++;
      if(selectedTextIndex == 1) {selectedTextIndex=3;} //Move to first month digit
      else if(selectedTextIndex == 5) {selectedTextIndex=6;} //Move to first year digit
      if(selectedTextIndex < 8)
        menu.adjustMsgBox(dateText, TEXT_SIZE_2, selectedTextIndex, false);
    }
    else if(selectBtnPressed) {
      selectBtnPressed = false;
      menuStartTime = millis();
      switch(selectedTextIndex) {
        case 0:
          monthVal++;
          if(monthVal > 12) 
            monthVal = 1;
          doubleDigitHighlight = true;
          break;
        case 3:
          if(monthVal == 2) {dayTensVal = (dayTensVal+1)%3;}
          else {dayTensVal = (dayTensVal+1)%4; }
          doubleDigitHighlight = false;
          break;
        case 4:
          dayOnesVal++;
          if(dayTensVal==3 && daysInMonth[monthVal-1] == 30)
            dayOnesVal = 0;
          else if(dayTensVal==3 && dayOnesVal>1)
            dayOnesVal = 0;
          else if(dayOnesVal > 9) {
            if(dayTensVal == 0) {dayOnesVal = 1;}
            else {dayOnesVal = 0;}
          }
          doubleDigitHighlight = false;
          break;
        case 6:
          yearTensVal = (yearTensVal+1)%10; 
          doubleDigitHighlight = false;
          break;
        case 7:
          yearOnesVal = (yearOnesVal+1)%10; 
          doubleDigitHighlight = false;
          break;
      }
      sprintf (dateText, "%2i/%i%i/%i%i", monthVal,dayTensVal,dayOnesVal,yearTensVal,yearOnesVal);
      menu.adjustMsgBox(dateText, TEXT_SIZE_2, selectedTextIndex, doubleDigitHighlight);
    }
    delay(10);
  }
  uint16_t yearText = yearThousandsVal*1000 + yearHundredsVal*100 + yearTensVal*10 + yearOnesVal;
  rtc.adjust(DateTime(yearText, monthVal, (dayTensVal*10)+dayOnesVal,  now.hour(),  now.minute(), now.second()));
  menu.InitMenu((const char ** )mnuDateClockSubmenu,cntDateClockSubmenu,2,2,SHOW_MENU);
  return 1;
}

void showEngineStartedViz(void) {
  int16_t xCenter = display.width()*2/3;
  int16_t yCenter = display.height()-1;
  //int16_t x2=0, y2=0;
  
  display.clearDisplay();
  writeScreen(0, 2, "EnginStrtd");  // Write Title
  //writeScreen(1, 2, "Jeep");  // Write Line1
  //writeScreen(1, 4, "XJ");  // Write Line2
  display.drawCircle(xCenter, yCenter, 47, WHITE);
  display.drawLine(xCenter, yCenter, xCenter-45, yCenter, WHITE);
  display.drawBitmap(0, 20,  icon32_engine, 32, 32, 1);
  display.display();
  delay(100);
  display.drawLine(xCenter, yCenter, xCenter-45, yCenter, BLACK);
  //radiusLineSweep(xCenter, yCenter, degStart, degEnd, length, step, step);
  
  radiusLineSweep(xCenter, yCenter, 175, 45, 44, 25, 1);
  radiusLineSweep(xCenter, yCenter, 35, 120, 44, 10, 0);
  radiusLineSweep(xCenter, yCenter, 110, 30, 44, 25, 1);
  radiusLineSweep(xCenter, yCenter, 35, 120, 44, 10, 0);
  radiusLineSweep(xCenter, yCenter, 110, 15, 44, 30, 1);
  delay(1500);
}

void radiusLineSweep(int16_t xCenter, int16_t yCenter, int16_t degStart, int16_t degEnd, int16_t length, int8_t step, int8_t dir) {
  int16_t x2,y2;
  for(int16_t r = degStart;dir==1?r>degEnd:r<degEnd;dir==1?r-=step:r+=step) {
    x2 = xCenter + (cos(radians(r))*(float)length);
    y2 = yCenter +-(sin(radians(r))*(float)length);
    display.drawLine(xCenter, yCenter, x2, y2, WHITE);
    display.display();
    display.drawLine(xCenter, yCenter, x2, y2, BLACK);
  }
}
