
 /*
 * RTCControl.h
 *
*/
#ifndef RTCCONTROL_H_               //this goes at the top of the document
#define RTCCONTROL_H_


#include "sam.h"
#include "stdint.h"
#include <stdarg.h>


#define SECOND_ADDR 0x00
#define MINUTE_ADDR 0x01
#define HOUR_ADDR 0x02
#define DAY_ADDR 0x03
#define DATE_ADDR 0x04
#define MONTH_ADDR 0x05
#define YEAR_ADDR 0x06


typedef union{ //seconds
	struct{
		uint8_t un_seconds  :4,
		dec_seconds :3,
		RESERVED          :1;
	}field;
	uint8_t reg;
}seconds_type;


typedef union{ //minutes
	struct{
		uint8_t un_minutes  :4,
		dec_minutes :3,
		RESERVED    :1;
	}field;
	uint8_t reg;
}minutes_type;


typedef union{ //hours
	struct{
		uint8_t un_hours  :4,
		dec_hours :1,
		RESERVED  :3;
	}field;
	uint8_t reg;
}hours_type;




typedef union { //date
	struct  {
		uint8_t  un_date :4,
		dec_date :2,
		RESERVED :2;
	} field;
	uint8_t  reg;
} date_type;

typedef union { //month
	struct  {
		uint8_t un_month :4,
		dec_month :1,
		RESERVED :3;
	} field;
	uint8_t reg;
} month_type;

typedef union{ //year
	struct{
		uint8_t un_year  :4,
		dec_year :4;
	}field;
	uint8_t reg;
} year_type;


typedef struct { //rtc data
	seconds_type   Seconds;
	minutes_type   Minutes;
	hours_type 	Hours;
	//day_type  Day;
	date_type   Date;
	month_type  Month;
	year_type   Year;

} rtc_type;

void sendI2CDataArray(uint8_t SlAddr, uint8_t *ptrData, uint32_t Size);
void SendI2CData(uint8_t SlAddr, uint8_t Data);
void StopCond();
void receiveI2CDataArray(uint8_t SlAddr, uint8_t *ptrData, uint32_t Size);
void initI2C(void);


#endif	/*RTCCONTROL_H_*/          //this goes at the bottom of the document