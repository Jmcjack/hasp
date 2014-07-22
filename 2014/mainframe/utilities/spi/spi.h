#ifndef _SPI_H_
#define _SPI_H_

#include <stdint.h>

typedef struct {
  int fd;
} spi;

inline int file_valid(const char* path);
int spi_init(const char *device, uint8_t mode, uint8_t bpw, uint32_t speed);
int spi_transfer(spi *spi_ptr, uint8_t *tx, uint8_t *rx, uint32_t len);
int spi_free (spi *spi_ptr);



#endif
