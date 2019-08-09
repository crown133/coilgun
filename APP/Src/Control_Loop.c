#include "Control_Loop.h"
#include "Motor_Ctrl.h"
#include "Can_Ctrl.h"
#include "Pc_Uart.h"
#include "usart.h"
#include "delay.h"
#include "mode.h"
#include "sys.h"

#define Yaw_Limit(x, num)	( (x) < (num) ? (1) : (0) )


float pitchPos = 0;
float yawPos = 0;

float kf0 = 0.008, kf1 = 0.002, kf2 = 0.005, kf3 = 0.0008;
float sum = 0.005, sum1 = -0.009;

kalman_filter_t yaw_kalman_filter, pitch_kalman_filter;
kalman_filter_t yaw_velo_kf;

/*********************************/
void sysControl(void)
{
	{			
		/*************** 视觉相关 *************/
		{	
//			TD_Calculate(&tdYawPc, yawInc);
//			TD_Calculate(&tdPitchPc, pitchInc);
//				TD4_track4(&trackerYawInc, yawInc, 1.0f/200);
//				ESO_AngularRate_run(&eso2, yawInc, 1.0f/200);
//				eso2.z2 = 0;
//				ADRC_LESO(&eso1, yawInc);
			
//				kalman_filter_calc(&yaw_kalman_filter, yawInc, 0);
//				kalman_filter_calc(&pitch_kalman_filter, pitchInc, 0);
//				Motor_AbsPos(&motorYaw.posCtrl, -yaw_kalman_filter.filtered_value[0] * kf0 - tdYawPc.v2 * kf1 + motorYaw.veloCtrl.filrawVel * sum, 360, -360);
//				Motor_AbsPos(&motorPitch.posCtrl, -pitch_kalman_filter.filtered_value[0] * kf2 - tdPitchPc.v2 * kf3, 12, -40);
//			Motor_IncPos(&motorYaw.posCtrl, -yawInc * kf0 - tdYawPc.v2 * kf1 + motorYaw.veloCtrl.filrawVel * sum, 685, -685);
//			Motor_IncPos(&motorPitch.posCtrl, -pitchInc * kf2 - tdPitchPc.v2 * kf3 + motorPitch.veloCtrl.rawVel * sum1, 1000, -100);
		}
		
//	Motor_IncPos(&(motorYaw.posCtrl), 0, 690, -690);   //对应 +-30° 稍微留了一些余量
		fire_fire();
		Gimbal_Control();  //电机控制函数
	}
	
}
