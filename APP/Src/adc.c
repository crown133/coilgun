/* Includes ------------------------------------------------------------------*/
#include "adc.h"

#include "gpio.h"
#include "dma.h"


ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

uint32_t adc_buf[CH_Num*ADC_SAMPLE_NUM];


uint32_t adc_val[3];

uint16_t CHL_flag = 0;
uint16_t CHR_flag = 0;

uint8_t CHL, CHR;

/* ADC1 init function */
void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 2;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 3;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;

  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

   HAL_ADC_Stop_DMA(&hadc1);
   HAL_ADC_Start_DMA(&hadc1, adc_buf, CH_Num*ADC_SAMPLE_NUM);    //开启第一次DMA传送
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(adcHandle->Instance==ADC1)
  {

    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();
  
    /**ADC1 GPIO Configuration    
    PC2     ------> ADC1_IN12
    PB0     ------> ADC1_IN8
    PB1     ------> ADC1_IN9 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();
  
    /**ADC1 GPIO Configuration    
    PC2     ------> ADC1_IN12
    PB0     ------> ADC1_IN8
    PB1     ------> ADC1_IN9 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0|GPIO_PIN_1);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);

    /* ADC1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(ADC_IRQn);

  }
} 

//ADC均值滤波
void ADC_Filter(uint32_t* adc_val)
{
	uint16_t i;
	uint32_t sum[CH_Num]={0};
	
	for(i=0; i<ADC_SAMPLE_NUM; i++)
	{
		sum[0]+=adc_buf[CH_Num*i+0];
		sum[1]+=adc_buf[CH_Num*i+1];
		sum[2]+=adc_buf[CH_Num*i+2];

	}
	adc_val[0]=sum[0]/ADC_SAMPLE_NUM;  //CHL
	adc_val[1]=sum[1]/ADC_SAMPLE_NUM;  //CHR
	adc_val[2]=sum[2]/ADC_SAMPLE_NUM;  //Voltage
}

void adc_get(void)
{
	ADC_Filter(adc_val);
	{
		{
			if(!CHL_flag)
			{
				if(adc_val[2] > 3000)
					CHL = 1;
				else if(adc_val[2] < 1000)
					CHL = 2;
				else 
					CHL = 0;
			}
			if(CHL != 0)
			{
				CHL_flag ++;
			}
			if(CHL_flag)
			{
				if(++CHL_flag > 40)
					CHL_flag = 0;
			}
		}
		
		{
			if(!CHR_flag)
			{
				if(adc_val[1] > 3000)
					CHR = 1;
				else if(adc_val[1] < 1000)
					CHR = 2;
				else 
					CHR = 0; 
			}
			if(CHR != 0)
			{
				CHR_flag ++;
			}
			if(CHR_flag)
			{
				if(++CHR_flag > 40)
					CHR_flag = 0;
			}
		}
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
