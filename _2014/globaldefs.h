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

#include <stdint.h>
#include <stdlib.h>
#include "spi.h"

typedef unsigned char   byte;           ///< typedef of byte
typedef unsigned short  word;           ///< typedef of word

enum errdefs
{
        got_invalid,            	///< No data received
        checksum_err,           	///< Checksum mismatch
        gps_nolock,                     ///< No GPS lock
        data_valid,                     ///< Data valid
        noPacketHeader,         	///< Some data received, but cannot find packet header
        incompletePacket,		///< Packet header found, but complete packet not received
};

enum PosVelType
{
	NONE, 				///< No solution
	FIXEDPOS, 			///< Position has been fixed by the FIX POSITION command
	FIXEDHEIGHT, 			///< Position has been fixed by the FIX HEIGHT/AUTO command
	DOPPLER_VELOCITY, 		///< Velocity computed using instantaneous Doppler
	SINGLE, 			///< Single point precision
	PSRDIFF, 			///< Psuedorange differential solution
	WAAS, 				///< Solution calculated using corrections from an SBAS
	PROPAGATED, 			///< Propagated by a Kalman filter without new observations
};

enum SolutionStatus
{
	SOL_COMPUTED,			///< Solution computed
	INSUFFICIENT_OBS,		///< Insufficient observations
	NO_CONVERGENCE,			///< No convergence
	SINGULARTY, 			///< Singularity at parameters matrix
	COV_TRACE, 			///< Covariance trace exceeds maximum (trace > 1000 m)
	TEST_DIST, 			///< Test distance exceeded (maximum of 3 rejections if distance > 10 km)
	COLD_START, 			///< Not yet converged from cold start
	V_H_LIMIT, 			///< Height or velocity limits exceeded (in accordance with export licensing restrictions)
	VARIANCE, 			///< Variance exceeds limits
	RESIDUALS, 			///< Residuals are too large
	DELTA_POS, 			///< Delta position is too large
	NEGATIVE_VAR, 			///< Negative variance
	INTEGRITY_WARNING,		///< Large residuals make position unreliable
	PENDING,			///< Not enough satellites being tracked to verify the FIX POSITION command
	INVALID_FIX, 			///< The fixed position, entered using the FIX POSITION command, is not valid
	ANTENNA_WARNING,		///< Antenna warnings
};

struct imu
{
        //unsigned long time;
	long stime;
	long mtime;
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
	int *response;
        int spi_handle;
	int badDataCounter;
	unsigned long rstTime;
};

struct xray
{
        //unsigned long time;
        int countsA;
        int countsB;
	int countsAB;
	//int spi_handle;
};

struct gps
{

	uint16_t weekRef;			///< GPS week reference number
        long time;	                  	///< [sec], timestamp of GPS data
	
	//enum SolutionStatus P-solStatus;		///< Solution status
	//enum PosVelType posType;			///< Position Type
	uint8_t timeStatus;					///< Time status
	double Xe;					///< [m], X position, ECEF
	double Ye;					///< [m], Y position, ECEF
	double Ze;					///< [m], Z position, ECEF

	float  Px;  					///< [m], X std dev
	float  Py;  					///< [m], Y std dev
	float  Pz;  					///< [m], Z std dev

	//enum SolutionStatus V-solStatus;		///< Solution status
	//enum PosvelType velType;			///< Velocity type
	double Ue;					///< [m/sec], X velocity, ECEF
	double Ve;					///< [m/sec], Y velocity, ECEF
	double We;					///< [m/sec], Z velocity, ECEF
	float  Pu;  					///< [m], U std dev
	float  Pv;  					///< [m], V std dev
	float  Pw;  					///< [m], W std dev

	char stnID[4];					///< Base Station ID
	float V_latency;				///< [s], latency in the velocity time tag; subtract from time for improved results
	float diff_age;					///< [s], differential age
	float sol_age;					///< [s], solution age

	uint8_t SV;					///< # of satellite vehicles tracked
	uint8_t solnSV;					///< # of satellite vehicles used in solution
	uint8_t ggL1;					///< # of GPS plus GLONASS L1 used in solution

	//uint8_t newData;				///< [bool], flag set when GPS data has been updated
	//uint8_t satVisible; 				///< Number satellites used in the position solution
	//uint8_t navValid;				///< flag indicating whether the solution is valid, 0 = valid
	//uint8_t GPS_week;				///< GPS week since current epoch.
		
	//enum errdefs err_type;			///< GPS status
	int port;					///< handle for accessing serial port
	unsigned char responseBuffer[144];		///< serial response buffer
    	int readState;					///< current state of serial data reader function
    	int read_calls;					///< number of times the read_gps function has been called
	int badDataCounter;				///< number of bad reads from the read_gps function (resets after 5)
	int posValFlag;
	unsigned long lastPosVal;
};

struct sensordata
{
        struct imu *imuData_ptr;                ///< pointer to imu data structure
        struct xray *xrayData_ptr;		///< pointer to xray data structure
        struct gps *gpsData_ptr;                ///< pointer to gps data structure
};

#endif

