/*
 * sensor_manager.c
 *
 *  Created on: 17 окт. 2022 г.
 *      Author: maxusmax
 */
#include "sensor_manager.h"
#define VOLTAGE_HIGH_VALUE 3500


#define UP_FIRST_PIN() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET)
#define DOWN_FIRST_PIN() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET)

#define UP_SECOND_PIN() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_SET)
#define DOWN_SECOND_PIN() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_RESET)

uint8_t dma_flag = 0;
extern uint16_t adc_data[ADC_CHANNELS_NUM];

void get_result_sensor(result_sensor * result)
{
	result->result_sensor_1 = result->result_sensor_2 = result->result_sensor_3 = 0;

	for(int  i = 0; i < COUNT_SCAN_ADC; i++)
	{
	  wait_dma();
	  result->result_sensor_1 += get_charge(adc_data[SENSOR1], adc_data[VREFINT] );
	  result->result_sensor_2 += get_charge(adc_data[SENSOR2], adc_data[VREFINT] );
	  result->result_sensor_3 += get_charge(adc_data[SENSOR3], adc_data[VREFINT] );
	  dma_flag = 0;
	}

	result->result_sensor_1 /= COUNT_SCAN_ADC;
	result->result_sensor_2 /= COUNT_SCAN_ADC;
	result->result_sensor_3 /= COUNT_SCAN_ADC;

}

void wait_dma()
{
	while(1)
	{
		if(dma_flag)
		{
			break;
		}
	}
}


float get_charge(uint16_t result,uint16_t standart_voltage)
{

	return (VREF * result * standart_voltage ) / (ADC_RESOLUTION * VREFINT_CAL) * CAL_TO_CURRENT_VOLTAGE;
}



void show_result(result_sensor * result)
{
	DOWN_SECOND_PIN();
	DOWN_FIRST_PIN();

	if(result->result_sensor_1 > 20)
	{
		UP_FIRST_PIN();
	}
	if(result->result_sensor_2 > 20)
	{
		UP_SECOND_PIN();
	}
	if(result->result_sensor_3 > 20)
	{
		UP_FIRST_PIN();
		UP_SECOND_PIN();
	}

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	dma_flag = 1;

}

