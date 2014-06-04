#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

// Put only function definitions that will be called inside of other code not located in hasp_spi.c
int hasp_spi_init(const char*, uint32_t, uint8_t, uint8_t, uint16_t);
int hasp_spi_close(int);