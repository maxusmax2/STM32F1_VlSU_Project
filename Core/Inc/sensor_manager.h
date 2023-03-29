/*
 * sensor_manager.h
 *
 *  Created on: 17 окт. 2022 г.
 *      Author: UUG009
 */
#include "adc.h"
#include "gpio.h"
#define COUNT_SCAN_ADC 10
#define VREF 3.3
#define ADC_CHANNELS_NUM 4
#define ADC_RESOLUTION 4096
#define VREFINT_CAL 1489
#define CAL_TO_CURRENT_VOLTAGE 24/VREF

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
int electronic_control_unit_OK();
int sensor_of_contour1();
int sensor_pressure1();
int sensor_pressure2();
float get_number_of_revolutions();
float get_sensor_pressure_value_1();
float get_sensor_pressure_value_2();
int get_engine_status();
float get_speed_automobile();
int get_mass_status();
int get_time();
