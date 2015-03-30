//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Kale Hedstrom, John Jackson (jacks974@umn.edu)
// Contributors:  Based on Zhefeng Li's i2c_mpc5xxx.c
// Date:          2013-20-07
// Description:   I2C driver for Freescale MPC5xxx processor for use specifically with
// 				  the University of Minnesota's HASP 2013 Project.  The as-is driver is only
//				  suitable to communicate with the MCP2031 ADC.
//####DESCRIPTIONEND####
//==========================================================================
#ifndef	_I2C_MPC5XXX_H_
#define _I2C_MPC5XXX_H_
#endif

#include <pkgconf/devs_i2c_mpc5xxx.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/i2c.h>

#include "../../globaldefs.h"  

// ************************************** //
// Raw register values for module 0 	  //
// ************************************** //
#define I2C_0_ADDRESS_REG	0xF0003D00
#define I2C_0_FDR_REG   	0xF0003D04
#define I2C_0_CONTROL_REG 	0xF0003D08
#define I2C_0_DATA_REG 		0xF0003D10
#define I2C_0_INTR_REG		0xF0003D20
#define I2C_0_FILTER_REG	0xF0003D24
#define I2C_0_STATUS_REG	0xF0003D0C

// *********************************** //
// Values to write to control register //
// *********************************** //

#define I2C_ENABLE			MPC5XXX_I2C_CR_EN		// Enables I2C module in the init function

#define I2C_BEGIN_TX		MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN | MPC5XXX_I2C_CR_STA | MPC5XXX_I2C_CR_TX  
#define I2C_BEGIN_RX        MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN | MPC5XXX_I2C_CR_STA      
#define I2C_SEND_TXAK		MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN | MPC5XXX_I2C_CR_STA | MPC5XXX_I2C_CR_TXAK            
#define I2C_SEND_STOP       MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN                                       

// ******************************************** //
// I2C Interrupt Emulation Register	and value	//
// ******************************************** //

#define I2C_EMULATION_REG	0xF0003D00
#define I2C_EMULATION_VAL	0x7F800040

// ************************************** //
// Define HASP I2C Transfer Modes		  //
// ************************************** //

externC cyg_uint32 i2c_flag; // comment

typedef enum cyg_mpc5xxx_i2c_xfer_mode {
    CYG_MPC5XXX_I2C_XFER_MODE_INVALID   = 0x00,
    CYG_MPC5XXX_I2C_XFER_MODE_RX1       = 0x01,
	CYG_MPC5XXX_I2C_XFER_MODE_RX2       = 0x02,
    CYG_MPC5XXX_I2C_XFER_MODE_STARTRX   = 0x03,
	CYG_MPC5XXX_I2C_XFER_MODE_DEFAULT   = 0x04
	
} cyg_mpc5xxx_i2c_xfer_mode;

// ************************************** //
// Initialise the main I2C functions      //
// ************************************** //

void        hasp_mpc5xxx_i2c_init();
cyg_uint32  get_peak_data(struct xray *xrayData_ptr);

cyg_uint32
mpc5xxx_i2c_isr(cyg_vector_t, cyg_addrword_t);
void
mpc5xxx_i2c_dsr(cyg_vector_t, cyg_ucount32, cyg_addrword_t);
