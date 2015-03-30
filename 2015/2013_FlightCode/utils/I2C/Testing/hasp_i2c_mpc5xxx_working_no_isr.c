//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Zhefeng Li
// Contributors:  
// Date:          2010-01-14
// Description:   I2C driver for Freescale MPC5xxx processor
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

#ifndef I2C_MODULE_0

static cyg_mpc5xxx_i2c_extra hasp_mpc5200_i2c_bus0_extra = 	
{                                   
    (volatile cyg_uint32*)(MPC5XXX_I2C+MPC5XXX_I2C_MODULE0),                                                                         
    CYGNUM_HAL_INTERRUPT_I2C1,                                                                      
    0,                                                                      
    400000                                                                           
};     
      
cyg_i2c_bus hasp_mpc5200_i2c_bus0  CYG_HAL_TABLE_ENTRY( i2c_buses ) = 
{                    
    .i2c_init_fn    = hasp_mpc5xxx_i2c_init,               	// change to actual init function                         
//  .i2c_tx_fn      = &cyg_mpc5xxx_i2c_tx = 0,            	// make tx, rx and stop dummy function (we are not using these yet)                              
//  .i2c_rx_fn      = &cyg_mpc5xxx_i2c_rx = 0,                                              
//  .i2c_stop_fn    = &cyg_mpc5xxx_i2c_stop = 0,                                
    .i2c_extra      = ((void*) & ( hasp_mpc5200_i2c_bus0_extra))                                               
};

#endif

cyg_uint32 			global_i2c_mode;

cyg_drv_mutex_t     i2c_lock; 
cyg_drv_cond_t      i2c_wait;
cyg_handle_t        i2c_interrupt_handle;
cyg_interrupt 		i2c_interrupt_data;

cyg_uint32			byte0;
cyg_uint32			byte1;

// ********************************************************************* //
// Peak Detection Data Retrieval                                         //
// ********************************************************************* //
// User-end function to begin I2C transaction w/ peak detection circuit
// Writes device address (MCP3021) to data register
// The rest of the I2C functionality happens in the Interrupt Service Routine (ISR)

cyg_uint32
get_peak_data(struct xray *xrayData_ptr)
{
	resetLow();
	
	cyg_uint32 dr;
	
    cyg_mpc5xxx_i2c_extra*  extra   = hasp_mpc5200_i2c_bus0.i2c_extra;
	
    global_i2c_mode     	 		= CYG_MPC5XXX_I2C_XFER_MODE_STARTRX;
	
	//*(extra->i2c_control_reg) = MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN  | MPC5XXX_I2C_CR_STA  | MPC5XXX_I2C_CR_TX;  // Set to master mode to send start signal
    
	HAL_WRITE_UINT32(I2C_0_CONTROL_REG, I2C_BEGIN_TX);
	HAL_WRITE_UINT32(I2C_0_DATA_REG, 0x9D000000);
	
	cyg_uint32 status;
	HAL_READ_UINT32(I2C_0_STATUS_REG, status);
	status &= 0x02000000;
	
	while(status == 0x00000000)
	{
		HAL_READ_UINT32(I2C_0_STATUS_REG, status);
		status &= 0x02000000;
	};
	
	HAL_WRITE_UINT32(I2C_0_STATUS_REG, 0x00000000);
	
	resetHigh();

	HAL_WRITE_UINT32(I2C_0_CONTROL_REG, MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN | MPC5XXX_I2C_CR_STA);
	HAL_READ_UINT32(I2C_0_DATA_REG,dr); // Read the dummy byte to initiate next transfer
	
	HAL_READ_UINT32(I2C_0_STATUS_REG, status);
	status &= 0x02000000;
	while(status == 0x00000000)
	{
		HAL_READ_UINT32(I2C_0_STATUS_REG, status);
		status &= 0x02000000;
	};
	
	resetLow();
	
	HAL_WRITE_UINT32(I2C_0_STATUS_REG, 0x00000000);
	HAL_WRITE_UINT32(I2C_0_CONTROL_REG, MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN | MPC5XXX_I2C_CR_STA | MPC5XXX_I2C_CR_TXAK);
	
	HAL_READ_UINT32(I2C_0_DATA_REG, byte0);
	byte0 >>= 16;
	
	HAL_READ_UINT32(I2C_0_STATUS_REG, status);
	status &= 0x02000000;
	while(status == 0x00000000)
	{
		HAL_READ_UINT32(I2C_0_STATUS_REG, status);
		status &= 0x02000000;
	};
	
	resetHigh();
	
	HAL_WRITE_UINT32(I2C_0_STATUS_REG, 0x00000000);
	HAL_WRITE_UINT32(I2C_0_CONTROL_REG, MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN);
	
	HAL_READ_UINT32(I2C_0_DATA_REG, byte1);
	byte1>>=24;
	
    // Any previous nack is no longer relevant. If the device cannot accept
    // more data it will nack the address.
	
    extra->i2c_got_nack = 0;					// What does this nack do now?

   // *(extra->i2c_data_reg) = 0x9D000000; 		// Write address to data reg: 0x9D000000 is our address for MCP3021

	//// DO IT //// Figure out what is happening here.
    //cyg_drv_mutex_lock(&(i2c_lock));
    //cyg_drv_dsr_lock();
        
    //while (extra->i2c_completed == 0)
    //{
    //	cyg_drv_cond_wait(&(i2c_wait));
    //}
        
    //cyg_drv_dsr_unlock();
    //cyg_drv_mutex_unlock(&(i2c_lock));
	//// END DO IT ////
	
	xrayData_ptr->peak = extra->i2c_peak_data;		// store data in the xray logger pointer

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
    cyg_mpc5xxx_i2c_extra*  extra   = hasp_mpc5200_i2c_bus0.i2c_extra;
    cyg_uint32*				base    = extra->i2c_base;									

    extra->i2c_address_reg 	= base + MPC5XXX_I2C_ADDR_OFF;        // Lots of incompatible poitner types (L82-86) warnings
    extra->i2c_control_reg 	= base + MPC5XXX_I2C_CR_OFF;
    extra->i2c_status_reg	= base + MPC5XXX_I2C_SR_OFF;
    extra->i2c_data_reg		= base + MPC5XXX_I2C_DR_OFF;
    extra->i2c_fdr_reg		= base + MPC5XXX_I2C_FDR_OFF;
    extra->i2c_intctrl_reg	= (cyg_uint32*)(MPC5XXX_I2C + MPC5XXX_I2C_INT_OFF);	// Pointer to integer
    extra->i2c_flt_reg		= (cyg_uint32*)(MPC5XXX_I2C + MPC5XXX_I2C_FLT_OFF);
    
    
	cyg_uint32 intr_vector = CYGNUM_HAL_INTERRUPT_I2C1;
	
    cyg_drv_mutex_init(&i2c_lock);
    cyg_drv_cond_init(&i2c_wait, &i2c_lock);
    cyg_drv_interrupt_create(intr_vector,				//
                             1,		 		
                             (cyg_addrword_t) extra,
                             &mpc5xxx_i2c_isr,
                             &mpc5xxx_i2c_dsr,
                             &(i2c_interrupt_handle),
                             &(i2c_interrupt_data));
    cyg_drv_interrupt_attach(i2c_interrupt_handle);

	// *(extra->i2c_fdr_reg) 		= 0x33000000;					// Set clock to 400MHz / 1024
	HAL_WRITE_UINT32(I2C_0_FDR_REG, 0x89000000);			// This is killing the program
    
	//*(extra->i2c_address_reg) 	= 0x00000000; 					// Set MPC5xxx slave address, not useful to us
	HAL_WRITE_UINT32(I2C_0_ADDRESS_REG, 0x00000000);
	
	//*(extra->i2c_control_reg) 	= MPC5XXX_I2C_CR_EN; 			// Enable the I2C device but do not start any transfers and leave interrupts disabled.
	HAL_WRITE_UINT32(I2C_0_DATA_REG, I2C_ENABLE);
	
	//*(extra->i2c_status_reg) 	= 0x00000000;					// Clear any pending conditions including interrupts.
	HAL_WRITE_UINT32(I2C_0_STATUS_REG, 0x00000000);
	
	HAL_WRITE_UINT32(I2C_0_INTR_REG, I2C_CPU_INTR_EN);
	
    HAL_INTERRUPT_UNMASK(intr_vector);					// Interrupts can now be safely unmasked
	
	initReset();												// initialise the reset pin

}

// ********************************************************************* //
// I2C Interrupt Service Routine                                         //
// ********************************************************************* //

cyg_uint32
mpc5xxx_i2c_isr(cyg_vector_t vec, cyg_addrword_t data)
{
	resetHigh();
    cyg_mpc5xxx_i2c_extra*  extra   = (cyg_mpc5xxx_i2c_extra*)data;
    volatile cyg_uint32     dr;
    
    // *(extra->i2c_status_reg) = 0x00000000;						// clear the IF bit
	HAL_WRITE_UINT32(I2C_0_STATUS_REG, 0x00000000);
	
    // What to do next depends on the current transfer mode.
	if (CYG_MPC5XXX_I2C_XFER_MODE_STARTRX == global_i2c_mode) 	// Sent the address and now read the dummy byte
    {
		
		*(extra->i2c_control_reg) = MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN | MPC5XXX_I2C_CR_STA;		// Change from transmit mode to receive mode
		// HAL_WRITE_UINT32(0xF0003D08, MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN | MPC5XXX_I2C_CR_STA);
		
        global_i2c_mode = CYG_MPC5XXX_I2C_XFER_MODE_RX1;		// Change transfer mode to receive first byte
          
        // dr = *(extra->i2c_data_reg);							// Read the dummy byte to initiate next transfer
		HAL_READ_UINT32(0xF0003D10,dr);
		
		// resetHigh();			// Set reset pin to High that drains the capacitor to 0V

    } 
	
    else if (CYG_MPC5XXX_I2C_XFER_MODE_RX1 == global_i2c_mode) 	// Transmit mode to read first data byte
    {

        // *(extra->i2c_control_reg) =  MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN | MPC5XXX_I2C_CR_STA | MPC5XXX_I2C_CR_TXAK;	// Send a NACK after we have received the next byte, still in master mode
		// HAL_WRITE_UINT32(0xF0003D08, MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN | MPC5XXX_I2C_CR_STA | MPC5XXX_I2C_CR_TXAK);
		
        // extra->i2c_peak_data = ((*(extra->i2c_data_reg) & 0xFF000000)>>16); 			// Read the high data byte and shift it to the proper location
		HAL_READ_UINT32(0xF0003D10,extra->i2c_peak_data);
		
		global_i2c_mode = CYG_MPC5XXX_I2C_XFER_MODE_RX2;		// Change transfer mode to receive second byte
            
    } 
	
	else if(CYG_MPC5XXX_I2C_XFER_MODE_RX2 == extra->i2c_mode) 	// Transmit mode to receive last byte
    {

		// *(extra->i2c_control_reg) = MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN;								// change to slave mode to generate stop signal
        // HAL_WRITE_UINT32(0xF0003D08, MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN);
        // extra->i2c_peak_data += ((*(extra->i2c_data_reg) & 0xFF000000)>>24); 		// Read the low data byte and shift it to the proper location
		// HAL_READ_UINT32(0xF0003D10,extra->i2c_peak_data);
		extra->i2c_peak_data >>= 2; 							// Shift resultant data to clear hanging zero bits
		
		global_i2c_mode = CYG_MPC5XXX_I2C_XFER_MODE_DEFAULT;	// Set to default mode to avoud spurious behaviour
		
		extra->i2c_completed    = 1;							// Set completed flag to 1
		
		// resetLow();			// Set reset pin to Low	that allows the capacitor to follow the input voltage
		
    } 
	
	else if(CYG_MPC5XXX_I2C_XFER_MODE_DEFAULT == extra->i2c_mode)										// Default case, just in case modes get corrupted
	{
	
		global_i2c_mode = CYG_MPC5XXX_I2C_XFER_MODE_STARTRX;	// Change mode to STARTRX in case the ISR finds itself in this mode
		*(extra->i2c_control_reg) = MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN;								// change to slave mode to generate stop signal
	
	}
	
    else				// Same as MODE_DEFAULT above
    {
		// Invalid state? Some kind of spurious interrupt? Just ignore it.
        // CYG_FAIL("I2C spurious interrupt");
		
		global_i2c_mode				= CYG_MPC5XXX_I2C_XFER_MODE_STARTRX;
		*(extra->i2c_control_reg) 	= MPC5XXX_I2C_CR_EN | MPC5XXX_I2C_CR_IEN;								// change to slave mode to generate stop signal
		
    }

    // NOTE: this will acknowledge the interrupt even in polled mode.
    // Probably harmless. Using I2C_ISRVEC rather than the vec arg
    // means a constant number for the singleton case, which may
    // allow the HAL to optimize the acknowledge away completely.
	
    HAL_INTERRUPT_ACKNOWLEDGE(extra->i2c_isrvec);
	
	// result = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
	resetLow();
    return CYG_ISR_HANDLED;
}

// ********************************************************************* //
// I2C Delayed Service Routine                                                             //
// ********************************************************************* //

void
mpc5xxx_i2c_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
    cyg_mpc5xxx_i2c_extra*  extra   = (cyg_mpc5xxx_i2c_extra*)data;
    extra->i2c_completed    = 1;
    cyg_drv_cond_signal(&(extra->i2c_wait));
}

//---------------------------------------------------------------------------
// EOF i2c_mpc5xxx.c


