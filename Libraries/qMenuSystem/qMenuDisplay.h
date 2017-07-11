/**
 * 	Version: 1.1
 *  Revised By: Kevin Carlborg
 *  Added adjustMsgBox
 * 
 */


#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#ifndef qMenuDisplay_h
#define qMenuDisplay_h

#define textHeight		8
#define textWidth  		6
#define xPadding  		4
#define yPadding  		4
#define borderPadding  	3
	
class qMenuDisplay
{
  public:
    qMenuDisplay(Adafruit_SSD1306 *disp);
    qMenuDisplay();
    void Begin();
    void Start();
    void Finish();
    void Title(const char text[], int8_t textSize);
	//void Title(char** text[], int8_t textSize);
    void Item(int8_t index,const char text[], int8_t textSize);
    void Highlight(int8_t index, int8_t textSize);
    void MessageBox(const char text[], int8_t textSize);
	void adjustMsgBox(const char text[], int8_t textSize, int8_t selectedText, int8_t highlightWidth);
	
  private:
	Adafruit_SSD1306 *_disp;
	//int8_t checkTextSize(char text[], int8_t textSize);
    //int textHeight = 8; //14
	//int textWidth = 6;
	//int textSize = 1;
//    char tempBuffer[32];
};




#endif
