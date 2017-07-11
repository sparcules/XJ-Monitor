/**
 * 	Version: 1.1
 *  Revised By: Kevin Carlborg
 *  Added adjustMsgBox
 *  Added adjustable text size
 *  Tweaked MessageBox to adapt to different text sizes
 * 
 */
 
 
#include "qMenuDisplay.h"

//DigoleSerialDisp _disp(8,9,10);
//const byte OLED_RESET               = 21;
//Adafruit_SSD1306 _disp(OLED_RESET); 

qMenuDisplay::qMenuDisplay()
{
}

qMenuDisplay::qMenuDisplay(Adafruit_SSD1306 *disp)
{
  _disp=disp;

}

void qMenuDisplay::Begin()
{
  _disp->begin(SSD1306_SWITCHCAPVCC, 0x3C);
  _disp->setTextColor(WHITE);
  _disp->setTextSize(1);
  _disp->setTextWrap(false);
}

void qMenuDisplay::Start()
{
  _disp->clearDisplay();
}

void qMenuDisplay::Finish()
{
  _disp->display();
}

void qMenuDisplay::Title(const char text[], int8_t textSize)
{
  int8_t titleWidth = strlen(text)*textWidth*textSize;
  int8_t titleHeight = textHeight*textSize;
  //_disp->setCursor(64-strlen(text)*9/2,11-textHeight/2);
  /*int8_t xcoord = ((64-(titleWidth/2)) < 0 ? 0 : 64-(titleWidth/2));*/
  int8_t ycoord = ((7-(titleHeight/2)) < 0 ? 0 : 7-(titleHeight/2));
  //_disp->setCursor(xcoord,ycoord);
  _disp->setCursor(0,ycoord);
  _disp->setTextSize(textSize);
  _disp->print(text);
  //_disp->drawFastHLine(0,14,128, WHITE);
  _disp->display();
}
//Example of truncating a char array
//http://stackoverflow.com/questions/12987760/passing-char-array-by-reference
void qMenuDisplay::Item(int8_t index, const char text[], int8_t textSize)
{
  // If text is too long to fit in one line,
  //   shrink text size to fit display - KKC
  // TO DO: Change this to scrolling text 
 /* while(strlen(text)*textWidth*textSize>128) {
	if(textSize>1)
	  textSize--;
  }*/
  //_disp->setCursor(5,(index*14)+32-textHeight/2);
  _disp->setCursor(5,(index*textHeight*textSize)+16);
  _disp->setTextSize(textSize);
  //_disp->setTextSize(checkTextSize(text, textSize));
  _disp->print(text);
  _disp->display();
}
/*
int8_t checkTextSize(char text[], int8_t textSize)
{
  while(strlen(text)*textWidth*textSize>128) {
	if(textSize>1)
	  textSize--;
	else
	  text[(int8_t)(128/textWidth/textSize)] = 0;
  }
  return textSize;
}*/

void qMenuDisplay::Highlight(int8_t index, int8_t textSize)
{
  //_disp->setMode('~');
  //for (int i = 0; i < titleHeight*textSize; i++) _disp->drawFastHLine(4,(index*14)+20+i,120, INVERSE);
  for (int8_t i = 0; i < textHeight*textSize; i++) _disp->drawFastHLine(4,(index*textHeight*textSize)+16+i,120, INVERSE);
  // _disp->drawBox(4,(index*14)+20,120,13);
 _disp->display();
}

void qMenuDisplay::MessageBox(const char text[], int8_t textSize)
{
  uint8_t msgWidth=strlen(text)*textWidth*textSize;
  
  int8_t x_text=64-(msgWidth/2);
  if(x_text<0+xPadding) {x_text = 0+xPadding;}
  
  int8_t y_text=40-(textHeight*textSize/2);	//40 is halfway down the blue section
  if(y_text < 16+yPadding) {y_text=16+yPadding;}
  
  int8_t x_line = x_text-xPadding;
  if(x_line<0) {x_line = 0;}
  
  int8_t y_line1 = y_text-(textHeight*textSize/2)-yPadding;
  if(y_line1<16) {y_line1=16;}
  
  int8_t y_line2 = y_text+(textHeight*textSize/2)+yPadding;
  if(y_line2>64) {y_line2=64;}
  
  int8_t lineLength = msgWidth+xPadding+xPadding;
  if(lineLength>127) {lineLength=127;}
  
  uint8_t yIdx = y_line1-borderPadding;
  if(yIdx<16) {yIdx = 16;}
  
  uint8_t yEnd = yIdx+(textHeight*textSize)+yPadding+yPadding+borderPadding+borderPadding+2;
  if(yEnd > 64) {yEnd = 64;}
  
  uint8_t blackOutWidth = lineLength+borderPadding+borderPadding+2;
  if(blackOutWidth > 127) {blackOutWidth = 127;}
  
  //Black out a section with a little more than the text box border for a contrasted border
  for (yIdx; yIdx< yEnd; yIdx++) {
	  _disp->drawFastHLine((x_line-borderPadding)<0?0:x_line-borderPadding,yIdx,blackOutWidth, BLACK);
  }
  //Make the text border
  //_disp->drawFastHLine(x_text-xPadding,y_text-yPadding-(textHeight*textSize/2),msgWidth+xPadding+xPadding+1, WHITE);
  //_disp->drawFastHLine(x_text-xPadding,y_text+yPadding+(textHeight*textSize/2),msgWidth+xPadding+xPadding+1, WHITE);
  //_disp->drawFastVLine(x_text-xPadding,y_text-yPadding-(textHeight*textSize/2),(textHeight*textSize)+8, WHITE);
  //_disp->drawFastVLine(x_text+msgWidth+xPadding,y_text-yPadding-(textHeight*textSize/2),(textHeight*textSize)+8, WHITE);
	
  _disp->drawFastHLine(x_line,y_line1,lineLength+1, WHITE);
  _disp->drawFastHLine(x_line,y_line2,lineLength+1, WHITE);
  _disp->drawFastVLine(x_line,y_line1,(textHeight*textSize)+8, WHITE);
  _disp->drawFastVLine(x_text+msgWidth+xPadding,y_line1,(textHeight*textSize)+8, WHITE);
  
  //Add the text
  _disp->setTextSize(textSize);
  _disp->setCursor(x_text,y_text-2);
  _disp->print(text);
  _disp->display();
}

void qMenuDisplay::adjustMsgBox(const char text[], int8_t textSize, int8_t highlightIdx, int8_t highlightWidth)
{
  int8_t msgWidth=strlen(text)*textWidth*textSize;
  int8_t x=64-(msgWidth/2);
  int8_t y=32;
  
  //Black out a section with a little more than the text box border for a contrasted border
  for (int8_t i = 0; i < (textHeight*textSize)+yPadding+yPadding+borderPadding+borderPadding+2; i++) {
	  _disp->drawFastHLine(x-xPadding-borderPadding,y-(textHeight*textSize/2)-xPadding-borderPadding+i,msgWidth+xPadding+xPadding+borderPadding+borderPadding+2, BLACK);
  }
  
  //Make the text border
  _disp->drawFastHLine(x-xPadding,y-yPadding-(textHeight*textSize/2),msgWidth+xPadding+xPadding+1, WHITE);
  _disp->drawFastHLine(x-xPadding,y+yPadding+(textHeight*textSize/2),msgWidth+xPadding+xPadding+1, WHITE);
  _disp->drawFastVLine(x-xPadding,y-yPadding-(textHeight*textSize/2),(textHeight*textSize)+8, WHITE);
  _disp->drawFastVLine(x+msgWidth+xPadding,y-yPadding-(textHeight*textSize/2),(textHeight*textSize)+8, WHITE);
	
  //Add the text
  _disp->setTextSize(textSize);
  _disp->setCursor(x,y+2-(textHeight*textSize/2));
  _disp->print(text);
  _disp->display();

  //Higlight the text
  for (int8_t i = 0; i < textHeight*textSize+2; i++) _disp->drawFastHLine(x-1+(textWidth*textSize*highlightIdx),y-(textHeight*textSize/2)-1+i,textWidth*textSize*highlightWidth, INVERSE);
  _disp->display();
}


