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

#include <stdio.h> 

#include <pkgconf/system.h>
#include <pkgconf/devs_i2c_mpc5xxx.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/io/i2c.h>
#include "hasp_i2c_mpc5xxx.h"
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>

#include "../../sensors/GPIO/reset_interface.h"		// functions for the reset pin

// ********************************************************************* //
// Local-Global Variable Initialisations                                 //
// ********************************************************************* //
cyg_uint32 			global_i2c_mode;

cyg_uint32 i2c_intr_vector = CYGNUM_HAL_INTERRUPT_I2C1;	// ISR Vector

cyg_drv_mutex_t     i2c_lock; 				
cyg_drv_cond_t      i2c_wait;			
cyg_handle_t        i2c_interrupt_handle;
cyg_interrupt 		i2c_interrupt_data;

cyg_uint32			byte0 = 0;			// Data byte 0
cyg_uint32			byte1 = 0;			// Data byte 1
cyg_uint32			bytec = 0;			// Combined data bytes

cyg_uint32			i2c_flag = 0;

// ********************************************************************* //
// Peak Detection Data Retrieval                                         //
// ********************************************************************* //
// User-end function to begin I2C transaction w/ peak detection circuit
// Writes device address (MCP3021) to data register
// The rest of the I2C functionality happens in the Interrupt Service Routine (ISR)

cyg_uint32
get_peak_data(struct xray *xrayData_ptr)
{
	i2c_flag = 1;
	// printf("I2C Disabled");
	
	HAL_WRITE_UINT32(I2C_0_CONTROL_REG, I2C_BEGIN_TX);		// Change control register to begin transmission
	HAL_WRITE_UINT32(I2C_0_DATA_REG, 0x9D000000);			// Write device address to the data register
	
	global_i2c_mode = CYG_MPC5XXX_I2C_XFER_MODE_STARTRX;	// Change device mode variable for ISR handling
	
	bytec = byte0+byte1;
	bytec >>= 2;
	
	xrayData_ptr->peak = bytec;		// store *previous* data in the xray logger pointer
	
	byte0 = byte1 = 0;
	
	// xrayData_ptr->peak = 11; // Test populate xray data structure

    return 2;
}

// ********************************************************************* //
// I2C Initialisation                                                    //
// ********************************************************************* //
// Set up the I2C device and data structures
// Set up the I2C Interrupt Serive Routine (ISR)

void
hasp_mpc5xxx_i2c_init()
{
	initReset();										// initialise the reset pin
	resetHigh();
	
    cyg_drv_mutex_init(&i2c_lock);
    cyg_drv_cond_init(&i2c_wait, &i2c_lock);
    cyg_drv_interrupt_create(i2c_intr_vector,				
                             0,		 		
                             (cyg_addrword_t) 0,
                             &mpc5xxx_i2c_isr,
                             &mpc5xxx_i2c_dsr,
                             &(i2c_interrupt_handle),
                             &(i2c_interrupt_data));
    cyg_drv_interrupt_attach(i2c_interrupt_handle);

	HAL_WRITE_UINT32(I2C_0_FDR_REG, 0x89000000);		// Set clock to 100MHz / 352

	HAL_WRITE_UINT32(I2C_0_ADDRESS_REG, 0x00000000);	// Set MPC5xxx slave address, not useful to us

	HAL_WRITE_UINT32(I2C_0_CONTROL_REG, I2C_ENABLE);	// Enable the I2C device but do not start any transfers and leave interrupts disabled.

	HAL_WRITE_UINT32(I2C_0_STATUS_REG, 0x00000000);		// Clear any pending conditions including interrupts.

    HAL_INTERRUPT_UNMASK(i2c_intr_vector);				// Interrupts can now be safely unmasked
	
	i2c_flag = 0;
	
	resetLow();
	
}

// ********************************************************************* //
// I2C Interrupt Service Routine                                         //
// ********************************************************************* //

cyg_uint32
mpc5xxx_i2c_isr(cyg_vector_t vector, cyg_addrword_t data)
{

    cyg_uint32     dr;
	HAL_WRITE_UINT32(I2C_0_STATUS_REG, 0x00000000);				// Clear the status register of all interrupts
	
	if (CYG_MPC5XXX_I2C_XFER_MODE_STARTRX == global_i2c_mode) 	// Sent the address and now read the dummy byte
    {

		HAL_WRITE_UINT32(I2C_0_CONTROL_REG, I2C_BEGIN_RX);
		HAL_READ_UINT32(I2C_0_DATA_REG,dr); 					// Read the dummy byte to initiate next transfer
		
        global_i2c_mode = CYG_MPC5XXX_I2C_XFER_MODE_RX1;		// Change transfer mode to receive first byte
		
		resetHigh();			// Set reset pin to High that drains the capacitor to 0V, IS THIS where it goes?
    } 
	
    else if (CYG_MPC5XXX_I2C_XFER_MODE_RX1 == global_i2c_mode) 	// Transmit mode to read first data byte
    {
	
		HAL_WRITE_UINT32(I2C_0_CONTROL_REG, I2C_SEND_TXAK);		
	
		HAL_READ_UINT32(I2C_0_DATA_REG, byte0);
		byte0 >>= 16;
		
		global_i2c_mode = CYG_MPC5XXX_I2C_XFER_MODE_RX2;		// Change transfer mode to receive second byte  
		
		resetLow();
    } 
	
	else if(CYG_MPC5XXX_I2C_XFER_MODE_RX2 == global_i2c_mode) 	// Transmit mode to receive last byte
    {
	
		HAL_WRITE_UINT32(I2C_0_STATUS_REG, 0x00000000);
		HAL_WRITE_UINT32(I2C_0_CONTROL_REG, I2C_SEND_STOP);
	
		HAL_READ_UINT32(I2C_0_DATA_REG, byte1);
		byte1>>=24;
		
		i2c_flag = 0;					
		global_i2c_mode = CYG_MPC5XXX_I2C_XFER_MODE_DEFAULT;
		
    } 
	
	else if(CYG_MPC5XXX_I2C_XFER_MODE_DEFAULT == global_i2c_mode)	// Default case, just in case modes get corrupted
	{
		global_i2c_mode = CYG_MPC5XXX_I2C_XFER_MODE_STARTRX;	// Change mode to STARTRX in case the ISR finds itself in this mode
		HAL_WRITE_UINT32(I2C_0_CONTROL_REG, I2C_SEND_STOP);		// change to slave mode to generate stop signal
	}
	
    else	// Same as CYG_MPC5XXX_I2C_XFER_MODE_DEFAULT above, just in case of badness!
    {
		global_i2c_mode	= CYG_MPC5XXX_I2C_XFER_MODE_STARTRX;
		HAL_WRITE_UINT32(I2C_0_CONTROL_REG, I2C_SEND_STOP);		// change to slave mode to generate stop signal
    }
	
    HAL_INTERRUPT_ACKNOWLEDGE(i2c_intr_vector);
	

    return CYG_ISR_HANDLED;
}

// ********************************************************************* //
// I2C Delayed Service Routine                                           //
// ********************************************************************* //

void
mpc5xxx_i2c_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{

}

//---------------------------------------------------------------------------
// EOF hasp_i2c_mpc5xxx.c


