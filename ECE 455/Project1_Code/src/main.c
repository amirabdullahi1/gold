/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include "stm32f4_discovery.h"
/* Kernel includes. */
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_adc.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"

/*-----------------------------------------------------------*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
#define flow_adjust  	0
#define traffic_gen  	1
#define light_state  	2
#define sys_display  	3

#define flowQUEUE_LENGTH 1
#define taskQUEUE_LENGTH 4

#define ADC_MAX 4095.0

/* Function that sets up the leds */
void myGPIO_LED_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Enable clock for GPIO peripherals */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    /* Configured as an output */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;

    /* Set to Push Pull to ensure LEDs are visible and bright */
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; // Look into this a bit more, need to do testing to 100% know which one is the best to use.

    /* Currently turns of any internal resistors */
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; // Adjust through testing, not sure currently what setting is good for LEDs (If we have no external resistors will need to turn on).

    /* Configuring speed of rising and galling edges */
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;

    /* Pin initialization for LEDs */
    /* Has been set up for the 3 different traffic lights, potentiometer and 3 middle car lights shift register */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

}

void myGPIO_ADC_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    /* Configured as analog  */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;

    /* Currently turns of any internal resistors */
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; // Adjust through testing, not sure currently what setting is good for LEDs (If we have no external resistors will need to turn on).

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

}

void myADC1_Init()
{
	ADC_InitTypeDef ADC_InitStruct = {0};

	/* Enable clock for ADC1 peripheral */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* Initialize the ADC_Mode member */
    ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;

    /* initialize the ADC_ScanConvMode member */
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;

    /* Initialize the ADC_ContinuousConvMode member */
    ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;

    /* Initialize the ADC_ExternalTrigConvEdge member */
    ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;

    /* Initialize the ADC_DataAlign member */
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;

    /* Initialize the ADC_NbrOfConversion member */
    ADC_InitStruct.ADC_NbrOfConversion = 1;

    ADC_Init(ADC1, &ADC_InitStruct);

    ADC_Cmd(ADC1, ENABLE);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_144Cycles);

    ADC_SoftwareStartConv(ADC1);
}

/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */
static void prvSetupHardware( void );

static void flow_adjust_task( void *pvParameters );
static void traffic_gen_task( void *pvParameters );
static void light_state_task( void *pvParameters );
static void sys_display_task( void *pvParameters );

xQueueHandle xTaskQueue_handle = 0;
xQueueHandle xFlowQueue_handle = 0;
/*-----------------------------------------------------------*/


int main(void)
{
    /* Configure the system. The clock configuration can be
	done here if it was not done before main() was called. */
    //	prvSetupHardware();

	myGPIO_LED_Init();
    myGPIO_ADC_Init();
    myADC1_Init();


    xTaskQueue_handle = xQueueCreate(
        taskQUEUE_LENGTH,		/* The number of items the queue can hold. */
        sizeof( uint16_t )      /* The size of each item the queue holds. */
    );
	/* Add to the registry, for the benefit of kernel aware debugging. */
    vQueueAddToRegistry( xTaskQueue_handle, "taskQueue" );

	xFlowQueue_handle = xQueueCreate(
        flowQUEUE_LENGTH,		/* The number of items the queue can hold. */
        sizeof( uint16_t )      /* The size of each item the queue holds. */
    );
	/* Add to the registry, for the benefit of kernel aware debugging. */
    vQueueAddToRegistry( xFlowQueue_handle, "flowQueue" );

	xTaskCreate( flow_adjust_task, "Flow Adjust", 256, NULL, 1, NULL);
	xTaskCreate( traffic_gen_task, "Traffic Gen", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate( light_state_task, "Light State", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate( sys_display_task, "Sys Display", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    uint16_t tx_data = flow_adjust;

    if( xQueueSend(xTaskQueue_handle,&tx_data,1000))
    {
        printf("TLS ON!\n");
    }
    else
    {
        printf("Manager Failed!\n");
    }

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

    return 0;
}

static void flow_adjust_task ( void *pvParameters ) {
    uint16_t ADC_val = 0;
	uint16_t rx_data;
    while(1)
	{
        if(xQueueReceive(xTaskQueue_handle, &rx_data, 500))
        {
            if(rx_data == flow_adjust)
            {
                if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {
                    ADC_val = (ADC_GetConversionValue(ADC1));
                	printf("ADC_val %u.\n", ADC_val);
                }

                xQueueOverwrite(xFlowQueue_handle, &ADC_val);
                rx_data = light_state; // traffic_gen;
                xQueueSend(xTaskQueue_handle,&rx_data,1000);
            }

            else {
                if(xQueueSend(xTaskQueue_handle,&rx_data,1000))
                {
                    // printf("Flow Adjust GWP (%u).\n", rx_data); // Got wrong Package
                    vTaskDelay(pdMS_TO_TICKS(5));
                }
            }

        }
    }
}

static void traffic_gen_task ( void *pvParameters ) {
    uint16_t rx_data;
    while(1)
	{
        if(xQueueReceive(xTaskQueue_handle, &rx_data, 500))
        {
            if(rx_data == traffic_gen)
            {

            }

            rx_data = light_state;
            xQueueSend(xTaskQueue_handle,&rx_data,1000);
        }

        else
        {
            if(xQueueSend(xTaskQueue_handle,&rx_data,1000))
            {
                // printf("Traffic Gen GWP (%u).\n", rx_data); // Got wrong Package
                vTaskDelay(pdMS_TO_TICKS(5));
            }
        }
    }
}

static void light_state_task ( void *pvParameters ) {
    GPIO_ResetBits(GPIOC, GPIO_Pin_0);  // R LED OFF
    GPIO_ResetBits(GPIOC, GPIO_Pin_1);  // Y LED OFF
    GPIO_ResetBits(GPIOC, GPIO_Pin_2);  // G LED OFF

    uint16_t r_dur = 3000.0; // assume milliseconds
    uint16_t y_dur = 3000.0; // assume milliseconds
    uint16_t g_dur = 3000.0; // assume milliseconds

    uint16_t ADC_val = 0;
    uint16_t rx_data;
    while(1)
	{
        if(xQueueReceive(xTaskQueue_handle, &rx_data, 500))
        {
            if(rx_data == light_state)
            {
                if(xQueuePeek(xFlowQueue_handle, &ADC_val, 500))
                {
                    r_dur = 9999.0 * (2 - ADC_val/ADC_MAX);
                    g_dur = 9999.0 * (1 + ADC_val/ADC_MAX);
                	// printf("r_dur %u.\n", r_dur);
                	// printf("y_dur %u.\n", y_dur);
                	// printf("g_dur %u.\n", g_dur);
                }
            }


            GPIO_ResetBits(GPIOC, GPIO_Pin_0);  // R LED OFF
            GPIO_SetBits(GPIOC, GPIO_Pin_2);    // G LED ON

            // GPIO_ResetBits(GPIOC, GPIO_Pin_2);  // G LED OFF
            // GPIO_SetBits(GPIOC, GPIO_Pin_1);    // Y LED ON

            // GPIO_ResetBits(GPIOC, GPIO_Pin_1);  // Y LED OFF
            // GPIO_SetBits(GPIOC, GPIO_Pin_0);    // R LED ON


            rx_data = flow_adjust; // sys_display;
            xQueueSend(xTaskQueue_handle,&rx_data,1000);
        }

        else
        {
            if(xQueueSend(xTaskQueue_handle,&rx_data,1000))
            {
                // printf("Light State GWP (%u).\n", rx_data); // Got wrong Package
                vTaskDelay(pdMS_TO_TICKS(5));
            }
        }
    }
}


static void sys_display_task ( void *pvParameters ) {
    uint16_t rx_data;
    while(1)
	{
        if(xQueueReceive(xTaskQueue_handle, &rx_data, 500))
        {
            if(rx_data == sys_display)
            {

            }

            rx_data = flow_adjust;
            xQueueSend(xTaskQueue_handle,&rx_data,1000);
        }

        else
        {
            if(xQueueSend(xTaskQueue_handle,&rx_data,1000))
            {
                // printf("Sys Display GWP (%u).\n", rx_data); // Got wrong Package
                vTaskDelay(pdMS_TO_TICKS(5));
            }
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
