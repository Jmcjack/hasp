#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>

#include "/home/root/HASP2014/globaldefs.h"
#include "/home/root/HASP2014/simple_gpio.h"
#include "gps_novatel.h"
// #include "/home/root/HASP2014/utils/misc.h"

#define CRC32_POLYNOMIAL 0xEDB88320L
#define PV_PIN 44

static unsigned char localBuffer[285];
static unsigned char varBuffer[37][20];
static unsigned char header_length = 28;

void init_GPS(struct gps *gpsData_ptr)
{

        //uart5 configuration using termios
        struct termios uart5;
        int fd = (int)malloc(sizeof(int));;
        //open uart5 for tx/rx, not controlling device
        if((fd = open("/dev/ttyO5", O_RDWR | O_NOCTTY)) < 0) {
                fprintf(stderr,"Unable to open UART5 access.\n");}
        //get attributes of uart5
        if(tcgetattr(fd, &uart5) < 0) {
                fprintf(stderr,"Could not get attributes of UART5 at ttyO1\n");}
        //set Baud rate
        if(cfsetospeed(&uart5, B115200) < 0) {
                fprintf(stderr,"Could not set baud rate\n");}
        else {
              	fprintf(stderr,"\n\nBaud rate: 115200\n");}
        //set attributes of uart5
        uart5.c_iflag = 0;
        uart5.c_oflag = 0;
        uart5.c_lflag = 0;

        tcsetattr(fd, TCSANOW, &uart5);
        gpsData_ptr->port = fd;
        gpsData_ptr->navValid = 0;
}


int read_GPS(struct gps *gpsData_ptr)
{

        //read UART5 port
        //B115200
        //8 bits, no parity, 1 stop bit
        //no flow control or handshaking
        int PV = (int)malloc(sizeof(int));
        int fd = (int)malloc(sizeof(int));

        PV = gpsData_ptr->navValid;
        fd = gpsData_ptr->port;

        gpio_get_value(PV_PIN,&PV);

        int i;
	unsigned int bytesRead = 0;

        switch(PV) {
                case 1:
                //fprintf(stdout,"\nPOSITION VALID LINE IS HIGH\n");
                for(i=0;i<285;i++) {

                        bytesRead += read(fd, &localBuffer[bytesRead], 1);
                        fprintf(stderr,"%c",localBuffer[i]);

                }

                break1:
                bytesRead=0;
                break;

                case 0:
                fprintf(stdout,"PNV-");
                bytesRead = 0;
                usleep(1000000);
                break;
        }

return 0;
}


// --------------------------------------------------------------------------
//Calculate a CRC value to be used by CRC calculation functions.
//--------------------------------------------------------------------------
unsigned int CRC32Value(int i)
{
        int j;
	unsigned int ulCRC;
        ulCRC = i;
        for ( j = 8 ; j > 0; j-- )
        {
                if ( ulCRC & 1 )
                        ulCRC = ( ulCRC >> 1 ) ^ CRC32_POLYNOMIAL;
                else
                    	ulCRC >>= 1;
        }
        return ulCRC;
}

//--------------------------------------------------------------------------
//Calculates the CRC-32 of a block of data all at once
//CRC_computed = CalculateBlockCRC32(140,localBuffer+counter-2);
//--------------------------------------------------------------------------
unsigned int CalculateBlockCRC32(unsigned int ulCount, unsigned char *ucBuffer)
{
        unsigned int ulTemp1;
        unsigned int ulTemp2;
        unsigned int ulCRC = 0;

        while ( ulCount-- != 0 )
        {
                ulTemp1 = ( ulCRC >> 8 ) & 0x00FFFFFFL;
                ulTemp2 = CRC32Value( ((int) ulCRC ^ *ucBuffer++ ) & 0xff );
                ulCRC = ulTemp1 ^ ulTemp2;
        }
        return( ulCRC );
}

