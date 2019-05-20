#include "spi.h"


void spiInit(void) {
	PM->APBCMASK.bit.SERCOM1_ = 1;
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_SERCOM1_CORE;
	while(GCLK->STATUS.bit.SYNCBUSY);
	const SERCOM_SPI_CTRLA_Type ctrla = {
		.bit.DORD = 0, // Set as MSB first
		.bit.CPHA = 0, // Set as Mode 0
		.bit.CPOL = 0,  // Keep this as 0
		.bit.FORM = 0, // Set as SPI frame
		.bit.DIPO = 0x3, // Set as MISO on PAD[3]
		.bit.DOPO = 0x0, // Set as MOSI on PAD[0], SCK on PAD[1], SS_ on PAD[2]
		.bit.MODE = 0x3 // Set as Master
	};
	SERCOM1->SPI.CTRLA.reg = ctrla.reg;
	const SERCOM_SPI_CTRLB_Type ctrlb = {
		.bit.RXEN = 1,   // Set as RX enabled
		.bit.MSSEN = 0, //1 // Set as HW SS
		.bit.CHSIZE = 0x0  // Set as 8-bit
	};
	SERCOM1->SPI.CTRLB.reg = ctrlb.reg;
	
	SERCOM1->SPI.BAUD.reg = 0; // Rate is clock / 2
	// Mux for SERCOM1 PA16,PA17,PA18,PA19
	const PORT_WRCONFIG_Type wrconfig = {
		.bit.WRPINCFG = 1,
		.bit.WRPMUX = 1,
		.bit.PMUX = MUX_PA16C_SERCOM1_PAD0,
		.bit.PMUXEN = 1,
		.bit.HWSEL = 1,
		.bit.PINMASK = (uint16_t)((PORT_PA16 | PORT_PA17 | PORT_PA19) >> 16)
	};
	PORT->Group[0].WRCONFIG.reg = wrconfig.reg;

	REG_PORT_DIRSET0 = PORT_PA18;
	REG_PORT_OUTSET0 = PORT_PA18;

	
	SERCOM1->SPI.CTRLA.bit.ENABLE = 1;
	while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE);
}


uint8_t spiSend (uint8_t data) {
	uint8_t temp;
	while(SERCOM1->SPI.INTFLAG.bit.DRE == 0);
	SERCOM1->SPI.DATA.reg = data;
	while(SERCOM1->SPI.INTFLAG.bit.TXC == 0);
	while(SERCOM1->SPI.INTFLAG.bit.RXC == 0);
	temp = SERCOM1->SPI.DATA.reg;
	return temp;
}

uint8_t spiSS(uint8_t device) {
	REG_PORT_OUTCLR0 = PORT_PA18;
}

uint8_t spiSR(uint8_t device) {
	REG_PORT_OUTSET0 = PORT_PA18;
}