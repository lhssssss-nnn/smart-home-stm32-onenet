#ifndef __MOTOR_H 
#define __MOTOR_H 

#include <stdint.h>  // 关键：包含int8_t的定义头文件

void Motor_SetSpeed(int16_t Speed);

void Motor_Stop(void);

#endif

