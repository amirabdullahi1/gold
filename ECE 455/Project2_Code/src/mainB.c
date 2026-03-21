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
#define msgQUEUE_LENGTH 10
#define tidQUEUE_LENGTH 1
#define rspQUEUE_LENGTH 1

#define PRIORITY_HI  2
#define PRIORITY_LO  1

#define TIM_DEV 1

xQueueHandle xDDS_MsgQueue_Handle = 0;
xQueueHandle xDDS_TidQueue_Handle = 0;
xQueueHandle xMON_RspQueue_Handle = 0;

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
	GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
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

	dd_task curr_task = {0};
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

	dd_task curr_task = {0};
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
	dd_task new_task = {0};
	new_task.t_handle = t_handle;
	new_task.type = type;
	new_task.task_id = task_id;
	new_task.release_time = xTaskGetTickCount() - TIM_DEV;
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
	dds_msg release_msg = {0};
    release_msg.dds_msg_type = msg_release_task;
    release_msg.t_handle = t_handle;
	release_msg.dd_t_type = type;
    release_msg.task_id = task_id;
    release_msg.absolute_deadline = absolute_deadline;

    xQueueSend(xDDS_MsgQueue_Handle, &release_msg, portMAX_DELAY);
}

void complete_dd_task(uint32_t task_id)
{
	dds_msg complete_msg = {0};
    complete_msg.dds_msg_type = msg_complete_task;
    complete_msg.task_id = task_id;

    xQueueSend(xDDS_MsgQueue_Handle, &complete_msg, portMAX_DELAY);
}

/**
 * if (HI task exists && "HIer" task exists) -> demote HI task, promote HIer task
 * if (HI task exists && "HIer" task !exists) -> do nothing
 * if (no HI task || HI task demoted) -> promote updater_list head
 */
void update_dd_task(dd_task_list *updater_list)
{
    dd_task_list *task_list_curr = updater_list;
    while (task_list_curr != NULL)
    {
        if(uxTaskPriorityGet(task_list_curr->task.t_handle) == PRIORITY_HI)
		{
			// if (HI task exists && "HIer" task exists) -> demote HI task, promote HIer task
			if(updater_list->task.absolute_deadline < task_list_curr->task.absolute_deadline)
			{
				vTaskPrioritySet(task_list_curr->task.t_handle, PRIORITY_LO);
				vTaskSuspend(task_list_curr->task.t_handle);
				break;
			}

			// if (HI task exists && "HIer" task !exists) -> do nothing
            return;
		}

        task_list_curr = task_list_curr->next_task;
    }

	// if (no HI task || HI task demoted) -> promote updater_list head
    if(updater_list != NULL)
	{
    	xQueueOverwrite(xDDS_TidQueue_Handle, &(updater_list->task.task_id));
        vTaskPrioritySet(updater_list->task.t_handle, PRIORITY_HI);
		vTaskResume(updater_list->task.t_handle);
	}
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
dd_task_list *get_active_dd_task_list(void)
{
    dds_msg get_msg = {0};
    get_msg.dds_msg_type = msg_get_active_dd_task_list;

    if (xQueueSend(xDDS_MsgQueue_Handle, &get_msg, 0) != pdPASS)
        return NULL;

    dd_task_list *system_dd_task_list = NULL;

    if (xQueueReceive(xMON_RspQueue_Handle, &system_dd_task_list, portMAX_DELAY) != pdPASS)
        return NULL;

    return system_dd_task_list;
}

dd_task_list *get_completed_dd_task_list(void)
{
    dds_msg get_msg = {0};
    get_msg.dds_msg_type = msg_get_completed_dd_task_list;

    if (xQueueSend(xDDS_MsgQueue_Handle, &get_msg, 0) != pdPASS)
        return NULL;

    dd_task_list *system_dd_task_list = NULL;

    if (xQueueReceive(xMON_RspQueue_Handle, &system_dd_task_list, portMAX_DELAY) != pdPASS)
        return NULL;

    return system_dd_task_list;
}

dd_task_list *get_overdue_dd_task_list(void)
{
    dds_msg get_msg = {0};
    get_msg.dds_msg_type = msg_get_overdue_dd_task_list;

    if (xQueueSend(xDDS_MsgQueue_Handle, &get_msg, 0) != pdPASS)
        return NULL;

    dd_task_list *system_dd_task_list = NULL;

    if (xQueueReceive(xMON_RspQueue_Handle, &system_dd_task_list, portMAX_DELAY) != pdPASS)
        return NULL;

    return system_dd_task_list;
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/*---- Min and geeksforgeeks.org/c/lcm-of-two-numbers-in-c --*/
uint32_t min(uint32_t a, uint32_t b)
{
	return a < b ? a : b;
}

uint32_t gcd(uint32_t a, uint32_t b)
{
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

uint32_t lcm(uint32_t a, uint32_t b)
{
    if (a == 0 || b == 0)
		return 0;
    uint32_t g = gcd(a, b), r = a / g;
    return (r <= UINT32_MAX / b) ? r * b : UINT32_MAX;
}
/*-----------------------------------------------------------*/

/*---- Timer ------------------------------------------------*/
static TimerHandle_t TIM_GEN;
//static TimerHandle_t TIM_MON;

void vGenTimerCallback(TimerHandle_t genTimer);
//void vMonTimerCallback(TimerHandle_t monTimer);

void myTIM_GEN_Init(uint32_t test_bench[3])
{
    TIM_GEN = xTimerCreate(
        "DD Task Gen",
        pdMS_TO_TICKS(TIM_DEV),
        pdFALSE,
        test_bench,
        vGenTimerCallback
    );
    configASSERT(TIM_GEN);
}

//void myTIM_MON_Init(uint32_t test_bench[3])
//{
//    TIM_MON = xTimerCreate(
//        "DD Task Mon",
//        pdMS_TO_TICKS(TIM_DEV),
//        pdFALSE,
//        test_bench,
//        vMonTimerCallback
//    );
//    configASSERT(TIM_MON);
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
	/* Disable time slicing. Enable preemption. */
	if(configUSE_TIME_SLICING || !configUSE_PREEMPTION)
		return -1;

	myGPIO_Init();

	/* Configure the system ready to run the demo.  The clock configuration
	can be done here if it was not done before main() was called. */
	prvSetupHardware();

	/* Create the queues used by the queue send and queue receive tasks.
	http://www.freertos.org/a00116.html */

    // Used for the DDS scheduler.
    xDDS_MsgQueue_Handle = xQueueCreate(msgQUEUE_LENGTH, sizeof(dds_msg));
	xDDS_TidQueue_Handle = xQueueCreate(tidQUEUE_LENGTH, sizeof(uint32_t));
	// xMON_RspQueue_Handle = xQueueCreate(rspQUEUE_LENGTH, sizeof(dd_task_list *));

	/* Add to the registry, for the benefit of kernel aware debugging. */
	vQueueAddToRegistry( xDDS_MsgQueue_Handle, "Msg Queue" );
	vQueueAddToRegistry( xDDS_TidQueue_Handle, "Tid Queue" );
	// vQueueAddToRegistry( xMON_RspQueue_Handle, "Mon Queue" );

	static uint32_t test_bench_1[2][3] = {{ 95, 150, 250}, {500, 500, 750}};
	static uint32_t test_bench_2[2][3] = {{ 95, 150, 250}, {250, 500, 750}};
	static uint32_t test_bench_3[2][3] = {{100, 200, 200}, {500, 500, 500}};
	static uint32_t test_bench_4[2][3] = {{ 5000, 2000, 3000}, {10999, 10997, 11000}};

	static uint32_t (*test_bench_i)[3] = test_bench_2;

	xTaskCreate(DD_Task1, "DD_Task1", 256, &test_bench_i[0][0], PRIORITY_LO, &xDD1_Handle);
	xTaskCreate(DD_Task2, "DD_Task2", 256, &test_bench_i[0][1], PRIORITY_LO, &xDD2_Handle);
	xTaskCreate(DD_Task3, "DD_Task3", 256, &test_bench_i[0][2], PRIORITY_LO, &xDD3_Handle);

	vTaskSuspend(xDD1_Handle);
	vTaskSuspend(xDD2_Handle);
	vTaskSuspend(xDD3_Handle);

	xTaskCreate(DDS, "DDS", 256, NULL, 3, NULL);
	// xTaskCreate(DD_Monitor, "DD_Monitor", 256, test_bench_i[1], 3, NULL);

	/* Initialize the timers. TODO: TIM_MON */
	myTIM_GEN_Init(test_bench_i[1]);
	// myTIM_MON_Init(test_bench_i[1]);
    xTimerStart(TIM_GEN, 0);
    // xTimerStart(TIM_MON, 0);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	return 0;
}

/*-----------------------------------------------------------*/
void vGenTimerCallback(TimerHandle_t genTimer)
{
	/* Initialization. */
	static uint8_t initialized = 0;

	static uint32_t *test_bench_X;
    static uint32_t DD_task1_period;
    static uint32_t DD_task2_period;
    static uint32_t DD_task3_period;

    static uint32_t task1_interval;
    static uint32_t task2_interval;
    static uint32_t task3_interval;

	uint32_t task_interval;

    if(initialized == 0)
    {
        test_bench_X = (uint32_t *)pvTimerGetTimerID(genTimer);
		configASSERT(test_bench_X);

        DD_task1_period = test_bench_X[0];
        DD_task2_period = test_bench_X[1];
        DD_task3_period = test_bench_X[2];

        task1_interval = DD_task1_period;
        task2_interval = DD_task2_period;
        task3_interval = DD_task3_period;

        initialized = 1;
    }

	static uint32_t task_id_counter = 0;
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
    dd_task_list *active_task_list 	  = NULL;
    dd_task_list *completed_task_list = NULL;
    dd_task_list *overdue_task_list   = NULL;

	dds_msg msg;
	while(1)
	{
		if(xQueueReceive(xDDS_MsgQueue_Handle, &msg, portMAX_DELAY) == pdTRUE)
		{
			// taskENTER_CRITICAL();

			do {
				switch(msg.dds_msg_type)
				{
					case msg_release_task:
					{
						dd_task_list *task_list_curr = active_task_list;
						while(task_list_curr != NULL)
						{
							if(task_list_curr->task.absolute_deadline < xTaskGetTickCount())
							{
								// Ignore completion time
								dd_task_list *task_list_next = task_list_curr->next_task;
								dd_task_list_add(&overdue_task_list, task_list_curr->task);

								// Demote and suspend overdue task first before delete
								vTaskPrioritySet(task_list_curr->task.t_handle, PRIORITY_LO);
								vTaskSuspend(task_list_curr->task.t_handle);
								delete_dd_task(task_list_curr->task.task_id, &active_task_list);

								task_list_curr = task_list_next;
								continue;
							}

							task_list_curr = task_list_curr->next_task;
						}

						// Promote and resume incompleted tasks second after create
						create_dd_task(msg.t_handle, msg.dd_t_type, msg.task_id, msg.absolute_deadline, &active_task_list);
						break;
					}

					case msg_complete_task:
					{
						dd_task_list *task_list_curr = active_task_list;
						while(task_list_curr != NULL)
						{
							if(task_list_curr->task.task_id == msg.task_id)
							{
								// Update completion time
								task_list_curr->task.completion_time = xTaskGetTickCount() - TIM_DEV;
								dd_task_list_add(&completed_task_list, task_list_curr->task);

								// Demote and suspend completed task first before delete
								vTaskPrioritySet(task_list_curr->task.t_handle, PRIORITY_LO);
								vTaskSuspend(task_list_curr->task.t_handle);
								delete_dd_task(msg.task_id, &active_task_list);

								break;
							}

							task_list_curr = task_list_curr->next_task;
						}

						break;
					}

					case msg_get_active_dd_task_list:
					{
						// xQueueOverwrite(xMON_RspQueue_Handle, &active_task_list);
						// xQueueOverwriteFromISR(xMON_RspQueue_Handle, &active_task_list, NULL);
						break;
					}

					case msg_get_completed_dd_task_list:
					{
						// xQueueOverwrite(xMON_RspQueue_Handle, &completed_task_list);
						// xQueueOverwriteFromISR(xMON_RspQueue_Handle, &completed_task_list, NULL);
						break;
					}

					case msg_get_overdue_dd_task_list:
					{
						// xQueueOverwrite(xMON_RspQueue_Handle, &overdue_task_list);
						// xQueueOverwriteFromISR(xMON_RspQueue_Handle, &overdue_task_list, NULL);
						break;
					}

					default:
					{
						break;
					}

				}
			} while(xQueueReceive(xDDS_MsgQueue_Handle, &msg, 0) == pdTRUE);

			update_dd_task(active_task_list);
			// taskEXIT_CRITICAL();
		}
	}
}

static void DD_Monitor( void *pvParameters )
{
	static uint32_t *test_bench_X;
	static uint32_t xTaskMaxTickCount;

	test_bench_X = (uint32_t *)pvParameters;
	xTaskMaxTickCount = pdMS_TO_TICKS(lcm(test_bench_X[0], lcm(test_bench_X[1], test_bench_X[2])));

	while(1)
	{
		taskENTER_CRITICAL();

		if((uint32_t)xTaskGetTickCount() > xTaskMaxTickCount)
			NVIC_SystemReset();

		// get_active_dd_task_list();
		// get_completed_dd_task_list();
		// get_overdue_dd_task_list();

		taskEXIT_CRITICAL();
	}
}
/*-----------------------------------------------------------*/

/*---- User-Defined F-Tasks ---------------------------------*/
static void DD_Task1(void *pvParameters)
{
	TickType_t execution_ticks = *(uint32_t *)pvParameters;
	for (;;)
    {
		// printf("Green LED ON!\n");
		uint32_t task_identification;
		xQueueReceive(xDDS_TidQueue_Handle, &task_identification, portMAX_DELAY);

		TickType_t completion_ticks = 0;
		TickType_t initiation_ticks = 0;

		while (completion_ticks < execution_ticks)
		{
			GPIO_ResetBits(GPIOD, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
			GPIO_SetBits(GPIOD, GPIO_Pin_12);
			initiation_ticks = xTaskGetTickCount();
			while (xTaskGetTickCount() == initiation_ticks);
			completion_ticks++;
		}

		GPIO_ResetBits(GPIOD, GPIO_Pin_12);
		complete_dd_task(task_identification);
    }
}

static void DD_Task2(void *pvParameters)
{
	TickType_t execution_ticks = *(uint32_t *)pvParameters;
	for (;;)
    {
		// printf("Red LED ON!\n");
		uint32_t task_identification;
		xQueueReceive(xDDS_TidQueue_Handle, &task_identification, portMAX_DELAY);

		TickType_t completion_ticks = 0;
		TickType_t initiation_ticks = 0;

		while (completion_ticks < execution_ticks)
		{
			GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15);
			GPIO_SetBits(GPIOD, GPIO_Pin_13);
			initiation_ticks = xTaskGetTickCount();
			while (xTaskGetTickCount() == initiation_ticks);
			completion_ticks++;
		}

		GPIO_ResetBits(GPIOD, GPIO_Pin_13);
        complete_dd_task(task_identification);
    }
}

static void DD_Task3(void *pvParameters)
{
	TickType_t execution_ticks = *(uint32_t *)pvParameters;
	for (;;)
    {
		// printf("Blue LED ON!\n");
		uint32_t task_identification;
		xQueueReceive(xDDS_TidQueue_Handle, &task_identification, portMAX_DELAY);

		TickType_t completion_ticks = 0;
		TickType_t initiation_ticks = 0;

		while (completion_ticks < execution_ticks)
		{
			GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14);
        	GPIO_SetBits(GPIOD, GPIO_Pin_15);
			initiation_ticks = xTaskGetTickCount();
			while (xTaskGetTickCount() == initiation_ticks);
			completion_ticks++;
		}

		GPIO_ResetBits(GPIOD, GPIO_Pin_15);
        complete_dd_task(task_identification);
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
