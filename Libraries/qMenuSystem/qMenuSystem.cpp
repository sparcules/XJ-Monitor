/////////////////////
// qMenuSystem
// Version: 1.1
// Revised by Kevin Carlborg
// Added Adjustable Text Size 
//
// Original Release
// version: 1.0
// 22.12.2013
// CLASS
/////////////////////
#include <avr/pgmspace.h>
#include "qMenuSystem.h"
#include "qMenuDisplay.h"

//DigoleSerialDisp disp(255,255,255);
//qMenuDisplay qmd;

qMenuSystem::qMenuSystem(Adafruit_SSD1306 *disp)
{
   
  qmd=qMenuDisplay(disp);
  
  _selectedTextIndex=0;
  _selectedIndex=0;
  _itemCount=0;
  _firstVisible=1;
  _textSize=1;
}

void qMenuSystem::InitMenu(const char ** page, int8_t itemCount, int8_t selectedIndex, int8_t textSize, bool showMenu)
{
  qmd.Begin();
  CurrentMenu=page;
  _selectedIndex=selectedIndex;
  _itemCount=itemCount;
  _textSize = textSize;
//  ProcessMenu(ACTION_NONE);
  if(showMenu)
	ShowMenu();
}

int8_t qMenuSystem::ProcessMenu(int8_t action)
{
  if (action==ACTION_DOWN)
	_selectedIndex++;
  if (action==ACTION_UP)
	_selectedIndex--;
      
  if (_selectedIndex>_itemCount)
    _selectedIndex=1;
  if (_selectedIndex<1)
    _selectedIndex=_itemCount;
  
  if (action==ACTION_SELECT)
    return _selectedIndex;
  
  ShowMenu();
  
  return 0;
}

void qMenuSystem::ShowMenu()
{
  if (_selectedIndex>_firstVisible+2)
    _firstVisible=_selectedIndex-2;
  else if (_selectedIndex<_firstVisible)
    _firstVisible=_selectedIndex;
  
  qmd.Start();
  
  // display title
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(CurrentMenu[0])));
  qmd.Title(tempBuffer, _textSize);
  
  // display items
  int p = 3;
  if (p > (_itemCount-_firstVisible+1))
    p=_itemCount-_firstVisible+1;
  for (int i=0;i<p;i++)
  {
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(CurrentMenu[i+_firstVisible])));
    qmd.Item(i,tempBuffer, _textSize);
  }
  
  // display selection
  qmd.Highlight(_selectedIndex-_firstVisible, _textSize);
  
  qmd.Finish();
}


int8_t checkTextSize(char text[], int8_t textSize)
{
  while(strlen(text)*textWidth*textSize>128) {
	if(textSize>1)
	  textSize--;
	else
	  text[(int8_t)(128/textWidth/textSize)] = 0;
  }
  return textSize;
}
