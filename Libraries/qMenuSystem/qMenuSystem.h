/////////////////////
// qMenuSystem
// Version: 1.1
// Revised by Kevin Carlborg
// Added Support for adjustMsgBox
//
// Original Release
// version: 1.0
// 22.12.2013
// HEADER
// 
/////////////////////
#include "qMenuDisplay.h"

#ifndef qMenuSystem_h
#define qMenuSystem_h

#define ACTION_NONE     0
#define ACTION_UP       1
#define ACTION_DOWN     2
#define ACTION_SELECT   3
#define ACTION_BACK     4

class qMenuSystem
{
  public:
    qMenuSystem(Adafruit_SSD1306 *disp);
    void InitMenu(const char ** page, int8_t itemCount, int8_t selectedIndex, int8_t textSize, bool showMenu);
    int8_t ProcessMenu(int8_t action);
    void ShowMenu();
    void MessageBox(const char text[], int8_t textSize) { qmd.MessageBox(text, textSize); };
	void adjustMsgBox(const char text[], int8_t textSize, int8_t highlightIdx, int8_t highlightWidth) { qmd.adjustMsgBox(text, textSize, highlightIdx, highlightWidth); };
    
    const char ** CurrentMenu;
  private:
	int8_t _selectedTextIndex;
    int8_t _selectedIndex;
    int8_t _itemCount;
    int8_t _firstVisible;
    int8_t _textSize;
    char tempBuffer[32];
	//char* tempBuffer = new tempBuffer[32];
    qMenuDisplay qmd;
};

#endif
