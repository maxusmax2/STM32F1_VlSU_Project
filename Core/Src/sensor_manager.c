/*
 * sensor_manager.c
 *
 *  Created on: 17 окт. 2022 г.
 *      Author: UUG009
 */
#include "sensor_manager.h"

uint8_t dma_flag = 0;
extern uint16_t adc_data[ADC_CHANNELS_NUM];
void get_result_sensor(result_sensor * result)
{
	result->result_sensor_1 = result->result_sensor_2 = result->result_sensor_3 = 0;
	for(int  i = 0; i < COUNT_SCAN_ADC; i++)
	{
	  wait_dma();
	  result->result_sensor_1 += adc_data[SENSOR1];//get_charge(adc_data[SENSOR1], adc_data[VREFINT] );
	  result->result_sensor_2 += adc_data[SENSOR2];//get_charge(adc_data[SENSOR2], adc_data[VREFINT] );
	  result->result_sensor_3 += adc_data[SENSOR3];//get_charge(adc_data[SENSOR3], adc_data[VREFINT] );
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

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	dma_flag = 1;
//	HAL_ADC_Stop_DMA(&hadc1);
}

float get_charge(uint16_t result,uint16_t standart_voltage)
{
	return (VREF * result * standart_voltage ) / (ADC_RESOLUTION * VREFINT_CAL);
}

void show_result(result_sensor * result)
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);

	if(result->result_sensor_1 > 3500)
	{
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
	}
	if(result->result_sensor_2 > 3500)
	{
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
	}
	if(result->result_sensor_3 > 3500)
	{
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
	}

}
