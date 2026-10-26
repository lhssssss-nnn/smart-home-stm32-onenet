/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "stdio.h"
#include "string.h"
#include "DHT11.h"
#include "light.h"
#include "motor.h"
#include "BUZZER.h"
#include "esp8266.h"
#include "onenet.h"
#include "app_config.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

int fputc(int ch,FILE *f)
{
    HAL_UART_Transmit(&huart1,(uint8_t *) &ch,1,HAL_MAX_DELAY);
    return ch;    
}
int fgetc(FILE *f)
{
    uint8_t ch;
    HAL_UART_Receive(&huart1,(uint8_t *)&ch,1,HAL_MAX_DELAY);
    return ch;
}

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Product and device identifiers are defined in the local configuration.
const char devPubTopic[] = "$sys/" APP_ONENET_PRODUCT_ID "/" APP_ONENET_DEVICE_NAME "/thing/property/post";
const char *devSubTopic[] = {"$sys/" APP_ONENET_PRODUCT_ID "/" APP_ONENET_DEVICE_NAME "/thing/property/set"};
unsigned char *dataPtr = NULL;



uint8_t flag1;//定时器标志
uint16_t TimeCount;//定时器计数

float light,light_max=90.0;//光强
DHT11_DataTypedef DHT11_Data;//温湿度
uint8_t temp_int_max=25,humi_int_max=70;
uint8_t Alarm_flag=0;//小程序控制报警器的开关

uint8_t feng=0;
char PUBLIS_BUF[512];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void JsonValue()
{
	uint8_t Temp = DHT11_Data.temp_int;
	uint8_t Hum = DHT11_Data.humi_int;
	
	memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
	
	sprintf(PUBLIS_BUF,"{\"id\":\"123\",\"params\":{\"W\":{\"value\":%d},\"S\":{\"value\":%d},\"G\":{\"value\":%d}}}",
					DHT11_Data.temp_int,DHT11_Data.humi_int,(int)light);	
}


void Alarm_Statue()
{
	if(Alarm_flag)
	{
		if(DHT11_Data.temp_int > temp_int_max || DHT11_Data.humi_int > humi_int_max || light > light_max)
		{
			BUZZER_ON();
		}
		else
		{
			BUZZER_OFF();
		}
	
	}
	else
	{
		BUZZER_OFF();
	}

}
	
void feng_Statue()
{
	switch(feng)
	{
		case 0:
      Motor_Stop();
			break;
		case 1:
      Motor_SetSpeed(300);//一档速度
			break;
		case 2:
      Motor_SetSpeed(500);//二档速度
			break;
		case 3:
      Motor_SetSpeed(750);//三档速度
			break;
	}
}
	
	
	
	
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	OLED_Init();
	OLED_Clear();
	light_ADC();
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);

	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);
	
printf("网络链接中!!!\r\n");
ESP8266_StartRecive();// 开启串口中断
ESP8266_Init();

	while(OneNet_DevLink())
	{
		delay_ms(500);
	}
	
	/*订阅Topic主题*/
	OneNet_Subscribe(devSubTopic, 1);		
	delay_ms(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		light = (4096.0f - (float)light_ADC_Value) / 4096.0f * 100.0f;
		DHT11_Read_Data(&DHT11_Data);
		Alarm_Statue();
		feng_Statue();
/*****************************定时上传数据到云平台**********************************************/
		if(flag1)
		{
			JsonValue();
			OneNet_Publish(devPubTopic, PUBLIS_BUF);
			ESP8266_Clear();
			TimeCount = 0;
			flag1=0;
		}					
		
/*****************************接收检测云平台发送过来的数据**********************************************/
			
		dataPtr = ESP8266_GetIPD(2);
		if(dataPtr != NULL)
		{ 
		 OneNet_RevPro(dataPtr);			
		}	
			
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef*hadc)
//{
//	flag1=1;
//}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// 判断是否是 TIM3 触发的中断
    if (htim->Instance == TIM3)
    {
			if(++TimeCount >= 40)
			{
				flag1=1;
			}
			
		}
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
