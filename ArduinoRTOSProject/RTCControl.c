#include "RTCControl.h"
#include "sam.h"

volatile uint32_t i;

void sendI2CDataArray(uint8_t SlAddr, uint8_t *ptrData, uint32_t Size){
	SERCOM3->I2CM.CTRLB.bit.CMD = 0x3;
	
	/* Acknowledge section is set as ACK signal by writing 0 in ACKACT bit */
	SERCOM3->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	while(SERCOM3->I2CM.SYNCBUSY.reg); // waiting load
	
	/* slave address with write signal (0) */
	SERCOM3->I2CM.ADDR.reg = (SlAddr << 1) | 0;    /* Sending slave address in write mode */
	while(SERCOM3->I2CM.INTFLAG.bit.MB == 0);		   /* MB = 1 if slave NACKS the address */
	
	SERCOM3->I2CM.DATA.reg = 0x0;
	while(SERCOM3->I2CM.INTFLAG.bit.MB == 0){}; /* MB = 1 if slave NACKS the address */

	for(i = 0; i < Size; i ++) {
		/* placing the data from transmitting buffer to DATA register*/
		SERCOM3->I2CM.DATA.reg = ptrData[i];
		while(SERCOM3->I2CM.INTFLAG.bit.MB == 0){}; /* MB = 1 if slave NACKS the address */
	}

	/* After transferring the last byte stop condition will be sent */
	SERCOM3->I2CM.CTRLB.bit.CMD = 0x3;
}

void SendI2CData(uint8_t SlAddr, uint8_t Data){
	SERCOM3->I2CM.CTRLB.bit.CMD = 0x3;
	
	/* Acknowledge section is set as ACK signal by writing 0 in ACKACT bit */
	SERCOM3->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	while(SERCOM3->I2CM.SYNCBUSY.reg); // waiting load
	
	/* slave address with write signal (0) */
	SERCOM3->I2CM.ADDR.reg = (SlAddr << 1) | 0;    /* Sending slave address in write mode */
	while(SERCOM3->I2CM.INTFLAG.bit.MB == 0);		   /* MB = 1 if slave NACKS the address */
	
	SERCOM3->I2CM.DATA.reg = 0x0;
	while(SERCOM3->I2CM.INTFLAG.bit.MB == 0){}; /* MB = 1 if slave NACKS the address */

	/* placing the data from transmitting buffer to DATA register*/
	SERCOM3->I2CM.DATA.reg = Data;
	while(SERCOM3->I2CM.INTFLAG.bit.MB == 0){}; /* MB = 1 if slave NACKS the address */
	
	/* After transferring the last byte stop condition will be sent */
	SERCOM3->I2CM.CTRLB.bit.CMD = 0x3;
}

void StopCond(){
	// Turn off the I2C
	SERCOM3->I2CM.CTRLA.bit.ENABLE = 0x0u;
}

void receiveI2CDataArray(uint8_t SlAddr, uint8_t *ptrData, uint32_t Size){
	SERCOM3->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	while(SERCOM3->I2CM.SYNCBUSY.reg); // waiting load
	/* slave address with write signal (0) */
	SERCOM3->I2CM.ADDR.reg = (SlAddr << 1) | 0;    /* Sending slave address in write mode */
	while(SERCOM3->I2CM.INTFLAG.bit.MB ==0);		   /* MB = 1 if slave NACKS the address */
	//	SERCOM3->I2CM.DATA.reg = ;                 /* Sending address (seconds) for internal pointer */
	SERCOM3->I2CM.DATA.reg = 0x00;                     /* Sending address (seconds) for internal pointer */
	while(SERCOM3->I2CM.INTFLAG.bit.MB ==0){};         /* MB = 1 if slave NACKS the address */
	SERCOM3->I2CM.CTRLB.bit.CMD = 0x3;                 /* Sending stop condition */
	
	
	/******************  RECIEVING SEQUENCE  *************************/

	/* slave address with read signal (1) */
	SERCOM3->I2CM.ADDR.reg = (SlAddr << 1) | 1;
	while(SERCOM3->I2CM.INTFLAG.bit.SB==0){};

	/* Acknowledge section is set as ACK signal by writing 0 in ACKACT bit */
	SERCOM3->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;

	for(i=0; i< Size; i++) {
		ptrData[i] = SERCOM3->I2CM.DATA.reg;
		while(SERCOM3->I2CM.INTFLAG.bit.SB==0){};
	}
	/* NACK should be send before reading the last byte*/
	SERCOM3->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
	SERCOM3->I2CM.CTRLB.bit.CMD = 0x3;
	ptrData[Size-1] = SERCOM3->I2CM.DATA.reg;
}

void initI2C(void)
{	
	/* port mux configuration */
	PORT->Group[0].PINCFG[PIN_PA22].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN | PORT_PINCFG_PULLEN; /* SDA */
	PORT->Group[0].PINCFG[PIN_PA23].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN | PORT_PINCFG_PULLEN; /* SCL */
	
	/* PMUX: even = n/2, odd: (n-1)/2 */
	PORT->Group[0].PMUX[11].reg |= 0x02u;
	PORT->Group[0].PMUX[11].reg |= 0x20u;
	
	/* APBCMASK */
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM3;

	/*GCLK configuration for sercom3 module*/
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID (SERCOM3_GCLK_ID_CORE) |
	GCLK_CLKCTRL_ID (SERCOM3_GCLK_ID_SLOW) |
	GCLK_CLKCTRL_GEN(4) |
	GCLK_CLKCTRL_CLKEN;
	GCLK->GENCTRL.reg |= GCLK_GENCTRL_SRC_OSC8M|GCLK_GENCTRL_GENEN|GCLK_GENCTRL_ID(4);

	/* set configuration for SERCOM3 I2C module */
	SERCOM3->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN; /* smart mode enable */
	while (SERCOM3->I2CM.SYNCBUSY.reg); // waiting loading
	
	/* calculate BAUDRATE */
	uint64_t tmp_baud =((8000000/100000)-10-(8000000*250 /1000000000))/2;
	SERCOM3->I2CM.BAUD.bit.BAUD = SERCOM_I2CM_BAUD_BAUD((uint32_t)tmp_baud);
	while (SERCOM3->I2CM.SYNCBUSY.reg); // waiting loading
	// value equals 0x22 or decimal 34
	
	SERCOM3->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_ENABLE   |/* enable module */
	SERCOM_I2CM_CTRLA_MODE_I2C_MASTER |		/* i2c master mode */
	SERCOM_I2CM_CTRLA_SDAHOLD(3);		 /* SDA hold time to 600ns */
	while (SERCOM3->I2CM.SYNCBUSY.reg);  /* waiting loading */

	SERCOM3->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1); /* set to idle state */
	while (SERCOM3->I2CM.SYNCBUSY.reg);  /* waiting loading */
}