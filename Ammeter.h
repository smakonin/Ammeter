/*
  Ammeter.h - An Arduino Due library to read CTs 

  Copyright (C) 2013 Stephen Makonin. All rights reserved.
  This project is here by released under the COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL).
  
  Project URL: https://github.com/smakonin/Ammeter
*/

#ifndef AMMETER_H
#define AMMETER_H

#include "Arduino.h"

#define MAX_ADC 12

class Ammeter
{
	private:
		bool _Apin[MAX_ADC];
		double _Iratio[MAX_ADC];
		int _Iraw[MAX_ADC];
		double _Ifilt[MAX_ADC];
		double _Irms[MAX_ADC];

	public:
		Ammeter();
		bool addCT(int Apin, int ADCbits, double Vcc, double Ipri, double Isec, double Rburden);
		double read(int samples);
		double Irms(int Apin);

};

#endif
