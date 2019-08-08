#include "Can_Ctrl.h"
#include "Motor_Ctrl.h"

uint8_t init_flag = 0;

/**
  *	@brief	receive callback function
  *	@param	hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  *	@retval	None
  */
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
	//�����CAN1
	
	{
		if (hcan == &hcan1)
		{
			if(hcan->pRxMsg->IDE == CAN_ID_STD && hcan->pRxMsg->RTR == CAN_RTR_DATA)
			{
				
				switch (hcan->pRxMsg->StdId)
				{
					//CAN1,��ͷYaw����
					case 0x205:
					{
						CAN_MotorRxMsgConv(hcan, &motorYaw);
					}break;
				
				 //CAN1,��ͷPitch����
					 case 0x206:
					{
						CAN_MotorRxMsgConv(hcan, &motorPitch);
					}break; 
				default:
					{
						
					}break;
				}
			}
			__HAL_CAN_ENABLE_IT(hcan, CAN_IT_FMP0);		//���´�CAN�ж�
		}
	}
}

/**
  *	@brief	ת�����͸�Motor�����ݸ�ʽ 
  *	@param	hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  *	@retval	None
  */
float temperature = 0;  //GM6020�����ʾ�¶�

void CAN_MotorRxMsgConv(CAN_HandleTypeDef *hcan, Motor_t *motor)
{
	motor->posCtrl.rawPosLast = motor->posCtrl.rawPos;
	motor->posCtrl.rawPos = (int16_t)(hcan->pRxMsg->Data[0] << 8 | hcan->pRxMsg->Data[1]);  //ʵ��λ��
	if(init_flag)
	{
		if((motor->posCtrl.rawPos - motor->posCtrl.rawPosLast) > 5000) 
		{
			motor->posCtrl.round--;
		}
		else if((motor->posCtrl.rawPos - motor->posCtrl.rawPosLast) < -5000)
		{
			motor->posCtrl.round++;
		}
	}
	init_flag = 1;
	motor->posCtrl.relaPos = motor->posCtrl.round*8192 + motor->posCtrl.rawPos;
	
	motor->veloCtrl.rawVel = (int16_t)(hcan->pRxMsg->Data[2] << 8 | hcan->pRxMsg->Data[3]);//ʵ���ٶ�
	motor->posCtrl.motorPos = (int16_t)(hcan->pRxMsg->Data[0] << 8 | hcan->pRxMsg->Data[1]);  //ʵ��λ��
	
	motor->torque = (int16_t)(hcan->pRxMsg->Data[4] << 8 | hcan->pRxMsg->Data[5]);  //ʵ��ת��
	
	temperature = hcan->pRxMsg->Data[6]; //�¶�
}


/**
	*	@brief	ת�����͸�Motor�����ݸ�ʽ
	*	@param	ƫ�����ֵ 
	*	@param	�������ֵ
	*	@param	�������ֵ 
	*	@param	���� 
	*	@retval None
	*/
uint8_t CAN_CMD_GIMBAL(int16_t Yaw, int16_t Pitch, int16_t Bodan, int16_t rev)
{
	hcan1.pTxMsg->StdId = SECOND_FOUR_ID;
	hcan1.pTxMsg->ExtId = SECOND_FOUR_ID;
	hcan1.pTxMsg->IDE = CAN_ID_STD;		//��׼��ʽ
	hcan1.pTxMsg->RTR = CAN_RTR_DATA;	//����֡
	hcan1.pTxMsg->DLC = DLC_LEN;
	
	
	hcan1.pTxMsg->Data[0] = (uint8_t)(Yaw >> 8);
	hcan1.pTxMsg->Data[1] = (uint8_t)Yaw;
	
	hcan1.pTxMsg->Data[2] = (uint8_t)(Pitch >> 8);
	hcan1.pTxMsg->Data[3] = (uint8_t)Pitch;
	
	hcan1.pTxMsg->Data[4] = (uint8_t)(Bodan >> 8);
	hcan1.pTxMsg->Data[5] = (uint8_t)Bodan;
	
	hcan1.pTxMsg->Data[6] = (uint8_t)(rev >> 8);
	hcan1.pTxMsg->Data[7] = (uint8_t)rev;
	
	if(HAL_CAN_Transmit(&hcan1, 0) != HAL_OK)
	return 0;
	
	return 1;
}
