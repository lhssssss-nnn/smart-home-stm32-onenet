// HAL库头文件（替换原标准库头文件）
#include "stm32f1xx_hal.h"

// 网络设备驱动
#include "esp8266.h"
#include "app_config.h"

// 引入TIM延时函数（依赖delay_ms）
#include "tim.h"
#include "usart.h"
// C库（补充stdarg.h用于格式化打印）
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

//// 宏定义：接收状态标识（原代码缺失，补充）
//#define REV_OK    1
//#define REV_WAIT  0

// WiFi配置（更新为你的WiFi信息）
#define ESP8266_WIFI_INFO "AT+CWJAP=\"" APP_WIFI_SSID "\",\"" APP_WIFI_PASSWORD "\"\r\n"
// OneNET配置（注意：mqtts需8883端口，TCP用1883，需确认服务器）
#define ESP8266_ONENET_INFO "AT+CIPSTART=\"TCP\",\"" APP_ONENET_HOST "\"," APP_STRINGIFY(APP_ONENET_PORT) "\r\n"

// 全局变量（替换标准库串口为HAL句柄）
extern UART_HandleTypeDef huart2;  // ESP8266通信串口（USART2，CubeMX配置）
extern UART_HandleTypeDef huart1;  // 调试串口（USART1，用于打印日志）

 unsigned char esp8266_buf[512];    // 接收缓存
 unsigned short esp8266_cnt = 0;    // 接收计数
 unsigned short esp8266_cntPre = 0; // 上一次接收计数（用于判断接收完成）


//==========================================================
// 函数名称：  ESP8266_Clear
// 函数功能：  清空接收缓存
// 说明：      逻辑不变，直接保留
//==========================================================
void ESP8266_Clear(void)
{
    memset(esp8266_buf, 0, sizeof(esp8266_buf));
    esp8266_cnt = 0;
}

//==========================================================
// 函数名称：  ESP8266_WaitRecive
// 函数功能：  等待接收完成
// 返回参数：  REV_OK-接收完成    REV_WAIT-接收超时未完成
// 说明：      逻辑不变，保留
//==========================================================
_Bool ESP8266_WaitRecive(void)
{
    if (esp8266_cnt == 0)
        return REV_WAIT;
    
    if (esp8266_cnt == esp8266_cntPre)
    {
        // 新增：打印接收的所有数据，排查是否有乱码或错误响应
        //UsartPrintf(&huart1, "接收缓存内容：%s\r\n", esp8266_buf);
        esp8266_cnt = 0;
        return REV_OK;
    }
    
    esp8266_cntPre = esp8266_cnt;
    return REV_WAIT;
}

//==========================================================
// 函数名称：  UsartPrintf（新增）
// 函数功能：  HAL库格式化打印（替代原UsartPrintf，适配HAL串口句柄）
//==========================================================
void UsartPrintf(UART_HandleTypeDef *huart, const char *fmt, ...)
{
    char buf[512] = {0};  // 打印缓冲区（可根据需求增大）
    va_list args;
    
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);  // 格式化字符串
    va_end(args);
    
    // HAL库串口发送（阻塞模式，超时100ms）
    HAL_UART_Transmit(huart, (uint8_t *)buf, strlen(buf), 100);
}



//==========================================================
// 函数名称：  ESP8266_SendCmd
// 函数功能：  发送AT指令并等待响应
// 修改点：    替换Usart_SendString为HAL_UART_Transmit，适配打印接口
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
    unsigned char timeOut = 200;  // 总超时：200×10ms=2s
    ESP8266_Clear();              // 清空接收缓存
    
    // 替换：Usart_SendString(USART2, ...) → HAL_UART_Transmit(&huart2, ...)
    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, strlen(cmd), 100);
    
    while (timeOut--)
    {
        if (ESP8266_WaitRecive() == REV_OK)  // 接收完成
        {
            if (strstr((const char *)esp8266_buf, res) != NULL)  // 检索响应关键词
            {
                ESP8266_Clear();  // 清空缓存
                return 0;         // 成功
            }
        }
        delay_ms(10);  // 延时10ms（复用tim.c的ms延时函数）
    }
    
    // 替换：UsartPrintf(USART_DEBUG, ...) → UsartPrintf(&huart1, ...)
    //UsartPrintf(&huart1, "AT Cmd Fail: %s\r\n", cmd);
    return 1;  // 超时失败
}

//==========================================================
// 函数名称：  ESP8266_SendData
// 函数功能：  发送TCP/MQTT数据
// 修改点：    替换Usart_SendString为HAL_UART_Transmit
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{
    char cmdBuf[32];
    ESP8266_Clear();  // 清空接收缓存
    
    sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);  // 拼接发送长度指令
    if (!ESP8266_SendCmd(cmdBuf, ">"))          // 收到">"表示可发送数据
    {
        // 替换：Usart_SendString(USART2, ...) → HAL_UART_Transmit(&huart2, ...)
        HAL_UART_Transmit(&huart2, data, len, 100);
       // UsartPrintf(&huart1, "Send Data: %s\r\n", data);  // 打印发送日志
    }
}

//==========================================================
// 函数名称：  ESP8266_GetIPD
// 函数功能：  解析OneNET返回的IPD格式数据
// 修改点：    适配打印接口，替换延时函数
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{
    char *ptrIPD = NULL;
    
    do
    {
        if (ESP8266_WaitRecive() == REV_OK)  // 接收完成
        {
            ptrIPD = strstr((char *)esp8266_buf, "IPD,");  // 搜索"IPD,"头
            if (ptrIPD == NULL)
            {
                // 替换打印接口：USART_DEBUG → &huart1
                //UsartPrintf(&huart1, "\"IPD\" not found\r\n");
            }
            else
            {
                ptrIPD = strchr(ptrIPD, ':');  // 找到分隔符":"
                if (ptrIPD != NULL)
                {
                    ptrIPD++;  // 跳过":"，指向数据内容
                    return (unsigned char *)ptrIPD;
                }
                else
                    return NULL;
            }
        }
        delay_ms(5);  // 延时5ms（替换原delay_ms，复用HAL适配后的函数）
        timeOut--;
    } while (timeOut > 0);
    
    //UsartPrintf(&huart1, "Get IPD Timeout\r\n");  // 打印超时日志
    return NULL;
}

//==========================================================
// 函数名称：  ESP8266_Init
// 函数功能：  初始化ESP8266（连接WiFi+OneNET）
// 修改点：    适配打印接口，添加ESP8266上电稳定延时
//==========================================================
void ESP8266_Init(void)
{
    ESP8266_Clear();  // 清空缓存
    
    // 新增：ESP8266上电稳定延时（必须加！否则模块未就绪导致AT指令失败）
    delay_ms(3000);
    
    // 替换打印接口：USART_DEBUG → &huart1
    UsartPrintf(&huart1, "1. Test AT Command\r\n");
    while (ESP8266_SendCmd("AT\r\n", "OK"))
        delay_ms(500);
    
    UsartPrintf(&huart1, "2. Set CWMODE=1\r\n");
    while (ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
        delay_ms(500);
    
    UsartPrintf(&huart1, "3. Enable DHCP\r\n");
    while (ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
        delay_ms(500);
    
    UsartPrintf(&huart1, "4. Connect WiFi\r\n");
    while (ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
        delay_ms(500);
    
    UsartPrintf(&huart1, "5. Connect OneNET\r\n");
    while (ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
        delay_ms(500);
    
    UsartPrintf(&huart1, "6. ESP8266 Init OK\r\n");
}

//==========================================================
// 函数名称：  HAL_UART_RxCpltCallback（新增）
// 函数功能：  USART2接收中断回调（替代原标准库USART2_IRQHandler）
// 说明：      HAL库中断处理核心，需手动重启中断接收
//==========================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)  // 仅处理ESP8266通信串口（USART2）
    {
        // 防止缓存溢出（留1字节存结束符）
        if (esp8266_cnt < sizeof(esp8266_buf) - 1)
        {
            // 读取接收数据（HAL库通过DR寄存器获取数据）
            esp8266_buf[esp8266_cnt++] = (uint8_t)huart->Instance->DR;
        }
        
        // 关键：重新开启中断接收（HAL库仅触发1次，需手动重启）
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&esp8266_buf[esp8266_cnt], 1);
    }
}

//==========================================================
// 函数名称：  ESP8266_StartRecive（新增）
// 函数功能：  开启ESP8266接收中断（必须在main函数中调用）
// 说明：      HAL库串口中断需手动初始化开启
//==========================================================
void ESP8266_StartRecive(void)
{
    // 开启USART2中断接收（每次接收1字节，触发回调函数）
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&esp8266_buf[esp8266_cnt], 1);
}
