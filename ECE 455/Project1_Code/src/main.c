/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
// Added for task management.
#define flow_adjust  	0
#define traffic_gen  	1
#define light_state  	2
#define sys_display  	3

// Added for queue management.
#define taskQUEUE_LENGTH 4
#define flowQUEUE_LENGTH 1
#define rygQUEUE_LENGTH 1

xQueueHandle xTaskQueue_handle = 0;
xQueueHandle xFlowQueue_handle = 0;
xQueueHandle xRygQueue_handle = 0;

// Added for ADC management.
#define ADC_MIN 0
#define ADC_MAX 4000
#define ADC_RES 30

// Added for traffic management.
#define NUM_TRAFFIC_LEDS 19
#define INTERSECTION 8
#define MAX_LEDS 10


/* Function that sets up the leds */
void myGPIO_LED_Init()
{
    /* Enable clock for GPIO peripherals */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;

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

typedef enum {
    R_STATE,
    Y_STATE,
    G_STATE
} LightState;

static TimerHandle_t TIM_RYG;

void vRygTimerCallback(TimerHandle_t xTimer)
{
    // uint16_t rx_data = light_state;
    // xQueueSendFromISR(xTaskQueue_handle, &rx_data, pdFALSE);
}

void myTIM_Init(void)
{
    TIM_RYG = xTimerCreate(
        "Traffic",
        pdMS_TO_TICKS(100),
        pdFALSE,
        NULL,
        vRygTimerCallback
    );
    configASSERT(TIM_RYG);
}

void red_foo()
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_1);  // Y LED OFF
    GPIO_ResetBits(GPIOC, GPIO_Pin_2);  // G LED OFF

    GPIO_SetBits(GPIOC, GPIO_Pin_0);    // R LED ON
}

void yellow_foo()
{

    GPIO_ResetBits(GPIOC, GPIO_Pin_0);  // R LED OFF
    GPIO_ResetBits(GPIOC, GPIO_Pin_2);  // G LED OFF

    GPIO_SetBits(GPIOC, GPIO_Pin_1);    // Y LED ON
}

void green_foo()
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_0);  // R LED OFF
    GPIO_ResetBits(GPIOC, GPIO_Pin_1);  // Y LED OFF

    GPIO_SetBits(GPIOC, GPIO_Pin_2);    // G LED ON
}


/*--- Start of *need to put in task* ----------------------------------------------------*/
volatile uint8_t head = 0; // Leading leds postion.
volatile uint8_t length = 1; // Current number of active leds.
volatile uint8_t red_light = 1; // 1 means block 0 means go.
volatile uint8_t data = 0b00000000;

unsigned int binary_sets[] = {0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000};

void delay(volatile uint32_t count){
    while(count--);
}

void reset_register(void){
    GPIO_ResetBits(GPIOC, GPIO_Pin_8);
    delay(2000);
    GPIO_SetBits(GPIOC, GPIO_Pin_8);
}

void shift_clock(void){
    GPIO_ResetBits(GPIOC, GPIO_Pin_7);
    delay(2000);
    GPIO_SetBits(GPIOC, GPIO_Pin_7);
    delay(2000);
    GPIO_ResetBits(GPIOC, GPIO_Pin_7);
}

void shift_bits(uint8_t data){
	for(int i = 0; i < 8; i++){
		if(data & (0x80 >> i)){
			GPIO_SetBits(GPIOC, GPIO_Pin_6);
		}else{
			GPIO_ResetBits(GPIOC, GPIO_Pin_6);
		}
		shift_clock();
	}

}

void before_intersection(uint8_t data){
	reset_register();
	for(int i = 0; i < 8; i++){
		if(data & (0x80 >> i)){
			GPIO_SetBits(GPIOC, GPIO_Pin_6);
		}else{
			GPIO_ResetBits(GPIOC, GPIO_Pin_6);
		}
		shift_clock();
	}
}

void intersection_and_beyond(uint8_t data){
	reset_register();
	for(int i = 0; i < 16; i++){
		if(data & (0x80 >> i)){
			GPIO_SetBits(GPIOC, GPIO_Pin_6);
		}else{
			GPIO_ResetBits(GPIOC, GPIO_Pin_6);
		}
		shift_clock();
	}
}

void traffic_handler(void){
	for(int i = 0; i < 8; i++){
		data = binary_sets[i];
		before_intersection(data);
	}
	if(red_light == 0)
		for(int i = 0; i < 8; i++){
			data = binary_sets[i];
			intersection_and_beyond(data);
		}

}
/*--- End of *need to put in task* ------------------------------------------------------*/


/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */
static void prvSetupHardware( void );

static void flow_adjust_task( void *pvParameters );
static void traffic_gen_task( void *pvParameters );
static void light_state_task( void *pvParameters );
static void sys_display_task( void *pvParameters );
/*-----------------------------------------------------------*/


int main(void)
{
    /* Configure the system. The clock configuration can be
	done here if it was not done before main() was called. */
    //	prvSetupHardware();

	myGPIO_LED_Init();
    myGPIO_ADC_Init();
    myADC1_Init();
    myTIM_Init();


    /* Start test sys_display? */
//    reset_register();
//    traffic_handler();
//    shift_bits(0b00000001);
    /* End test sys_display? */



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

    xRygQueue_handle = xQueueCreate(
        rygQUEUE_LENGTH,		/* The number of items the queue can hold. */
        sizeof( LightState )      /* The size of each item the queue holds. */
    );
	/* Add to the registry, for the benefit of kernel aware debugging. */
    vQueueAddToRegistry( xRygQueue_handle, "rygQueue" );

    // All tasks have priority 1. Might need to vary priorities.
	xTaskCreate( flow_adjust_task, "Flow Adjust", 256, NULL, 1, NULL);
	xTaskCreate( traffic_gen_task, "Traffic Gen", 256, NULL, 1, NULL);
	xTaskCreate( light_state_task, "Light State", 256, NULL, 1, NULL);
	xTaskCreate( sys_display_task, "Sys Display", 256, NULL, 1, NULL);

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
	/*
	 * Does vTaskStartScheduler(); seem to stall?
	 * Check line 95 in FreeRTOSConfig.h and increase configTOTAL_HEAP_SIZE.
	 * #define configTOTAL_HEAP_SIZE			( ( size_t ) ( 24 * 1024 ) )
	 */
    return 0;
}

static void flow_adjust_task ( void *pvParameters ) {
    uint16_t ADC_val = 0;
	uint16_t rx_data;
    while(1)
	{
        if(xQueueReceive(xTaskQueue_handle, &rx_data, portMAX_DELAY))
        {
            if(rx_data == flow_adjust)
            {
                /**
                 * Potential necessary improvement:
                 * Avoid polling ADC_GetFlagStatus in a tight loop.
                 * Consider using ADC interrupt mode to automatically store values in xFlowQueue_handle.
                 * This frees up the CPU.
                 */
                if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {
                    ADC_val = (ADC_GetConversionValue(ADC1) & 0x0FFF);
                	// printf("ADC_val %u.\n", ADC_val);
                }

                xQueueOverwrite(xFlowQueue_handle, &ADC_val);
                rx_data = light_state;
                xQueueSend(xTaskQueue_handle,&rx_data,1000);
            }

            else {
                if(xQueueSend(xTaskQueue_handle,&rx_data,1000))
                {
                    // printf("Flow Adjust GWP (%u).\n", rx_data); // Got wrong Package
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }
        }
    }
}

static void traffic_gen_task ( void *pvParameters ) {
    uint16_t ADC_old, ADC_new;
    uint16_t rx_data;
    while(1)
	{
        if(xQueueReceive(xTaskQueue_handle, &rx_data, portMAX_DELAY))
        {
            if(rx_data == traffic_gen)
            {
                rx_data = sys_display;
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
}

static void light_state_task ( void *pvParameters ) {
    LightState next_light_state = Y_STATE;

    uint16_t r_dur, g_dur; // assume milliseconds

    uint16_t ADC_old = INT16_MAX;
    uint16_t ADC_new = INT16_MIN;
    uint16_t rx_data;
    while(1)
	{
        if(xQueueReceive(xTaskQueue_handle, &rx_data, portMAX_DELAY))
        {
            if(rx_data == light_state)
            {
                if(xQueuePeek(xFlowQueue_handle, &ADC_new, 0))
                {
                    if(abs(ADC_old - ADC_new) > ADC_RES)
                    {
                        r_dur = (9999 - (5000 * ADC_new / ADC_MAX)); // 10s -> 5s
                        g_dur = (4999 + (5000 * ADC_new / ADC_MAX)); // 5s -> 10s
                        //  printf("r_dur %u.\n", r_dur);
                        //  printf("g_dur %u.\n", g_dur);

                        if (next_light_state == Y_STATE)
                            xTimerChangePeriod(TIM_RYG, pdMS_TO_TICKS(r_dur), 0);

                        if (next_light_state == R_STATE)
                            xTimerChangePeriod(TIM_RYG, pdMS_TO_TICKS(g_dur), 0);
                    }
					ADC_old = ADC_new;
                }

                if(xTimerIsTimerActive(TIM_RYG) == pdFALSE)
                {
                    switch (next_light_state)
                    {
                        case Y_STATE:
                            /**
                             * When next_light_state = Yellow
                             * Then this_light_state = Green
                             * ------------------------------
                             * Then next_light_state = Red
                             */
                        	green_foo();
                        	next_light_state = R_STATE;
                            xTimerChangePeriod(TIM_RYG, pdMS_TO_TICKS(g_dur), 0);
                            break;

                        case R_STATE:
                            /**
                             * When next_light_state = Red
                             * Then this_light_state = Yellow
                             * ------------------------------
                             * Then next_light_state = Green
                             */
                        	yellow_foo();
                        	next_light_state = G_STATE;
                            xTimerChangePeriod(TIM_RYG, pdMS_TO_TICKS(3000), 0);
                            break;

                        case G_STATE:
                            /**
                             * When next_light_state = Green
                             * Then this_light_state = Red
                             * ------------------------------
                             * Then next_light_state = Yellow
                             */
                        	red_foo();
                        	next_light_state = Y_STATE;
                            xTimerChangePeriod(TIM_RYG, pdMS_TO_TICKS(r_dur), 0);
                            break;
                    }
                    xTimerStart(TIM_RYG, 0);
                }

                xQueueOverwrite(xRygQueue_handle, &next_light_state);
                rx_data = flow_adjust; // traffic_gen;
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
}

static void sys_display_task ( void *pvParameters ) {
    uint16_t rx_data;
    while(1)
	{
        if(xQueueReceive(xTaskQueue_handle, &rx_data, 500))
        {
            if(rx_data == sys_display)
            {
                // Reset the register before rewriting to it.
                reset_register();

                int current_mapping = 0;

                // Updates all traffic care related lights.
                for(int j = 24; j > 0; j = j - 8){
                    for int i = 0; i < j; i++){
                        if(binary_memory_tree[current_mapping] & (0x80 >> i)){
                            GPIO_SetBits(GPIOC, GPIO_Pin_6);
                        }else{
                            GPIO_ResetBits(GPIOC, GPIO_Pin_6);
                        }
                        shift_clock();
                    }
                    current_mapping++;
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
