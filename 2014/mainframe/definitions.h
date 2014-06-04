

/// Error enumerations
enum errdefs 
{
	DATA_VAL,		///< Valid data
	DATA_INV,		///< Invalid data
	COMM_ERR		///< Communication protocol error
	// Add more
};

struct imu
{
	// Data Containers
	float ax;
	float ay;
	float az;
	
	float hx;
	float hy;
	float hz;
	
	float p;
	float q;
	float r;
	
	float Temp;
	float Volt;
	
	// Pointer to SPI Port structure
	struct spi *spi_ptr;
	// IMU Error
	error_type;
};

struct spi
{
	const char *device;
	uint32_t speed;
	uint8_t mode;
	uint8_t bits;
	uint16_t delay;
	int handle;
	unsigned long *rx_data;
};