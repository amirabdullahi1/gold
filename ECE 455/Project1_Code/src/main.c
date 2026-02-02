/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include "stm32f4_discovery.h"
/* Kernel includes. */
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.c"
#include "stf32f4xx_adc.c"
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

void myADC1_Init()
{
	/* Enable clock for ADC1 peripheral */
    RCC_APB2PeriphClockCmd(RCC_APB2_Periph_ADC1, ENABLE);

    ADC_InitTypeDef adc1_init_struct;

    /* Initialize the ADC_Mode member */
    adc1_init_struct.ADC_Resolution = ADC_Resolution_12b;

    /* initialize the ADC_ScanConvMode member */
    adc1_init_struct.ADC_ScanConvMode = DISABLE;

    /* Initialize the ADC_ContinuousConvMode member */
    adc1_init_struct.ADC_ContinuousConvMode = ENABLE;

    /* Initialize the ADC_ExternalTrigConvEdge member */
    adc1_init_struct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;

    /* Initialize the ADC_DataAlign member */
    adc1_init_struct.ADC_DataAlign = ADC_DataAlign_Right;

    /* Initialize the ADC_NbrOfConversion member */
    adc1_init_struct.ADC_NbrOfConversion = 1;

    ADC_Init(ADC1, &adc1_init_struct);

    ADC_Cmd(ADC1, ENABLE);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_3Cycles);

    ADC_SoftwareStartConv(ADC1);
}

int main(void)
{
    /* Configure the system. The clock configuration can be
	done here if it was not done before main() was called. */
	prvSetupHardware();

    myADC1_Init();

    xTaskQueue_handle = xQueueCreate( 	
        taskQUEUE_LENGTH,		/* The number of items the queue can hold. */
        sizeof( uint16_t )      /* The size of each item the queue holds. */
    );
	/* Add to the registry, for the benefit of kernel aware debugging. */
    vQueueAddToRegistry( xQueue_handle, "taskQueue" );

	xFlowQueue_handle = xQueueCreate( 	
        flowQUEUE_LENGTH,		/* The number of items the queue can hold. */
        sizeof( uint16_t )      /* The size of each item the queue holds. */
    );	
	/* Add to the registry, for the benefit of kernel aware debugging. */
    vQueueAddToRegistry( xQueue_handle, "flowQueue" );

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
                
                xQueueOverwrite(xFlowQueue_handle, &ADC_val);
                rx_data = traffic_gen;

                if(xQueueSend(xTaskQueue_handle,&rx_data,1000))
                {
                    // printf("Flow Adjust GWP (%u).\n", rx_data); // Got wrong Package
                    vTaskDelay(pdMS_TO_TICKS(5));
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
