/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __adc_H
#define __adc_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "main.h"

#define    CH_Num 		   3
#define    ADC_SAMPLE_NUM  10

extern uint8_t CHL, CHR;

extern ADC_HandleTypeDef hadc1;

extern void _Error_Handler(char *, int);

extern void MX_ADC1_Init(void);
extern void ADC_Filter(uint32_t* adc_val);
extern void adc_get(void);
extern float voltage_get(void);


#ifdef __cplusplus
}
#endif
#endif /*__ adc_H */

