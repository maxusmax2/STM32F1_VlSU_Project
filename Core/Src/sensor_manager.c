/*
 * sensor_manager.c
 *
 *  Created on: 17 окт. 2022 г.
 *      Author: maxusmax
 */
#include "sensor_manager.h"
#include "net_can.h"
#include "pcs.h"
#include "main.h"
#define VOLTAGE_HIGH_VALUE 20
#define VOLTAGE_LOW_VALUE 2.4

#define UP_FIRST_PIN() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET)
#define DOWN_FIRST_PIN() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET)

#define UP_SECOND_PIN() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_SET)
#define DOWN_SECOND_PIN() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_RESET)
static void wait_dma();
static float get_charge(uint16_t result, uint16_t standart_voltage);

uint8_t dma_flag = 0;
extern uint16_t adc_data[ADC_CHANNELS_NUM];
extern pcs_conf_t pcs_config;
pcs_resp_t *result;
result_sensor sensor_result;
syspkg_t req;
void get_result_sensor(result_sensor * result)
{
	result->result_sensor_1 = result->result_sensor_2 = 0;

	for(int  i = 0; i < COUNT_SCAN_ADC; i++)
	{
	  wait_dma();
	  result->result_sensor_1 += get_charge(adc_data[SENSOR1], adc_data[VREFINT] );
	  result->result_sensor_2 += get_charge(adc_data[SENSOR2], adc_data[VREFINT] );
	  dma_flag = 0;
	}

	result->result_sensor_1 /= COUNT_SCAN_ADC;
	result->result_sensor_2 /= COUNT_SCAN_ADC;

}

static void wait_dma()
{
	while(1)
	{
		if(dma_flag)
		{
			break;
		}
	}
}


static float get_charge(uint16_t result, uint16_t standart_voltage)
{
	return (VREF * result * standart_voltage) / (ADC_RESOLUTION * VREFINT_CAL) * CAL_TO_CURRENT_VOLTAGE;
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
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	dma_flag = 1;
}
int electronic_control_unit_OK()
{
	return CMD_OK;
}

int get_sensor_of_contour1()
{
	return HAL_GPIO_ReadPin(DK1_GPIO_Port,DK1_Pin);
}

int sensor_of_contour1_status()
{
	return CMD_OK;
}

int sensor_pressure1_status()
{
	get_result_sensor(&sensor_result);
	if(sensor_result.result_sensor_1 < VOLTAGE_LOW_VALUE)
	{
		return CMD_ERROR;
	}
	return CMD_OK;
}

int sensor_pressure2_status()
{
	get_result_sensor(&sensor_result);
	if(sensor_result.result_sensor_2 < VOLTAGE_LOW_VALUE)
	{
		return CMD_ERROR;
	}
	return CMD_OK;
}

float get_sensor_pressure_value_1()
{
	get_result_sensor(&sensor_result);
	return sensor_result.result_sensor_1;//todo формула перевода
}

float get_sensor_pressure_value_2()
{
	get_result_sensor(&sensor_result);
	return sensor_result.result_sensor_2;
}

float get_number_of_revolutions()
{
	req.cmd = CMD_GET_ICE_REVOLUTION;
	result = pcs_send_with_resp(&pcs_config, CAR_ID, &req,NULL, 0,CMD_OK, 100);
	if(result == NULL)
		return CMD_ERROR;
	return *((float*)result->data);
}

int get_engine_status()
{
	req.cmd = CMD_GET_ENGINE_STATUS;
	result = pcs_send_with_resp(&pcs_config, CAR_ID, &req,NULL, 0,CMD_OK, 100);
	if(result == NULL)
		return CMD_ERROR;
	return *((int*)result->data);
}

float get_speed_automobile()
{
	req.cmd = CMD_GET_CAR_SPEED;
	result = pcs_send_with_resp(&pcs_config, CAR_ID, &req,NULL, 0,CMD_OK, 100);
	if(result == NULL)
		return CMD_ERROR;
	return *((float*)result->data);
}

int get_mass_status()
{
	return 1;//todo
	req.cmd = CMD_GET_MASS_STATUS;
	result = pcs_send_with_resp(&pcs_config, CAR_ID, &req,NULL, 0,CMD_OK, 100);
	if(result == NULL)
		return CMD_ERROR;
	return *((int*)result->data);
}

int get_time()
{
	req.cmd = CMD_GET_CURRENT_TIME;
	result = pcs_send_with_resp(&pcs_config, CAR_ID, &req,NULL, 0,CMD_OK, 100);
	if(result == NULL)
		return CMD_ERROR;
	return *((int*)result->data);
}
