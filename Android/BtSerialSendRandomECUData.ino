/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 7 (connect to TX of other device)
 * TX is digital pin 8 (connect to RX of other device)

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.
 */

#include <SoftwareSerial.h>

// Best for Teensy LC & 3.2
//SoftwareSerial mySerial(0, 1); // RX,TX
SoftwareSerial btSerial(7, 8);
//SoftwareSerial mySerial(9, 10);

// Best for Teensy 3.5 & 3.6
//SoftwareSerial mySerial(0, 1); // RX,TX
//SoftwareSerial mySerial(7, 8);
//SoftwareSerial mySerial(9, 10);
//SoftwareSerial mySerial(31, 32);
//SoftwareSerial mySerial(34, 33);
//SoftwareSerial mySerial(47, 48);

// Best for Teensy 4.1
//SoftwareSerial mySerial(0, 1); // RX,TX
//SoftwareSerial mySerial(7, 8);
//SoftwareSerial mySerial(15, 14);
//SoftwareSerial mySerial(16, 17);
//SoftwareSerial mySerial(21, 20);
//SoftwareSerial mySerial(25, 24);
//SoftwareSerial mySerial(28, 29);
//SoftwareSerial mySerial(34, 35);
#define BT_CONTROL_ID         119
#define ECU_DATA_STREAM_SIZE  31
#define BT_DATA_STREAM_SIZE   16
#define REM_BT_LOGGING_INTERVAL   40 //ms
byte dataStream[ECU_DATA_STREAM_SIZE];  // 32 bytes. we only have 1024 to play with. 31 for data, 1 for sign apart from it doesn't use the sign. bah.
bool dataStreamCounter = false;// used to check for double 255's
byte bluetoothStream[BT_DATA_STREAM_SIZE];
bool ecuStreamUpdate;
unsigned int RPM = 0;
byte LoopStatus = 0;
const byte Clsd = true;
const byte Open = false;
const byte Dcel = 2;
byte Exhaust = 0;
const byte Lean = false;
const byte Rich = true;

void btTestMode(void) {
  for(int i=0; i< sizeof(dataStream) / sizeof(dataStream[0]); i++) {
    if(dataStream[i] == 0 || dataStream[i] > 100) {
     dataStream[i] = i + 1;
    } else {
      dataStream[i] = dataStream[i] + 1;
    }
  }
  RPM = word(dataStream[9], dataStream[8]);
}

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  btSerial.begin(38400);
  btSerial.println("Hello, world?");
}

void loop() 
{
  /*if (btSerial.available())
    Serial.write(btSerial.read());
  if (Serial.available())
    btSerial.write(Serial.read());
*/
  btTestMode(); 
      bluetoothStream[0] = dataStream[3]; //MAP
      bluetoothStream[1] = dataStream[4]; //CTS
      bluetoothStream[2] = dataStream[5]; //IAT
      bluetoothStream[3] = dataStream[6]; //VOLTS
      bluetoothStream[4] = dataStream[7]; //O2
      bluetoothStream[5] = RPM / 20;//dataStream[8]; //RPM Lo bootleg convert for app
      bluetoothStream[6] = dataStream[9]; //RPM Hi, not used currently
      bluetoothStream[7] = dataStream[12]; //TPS
      bluetoothStream[8] = dataStream[13]; //IGN ADVANCE
  
      if (LoopStatus == Clsd)               //LOOP
        bluetoothStream[9] = 2; //clsd
      else if (LoopStatus == Dcel)
        bluetoothStream[9] = 3; // decel
      else bluetoothStream[9] = 1; //open

      if (Exhaust == Lean)                   //EXHAUST
        bluetoothStream[10] = 1; 
      else
        bluetoothStream[10] = 2;
      
      bluetoothStream[11] = dataStream[19]; //INJ MS
      bluetoothStream[12] = dataStream[27]; //KNOCK
      bluetoothStream[13] = dataStream[16]; //BARO
      bluetoothStream[14] = dataStream[24]; //STFT
      bluetoothStream[15] = dataStream[26]; //LTFT
    
    
    btSerial.write(byte(BT_CONTROL_ID));           //Control byte 119
    btSerial.write(bluetoothStream, BT_DATA_STREAM_SIZE);
 
   delay(50);
}
