/*
  Ammeter.cpp - An Arduino Due library to read CTs 

  Copyright (C) 2013 Stephen Makonin. All rights reserved.
  This project is here by released under the COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL).
  
  Project URL: https://github.com/smakonin/Ammeter
*/

#include "Arduino.h"
#include "Ammeter.h"

Ammeter::Ammeter()
{
	memset(_Apin, 0, sizeof(bool) * MAX_ADC);
	memset(_Iratio, 0, sizeof(double) * MAX_ADC);
	memset(_Iraw, 0, sizeof(int) * MAX_ADC);
	memset(_Ifilt, 0, sizeof(double) * MAX_ADC);
	memset(_Irms, 0, sizeof(double) * MAX_ADC);
}


bool Ammeter::addCT(int Apin, int ADCbits, double Vcc, double Ipri, double Isec, double OHMburden)
{
	if(Apin < 0 || Apin > MAX_ADC - 1)
		return false;

	_Apin[Apin] = true;
	_Iratio[Apin] = (Ipri / Isec / OHMburden) * (Vcc / ((1 << ADCbits) - 1));
}

double Ammeter::read(int samples)
{
	int Iraw = 0;
	double Ifilt = 0;
	double Isum[MAX_ADC];

	memset(Isum, 0, sizeof(double) * MAX_ADC);

	for(int i = 0; i < samples; i++)
	{
		for(int Apin = 0; Apin < MAX_ADC; Apin++)
		{
			if(!_Apin[Apin])
				continue;

		    Iraw = analogRead(Apin);
		    Ifilt = 0.996 * (_Ifilt[Apin] + Iraw - _Iraw[Apin]);
			_Iraw[Apin] = Iraw;
			_Ifilt[Apin] = Ifilt;
		    Isum[Apin] += Ifilt * Ifilt;

		}
	}

	for(int Apin = 0; Apin < MAX_ADC; Apin++)
		_Irms[Apin] = _Iratio[Apin] * sqrt(Isum[Apin] / (double)samples); 
}

double Ammeter::Irms(int Apin)
{
	if(Apin < 0 || Apin > MAX_ADC - 1)
		return -1;

	if(!_Apin[Apin])
		return -1;

	return _Irms[Apin];
}
