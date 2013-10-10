/*******************************************************************************
*
*  Precision Ammeter Project for Arduino Due
*
*  Coopyright (C) 2013 by Stephen Makonin and contributors.
*  All rights reserved.
*
*  This project is here by released under the:
*     COMMON DEVELOPMENT AND DISTRIBUTION LICENSE (CDDL).
*  Project URL: https://github.com/smakonin/Ammeter
*
*  Timers IRQ module file.
*
*******************************************************************************/

// Structure used to find best timer
struct 
{
  byte flag;         // The timer flag used for identification
  byte divisor;      // The timer resolution
} clock_config[] = { { TC_CMR_TCCLKS_TIMER_CLOCK1,   2 },
         	     { TC_CMR_TCCLKS_TIMER_CLOCK2,   8 },
		     { TC_CMR_TCCLKS_TIMER_CLOCK3,  32 },
		     { TC_CMR_TCCLKS_TIMER_CLOCK4, 128 } };

// Setup the Timer intergupt to call the handler function
void start_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency)
{
  // Code based on examples from MCU documentation
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk((uint32_t)irq);
  
  // Find the best timer based on desired frequency
  int best_clock = 3;
  float best_err = 1.0;
  for(int i = 3; i > -1; i--)
  {
    float ticks = (float)VARIANT_MCK / (float)frequency / (float)clock_config[i].divisor;
    float err = abs(ticks - round(ticks));
    if(err < best_err) 
    {
      best_clock = i;
      best_err = err;
    }
  }
  
  // Add values to registers and setup the Timer
  // Code based on examples from MCU documentation
  TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | clock_config[best_clock].flag);  
  uint32_t rc = (float)VARIANT_MCK / (float)frequency / (float)clock_config[best_clock].divisor;
  TC_SetRA(tc, channel, rc / 2);
  TC_SetRC(tc, channel, rc);
  TC_Start(tc, channel);
  tc->TC_CHANNEL[channel].TC_IER = TC_IER_CPCS;
  tc->TC_CHANNEL[channel].TC_IDR = ~TC_IER_CPCS;
  NVIC_EnableIRQ(irq);
}
