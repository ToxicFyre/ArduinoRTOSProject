

/* Kernel includes. */
#include "sam.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "uart.h"
#include "myprintf.h"
#include "stdbool.h"
#include "RTCControl.h"

#define SLAVE_ADDR 0x68u
#define BUF_SIZE 4
#define WaitingTime 600000

/* Priorities at which the tasks are created. */
#define	myTASK_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

xQueueHandle Global_Queue_Handle = 0;
xQueueHandle Door_Queue_Handle = 1;
xSemaphoreHandle rx_binary_sem;

void CheckRx(void)
{
	while (1)
	{
		do 
		{
			myprintf("X");
			vTaskDelay(10);
		} while(!(SERCOM0->USART.INTFLAG.bit.RXC));
		xSemaphoreGive(rx_binary_sem);
	}
}

void PingRasp(void *p) {

	while (1) {
		uint8_t rx_data;
		if(xSemaphoreTake(rx_binary_sem, ( TickType_t ) 3000 ) == pdTRUE)
		{
			rx_data = SERCOM0->USART.DATA.reg;
			xQueueSend(Global_Queue_Handle, &rx_data, 2000);
		}
		else
		{
			//Send connection error here
			myprintf("UART Connection Error");
		}
	}
}

void PassData(void *p) {
	while(1) {
		uint8_t *pointer;
		rtc_type rtc;
		pointer = (uint8_t*) &rtc;
		static uint8_t rx_data = 0;
		if(xQueueReceive(Global_Queue_Handle, &rx_data, 1500))
		{
			if (rx_data - '0' == 4)
			{
				myprintf("Internet Connection Error");
			} 
			else
			{
				if (rx_data - '0' == 1) //Opening door requires more info
				{
					xQueueSend(Door_Queue_Handle, &rx_data, 2000);
				} 
				else //Closing door requires no auth
				{
					xQueueSend(Door_Queue_Handle, &rx_data, 2000);
				}
				receiveI2CDataArray((uint8_t)SLAVE_ADDR, pointer, (uint8_t)8);
			}
		}
	}
}

void HandleDoor(void *p)
{
	while (1)
	{
		static uint8_t rx_data = 0;
		if (xQueueReceive(Door_Queue_Handle, &rx_data, 1500))
		{

			if (rx_data - '0' == 0)
			{
				PORT->Group[ 0 ].OUTCLR.reg = PORT_PA19;
				myprintf("close door");
			} 
			else
			{
				PORT->Group[ 0 ].OUTSET.reg = PORT_PA19;
				myprintf("open door");
			}
		}
	}
}

int main()
{
	SystemInit();
	/* Switch to 8MHz clock (disable prescaler) */
	SYSCTRL->OSC8M.bit.PRESC = 0;
	initUART();
	initI2C();

	PORT->Group[ 0 ].PINCFG[ PIN_PA19 ].reg = 0x0; //bit PMUXEN must be clear for GPIOs
	
	PORT->Group[ 0 ].DIRSET.reg = PORT_PA19; //pin 16 declared as data output
	
	uint8_t *pointer;
	rtc_type rtc;
	pointer = (uint8_t*) &rtc;
	pointer[0] = 0x10;	//set seconds
	pointer[1] = 0x34;	//set minutes
	pointer[2] = 0x18;	//set hours
	pointer[4] = 0x22;	//set date
	pointer[5] = 0x04;	//set month
	pointer[6] = 0x19;	//set year

	sendI2CDataArray((uint8_t)SLAVE_ADDR, pointer, (uint8_t)7);
	
	vSemaphoreCreateBinary(rx_binary_sem);

	Door_Queue_Handle = xQueueCreate(1, sizeof(uint8_t));
	Global_Queue_Handle = xQueueCreate(1, sizeof(uint8_t));
	xTaskCreate(CheckRx,"CheckRx", 512,NULL,3,NULL);
	xTaskCreate(PingRasp,"PingRasp", 512, NULL, 2, NULL);
	xTaskCreate(PassData,"PassData", 512, NULL, 2, NULL);
	xTaskCreate(HandleDoor,"HandleDoor", 512, NULL,1,NULL);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();
	return(0);
}
