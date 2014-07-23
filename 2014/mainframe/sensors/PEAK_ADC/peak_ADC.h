#ifndef _PEAK_ADC_H_
#define _PEAK_ADC_H_
#endif

// PEAK ADC SPI Info
static const char *PADC_device = "/dev/spidev1.0";
static uint8_t PADC_mode = 0;
static uint8_t PADC_bpw = 8;
static uint32_t PADC_speed = 3200000;
static uint16_t PADC_delay=100;

#define readCH0 0xD0	// Which channel
#define readCH1 0xF0	// Is which?

int read_event();

