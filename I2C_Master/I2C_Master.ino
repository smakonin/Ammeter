/*******************************************************************************
*
*  Ammeter Project for Arduino Due
*
*  Coopyright (C) 2013 by Stgephen Makonin, Ryan Dela Cruz, William Sung.
*  All rights reserved.
*
*  This project is here by released under the:
*     COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL).
*  Project URL: https://github.com/smakonin/Ammeter
*
*  Example of an I2C master that communicats with the Ammeter.
*  Data returend is displayed on a 2-line character lcd screen.
*
*******************************************************************************/

#include <Wire.h>

const byte I2C_SLAVE_ADDRESS = 0x10;
const word CT_COUNT = 4;

void setup() 
{
   Wire.begin();
   Serial.begin(9600);
   backlightOn();
}

void loop() 
{
  unsigned long l[CT_COUNT];


  Wire.requestFrom(I2C_SLAVE_ADDRESS, CT_COUNT * sizeof(unsigned long));
  if(Wire.available())
  {
    for(int i = 0; i < CT_COUNT; i++)
    {
      long c4 = Wire.read();
      long c3 = Wire.read();
      long c2 = Wire.read();
      long c1 = Wire.read();
      l[i] = (((((c1 << 8) + c2) << 8) + c3) << 8) + c4; 

      Serial.print("CT ");
      Serial.print(i);
      Serial.print(" = ");
      Serial.print(l[i]);
      Serial.print(", ");
    }    
    
    Serial.println();
    delay(1000);
  } 
}

void selectLineOne()
{  //puts the cursor at line 0 char 0.
   Serial.write(0xFE);   //command flag
   Serial.write(128);    //position
   delay(10);
}
void backlightOn()
{  //turns on the backlight
    Serial.write(0x7C);   //command flag for backlight stuff
    Serial.write(157);    //light level.
   delay(10);
}
