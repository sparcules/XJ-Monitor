/*
======================================
qMenuSystem example #1
======================================


// libraries needed:
https://github.com/adafruit/Adafruit_SSD1306
https://github.com/adafruit/Adafruit-GFX-Library

// see https://learn.adafruit.com/monochrome-oled-breakouts/arduino-library-and-examples
// for more info about witing / SPI / I2C

1. Connect OLED
2. Compile & upload
3. Open Serial monitor (Ctrl+Shift+M)
4. Use "q", "a", "s" to move across menu ("q" = up, "a" = down, "s" = select)

*/

#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <qMenuDisplay.h>
#include <qMenuSystem.h>

#include "TestMenu.h"


const byte OLED_RESET               = 4;
Adafruit_SSD1306 mydisp(OLED_RESET); 

qMenuSystem menu=qMenuSystem(&mydisp);

void setup()
{
  menu.InitMenu((const char ** )mnuRoot,cntRoot,1);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Usage: 'q' -> UP   'a' -> DOWN   's' -> SELECT");
}

void loop()
{
  int keycode=0;
  int clickedItem=0; 
  
  if (Serial.available()>0)
  {
    switch(Serial.read())
    {
      case 97:
        menu.ProcessMenu(ACTION_DOWN);
        break;
      case 113:
        menu.ProcessMenu(ACTION_UP);
        break;
      case 115:
        clickedItem=menu.ProcessMenu(ACTION_SELECT);
        break;
    }
  }

  if (clickedItem>0)
  {
    // Logic for Root menu
    if (menu.CurrentMenu==mnuRoot)
      switch (clickedItem)
      {
        case 1:
          menu.InitMenu((const char ** )mnuSubmenu1,cntSubmenu1,1);
          break;
        case 2:
          menu.InitMenu((const char ** )mnuSubmenu2,cntSubmenu2,1);
          break;
        case 3:
          menu.InitMenu((const char ** )mnuSubmenu3,cntSubmenu3,1);
          break;
        case 4:
          menu.MessageBox("Some message!");
          break; 
      }
    // Logic for Submenu 1
    else if (menu.CurrentMenu==mnuSubmenu1)
      switch (clickedItem)
      {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
          menu.MessageBox("Item clicked");
          break;
        case 6:
          menu.InitMenu((const char ** )mnuRoot,cntRoot,1);
          break;
      }
    // Logic for Submenu 2
    else if (menu.CurrentMenu==mnuSubmenu2)
      switch (clickedItem)
      {
        case 1:
          menu.MessageBox("On");
          break;
        case 2:
          menu.MessageBox("Off");
          break;
        case 3:
          menu.InitMenu((const char ** )mnuRoot,cntRoot,2);
          break;
      }
    // Logic for Submenu 3
    else if (menu.CurrentMenu==mnuSubmenu3)
      switch (clickedItem)
      {
        case 1:
          menu.MessageBox("Enabled");
          break;
        case 2:
          menu.MessageBox("Disabled");
          break;
        case 3:
          menu.InitMenu((const char ** )mnuRoot,cntRoot,3);
          break;
      }

  } 
}

