#include "BUZZER.h"


void BUZZER_ON ()
{
 
 HAL_GPIO_WritePin(GPIOA,  GPIO_PIN_15, GPIO_PIN_RESET);
 
}

void BUZZER_OFF ()
{
 
 HAL_GPIO_WritePin(GPIOA,  GPIO_PIN_15, GPIO_PIN_SET);
 
}







