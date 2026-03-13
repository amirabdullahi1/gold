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
#define data  	0
#define msgQUEUE_LENGTH 100

xQueueHandle xMsgQueue_handle = 0;

/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/


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

void dd_task_list_rmv(dd_task_list **task_list, uint32_t this_task_id) {
	dd_task_list *task_list_prev = NULL;
	dd_task_list *task_list_curr = *task_list;

	dd_task curr_task;
	while(task_list_curr != NULL)
	{
		curr_task = task_list_curr->task;
		if(this_task_id == curr_task.task_id)
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


/*---- DD Task Create and Delete ----------------------------*/
void create_dd_task(TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t absolute_deadline)
{
	dd_task *new_task = malloc(sizeof(dd_task));
	new_task->t_handle = t_handle;
	new_task->type = type;
	new_task->task_id = task_id;
	new_task->release_time = (uint32_t)xTaskGetTickCount();
	new_task->absolute_deadline = absolute_deadline;
	new_task->completion_time = 0; // Intialize with default value until task's execution and completion.

	dd_task_list_add(&active_task_list, *new_task); // Calls the function to add to the task list.
	free(new_task);
}

void delete_dd_task(uint32_t task_id)
{

	if(active_task_list == NULL)
	{
		// printf("There is nothing to remove the task list is empty.\n");
		return;
	}

	dd_task_list_rmv(&active_task_list, task_id);
}
/*-----------------------------------------------------------*/


/*---- DD Task Release and Complete -------------------------*/
void release_dd_task(TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t absolute_deadline)
{
	
}

void complete_dd_task()
{

}
/*-----------------------------------------------------------*/


/*---- geeksforgeeks.org/c/lcm-of-two-numbers-in-c ----------*/
uint16_t lcm(uint16_t a, uint16_t b)
{
    int max = (a > b) ? a : b;

    while (1) {
        if (max % a == 0 && max % b == 0) {
			return max;
        }
        ++max;
    }
}
/*-----------------------------------------------------------*/


/*---- Timer ------------------------------------------------*/
static TimerHandle_t TIM_GEN;

void vGenTimerCallback(TimerHandle_t xTimer);

void myTIM_Init(uint16_t (*test_bench)[2])
{
    TIM_GEN = xTimerCreate(
        "DD Task Gen",
        pdMS_TO_TICKS(5),
        pdFALSE,
        test_bench,
        vGenTimerCallback
    );
    configASSERT(TIM_GEN);
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/
static void DD_Task_Generator( void *pvParameters );
static void DD_Task1( void *pvParameters );
static void DD_Task2( void *pvParameters );
static void DD_Task3( void *pvParameters );
/*-----------------------------------------------------------*/

int main(void)
{

	/* Configure the system ready to run the demo.  The clock configuration
	can be done here if it was not done before main() was called. */
	prvSetupHardware();

	/* Create the queue used by the queue send and queue receive tasks.
	http://www.freertos.org/a00116.html */
	xMsgQueue_handle = xQueueCreate( 	msgQUEUE_LENGTH,		/* The number of items the queue can hold. */
							sizeof( uint16_t ) );	/* The size of each item the queue holds. */

	/* Add to the registry, for the benefit of kernel aware debugging. */
	vQueueAddToRegistry( xMsgQueue_handle, "MainQueue" );

	xTaskCreate( DD_Task_Generator, "DD Task Gen", 256, NULL, 2, NULL);
	xTaskCreate(DD_Task1, "DD_Task1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(DD_Task2, "DD_Task2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(DD_Task3, "DD_Task3", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	uint16_t test_bench_1[3][2] = {{95, 500}, {150, 500}, {250, 750}};
	// uint16_t test_bench_2[3][2] = {{95, 500}, {150, 500}, {250, 750}};
	// uint16_t test_bench_3[3][2] = {{95, 500}, {150, 500}, {250, 750}};

	/* Initialize the timer. */
    myTIM_Init(test_bench_1);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return 0;
}
/*-----------------------------------------------------------*/


void vGenTimerCallback(TimerHandle_t xTimer)
{
	/* Initialization. */
	static int initialized = 0;

    static uint16_t (*test_bench_X)[2];
    static uint16_t DD_task1_period;
    static uint16_t DD_task2_period;
    static uint16_t DD_task3_period;

    static uint16_t task1_interval;
    static uint16_t task2_interval;
    static uint16_t task3_interval;
    
	uint16_t task_interval;

    if(initialized == 0)
    {
        test_bench_X = pvTimerGetTimerID(xTimer);
		configASSERT(test_bench_X);

        DD_task1_period = test_bench_X[0][1];
        DD_task2_period = test_bench_X[1][1];
        DD_task3_period = test_bench_X[2][1];

        task1_interval = DD_task1_period;
        task2_interval = DD_task2_period;
        task3_interval = DD_task3_period;

        initialized = 1;
    }

	// uint16_t rx_data = 0;
    // xQueueSendFromISR(xTaskQueue_handle, &rx_data, pdFALSE);

	static uint16_t task_id_counter = 0;
	TaskHandle_t new_task_handle1 = NULL;
	TaskHandle_t new_task_handle2 = NULL;
	TaskHandle_t new_task_handle3 = NULL;

	if(task1_interval >= DD_task1_period)
	{
		task1_interval = 0;
		// deadline = ??
		// release_dd_task
	}
	if(task2_interval >= DD_task2_period) 
	{
		task2_interval = 0;
		// deadline = ??
		// release_dd_task
	}
	if(task3_interval >= DD_task3_period)
	{
		task3_interval = 0;
		// deadline = ??
		// release_dd_task
	}

	task_interval = min(DD_task1_period - task1_interval, min(DD_task2_period - task2_interval, DD_task3_period - task3_interval));
	task1_interval += task_interval;
	task2_interval += task_interval;
	task3_interval += task_interval;

	xTimerChangePeriod(xTimer, pdMS_TO_TICKS(task_interval), 0);
}
/*-----------------------------------------------------------*/

static void DDS( void *pvParameters )
{
	// pretty sure these calls need to happen from DDS task
	// create_dd_task(new_task_handle1, PERIODIC, task_id_counter++, deadline);

	// dd_task_list_add(&active_task_list, new_task_handle1); // Calls the function to add to the task list.
}

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
		if(1)
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
		if(1)
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
		if(1)
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
