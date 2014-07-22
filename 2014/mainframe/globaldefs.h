#ifndef GLOBALDEFS_H_
#define GLOBALDEFS_H_

#define NSECS_PER_SEC           1000000000                      ///< [nsec/sec] nanoseconds per second */
#define D2R                     0.017453292519943               ///< [rad] degrees to radians */
#define R2D                     57.295779513082323              ///< [deg] radians to degrees */
#define PSI_TO_KPA              6.89475729                      ///< [KPa] PSI to KPa */
#define g                       9.814                           ///< [m/sec^2] gravity */
#define g2                      19.62                           ///< [m/sec^2] 2*g */
#define PI                      3.14159265358979                ///< pi */
#define PI2                     6.28318530717958                ///< pi*2 */
#define half_pi                 1.57079632679490                ///< pi/2 */

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#include "utilities/spi/spi.h"

typedef unsigned char   byte;                                   ///< typedef of byte */
typedef unsigned short  word;                                   ///< typedef of word */

enum errdefs
{
        got_invalid,            ///< No data received
        checksum_err,           ///< Checksum mismatch
        gps_nolock,             ///< No GPS lock
        data_valid,             ///< Data valid
        noPacketHeader,         ///< Some data received, but cannot find packet header
        incompletePacket,	///< Packet header found, but complete packet not received
};

struct imu
{
        int time;
        uint16_t supply_raw;
        int gyroX_raw;
        int gyroY_raw;
        int gyroZ_raw;
        int accelX_raw;
        int accelY_raw;
        int accelZ_raw;
        int magX_raw;
        int magY_raw;
        int magZ_raw;
        int temp_raw;
        uint16_t ADC_raw;
        spi *spi_ptr;
};

struct xray
{
        unsigned long time;
        unsigned int peakA;
        unsigned int peakB;
};

struct gps {
	double Xe;	///< [m], X position, ECEF
        double Ye;	///< [m], Y position, ECEF
        double Ze;	///< [m], Z position, ECEF
        double Px;  ///< [m], X std dev
        double Py;  ///< [m], Y std dev
        double Pz;  ///< [m], Z std dev
        double Ue;	///< [m/sec], X velocity, ECEF
        double Ve;	///< [m/sec], Y velocity, ECEF
        double We;	///< [m/sec], Z velocity, ECEF
        double Pu;  ///< [m], U std dev
        double Pv;  ///< [m], V std dev
        double Pw;  ///< [m], W std dev
        double GPS_TOW; ///< [sec], GPS Time Of Week
        double courseOverGround;///< [rad], course over the ground, relative to true North
        double speedOverGround; ///< [rad], speed over the ground
        double time;    ///< [sec], timestamp of GPS data
        unsigned short newData; ///< [bool], flag set when GPS data has been updated
        unsigned short satVisible; ///< Number satellites used in the position solution
        unsigned short navValid;///< flag indicating whether the solution is valid, 0 = valid
        unsigned short GPS_week;///< GPS week since current epoch.
        enum errdefs err_type;  ///< GPS status
        int baudRate;           ///< Baud rate for serial port
        char* portName;         ///< Name of serial port
        int port;                       ///< handle for accessing serial port
        unsigned char* localBuffer; ///< local buffer to store partial serial data packets
        int bytesInLocalBuffer; ///< number of bytes in the local buffer
        int readState;                  ///< current state of serial data reader function
        int read_calls;                 ///< number of times the read_gps function has been called
};

struct sensordata
{
        struct imu *imuData_ptr;                ///< pointer to imu data structure
        struct xray *xrayData_ptr;
        struct gps *gpsData_ptr;                ///< pointer to gps data structure
};

#endif

