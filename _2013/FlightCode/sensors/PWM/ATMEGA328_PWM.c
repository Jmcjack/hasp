/*! \file ATMEGA328_PWM.c
 *	\brief Ardupilot ATMEGA328 PWM source code
 *
 *	\details This file implements the code to read the Ardupilot ATMEGA328 PWM reader chip.
 *	\ingroup pwm_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: ATMEGA328_PWM.c 756 2012-01-04 18:51:43Z murch $
 */

#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>
#include <cyg/io/i2c_mpc5xxx.h>

#include "../../utils/misc.h"
#include "../../globaldefs.h"
#include "pwm_interface.h"
#include "ATMEGA328_PWM.h"


void init_pwm(){}

int read_pwm(uint16_t *pwm_signal_ptr){
	unsigned char  DataBuf[16];

	// Poll Ardupilot for 16 bytes of data (8 channels)
	if(16 != cyg_i2c_rx(&ATMEGA328_PWM, &DataBuf[0], 16)){
		fprintf(stderr,"\n read_pwm: 16 bytes not read");
		return 0;
	}
	endian_swap(&DataBuf[0],0,16); // swap bytes
	memcpy(pwm_signal_ptr,&DataBuf,16); // copy to uint16_t array

	return 1;
}

