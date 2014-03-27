// ************************************************** //  
// Author: 	John Jackson | jacks974@umn.edu			  //
// Date: 	15/17/2013								  //
//													  //	
// For systemwide calling of the peak-detection 	  //
// circuit reset pin.								  //
// ************************************************** //

#include <cyg/io/mpc5xxx_gpio.h>
#include <cyg/hal/mpc5xxx.h>

cyg_uint32 peakResetChannel = MPC5XXX_GPW_GPIO_WKUP_6;

BOOL enable 	= true;
BOOL bOutput	= 0;	
BOOL bHigh 		= 1;
BOOL bLow 		= 0;

void initReset()
{
	// peakResetChannel initialisation | This can be put into gpio_interface.c
	GPIO_GPW_EnableGPIO(peakResetChannel, enable);			// Append: Enable reset GPIO
	GPIO_GPW_SetDirection(peakResetChannel, bOutput);		// Append: Set PGIO direction
	GPIO_GPW_Output(peakResetChannel, bLow);				// Append: Initialise reset pin to be HIGH
};

void resetHigh()
{
	GPIO_GPW_Output(peakResetChannel, bHigh);
};

void resetLow()
{
	GPIO_GPW_Output(peakResetChannel, bLow);
};