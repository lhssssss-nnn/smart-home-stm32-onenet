#include "LED.h"

void LED_ON ()
{
 
 HAL_GPIO_WritePin(GPIOA,  GPIO_PIN_12, GPIO_PIN_RESET);
 
}

void LED_OFF ()
{
 
 HAL_GPIO_WritePin(GPIOA,  GPIO_PIN_12, GPIO_PIN_SET);
 
}










