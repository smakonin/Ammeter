/*
  ReadCT.ino - an excample for using the Ammeter library with a CT

  Copyright (C) 2013 Stephen Makonin and contributors. All rights reserved.
  This project is here by released under the COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL).
  
  Project URL: https://github.com/smakonin/Ammeter

  CT used for this project was a EChun ECS1030-L72 bought from SparkFun.
  Product URL: https://www.sparkfun.com/products/11005?
*/

#include "Ammeter.h"
Ammeter ammeter;

int ADCbits = 12;
long previousMillis = 0; 
long interval = 1000;

void setup() 
{
  Serial.begin(9600);
  analogReadResolution(ADCbits);  
  ammeter.addCT(1, ADCbits, 3.3, 30, 0.015, 82);
  ammeter.read(1480);
}

void loop()
{
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis > interval) 
  {
    previousMillis = currentMillis;  

    ammeter.read(1480);
    double Irms = ammeter.Irms(1);

    Serial.print(Irms, 3);
    Serial.print(" Amps, ");
    Serial.print(Irms * 120.0, 1);
    Serial.println(" Watts");
  }
}

