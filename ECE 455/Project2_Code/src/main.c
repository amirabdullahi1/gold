/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4_discovery.h"
/* Kernel includes. */
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
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


/*--------Global Variables-----------------------------------*/
#define data  	0
#define msgQUEUE_LENGTH 100
#define PRIORITY_HI  2
#define PRIORITY_LO  1

xQueueHandle xDDS_MsgQueue_Handle = 0;
xQueueHandle xDDS_RspQueue_Handle = 0;
xQueueHandle xDDS_TidQueue_Handle = 0;

xQueueHandle xDDS_AtlQueue_Handle = 0;
 xQueueHandle xDDS_CmpQueue_Handle = 0;
 xQueueHandle xDDS_OvrQueue_Handle = 0;

TaskHandle_t xDDS_Handle = NULL;
TaskHandle_t xDD1_Handle = NULL;
TaskHandle_t xDD2_Handle = NULL;
TaskHandle_t xDD3_Handle = NULL;
/*-----------------------------------------------------------*/

/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

/*---- typedefs ---------------------------------------------*/
typedef enum {PERIODIC, APERIODIC} task_type;

typedef enum {
	msg_release_task,
	msg_complete_task,
    msg_get_active_dd_task_list,
    msg_get_completed_dd_task_list,
    msg_get_overdue_dd_task_list
} msg_type;

typedef struct {
	msg_type dds_msg_type;
    TaskHandle_t t_handle;
    task_type dd_t_type;
    uint32_t task_id;
	uint32_t release_time;
    uint32_t absolute_deadline;
} dds_msg;

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
/*-----------------------------------------------------------*/

/*---- LED Init ---------------------------------------------*/
void myGPIO_Init()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef GPIO_LED_InitStruct = {0};

	GPIO_LED_InitStruct.GPIO_Mode = GPIO_Mode_OUT;

	GPIO_LED_InitStruct.GPIO_OType = GPIO_OType_PP;

	GPIO_LED_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_LED_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;

	GPIO_LED_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_LED_InitStruct);
}
/*-----------------------------------------------------------*/

/*---- DD Task List Add and Remove --------------------------*/
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
void create_dd_task(TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t absolute_deadline, dd_task_list **creator_list)
{
	dd_task new_task;
	new_task.t_handle = t_handle;
	new_task.type = type;
	new_task.task_id = task_id;
	new_task.release_time = xTaskGetTickCount();
	new_task.absolute_deadline = absolute_deadline;
	new_task.completion_time = 0;

	dd_task_list_add(creator_list, new_task);
}

void delete_dd_task(uint32_t task_id, dd_task_list **deleter_list)
{
	if(*deleter_list == NULL)
		return;

	dd_task_list_rmv(deleter_list, task_id);
}
/*-----------------------------------------------------------*/

/*---- DD Task Release, Complete and Update -----------------*/
void release_dd_task(TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t absolute_deadline)
{
	dds_msg release_msg;
    release_msg.dds_msg_type = msg_release_task;
    release_msg.t_handle = t_handle;
	release_msg.dd_t_type = type;
    release_msg.task_id = task_id;
    release_msg.release_time = (uint32_t)xTaskGetTickCount();
    release_msg.absolute_deadline = absolute_deadline;

    xQueueSend(xDDS_MsgQueue_Handle, &release_msg, portMAX_DELAY);
}

void complete_dd_task(uint32_t task_id)
{
	dds_msg complete_msg;
    complete_msg.dds_msg_type = msg_complete_task;
    complete_msg.task_id = task_id;

    xQueueSend(xDDS_MsgQueue_Handle, &complete_msg, portMAX_DELAY);
}

/**
 * if HI task exists -> do nothing
 * if no HI task -> promote within updater_list
 */
void update_dd_task(dd_task_list *updater_list)
{
    dd_task_list *task_list_curr = updater_list;
    while (task_list_curr != NULL)
    {
    	// if HI task exists -> do nothing
        if(uxTaskPriorityGet(task_list_curr->task.t_handle) == PRIORITY_HI)
            return;

        task_list_curr = task_list_curr->next_task;
    }

    // No HI task -> promote within updater_list
    if(updater_list != NULL)
	{
		xQueueOverwrite(xDDS_TidQueue_Handle, &(updater_list->task.task_id));
        vTaskPrioritySet(updater_list->task.t_handle, PRIORITY_HI);
		vTaskResume(updater_list->task.t_handle);
	}
}
/*-----------------------------------------------------------*/

/*---- Monitor Task Functions--------------------------------*/
void get_active_dd_task_list(dd_task_list *monitor_list)
{
	dd_task_list *curr = monitor_list;

	printf("Active Task List: ");
	while(curr != NULL)
	{
		printf("%u, ", (unsigned int)curr->task.task_id);
		curr = curr->next_task;
	}
	printf("\n");
}

void get_completed_dd_task_list(dd_task_list *monitor_list)
{
	dd_task_list *curr = monitor_list;

	printf("Completed Task List: ");
	while(curr != NULL)
	{
		printf("%u, ", (unsigned int)curr->task.task_id);
		curr = curr->next_task;
	}
	printf("\n");
}

void get_overdue_dd_task_list(dd_task_list *monitor_list)
{
	dd_task_list *curr = monitor_list;

	printf("Overdue Task List: ");
	while(curr != NULL)
	{
		printf("%u, ", (unsigned int)curr->task.task_id);
		curr = curr->next_task;
	}
	printf("\n");
}
/*-----------------------------------------------------------*/

/*---- Min and geeksforgeeks.org/c/lcm-of-two-numbers-in-c --*/
uint16_t min(uint16_t a, uint16_t b)
{
	return a < b ? a : b;
}

uint16_t lcm(uint16_t a, uint16_t b)
{
    uint16_t max = (a > b) ? a : b;

    while (1) {
        if(max % a == 0 && max % b == 0)
			return max;
        ++max;
    }
}
/*-----------------------------------------------------------*/

/*---- Timer ------------------------------------------------*/
static TimerHandle_t TIM_GEN;
static TimerHandle_t TIM_MON;
//static TimerHandle_t TIM_OVR;

void vGenTimerCallback(TimerHandle_t genTimer);
//void vOvrTimerCallback(TimerHandle_t ovrTimer);

void myTIM_GEN_Init(uint16_t test_bench[3])
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

void myTIM_MON_Init(uint16_t test_bench[3])
{
    TIM_MON = xTimerCreate(
        "DD Task Mon",
        pdMS_TO_TICKS(5 /* lcm(test_bench[0], lcm(test_bench[1], test_bench[2])) / 2 */),
        pdTRUE,
        test_bench,
        vGenTimerCallback
    );
    configASSERT(TIM_MON);
}

//void myTIM_OVR_Init(uint16_t test_bench[3])
//{
//    TIM_OVR = xTimerCreate(
//        "DD Task Ovr",
//        pdMS_TO_TICKS(5),
//        pdFALSE,
//        test_bench,
//		vOvrTimerCallback
//    );
//    configASSERT(TIM_OVR);
//}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
static void DDS( void *pvParameters );
static void DD_Monitor( void *pvParameters );
static void DD_Task1( void *pvParameters );
static void DD_Task2( void *pvParameters );
static void DD_Task3( void *pvParameters );
/*-----------------------------------------------------------*/

int main(void)
{
	// Disable time slicing
	if(configUSE_TIME_SLICING)
		return -1;

	myGPIO_Init();

	/* Configure the system ready to run the demo.  The clock configuration
	can be done here if it was not done before main() was called. */
	prvSetupHardware();

	/* Create the queue used by the queue send and queue receive tasks.
	http://www.freertos.org/a00116.html */

    // Used for the DDS scheduler.
    xDDS_MsgQueue_Handle = xQueueCreate(20, sizeof(dds_msg));
	xDDS_RspQueue_Handle = xQueueCreate(1,  sizeof(dd_task_list *));
	xDDS_TidQueue_Handle = xQueueCreate(1,  sizeof(uint32_t));

	xDDS_AtlQueue_Handle = xQueueCreate(1,  sizeof(dd_task_list *));
	xDDS_CmpQueue_Handle = xQueueCreate(1,  sizeof(dd_task_list *));
	xDDS_OvrQueue_Handle = xQueueCreate(1,  sizeof(dd_task_list *));

	/* Add to the registry, for the benefit of kernel aware debugging. */
	vQueueAddToRegistry( xDDS_MsgQueue_Handle, "Msg Queue" );
	vQueueAddToRegistry( xDDS_RspQueue_Handle, "Rsp Queue" );
	vQueueAddToRegistry( xDDS_TidQueue_Handle, "Tid Queue" );

	vQueueAddToRegistry( xDDS_AtlQueue_Handle, "Atl Queue" );
	vQueueAddToRegistry( xDDS_CmpQueue_Handle, "Cmp Queue" );
	vQueueAddToRegistry( xDDS_OvrQueue_Handle, "Ovr Queue" );

	 static uint16_t test_bench_1[2][3] = {{ 95, 150, 250}, {500, 500, 750}};
	// static uint16_t test_bench_2[2][3] = {{ 95, 150, 250}, {250, 500, 750}};
	// static uint16_t test_bench_3[2][3] = {{100, 200, 200}, {500, 500, 500}};
	static uint16_t test_bench_4[2][3] = {{ 5000, 2000, 3000}, {10999, 10997, 11000}};

	static uint16_t (*test_bench_i)[3] = test_bench_4;

	xTaskCreate(DD_Task1, "DD_Task1", configMINIMAL_STACK_SIZE, &test_bench_i[0][0], PRIORITY_LO, &xDD1_Handle);
	xTaskCreate(DD_Task2, "DD_Task2", configMINIMAL_STACK_SIZE, &test_bench_i[0][1], PRIORITY_LO, &xDD2_Handle);
	xTaskCreate(DD_Task3, "DD_Task3", configMINIMAL_STACK_SIZE, &test_bench_i[0][2], PRIORITY_LO, &xDD3_Handle);

	vTaskSuspend(xDD1_Handle);
	vTaskSuspend(xDD2_Handle);
	vTaskSuspend(xDD3_Handle);

	xTaskCreate(DDS, "DDS", 256, NULL, 3, NULL /* &xDDS_Handle */);
//	xTaskCreate(DD_Monitor, "DD_Monitor", configMINIMAL_STACK_SIZE, NULL, 3, NULL);

	/* Initialize the timer. */
	myTIM_GEN_Init(test_bench_i[1]);
	myTIM_MON_Init(test_bench_i[1]);
    xTimerStart(TIM_GEN, 0);
    xTimerStart(TIM_MON, 0);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return 0;
}

/*-----------------------------------------------------------*/
void vGenTimerCallback(TimerHandle_t genTimer)
{
	/* Initialization. */
	static uint8_t initialized = 0;

	static uint16_t *test_bench_X;
    static uint16_t DD_task1_period;
    static uint16_t DD_task2_period;
    static uint16_t DD_task3_period;

    static uint16_t task1_interval;
    static uint16_t task2_interval;
    static uint16_t task3_interval;

	uint16_t task_interval;

    if(initialized == 0)
    {
        test_bench_X = (uint16_t *)pvTimerGetTimerID(genTimer);
		configASSERT(test_bench_X);

        DD_task1_period = test_bench_X[0];
        DD_task2_period = test_bench_X[1];
        DD_task3_period = test_bench_X[2];

        task1_interval = DD_task1_period;
        task2_interval = DD_task2_period;
        task3_interval = DD_task3_period;

        initialized = 1;
    }

	static uint16_t task_id_counter = 0;
	if(task1_interval >= DD_task1_period)
	{
		task1_interval = 0;
		release_dd_task(xDD1_Handle, PERIODIC, task_id_counter++, (uint32_t)xTaskGetTickCount() + DD_task1_period);
	}
	if(task2_interval >= DD_task2_period)
	{
		task2_interval = 0;
		release_dd_task(xDD2_Handle, PERIODIC, task_id_counter++, (uint32_t)xTaskGetTickCount() + DD_task2_period);
	}
	if(task3_interval >= DD_task3_period)
	{
		task3_interval = 0;
		release_dd_task(xDD3_Handle, PERIODIC, task_id_counter++, (uint32_t)xTaskGetTickCount() + DD_task3_period);
	}

	task_interval = min(DD_task1_period - task1_interval, min(DD_task2_period - task2_interval, DD_task3_period - task3_interval));
	task1_interval += task_interval;
	task2_interval += task_interval;
	task3_interval += task_interval;

	xTimerChangePeriod(genTimer, pdMS_TO_TICKS(task_interval), 0);
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
static void DDS( void *pvParameters )
{
    dd_task_list *active_task_list = NULL;
    dd_task_list *completed_task_list = NULL;
    dd_task_list *overdue_task_list = NULL;

	dds_msg msg;
	while(1)
	{
		if(xQueueReceive(xDDS_MsgQueue_Handle, &msg, portMAX_DELAY) == pdTRUE)
		{
			do {
				switch(msg.dds_msg_type)
				{
					case msg_release_task:
					{
						// Promote and resume incompleted tasks second after create
						create_dd_task(msg.t_handle, msg.dd_t_type, msg.task_id, msg.absolute_deadline, &active_task_list);
						xQueueOverwrite(xDDS_AtlQueue_Handle, &active_task_list);
						break;
					}

					case msg_complete_task:
					{
						dd_task_list *task_list_curr = active_task_list;
						while(task_list_curr != NULL)
						{
							if(task_list_curr->task.task_id == msg.task_id)
							{

								break;
							}

							task_list_curr = task_list_curr->next_task;
						}
						if(task_list_curr == NULL)
							break;

						// Demote and suspend completed task first before delete
						vTaskPrioritySet(task_list_curr->task.t_handle, PRIORITY_LO);
						vTaskSuspend(task_list_curr->task.t_handle);

						// Completion_time update
						task_list_curr->task.completion_time = xTaskGetTickCount();
						dd_task_list_add(&completed_task_list, task_list_curr->task);
						delete_dd_task(msg.task_id, &active_task_list);
						xQueueOverwrite(xDDS_AtlQueue_Handle, &active_task_list);
						break;
					}

                    case msg_get_active_dd_task_list:
                    {
        			    get_active_dd_task_list(active_task_list);
        			    break;
                    }

        		    case msg_get_completed_dd_task_list:
                    {
        			    get_completed_dd_task_list(completed_task_list);
        			    break;
                    }

        		    case msg_get_overdue_dd_task_list:
                    {
        			    // get_overdue_dd_task_list(overdue_task_list);
        			    break;
                    }

				}

			} while(xQueueReceive(xDDS_MsgQueue_Handle, &msg, 0) == pdTRUE);

			update_dd_task(active_task_list);
		}
	}
}

/*---- DD Monitor ----------------------------------------*/

//static void DD_Monitor( void *pvParameters )
//{
//	uint16_t tx_data = 0;
//	GPIO_SetBits(GPIOD, GPIO_Pin_14);
//
//	while(1)
//	{
//		if(tx_data == 0)
//		{
//			dds_msg *monitor_message_active = malloc(sizeof(dds_msg));
//			monitor_message_active->dds_msg_type = msg_get_active_dd_task_list;
//			xQueueSend(xDDS_MsgQueue_Handle, monitor_message_active, portMAX_DELAY);
//
//			dds_msg *monitor_message_completed = malloc(sizeof(dds_msg));
//			monitor_message_completed->dds_msg_type = msg_get_completed_dd_task_list;
//			xQueueSend(xDDS_MsgQueue_Handle, monitor_message_completed, portMAX_DELAY);
//
//			dds_msg *monitor_message_overdue = malloc(sizeof(dds_msg));
//			monitor_message_overdue->dds_msg_type = msg_get_overdue_dd_task_list;
//			xQueueSend(xDDS_MsgQueue_Handle, monitor_message_overdue, portMAX_DELAY);
//
//			free(monitor_message_active);
//			free(monitor_message_completed);
//			free(monitor_message_overdue);
//
//			vTaskResume(xDDS_Handle);
//		}
//	}
//}

/*-----------------------------------------------------------*/


/*---- User-Defined F-Tasks ---------------------------------*/
static void DD_Task1(void *pvParameters)
{
	TickType_t execution_ticks = *(uint16_t *)pvParameters;
	for (;;)
    {
		// printf("Green LED ON!\n");
		uint32_t task_identifcation;
		xQueueReceive(xDDS_TidQueue_Handle, &task_identifcation, portMAX_DELAY);

		// printf("TickCount1: %u\n", (unsigned int)xTaskGetTickCount());
		GPIO_SetBits(GPIOD, GPIO_Pin_12);
		vTaskDelay(pdMS_TO_TICKS(execution_ticks)); // busy "loop"

		// printf("TickCount1: %u\n", (unsigned int)xTaskGetTickCount());
		GPIO_ResetBits(GPIOD, GPIO_Pin_12);
		complete_dd_task(task_identifcation);
		vTaskSuspend(NULL);
    }
}

static void DD_Task2(void *pvParameters)
{
	TickType_t execution_ticks = *(uint16_t *)pvParameters;
	for (;;)
    {
		// printf("Red LED ON!\n");
		uint32_t task_identifcation;
		xQueueReceive(xDDS_TidQueue_Handle, &task_identifcation, portMAX_DELAY);

    	// printf("TickCount2: %u\n", (unsigned int)xTaskGetTickCount());
        GPIO_SetBits(GPIOD, GPIO_Pin_13);
		vTaskDelay(pdMS_TO_TICKS(execution_ticks)); // busy "loop"

    	// printf("TickCount2: %u\n", (unsigned int)xTaskGetTickCount());
		GPIO_ResetBits(GPIOD, GPIO_Pin_13);
        complete_dd_task(task_identifcation);
		vTaskSuspend(NULL);
    }
}

static void DD_Task3(void *pvParameters)
{
	TickType_t execution_ticks = *(uint16_t *)pvParameters;
	for (;;)
    {
		// printf("Blue LED ON!\n");
		uint32_t task_identifcation;
		xQueueReceive(xDDS_TidQueue_Handle, &task_identifcation, portMAX_DELAY);

    	// printf("TickCount3: %u\n", (unsigned int)xTaskGetTickCount());
        GPIO_SetBits(GPIOD, GPIO_Pin_15);
		vTaskDelay(pdMS_TO_TICKS(execution_ticks)); // busy "loop"

    	// printf("TickCount3: %u\n", (unsigned int)xTaskGetTickCount());
		GPIO_ResetBits(GPIOD, GPIO_Pin_15);
        complete_dd_task(task_identifcation);
		vTaskSuspend(NULL);
    }
}
/*-----------------------------------------------------------*/

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
