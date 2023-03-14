/*
 * business_logic.h
 *
 *  Created on: 14 мар. 2023 г.
 *      Author: maxus
 */

#ifndef INC_BUSINESS_LOGIC_H_
#define INC_BUSINESS_LOGIC_H_
#define OK 1
#define NOT_OK;
void main_loop();
int status1Error(int code);
int electronic_control_unit_OK();
int sensor_pressure1();
int sensor_pressure2();
void status0OK();


typedef enum ERROR_CODE{
	FC00 = 0,
	FC10,
	FC11,
	FC21,
	FC22,
};

#endif /* INC_BUSINESS_LOGIC_H_ */
