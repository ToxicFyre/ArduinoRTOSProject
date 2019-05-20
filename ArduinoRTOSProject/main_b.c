

/* Kernel includes. */
#include "sam.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart.h"
#include "myprintf.h"

/* Priorities at which the tasks are created. */
#define	myTASK_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

xQueueHandle Global_Queue_Handle = 0;



void sender_task(void *p) {
	while(1) {
		static uint32_t i = 0;
		myprintf("\nSend %d to receiver task\n", i);
		if (!xQueueSend(Global_Queue_Handle, &i, 1000))
		{
			myprintf("Failed to send to queue\n");
		}
		i++;
		vTaskDelay(2000);
	}
}

void receiver_task(void *p) {
	while(1) {
		static uint32_t rx_int = 0;
		if(xQueueReceive(Global_Queue_Handle, &rx_int, 1000))
		{
			myprintf("Received %d\n", rx_int++);
		}
		else
		{
			myprintf("Failed to receive data from queue");	
		}
	}
}



int main()
{
	SystemInit();
	SYSCTRL->OSC8M.bit.PRESC = 0;
	initUART();

	Global_Queue_Handle = xQueueCreate(3, sizeof(uint32_t));
	xTaskCreate(sender_task,"tx_task", 512, NULL, 1, NULL);
	xTaskCreate(receiver_task,"rx_task", 512, NULL, 1, NULL);

	vTaskStartScheduler();
	return(0);
}*/
