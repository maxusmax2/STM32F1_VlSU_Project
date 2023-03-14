/*
 * business_logic.c
 *
 *  Created on: 14 мар. 2023 г.
 *      Author: maxus
 */
#include "business_logic.h"
int error_code = 0;
void main_loop(){
	while(1){
		if(electronic_control_unit_OK() == NOT_OK){
			error_code = FC10;
		}
		else if (sensor_of_contour1()== NOT_OK){
			error_code = FC11;
		}
		else if (sensor_pressure1() == NOT_OK){
			error_code = FC21;
		}
		else if (sensor_pressure2() == NOT_OK){
			error_code = FC22;
		}
		if(error_code != 0){
			status1Error(error_code);
		}
		error_code = FC00;
		status0OK();
	}
}

void status1Error(int code){
	switch(code){
	case FC10:
		break;
	case FC11:
		break;
	case FC21:
		break;
	case FC22:
		break;
	}
}

int electronic_control_unit_OK(){
	return NOT_OK;
}

int sensor_of_contour1(){
	return OK;
}

int sensor_pressure1(){

}

int sensor_pressure2(){

}
void status0OK(){

}
