//  ii_udraw2arduino2linux
//  The Arduino portion of the code
//  By Matthew Watts 2018
//
//  Tablet wiring differs from normal nunchuck wiring
//  Yellow > A5/SCL
//  Green > A4/SDA
//  Blue > GND
//  Brown > 5v
//
//  With thanks to everybody who has already published
//  code to use wii nunchuck for the init sequences
//  The tablet data was decoded by trial and error

// udraw responds to address I2C 0x52
#include <Wire.h>

// global most up to date tablet state
uint16_t x_raw;
uint16_t y_raw;
uint8_t pressure_raw;
uint8_t stylusb2;
uint8_t stylusb1;
uint8_t stylusb0;

// Setup serial and send init to tablet
void setup() {
  Serial.begin(38400);
  Wire.begin();
  Wire.beginTransmission(0x52);
  Wire.write(0xF0);
  Wire.write(0x55);
  Wire.endTransmission();
  delay(100);
  Wire.beginTransmission(0x52);
  Wire.write(0xFB);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(5);
}

// get data from tablet and check if state has
// altered from last run
uint8_t hasChange = 0;
uint8_t reply[10];
uint8_t lastReply[10];
uint8_t firstRun = 1;
void getDataFromTablet() {
  Wire.beginTransmission(0x52);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x52, 6);
  
  // Read bytes
  hasChange = 0;
  unsigned long timeout=millis();
  for(int lp=0; lp<6; lp++) {
    while( !Wire.available() ) {
      if( millis() - timeout > 2 ) {
        return;
      } 
    };
    reply[lp] = (uint8_t)Wire.read();
    if( firstRun == 1 ) {
      lastReply[lp] = reply[lp];
    } else {
      if(lastReply[lp] != reply[lp]) {
        lastReply[lp] = reply[lp];
        hasChange = 1;
      }
    }
  }
  firstRun = 0;
  
  // decode
  // the x and y are 12bit values that are
  // mixed together in the first 3 bytes of the
  // data
  uint8_t x_nibble_one = reply[2] & 0x0F;  // 2R
  uint8_t x_nibble_two = reply[0] >> 4;    // 0L
  uint8_t x_nibble_three = reply[0] & 0x0F;// 0R
  
  uint8_t y_nibble_one = reply[2] >> 4;    // 2L
  uint8_t y_nibble_two = reply[1] >> 4;    // 1L
  uint8_t y_nibble_three = reply[1] & 0x0F;// 1R
 
  x_raw = (uint16_t)x_nibble_one<<8 | (uint16_t)x_nibble_two<<4 | (uint16_t)x_nibble_three;
  y_raw = (uint16_t)y_nibble_one<<8 | (uint16_t)y_nibble_two<<4 | (uint16_t)y_nibble_three;

  // pressure is an 8bit value from 4th byte
  pressure_raw = reply[3];

  // byte 5 contains 3 digital button states
  // the two buttons on the side of the pen
  // and one that comes on when pressure is high
  stylusb2 = !(reply[5] & 1);
  stylusb1 = !(reply[5]>>1 & 1);
  stylusb0 = (reply[5]>>2 & 1);
}

// send out data via serial
// only mode 0 is implemented here
// but there is room to add modes later
// for ISDV4 serial tablet emulation
uint8_t serialMode=0;
void sendData() {
   // outgoing data
  switch(serialMode) {
     // normal mode spits out space-delimited raw x, y, pressure, button 0, 1, 2 then newline
    // only sends data if something has changed
    case 0:
      if( hasChange == 0 ) {
        return;
      }
      Serial.print(x_raw);
      Serial.print(" ");
      Serial.print(y_raw);
      Serial.print(" ");
      Serial.print(pressure_raw);
      Serial.print(" ");
      Serial.print(stylusb0);
      Serial.print(" ");
      Serial.print(stylusb1);
      Serial.print(" ");
      Serial.print(stylusb2);
      Serial.println("");
      Serial.flush();
      while ((UCSR0A & _BV (TXC0)) == 0) {}
      break;
    case 1: // wait mode do nothing
      break;
    case 2:
      break;
  }
}

// receive data via serial
// DOES NOTHING right now but is the beginning of 
// creating ISDV4 protocol support
void receiveData() {
  // incoming data (ISDV4)
  // read from port 0, send to port 1:
  if (Serial.available()) {
    char inByte = Serial.read();
    uint8_t reponseByte = 0;
    switch(inByte) {
      case '*': // ISDV4_QUERY - Query stylus configuration data (expects a response)
        //Stylus Query Response
        //Byte 	Bits 	Description
        //0 	  7 	Always 1 -- marks beginning of packet
	//        6 	Always 1 - indicates control packet
	//        0-5 	Data ID
        //1       7 	Always 0
	//        0-6 	Max X (bits 7-13 of X coordinate)
        //2 	  7 	Always 0
	//        2-6 	Max X (bits 2-6 of X coordinate)
        //3 	  7 	Always 0
	//        0-6 	Max Y (bits 7-13 of Y coordinate)
        //4 	  7 	Always 0
	//        2-6 	Max Y (bits 2-6 of Y coordinate)
        //5 	  7 	Always 0
	//        0-6 	Max Pressure (bits 0-6 of Pressure).
        //6 	  7 	Always 0
	//        5-6 	Max X (bits 0-1 of X coordinate)
	//        3-4 	Max Y (bits 0-1 of Y coordinate)
	//        0-2 	Max Pressure (bits 7-9 of Pressure)
        //7 	  7 	Always 0
	//        0-6 	Max Y Tilt
        //8 	  7 	Always 0
	//        0-6 	Max X Tilt
        //9 	  7 	Always 0
	//        0-6 	Version (bits 14-7)
        //10 	  7 	Always 0
	//        0-6 	Version (bits 0-6)
        
        break;
      case '%': // ISDV4_TOUCH_QUERY - Query touchscreen configuration data (expects a response)
      
        break;
      case '0': // ISDV4_STOP - Stop sending coordinates
      
        break;
      case '1': // ISDV4_START - Start sending coordinates
      
        break;
      case '&': // ISDV4_RESET - Touch panel reset request
      
        break;
    }
  } 
}

// the main loop of course ;-)
void loop() {
  getDataFromTablet();
  sendData();
  receiveData();
}




