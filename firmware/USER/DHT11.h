#ifndef __DHT11_H__
#define __DHT11_H__
#include "main.h"

// ********** 新增：DHT11数据结构体（分离整数和小数部分）**********
typedef struct {
    uint8_t humi_int;    // 湿度整数部分 (0-99)
    uint8_t humi_dec;    // 湿度小数部分 (0-9)
    uint8_t temp_int;    // 温度整数部分 (0-60)
    uint8_t temp_dec;    // 温度小数部分 (0-9)
} DHT11_DataTypedef;

// DQ引脚操作宏定义（PA6）
#define DHT11_DQ_OUT_HIGH  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET)
#define DHT11_DQ_OUT_LOW   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET)
#define DHT11_DQ_IN        HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)

// IO方向设置（修正原有DS18B20的笔误）
void DHT11_IO_IN(void);
void DHT11_IO_OUT(void);

// 函数声明（修改Read_Data函数参数为结构体指针）
uint8_t DHT11_Init(void);                      // 初始化DHT11并检测存在性
uint8_t DHT11_Read_Data(DHT11_DataTypedef *data); // 读取数据到结构体
uint8_t DHT11_Read_Byte(void);                 // 读取一个字节
uint8_t DHT11_Read_Bit(void);                  // 读取一个位
uint8_t DHT11_Check(void);                     // 检测DHT11存在性
void DHT11_Rst(void);                          // 复位DHT11

#endif

