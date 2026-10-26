
#include "light.h"
#include "adc.h"
#include "dma.h"

uint16_t light_ADC_Value;


void light_ADC(void)
{
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&light_ADC_Value,1);
}

