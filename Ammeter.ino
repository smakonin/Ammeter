/*******************************************************************************
*
*  Ammeter Project for Arduino Due
*
*  Coopyright (C) 2013 by Stgephen Makonin, William Sung, Ryan Dela Cruz.
*  All rights reserved.
*
*  This project is here by released under the:
*     COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL).
*  Project URL: https://github.com/smakonin/Ammeter
*
*  Main project file.
*
*******************************************************************************/

#include <Wire.h>

const boolean OUTOUT_TO_SERIAL = true;
const byte I2C_ADDRESS = 0x10;

const word CT_COUNT = 4;
const word CT_MVOLTS = 333;
const word CT_PINS[] = { 0, 1, 2, 3 };
const word CT_PRIMARY[] = { 20, 20, -1, -1 };
const double CT_MVOLTS_PER_AMP[] = { (double)CT_MVOLTS / (double)CT_PRIMARY[0], 
                                     (double)CT_MVOLTS / (double)CT_PRIMARY[1], 
                                     (double)CT_MVOLTS / (double)CT_PRIMARY[2], 
                                     (double)CT_MVOLTS / (double)CT_PRIMARY[3] };
const word SAMPLE_FREQ = 1000;
const double REF_MVOLTS = 3380.0;
const double MAX_ADC_VAL = 4095.0;
const double I_RATIO = REF_MVOLTS / MAX_ADC_VAL / sqrt(2);

volatile word sample_idx;
volatile word ct_samples[CT_COUNT][SAMPLE_FREQ];
volatile uint32_t ct_totals[CT_COUNT];
volatile uint32_t ct_readings[CT_COUNT];

uint32_t local_copy[CT_COUNT];

void make_local_copy()
{
  noInterrupts();
  memcpy(local_copy, (const void *)ct_readings, sizeof(uint32_t) * CT_COUNT);
  interrupts();
}

//TC1 ch 0
void TC3_Handler()
{  
  TC_GetStatus(TC1, 0);
    
  analogReadResolution(12);

  for(word i = 0; i < CT_COUNT; i++)
  {
    ct_totals[i] -= ct_samples[i][sample_idx];
    ct_samples[i][sample_idx] = (CT_PRIMARY[i] == -1) ? 0: analogRead(CT_PINS[i]);
    ct_totals[i] += ct_samples[i][sample_idx];    
    ct_readings[i] = round((double)ct_totals[i] / (double)SAMPLE_FREQ * I_RATIO / CT_MVOLTS_PER_AMP[i] * 100.0);
  }
    
  ++sample_idx %= SAMPLE_FREQ;
}

void request_event()
{
  if(OUTOUT_TO_SERIAL)
      Serial.print("I2C request made, sending: ");

  for(int i = 0; i < CT_COUNT; i++)
  {
    int c = Wire.write((byte)ct_readings[i]);

    if(OUTOUT_TO_SERIAL)
    {
      Serial.print(c);
      Serial.print(" ");
    }
  }

  if(OUTOUT_TO_SERIAL)
      Serial.println();
}
   
void setup() 
{
  if(OUTOUT_TO_SERIAL)
    Serial.begin(19200);
    
  sample_idx = 0;
  memset((void *)ct_samples, 0, sizeof(uint32_t) * CT_COUNT * SAMPLE_FREQ);
  memset((void *)ct_totals, 0, sizeof(uint32_t) * CT_COUNT);
  memset((void *)ct_readings, 0, sizeof(uint32_t) * CT_COUNT);
  
  start_timer(TC1, 0, TC3_IRQn, SAMPLE_FREQ);
  
   Wire.begin(I2C_ADDRESS);
   Wire.onRequest(request_event);
}

void loop() 
{
  if(OUTOUT_TO_SERIAL)
  {
    make_local_copy();
    
    Serial.print("Reading "); 
    for(word i = 0; i < CT_COUNT; i++)
    {
      Serial.print("CT ");
      Serial.print(i); 
      Serial.print(": ");   
      Serial.print(local_copy[i]);
      Serial.print(" mA ");   
      Serial.print(" -- ");
    }  
    Serial.println();
    
    delay(1000);
  }
}














