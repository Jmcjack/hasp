#!/bin/bash
gcc -lm -o MAINLOOP HASP_SPI_devices.c datalogger.c downlink.c gps_novatel.c simple_gpio.c spi.c timing.c mainframe.c updateEventCounter.c
