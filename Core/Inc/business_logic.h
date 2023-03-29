/*
 * business_logic.h
 *
 *  Created on: 14 мар. 2023 г.
 *      Author: maxus
 */

#ifndef INC_BUSINESS_LOGIC_H_
#define INC_BUSINESS_LOGIC_H_
#define OK 1
#define NOT_OK 0
void main_loop();
int status1Error(int code);
void status0OK();
void set_poff(float p);
void set_toff(float t);
float pmax(int mode);

typedef enum ERROR_CODE{
	FC00 = 0,
	FC10,
	FC11,
	FC21,
	FC22,
};

#endif /* INC_BUSINESS_LOGIC_H_ */
