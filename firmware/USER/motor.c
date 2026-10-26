
#include "motor.h"
#include "tim.h"
#include "gpio.h"
/**
  * 函    数：直流电机设置速度
  * 参    数：Speed 要设置的速度，范围：-999~999
  * 返 回 值：无
  */
void Motor_SetSpeed(int16_t Speed)
{
	if (Speed >= 0)							//如果设置正转的速度值
	{
		HAL_GPIO_WritePin(GPIOB, MOTOR1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, MOTOR2_Pin, GPIO_PIN_SET);//0 1 正转
		__HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_3,Speed);				//PWM设置为速度值
	}
	else									//否则，即设置反转的速度值
	{
		HAL_GPIO_WritePin(GPIOB, MOTOR1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, MOTOR2_Pin, GPIO_PIN_RESET);//1 0  反转
		__HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_3,-Speed);				//PWM设置为速度值
	}
}

void Motor_Stop()
{
	//__HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_1,0);				//PWM设置为速度值
	HAL_GPIO_WritePin(GPIOB, MOTOR1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, MOTOR2_Pin, GPIO_PIN_RESET);  // 0 0 → 刹车停止
}
