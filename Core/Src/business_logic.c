/*
 * business_logic.c
 *
 *  Created on: 14 мар. 2023 г.
 *      Author: maxus
 */
#include "business_logic.h"
#include "sensor_manager.h"

#define CRITICAL_PRESSURE 2.9
#define SPEED_THRESHOLD 40
#define DP0 500
#define PMAX1 2500
#define PMAX2 2750
#define VRR 7
int error_code = 0;
float vn =0;//Tекущее значение объема накачанного воздуха с момента начала накачки
float vns = 0;//Суммарное значение объема накачанного воздуха с момента начала накачки
float qk = 0;//Производительность компрессора
float kr = 0;//Производительность
float vrs = 0;//Суммарное  значение объема воздуха для регенерации фильтра-осушителя
float p = 0;//
int m1 = 0;//Включение электромагнитного клапана
int r = 0;//Маркер завершения расчета

void main_loop(){
	while(1){
		if(electronic_control_unit_OK() == NOT_OK){
			error_code = FC10;
		}
		else if (sensor_of_contour1() == NOT_OK){
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
			break;
		}
		error_code = FC00;
		status0OK();
	}
}

int status1Error(int code){
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

int speed_mode = 0;
void status0OK(){
	for (int i = 0;;i++){
		if (!get_mass_status){
			set_poff(p);
			set_toff(get_time());
			break;
		}
		vn = qk * get_number_of_revolutions();
		vr = kr * vn;
		vrs += vr;
		p = max(get_sensor_pressure_value_1(),get_sensor_pressure_value_2());

		if(p < CRITICAL_PRESSURE){
			status1Error(FC11);
		}

		if(get_engine_status() == 0){
			continue;
		}

		if(get_speed_automobile < SPEED_THRESHOLD){
			speed_mode = 1;
		}else{
			speed_mode = 2;
		}
		if( p < pmax(speed_mode) - DP0){

			m1 = 0;
			r = 0;
			continue;
		}
		if( p < pmax(speed_mode)){
			continue;
		}

		m1 = 1;
		if(r > 0 ){
			continue;
		}

		vrs -= VRR;
		r = 1;

		if(!(vrs > VRR)){
			continue;
		}
		if( a = 2){
			status3(FC31);
		}
	}
}
void status3(error_code){}
float max(float dd1,float dd2){
	if(dd1>=dd2){
		return dd1;
	}else{
		return dd2;
	}
}
float pmax(int mode){
	if(mode == 1){
		return PMAX1;
	}else{
		return PMAX2;
	}

}
void set_poff(float p){}
void set_toff(float t){}
