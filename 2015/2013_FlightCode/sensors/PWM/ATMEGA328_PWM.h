/*! \file ATMEGA328_PWM.h
 *	\brief Ardupilot ATMEGA328 PWM header file
 *
 *	\details Header file for the Ardupilot ATMEGA328 PWM reader.
 *	\ingroup pwm_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: ATMEGA328_PWM.h 756 2012-01-04 18:51:43Z murch $
 */
#ifndef ATMEGA328_PWM_H_
#define ATMEGA328_PWM_H_

/// Configuration for the Ardupilot ATMEGA328 PWM chip.
/// I2C slave address is 0xF2 write, 0xF3 read.
static const cyg_i2c_device ATMEGA328_PWM  = {
      .i2c_bus        = &mpc5200_i2c_bus0,
      .i2c_address    = 0xF3,
      .i2c_flags      = 0x00,            
      .i2c_delay      = CYG_I2C_DEFAULT_DELAY
  };
  
#endif
