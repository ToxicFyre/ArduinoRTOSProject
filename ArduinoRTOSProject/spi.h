/*
 * spi.h
 */ 
#ifndef SPI_H_
#define SPI_H_
#include "sam.h"
#define SLAVE_CAN_0 0
void	spiInit(void);
uint8_t spiSend (uint8_t data);
uint8_t spiSS(uint8_t device);
uint8_t spiSR(uint8_t device);
#endif /* SPI_H_ */