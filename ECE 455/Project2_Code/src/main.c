/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include "stm32f4_discovery.h"
/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"



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

dd_task_list active_task_list;
dd_task_list completed_task_list;
dd_task_list overdue_task_list;
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
void create_dd_task(TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t absolute_deadline){
	dd_message msg;
    msg.msg_type = DD_MSG_CREATE;

    msg.task.t_handle = t_handle;
    msg.task.task_id = task_id;
    msg.task.type = type;
    msg.task.release_time = (uint32_t)xTaskGetTickCount();
    msg.task.absolute_deadline = absolute_deadline;
    msg.task.completion_time = 0;

    if(xQueueSend(xDDS_Queue, &msg, 1000) != pdPASS)
    	printf("create_dd_task: failed when sending to DDS queue\n");
}

void delete_dd_Task(uint32_t task_id){
	dd_message msg;
    msg.msg_type = DD_MSG_DELETE;
    msg.task.task_id = task_id;
    msg.task.completion_time = (uint32_t)xTaskGetTickCount();

    if(xQueueSend(xDDS_Queue, &msg, 1000) != pdPASS)
    	printf("delete_dd_Task: failed when sending to DDS queue\n");
}



/*-----------------------------------------------------------*/
static void Manager_Task( void *pvParameters );
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

	xTaskCreate( Manager_Task, "Manager", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return 0;
}


/*-----------------------------------------------------------*/

static void Manager_Task( void *pvParameters )
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
			printf("Manager: %u ON!\n", tx_data);
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
static void DD_Task_Generator(void *pvParameters){
	const uint32_t *params = (unint32_t *)pvParameters;
	uint32_t period_ticks;
	if(params != NULL){
    	period_ticks = *params;
	} else {
    	period_ticks = pdMS_TO_TICKS(500);
	}
    static uint32_t task_id_counter = 0;
    TaskHandle_t new_task_handle = NULL;

    while(1){
    	uint32_t release = (uint32_t)xTaskGetTickCount();
		uint32_t deadline = release + period_ticks;

        xTaskCreate(User_DD_Task, "DD_Task", configMINIMAL_STACK_SIZE, NULL, 1, &new_task_handle);

        if(new_task_handle != NULL){
        	create_dd_task(new_task_handle, PERIODIC, task_id_counter++, deadline);
        }

        vTaskDelay(period_ticks);
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
