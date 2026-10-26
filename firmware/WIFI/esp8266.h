
#include "stm32f1xx_hal.h"


#ifndef _ESP8266_H_

#define _ESP8266_H_


extern unsigned char esp8266_buf[512];    // 接收缓存
extern unsigned short esp8266_cnt ;    // 接收计数
extern unsigned short esp8266_cntPre ; // 上一次接收计数（用于判断接收完成）


#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志


void ESP8266_Init(void);

void ESP8266_Clear(void);

_Bool ESP8266_SendCmd(char *cmd, char *res);

void ESP8266_SendData(unsigned char *data, unsigned short len);

unsigned char *ESP8266_GetIPD(unsigned short timeOut);

void UsartPrintf(UART_HandleTypeDef *huart, const char *fmt, ...);
void ESP8266_StartRecive(void);
#endif

