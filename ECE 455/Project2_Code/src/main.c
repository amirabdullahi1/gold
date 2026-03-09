/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4_discovery.h"
/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"
/*-----------------------------------------------------------*/


/*---- Pragmas ----------------------------------------------*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/
#define mainQUEUE_LENGTH 100

#define data  	0


/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */
static void prvSetupHardware( void );

/*-----------------------------------------------------------*/
typedef enum {PERIODIC,APERIODIC} task_type;

typedef struct {
	TaskHandle_t t_handle;
	task_type type;
	uint32_t task_id;
	uint32_t release_time;
	uint32_t absolute_deadline;
	uint32_t completion_time;
	// uint32_t *interrupt_times;
} dd_task;

typedef struct dd_task_list {
	dd_task task;
	struct dd_task_list *next_task;
} dd_task_list;

dd_task_list *active_task_list = NULL;
dd_task_list *completed_task_list = NULL;
dd_task_list *overdue_task_list = NULL;
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
void dd_task_list_add(dd_task_list **task_list, dd_task this_task) {
	dd_task_list *task_list_this = malloc(sizeof(dd_task_list));
	if(task_list_this == NULL)
		return;

	task_list_this->task = this_task;
	task_list_this->next_task = NULL;

	dd_task_list *task_list_prev = NULL;
	dd_task_list *task_list_curr = *task_list;

	dd_task curr_task;
	while(task_list_curr != NULL)
	{
		curr_task = task_list_curr->task;
		if(this_task.absolute_deadline < curr_task.absolute_deadline)
			break;

		task_list_prev = task_list_curr;
		task_list_curr = task_list_curr->next_task;
	}

	task_list_this->next_task = task_list_curr;

	if(task_list_prev == NULL)
		*task_list = task_list_this;
	else
		task_list_prev->next_task = task_list_this;
}

void dd_task_list_rmv(dd_task_list **task_list, dd_task this_task) {
	dd_task_list *task_list_prev = NULL;
	dd_task_list *task_list_curr = *task_list;

	dd_task curr_task;
	while(task_list_curr != NULL)
	{
		curr_task = task_list_curr->task;
		if(this_task.t_handle == curr_task.t_handle)
			break;

		task_list_prev = task_list_curr;
		task_list_curr = task_list_curr->next_task;
	}

	if(task_list_curr == NULL)
		return;
	if(task_list_prev == NULL)
		*task_list = task_list_curr->next_task;
	else
		task_list_prev->next_task = task_list_curr->next_task;

	free(task_list_curr);
}
/*-----------------------------------------------------------*/
/* Create and delete task functions */

void create_dd_task(TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t absolute_deadline)
{
	dd_task *new_task = malloc(sizeof(dd_task));
	new_task->t_handle = t_handle;
	new_task->type = type;
	new_task->task_id = task_id;
	new_task->release_time = (uint32_t)xTaskGetTickCount();
	new_task->absolute_deadline = absolute_deadline;
	new_task->completion_time = 0; // Setting as 0 so that it does not take on some random value until it can be assigned properly upon actual completion.

	dd_task_list_add(&active_task_list, *new_task); // Calls the function to add to the task list.
	free(new_task);
}

// Have this here for reference for now (DELETE WHEN COMPLETED!!!!)
//typedef struct dd_task_list {
//	dd_task task;
//	struct dd_task_list *next_task;
//} dd_task_list;

void delete_dd_task(uint32_t task_id)
{
//	if(){
//		printf("There is nothing to remove the task list is empty.\n");
//		return;
//	}
//
//	// Need to understand where the head for the specific task list is to delete properly...
//	dd_task_list head = active_task_list;
//	if(head.task.task_id == task_id){
//		printf("Task %d is being removed", task_id);
//	}else{
////		prev = head;
//		head = head->next_task;
//
//	}

}

/*-----------------------------------------------------------*/





/*-----------------------------------------------------------*/
static void DD_Task_Generator( void *pvParameters );
static void DD_Task1( void *pvParameters );
static void DD_Task2( void *pvParameters );
static void DD_Task3( void *pvParameters );

xQueueHandle xQueue_handle = 0;
/*-----------------------------------------------------------*/

int main(void)
{

	/* Configure the system ready to run the demo.  The clock configuration
	can be done here if it was not done before main() was called. */
	prvSetupHardware();


	/* Create the queue used by the queue send and queue receive tasks.
	http://www.freertos.org/a00116.html */
	xQueue_handle = xQueueCreate( 	mainQUEUE_LENGTH,		/* The number of items the queue can hold. */
							sizeof( uint16_t ) );	/* The size of each item the queue holds. */

	/* Add to the registry, for the benefit of kernel aware debugging. */
	vQueueAddToRegistry( xQueue_handle, "MainQueue" );

	xTaskCreate( DD_Task_Generator, "DD Task Gen", 256, NULL, 2, NULL);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return 0;
}
/*-----------------------------------------------------------*/


static void DD_Task_Generator(void *pvParameters)
{
	const uint32_t *params = (uint32_t *)pvParameters;
	uint32_t period_ticks;
	if(params != NULL){
		period_ticks = *params;
	}else{
		period_ticks = pdMS_TO_TICKS(500);
	}
	static uint32_t task_id_counter = 0;
	TaskHandle_t new_task_handle1 = NULL;
	TaskHandle_t new_task_handle2 = NULL;
	TaskHandle_t new_task_handle3 = NULL;


// Test bench 1 LCM 1500
//	uint32_t DD_Task1_Period = 500
//	uint32_t DD_Task2_Period = 500
//	uint32_t DD_Task3_Period = 750

	xTaskCreate(DD_Task1, "DD_Task1", configMINIMAL_STACK_SIZE, NULL, 1, &new_task_handle1);
	xTaskCreate(DD_Task2, "DD_Task2", configMINIMAL_STACK_SIZE, NULL, 1, &new_task_handle2);
	xTaskCreate(DD_Task3, "DD_Task3", configMINIMAL_STACK_SIZE, NULL, 1, &new_task_handle3);

	while(1){


		uint32_t release = (uint32_t)xTaskGetTickCount();
		uint32_t deadline = release + period_ticks;

		create_dd_task(new_task_handle1, PERIODIC, task_id_counter++, deadline);





		vTaskDelay(period_ticks);
	}
}

/*-----------------------------------------------------------*/

static void DD_Task1( void *pvParameters )
{
	uint16_t tx_data = 0;


	while(1)
	{

		if(tx_data == 0) {
			//   Do task0.
		}

		/* xQueue_handle: * xQueueSend Posts an item on queue defined by xQueue_handle
		 * rx_data: A pointer to the item that is to be placed on the queue
		 * 1000: The maximum amount of time the task should block waiting for space to become
		 */
		if( xQueueSend(xQueue_handle,&tx_data,1000))
		{
			printf("DD_Task1 ON!\n");
			if(tx_data == 0)
				tx_data = 0;
			vTaskDelay(1000);
		}
		else
		{
			printf("Manager Failed!\n");
		}
	}
}

static void DD_Task2( void *pvParameters )
{
	uint16_t tx_data = 0;


	while(1)
	{

		if(tx_data == 0) {
			//   Do task0.
		}

		/* xQueue_handle: * xQueueSend Posts an item on queue defined by xQueue_handle
		 * rx_data: A pointer to the item that is to be placed on the queue
		 * 1000: The maximum amount of time the task should block waiting for space to become
		 */
		if( xQueueSend(xQueue_handle,&tx_data,1000))
		{
			printf("DD_Task2 ON!\n");
			if(tx_data == 0)
				tx_data = 0;
			vTaskDelay(1000);
		}
		else
		{
			printf("Manager Failed!\n");
		}
	}
}

static void DD_Task3( void *pvParameters )
{
	uint16_t tx_data = 0;


	while(1)
	{

		if(tx_data == 0) {
			//   Do task0.
		}

		/* xQueue_handle: * xQueueSend Posts an item on queue defined by xQueue_handle
		 * rx_data: A pointer to the item that is to be placed on the queue
		 * 1000: The maximum amount of time the task should block waiting for space to become
		 */
		if( xQueueSend(xQueue_handle,&tx_data,1000))
		{
			printf("DD_Task3 ON!\n");
			if(tx_data == 0)
				tx_data = 0;
			vTaskDelay(1000);
		}
		else
		{
			printf("Manager Failed!\n");
		}
	}
}
/*-----------------------------------------------------------*/



void vApplicationMallocFailedHook( void )
{
	/* The malloc failed hook is enabled by setting
	configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

	Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software 
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.  pxCurrentTCB can be
	inspected in the debugger if the task name passed into this function is
	corrupt. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
	FreeRTOSConfig.h.

	This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amount of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Ensure all priority bits are assigned as preemption priority bits.
	http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	NVIC_SetPriorityGrouping( 0 );

	/* TODO: Setup the clocks, etc. here, if they were not configured before
	main() was called. */
}

/*-----------------------------------------------------------*/
#pragma GCC diagnostic pop
