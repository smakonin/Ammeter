/*******************************************************************************
*
*  Precision Ammeter Project for Arduino Due
*
*  Coopyright (C) 2013 by Stephen Makonin, William Sung, Ryan Dela Cruz.
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

const boolean OUTOUT_TO_SERIAL = true;              // Set to false to stop serial debugging
const byte I2C_ADDRESS = 0x10;                      // The address fo this I2C slave

const word SAMPLE_FREQ = 2000;                      // The frequency at with to sample ADC
const word CT_COUNT = 4;                            // The number of CTs that can be connected
const double CT_MVOLTS = 333.0;                     // The max CT secondary output in mV
const double REF_MVOLTS = 3300.0;                   // The refernce V of the Due board
const double MAX_ADC_VAL = 4095.0;                  // The max bits of the ADC, 4095 for 12-bits
const double OPAMP_GAIN = 6.9;                      // The gain of the opamp amplifier -- may need to change when calibtating 
                                                    // The max ADC value that the ammeter will ever reach due to rectification, etc.
const word MAX_RECT_VAL = CT_MVOLTS * OPAMP_GAIN / REF_MVOLTS * MAX_ADC_VAL;

const word CT_PINS[] = { 0, 1, 3, 2 };              // What analog pin is assigned to wich CT
const word CT_PRIMARY[] = { 20, -1, -1, -1 };       // The CT primary A, -1 means no CT is connected

volatile word sample_idx;                           // Where in the sampling array are we
volatile word ct_samples[CT_COUNT][SAMPLE_FREQ];    // Store a number of ADC sample to do averaging
volatile uint32_t ct_totals[CT_COUNT];              // The sum of the samples array 
volatile float ct_readings[CT_COUNT];               // The final mA value, DO NOT ACCESS, use  make_local_copy() and local_copy[]

float local_copy[CT_COUNT];                         // A non volitle copy of ct_readings to by used by non-timer code

// Makes a safe copy of ct_readings[] which can be uddated at any time by IRQ
// Call this function then use local_copy[] to seat the final mA values of the CTs
void make_local_copy()
{
  noInterrupts();
  memcpy(local_copy, (const void *)ct_readings, sizeof(float) * CT_COUNT);
  interrupts();
}

int pinstat = 0;

// TC1 ch 0
// The timer interupt handler function
void TC3_Handler()
{
  // Clear clags and per MCU datasheet
  TC_GetStatus(TC1, 0);
  
  pinstat = ~pinstat;
  digitalWrite(2, pinstat);
  digitalWrite(3, 1);
  
  noInterrupts();
    
  // Set ADC sampling to 12-bit
  analogReadResolution(12);

  // For each CT, sample and calc final value
  for(word i = 0; i < CT_COUNT; i++)
  {
    // Use sliding window averaging, optimized so that the whole array not needed to be summed each time
    ct_totals[i] -= ct_samples[i][sample_idx];
    ct_samples[i][sample_idx] = (CT_PRIMARY[i] == -1) ? 0: analogRead(CT_PINS[i]);
    ct_totals[i] += ct_samples[i][sample_idx];
    double ct_average = (double)ct_totals[i] / (double)SAMPLE_FREQ;
        
    // Convert ADC value to A reading with 1 decimal precision 
    // for deci-amps anything lower is too noisy.
    ct_readings[i] = round(ct_average * (double)CT_PRIMARY[i] * 1000.0 / (double)MAX_RECT_VAL / 100.0) / 10.0;
  }
  
  // Incroment index, wrap to 0 when max is reached
  ++sample_idx %= SAMPLE_FREQ;
  
  interrupts();

  digitalWrite(3, 0);
}

// Interupt handler for I2C requests from mater
void request_event()
{
  if(OUTOUT_TO_SERIAL)
      Serial.print("I2C request made, sending: ");

  // Make a stable, local copy
  make_local_copy();
  // Send each unsigned long as an array of bytes for each CT
  int c = Wire.write((byte *)local_copy, sizeof(uint32_t) * CT_COUNT);
  
  if(OUTOUT_TO_SERIAL)
      Serial.print(c);    
      Serial.println(" bytes sent.");
}
   
void setup() 
{
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  
  // If requested setup the serical connection
  if(OUTOUT_TO_SERIAL)
    Serial.begin(19200);
  
  // Initiallize all to 0
  sample_idx = 0;
  memset((void *)ct_samples, 0, sizeof(uint32_t) * CT_COUNT * SAMPLE_FREQ);
  memset((void *)ct_totals, 0, sizeof(uint32_t) * CT_COUNT);
  memset((void *)ct_readings, 0, sizeof(uint32_t) * CT_COUNT);
  
  // Setup the Timer interupt
  start_timer(TC1, 0, TC3_IRQn, SAMPLE_FREQ);
  
  // Setup for slave I2C operations
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(request_event);
}

void loop() 
{
  // Only send to serial console if flag is set
  if(OUTOUT_TO_SERIAL)
  {
      // Make a stable, local copy
    make_local_copy();
    
    // Prin the values for each CT to the serial console
    Serial.print("Reading "); 
    for(word i = 0; i < CT_COUNT; i++)
    {
      Serial.print("CT ");
      Serial.print(i); 
      Serial.print(": ");   
      Serial.print(local_copy[i]);
      Serial.print(" A ");   
      Serial.print(" -- ");
    }  
    Serial.println();
    
    // Delay by 1 second
    delay(1000);
  }
}
