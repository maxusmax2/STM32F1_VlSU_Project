/*
 * sensor_manager.h
 *
 *  Created on: 17 окт. 2022 г.
 *      Author: UUG009
 */
#include "adc.h"
#include "gpio.h"
#define COUNT_SCAN_ADC 10
#define ADC_CHANNELS_NUM 4
#define ADC_RESOLUTION 4096
#define VREFINT_CAL 1645.00
#define VREF 3.00

#define SENSOR1 0
#define SENSOR2 1
#define SENSOR3 2
#define VREFINT 3
typedef struct{
	float result_sensor_1;
	float result_sensor_2;
	float result_sensor_3;
}result_sensor;
void show_result(result_sensor * result);
void get_result_sensor(result_sensor * result);
float get_charge(uint16_t result,uint16_t standart_voltage);
