/* USER CODE BEGIN Includes */
#include "tim.h"
#include "can.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"
#include "adc.h"

#include "sys.h"
#include "delay.h"

#include "Pc_Uart.h"
#include "Control_Loop.h"
#include "oled.h"
#include "Motor_Ctrl.h"
#include "mode.h"

#include "main.h"

#include "adrc.h"
#include "kalman_filter.h"
/*---------------------------------------------------------*/

void SystemClock_Config(void);

extern kalman_filter_t yaw_kalman_filter, pitch_kalman_filter;
extern kalman_filter_t yaw_velo_kf;

int8_t key = -1;
uint8_t gimbal_init_flag = 1;  //��̨λ�ó�ʼ��

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  SystemClock_Config();
  delay_init(168);
	
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  
  gram_clear(Pen_Clear);
  MX_SPI1_Init();  //OLED
  oled_init();    //OLED	
  oled_LOGO();
	
  MX_DMA_Init();
  MX_USART1_UART_Init();  

  MX_TIM3_Init();  		//��ʱ��3�жϣ��ܿ����ж�
  
  MX_CAN1_Init();
  CANFilter_Init(&hcan1);

  MX_ADC1_Init();  //ҡ��ֵ�����ݵ�ѹֵ
  
  SysNVIC_SetPriority();  //ͳһ�ڴ��������ȼ�
  delay_ms(100);

  Motor_IncPos(&(motorPitch.posCtrl), 6800, 6800, -100); //��ʼλ��
  Motor_IncPos(&(motorYaw.posCtrl), 690, 6800, -685); 
  /*********** Motor init position ***********/

  
  /********** motor control value set **********/  
  
  Motor_ValueSet(&motorPitch, 30, 0.2, 0, 25000, -25000, 2.5, \
		0, 0.5, 900, -900, DISABLE);  				 //Pitch����
		
//  Motor_ValueSet(&motorYaw, 5, 0.1, 0, 20000, -20000, 2, \
//		0, 0.02, 600, -600, DISABLE);  //Yaw����
  
  Motor_ValueSet(&motorYaw, 30, 0.2, 0, 20000, -20000, 2.5, \
		0, 1, 600, -600, DISABLE);  //Yaw����
/********** Various Filters Init *************/
  TD_Init(&td1, 800, 0.06, 10);
  TD_Init(&td2, 500, 0.05, 2);
  TD_Init(&td1_velo, 150, 0.05, 0.005);
  TD_Init(&td2_pos, 150, 0.05, 0.005);
  TD_Init(&td2_velo, 1500, 0.005, 15);
  TD_Init(&tdYawPc, 100, 0.005, 0.005);
  TD_Init(&tdPitchPc, 100, 0.005, 0.005);
  
  LESO_Init(&eso1, 0.035, 15, 75, 125, 0.015);
  TD4_init(&trackerYawInc, 15, 15, 15, 15);
  TD4_init(&trackerPitchInc, 15, 15, 15, 15);
  TD4_init(&trackerYaw, 15, 15, 15, 15);
  TD4_init(&trackerPitch, 15, 15, 15, 15);
  init_ESO_AngularRateYaw(&eso2, 0, 0, 0, 0.5f, 6);
  
  kalman_filter_init_t yaw_kalman_filter_para = {
						  .P_data = {2, 0, 0, 2},
						  .A_data = {1, 0.001, 0, 1},
						  .H_data = {1, 0, 0, 1},
						  .Q_data = {1, 0, 0, 1},
						  .R_data = {1000, 0, 0, 0}
						};
  kalman_filter_init_t pitch_kalman_filter_para = {
						  .P_data = {2, 0, 0, 2},
						  .A_data = {1, 0.001, 0, 1},
						  .H_data = {1, 0, 0, 1},
						  .Q_data = {1, 0, 0, 1},
						  .R_data = {8000, 0, 0, 0}
						};
  kalman_filter_init(&yaw_kalman_filter, &yaw_kalman_filter_para);
  kalman_filter_init(&pitch_kalman_filter, &pitch_kalman_filter_para);
  kalman_filter_init(&yaw_velo_kf, &pitch_kalman_filter_para);
						
/***********************************************/
  delay_ms(100);
  
  HAL_TIM_Base_Start_IT(&htim3);  //�����ж�
  
  delay_ms(100);
  PBout(9) = 1;	  //led light
						
  oled_clear(Pen_Clear);		
  menu_init();

  adc_get();
  delay_ms(500);
  while (1)
  {
	if((abs(motorYaw.posCtrl.rawPos - motorYaw.posCtrl.refPos) < 50) && (abs(motorPitch.posCtrl.rawPos - motorPitch.posCtrl.refPos) < 50) && gimbal_init_flag) //���ģʽ�л�
	{
		gimbal_init_flag = 0;
		motorPitch.posCtrl.motorBias = motorPitch.posCtrl.rawPos;
		motorYaw.posCtrl.motorBias = motorYaw.posCtrl.rawPos;
		  
		Motor_IncPos(&(motorPitch.posCtrl), 0, 1000, -100); //��ʼλ��
		Motor_IncPos(&(motorYaw.posCtrl), 0, 690, -690);   //��Ӧ +-30�� ��΢����һЩ����
		motorPitch.posCtrl.refPos = 0;
		motorYaw.posCtrl.refPos = 0;		
	}
	
	adc_get();  //��ȡҡ��ֵ 0 1 2
	item_selection();
	mode_change();
	  

//	PEout(2) = 1;  //�̵�����



  	oled_refresh_gram();  //OLED 
  }
  
}

/********** ϵͳʱ������ **********/
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)25Mhz
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 12000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

