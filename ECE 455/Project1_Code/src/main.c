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
// #define GLO_VAR 0
#define flow_adjust  	0
#define traffic_gen  	1
#define light_state  	2
#define sys_display  	3

#define flowQUEUE_LENGTH 1
#define taskQUEUE_LENGTH 4

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
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    /* Configured as an output */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;

    /* Set to Push Pull to ensure LEDs are visible and bright */
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; // Look into this a bit more, need to do testing to 100% know which one is the best to use.

    /* Currently turns of any internal resistors */
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; // Adjust through testing, not sure currently what setting is good for LEDs (If we have no external resistors will need to turn on).

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void myADC1_Init()
{
	ADC_InitTypeDef ADC_InitStruct;

//	/* Enable clock for ADC1 peripheral */
//    RCC_APB2PeriphClockCmd(RCC_APB2_Periph_ADC1, ENABLE);

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

    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_3Cycles);

    ADC_SoftwareStartConv(ADC1);
}

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

	myGPIO_LED_Init(); // Example code to turn LEDs on/off: ON: GPIO_SetBits(GPIOC, GPIO_Pin_0); OFF GPIO_ResetBits(GPIOC, GPIO_Pin_0);
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

	xTaskCreate( flow_adjust_task, "Flow Adjust", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
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
                if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
                    ADC_val = (ADC_GetConversionValue(ADC1) & 0x0FFF);
                	printf("ADC_val %u.\n", ADC_val);
                
                xQueueOverwrite(xFlowQueue_handle, &ADC_val);
                rx_data = flow_adjust; // traffic_gen;

                if(xQueueSend(xTaskQueue_handle,&rx_data,1000))
                {
                    // printf("Flow Adjust GWP (%u).\n", rx_data); // Got wrong Package
                    vTaskDelay(pdMS_TO_TICKS(2000));
                }
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
    while(1)
	{
        
    }
}

static void light_state_task ( void *pvParameters ) {
    while(1)
	{
        
    }
}

static void sys_display_task ( void *pvParameters ) {
    while(1)
	{
        
    }
}
