#include "dht11.h"
#include "tim.h"

// 设置DQ为输入模式（优化：添加浮空配置，避免误触发）
void DHT11_IO_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE(); // 使能GPIOA时钟（确保初始化成功）
    
    GPIO_InitStructure.Pin = GPIO_PIN_6;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_NOPULL; // 无上下拉，符合DHT11通信要求
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// 设置DQ为输出模式
void DHT11_IO_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE(); // 使能GPIOA时钟
    
    GPIO_InitStructure.Pin = GPIO_PIN_6;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// 复位DHT11
void DHT11_Rst(void)	   
{                 
    DHT11_IO_OUT(); 	// 设置为输出
    DHT11_DQ_OUT_LOW; 	// 拉低DQ引脚
    HAL_Delay(20);    	// 拉低至少18ms（满足DHT11协议要求）
    DHT11_DQ_OUT_HIGH; 	// 释放DQ引脚（拉高）
    delay_us(30);     	// 主机拉高20~40us，等待DHT11响应
}

// 等待DHT11的回应
// 返回1: 未检测到DHT11存在  |  返回0: 检测到DHT11存在
uint8_t DHT11_Check(void) 	   
{   
    uint8_t retry = 0;
    DHT11_IO_IN();      // 切换为输入模式，等待DHT11响应
    
    // DHT11会先拉低DQ 40~80us
    while (DHT11_DQ_IN && retry < 100)
    {
        retry++;
        delay_us(1);
    };	 
    if (retry >= 100) return 1; // 超时未检测到低电平，无响应
    
    retry = 0;
    // DHT11拉低后会再次拉高40~80us
    while (!DHT11_DQ_IN && retry < 100)
    {
        retry++;
        delay_us(1);
    }
    if (retry >= 100) return 1;	// 超时未检测到高电平，响应异常
    
    return 0; // 检测到正常响应，DHT11存在
}

// 从DHT11读取一个位
// 返回值：1/0（数据位）
uint8_t DHT11_Read_Bit(void) 			 
{
    uint8_t retry = 0;
    // 等待DQ从高变低（数据位起始信号）
    while (DHT11_DQ_IN && retry < 100)
    {
        retry++;
        delay_us(1);
    }
    retry = 0;
    // 等待DQ从低变高（数据位有效信号）
    while (!DHT11_DQ_IN && retry < 100)
    {
        retry++;
        delay_us(1);
    }
    delay_us(40); // 等待40us（关键：DHT11低电平=0，高电平=1的判断阈值）
    if (DHT11_DQ_IN) return 1; // 40us后仍为高，说明是数据1
    else return 0;             // 40us后为低，说明是数据0
}

// 从DHT11读取一个字节（高位在前）
// 返回值：读到的字节数据
uint8_t DHT11_Read_Byte(void)    
{        
    uint8_t i, dat = 0;
    for (i = 0; i < 8; i++) 
    {
        dat <<= 1; // 左移一位，准备接收下一位（高位在前）
        dat |= DHT11_Read_Bit(); // 读取当前位并写入最低位
    }						    
    return dat;
}

// ********** 核心修改：读取数据到结构体（分离整数/小数）**********
// 参数：data - 结构体指针，用于存储温湿度整数/小数部分
// 返回值：0-读取成功  |  1-读取失败（无响应或校验错误）
uint8_t DHT11_Read_Data(DHT11_DataTypedef *data)    
{        
    uint8_t buf[5]; // 存储DHT11的5字节数据（湿整+湿小+温整+温小+校验和）
    uint8_t i;
    
    // 检查结构体指针是否有效（避免空指针错误）
    if (data == NULL) return 1;
    
    DHT11_Rst();          // 复位DHT11
    if (DHT11_Check() == 0) // 检测到DHT11响应
    {
        for (i = 0; i < 5; i++) // 读取40位数据（5个字节）
        {
            buf[i] = DHT11_Read_Byte();
        }
        
        // 数据校验：前4字节累加和 == 第5字节校验位
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            // 直接赋值到结构体（分离整数和小数部分）
            data->humi_int = buf[0];  // 湿度整数（buf[0]：0-99）
            data->humi_dec = buf[1];  // 湿度小数（buf[1]：0-9，DHT11通常为0）
            data->temp_int = buf[2];  // 温度整数（buf[2]：0-60）
            data->temp_dec = buf[3];  // 温度小数（buf[3]：0-9，DHT11通常为0）
        }
        else
        {
            return 1; // 数据校验失败
        }
    }
    else
    {
        return 1; // DHT11无响应
    }
    
    return 0; // 读取成功
}

// 初始化DHT11的IO口，同时检测DHT11存在性
// 返回1: 不存在  |  返回0: 存在
uint8_t DHT11_Init(void)
{ 
    DHT11_Rst();   // 复位DHT11
    return DHT11_Check(); // 检测存在性并返回结果
}
