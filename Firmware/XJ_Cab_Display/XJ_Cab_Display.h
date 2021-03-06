// required for "const char" and "PROGMEM"
#include <avr/pgmspace.h>

/*********  Texts for Date  *********/
const char sunday[] PROGMEM = "Sun";
const char monday[] PROGMEM = "Mon";
const char tuesday[] PROGMEM = "Tue";
const char wednesday[] PROGMEM = "Wed";
const char thursday[] PROGMEM = "Thu";
const char friday[] PROGMEM = "Fri";
const char saturday[] PROGMEM = "Sat";
PROGMEM const char * const daysOfTheWeek[] = {
  sunday,monday,tuesday,wednesday,thursday,friday,saturday};
  
/*********  Display Data Text  *********/
const char itmTime[] PROGMEM = "Time";

  
/*********  Texts for Menus  *********/
const char itmRoot[] PROGMEM = "Root menu";
  const char itmAlternator[] PROGMEM = "Alternator";
      const char itmTurboMode[] PROGMEM = "Turbo Mode";
      const char itmVoltageSet[] PROGMEM = "SetVoltage";  //was V Setpoint
      const char itmSaveVoltageSet[] PROGMEM = "Save V Set";
      const char itmTargetV[] PROGMEM = "Target V";
      const char itmTargetPWM[] PROGMEM = "Target PWM";
      //const char itmAltSettings[] PROGMEM = "Settings";
      
  const char itmDisplay[] PROGMEM = "Display";
      const char itmClock[] PROGMEM = "Clock";
      const char itmDate[] PROGMEM = "Date";
      const char itmVoltage[] PROGMEM = "Voltage";
      const char itmEngineTemp[] PROGMEM = "EngineTemp";
      const char itmAirTemp[] PROGMEM = "Air Temp";
      const char itmSaveSettings[] PROGMEM = "SaveSetngs";
    
  const char itmDateClock[] PROGMEM = "Date/Clock"; 
      const char itmClockSet[] PROGMEM = "Set Clock"; 
      const char itmDateSet[] PROGMEM = "Set Date"; 

  //const char itmSubmenu3[] PROGMEM = "Exit";
  //  const char itmEnabled[] PROGMEM = "Enabled";
  //  const char itmDisabled[] PROGMEM = "Disabled";

  const char itmMessageBox[] PROGMEM = "Message box";
  
  const char itmExit[] PROGMEM = "< Exit >";

/*********  Common Menu Items   *********/
const char itmBack[] PROGMEM = "< Back";
//const char itmOn[] PROGMEM = "On";
//const char itmOff[] PROGMEM = "Off";

PROGMEM const char * const modeNames[] = {
  itmTime,itmDate,itmVoltage,itmEngineTemp,itmAirTemp,itmVoltageSet,itmTargetV,itmTargetPWM};

static bool modeEnabled[] = {true,true,true,false,true};

struct ModeEnableObject{
  bool en_time;
  bool en_date;
  bool en_voltage;
  bool en_engine_temp;
  bool en_air_temp;
};
  
////////////////////////////////////////////////////////////////
// menus - first item is menu title and it does not count toward cnt

PROGMEM const char * const mnuRoot[] = {
  itmRoot,
  itmAlternator,itmDisplay,itmDateClock,itmExit};
PROGMEM const int8_t cntRoot = 4;

PROGMEM const char * const mnuDisplaySubmenu[] = {
  itmDisplay,
  itmClock,itmDate,itmVoltage,itmEngineTemp,itmAirTemp,itmSaveSettings,itmExit};
PROGMEM const int8_t cntDisplaySubmenu = 7;

PROGMEM const char * const mnuAlternatorSubmenu[] = {
  itmAlternator,
  itmTurboMode,itmVoltageSet,itmSaveVoltageSet, itmTargetV,itmTargetPWM,itmExit};
PROGMEM const int8_t cntAlternatorSubmenu = 6;

PROGMEM const char * const mnuDateClockSubmenu[] = {
  itmDateClock,
  itmClockSet,itmDateSet,itmBack};
PROGMEM const int8_t cntDateClockSubmenu = 3;

PROGMEM const char * const mnuExit[] = { };
PROGMEM const int8_t cntExit = 0;


/*********  Icon defines *********/
static const unsigned char PROGMEM icon16_clock[] = {
0x07, 0xE0, 0x18, 0x18, 0x21, 0x04, 0x41, 0x02, 0x41, 0x02, 0x81, 0x01, 0x81, 0x01, 0x81, 0x01, 
0x80, 0x81, 0x80, 0x41, 0x80, 0x21, 0x40, 0x12, 0x40, 0x02, 0x20, 0x04, 0x18, 0x18, 0x07, 0xE0, 
};

static const unsigned char PROGMEM icon16_calendar[] = {
0x18, 0x18, 0x18, 0x18, 0xDB, 0xDB, 0xC3, 0xC3, 0xFF, 0xFF, 0x80, 0x01, 0x83, 0x6D, 0x83, 0x6D, 
0x80, 0x01, 0x9B, 0x6D, 0x9B, 0x6D, 0x80, 0x01, 0x9B, 0x6D, 0x9B, 0x6D, 0x80, 0x01, 0x7F, 0xFE, 
};

static const unsigned char PROGMEM icon16_battery[] = {
0x38, 0x1C, 0x38, 0x1C, 0xFF, 0xFF, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x11, 0x80, 0x11, 
0xBE, 0x7D, 0x80, 0x11, 0x80, 0x11, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x7F, 0xFE, 
};

static const unsigned char PROGMEM icon16_engine_temp[] = {
0x03, 0x00, 0x03, 0x00, 0x03, 0xF0, 0x03, 0x00, 0x03, 0xF0, 0x03, 0x00, 0x03, 0xF0, 0x03, 0x00, 
0x03, 0x00, 0x03, 0x00, 0x07, 0x80, 0x67, 0xA6, 0x93, 0x19, 0x00, 0x00, 0x66, 0x66, 0x99, 0x98, 
};

static const unsigned char PROGMEM icon16_air_temp[] = {
0x01, 0x80, 0x02, 0x58, 0x02, 0x40, 0x02, 0x58, 0x02, 0x40, 0x02, 0x58, 0x03, 0xC0, 0x03, 0xC0, 
0x03, 0xC0, 0x03, 0xC0, 0x07, 0xE0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x07, 0xE0, 0x03, 0xC0, 
};

const uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

static const unsigned char PROGMEM icon32_engine[] = {
0x00, 0x3F, 0xF8, 0x00,
0x00, 0x03, 0x00, 0x00,
0x00, 0x03, 0x00, 0x00,
0x07, 0xFF, 0xFF, 0x00,
0x04, 0x00, 0x01, 0x00,
0x1C, 0x00, 0x01, 0xC0,
0x90, 0x00, 0x00, 0x40,
0x90, 0x00, 0x00, 0x40,
0x92, 0x20, 0x3C, 0x4F,
0x92, 0x20, 0x42, 0x49,
0xF2, 0x20, 0x42, 0x79,
0xF2, 0x20, 0x42, 0x01,
0x93, 0xF8, 0x42, 0x01,
0x90, 0x20, 0x42, 0x01,
0x90, 0x20, 0x42, 0x01,
0x90, 0x20, 0x42, 0x01,
0x90, 0x23, 0x42, 0x01,
0x1C, 0x23, 0x3C, 0x79,
0x06, 0x00, 0x00, 0x49,
0x03, 0x00, 0x00, 0x4F,
0x01, 0x80, 0x00, 0xC0,
0x00, 0xC0, 0x01, 0x80,
0x00, 0x7F, 0xFF, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
};




