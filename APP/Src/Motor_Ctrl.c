#include "Motor_Ctrl.h"
#include "usart.h"
#include "Can_Ctrl.h"

#include "math.h"

#define limit_output(x, min, max)	( (x) <= (min) ? (min) : (x) >= (max) ? (max) : (x) )//限幅函数

/**********************************************/
TD td1, td2, td1_velo, td2_pos, td2_velo;; 		  //速度环参考值使用跟踪微分器安排过渡过程
TD tdYawPc, tdPitchPc;//速度反馈值使用跟踪微分器进行微分
ESO eso1;
ESO_AngularRate eso2;

ADRC_Data ADRC_Yaw;   //Yaw 电机 adrc控制体
TD4 trackerYawInc, trackerPitchInc, trackerYaw, trackerPitch;

/*********** 无人机云台中需要控制的电机 ************/
Motor_t motorYaw, motorPitch;

/************************************************/
static void motorYawCalcuPos(void);		//Yaw 电机位置环pid计算
static void motorPitchCalcuPos(void);   //Pitch 电机位置环pid计算
static void Motor_VeloCtrl(VeloPidCtrl_t *vel_t);  //速度环pid计算
static inline void MotorFliter_VeloCtrl(VeloPidCtrl_t *vel_t);
static inline void motor_pitch_veloCtrl(void);


//云台电机计算控制函数
void Gimbal_Control(void)
{
	/*	0x205  yaw */
	motorYawCalcuPos(); 				//位置pid计算
//	TD_Calculate(&td1, motorYaw.posCtrl.output);
	motorYaw.veloCtrl.refVel = motorYaw.posCtrl.output;
	Motor_VeloCtrl(&motorYaw.veloCtrl); //速度pid计算
	
//	ADRC_Control(&ADRC_Yaw, kp, motorYaw.veloCtrl.rawVel);
//	ADRC_Yaw.u = limit_float(ADRC_Yaw.u, -16000, 16000);
	
	/*  pitch	*/
	motorPitchCalcuPos();  				  //位置pid计算
	
//	TD_Calculate(&td2, motorPitch.posCtrl.output);	
//	motorPitch.veloCtrl.refVel = td2.v1;  //速度参考值由跟踪微分器给出，来达到安排过渡过程的目的
	
 	motorPitch.veloCtrl.refVel = motorPitch.posCtrl.output;
	Motor_VeloCtrl(&motorPitch.veloCtrl); //速度pid计算
//	motor_pitch_veloCtrl();
	
	/*	电机值输出	*/
	CAN_CMD_GIMBAL(motorYaw.veloCtrl.output, motorPitch.veloCtrl.output, 0, 0);
}


/*	Motor PID Value Set  */
void Motor_ValueSet(Motor_t *motor, float veloKp, float veloKi, float veloKd, float veloMax, float veloMin, \
					float posKp, float posKi, float posKd, float posOutMax, float posOutMin, int state)
{	
//	motor->status = state;
	motor->veloCtrl.kp = veloKp;
	motor->veloCtrl.ki = veloKi;
	motor->veloCtrl.kd = veloKd;
	motor->veloCtrl.outputMax = veloMax;
	motor->veloCtrl.outputMin = veloMin;
	
	motor->posCtrl.kp = posKp;
	motor->posCtrl.ki = posKi;
	motor->posCtrl.kd = posKd;
	motor->posCtrl.outputMax = posOutMax;
	motor->posCtrl.outputMin = posOutMin;
}

/**
  * @brief	给电机赋期望速度值
  * @param	motor:	Motor_t结构体的指针
  * @param	speed:	预设的速度值
  * @retval	None
  * @note	注意电机速度方向和实际需要的方向是否相同
  */
void Motor_SetVel(VeloPidCtrl_t *vel_t, float velo)
{
	vel_t->refVel = velo;
}


/*	电机编码器模式下的位置值设置	*/
void Motor_IncPos(PosPidCtrl_t *pos_t, float pos, float posMax, float posMin)  
{
	pos_t->refPos += pos;
	
	if(pos_t->refPos > posMax)
		pos_t->refPos = posMax;
	if(pos_t->refPos < posMin)
		pos_t->refPos = posMin;
	
}

/*	电机IMU反馈闭环模式下的位置值设置 */
void Motor_AbsPos(PosPidCtrl_t *pos_t, float pos, float posMax, float posMin)  
{
	pos_t->refPos += pos;
	
	if(pos_t->refPos > posMax)
		pos_t->refPos = posMax;
	if(pos_t->refPos < posMin)
		pos_t->refPos = posMin;
	
}

/* Yaw 电机位置环pid计算 */
void motorYawCalcuPos(void)
{
	float diff;
	
	// 计算误差值，err保存当前的误差，errLast保存上一次的误差 
	motorYaw.posCtrl.errLast = motorYaw.posCtrl.err;

	motorYaw.posCtrl.err = motorYaw.posCtrl.refPos - motorYaw.posCtrl.relaPos;
	{	
		// 计算积分值，注意末尾积分限幅 */
		motorYaw.posCtrl.integ += motorYaw.posCtrl.err;
		
		//积分限幅
		motorYaw.posCtrl.integ = limit_output(motorYaw.posCtrl.integ, -1000, 1000);
		
		diff = motorYaw.posCtrl.err - motorYaw.posCtrl.errLast;	//计算误差变化率
				
		// 绝对式方法计算PID输出 */ 	             //* abs(motorYaw.posCtrl.err) / 2
		motorYaw.posCtrl.output = motorYaw.posCtrl.kp * motorYaw.posCtrl.err + motorYaw.posCtrl.ki * motorYaw.posCtrl.integ + motorYaw.posCtrl.kd * diff;
	
		// 输出限幅 */
		motorYaw.posCtrl.output = limit_output(motorYaw.posCtrl.output, motorYaw.posCtrl.outputMin, motorYaw.posCtrl.outputMax);
	}
}

/* Pitch 电机位置环pid计算 */
void motorPitchCalcuPos(void)
{
	float diff;
//	float refVel;
//	float sign = 1.0f;
	
	// 计算误差值，err保存当前的误差，errLast保存上一次的误差 */
	motorPitch.posCtrl.errLast = motorPitch.posCtrl.err;
	motorPitch.posCtrl.err = motorPitch.posCtrl.refPos - motorPitch.posCtrl.relaPos;
	{	
		// 计算积分值，注意末尾积分限幅 
		motorPitch.posCtrl.integ += motorPitch.posCtrl.err;
		
		//积分限幅
		motorPitch.posCtrl.integ = limit_output(motorPitch.posCtrl.integ, -1000, 1000);
					
		diff = motorPitch.posCtrl.err - motorPitch.posCtrl.errLast;	//计算误差变化率
//		ESO_AngularRate_run(&eso2, diff, 1.0f/200);
		
		// 绝对式方法计算PID输出                         // * abs(motorPitch.posCtrl.err)
		motorPitch.posCtrl.output = motorPitch.posCtrl.kp * motorPitch.posCtrl.err + motorPitch.posCtrl.ki * motorPitch.posCtrl.integ + motorPitch.posCtrl.kd * diff;
		
//		if(motorPitch.posCtrl.output > 100)
//		{
//			motorPitch.posCtrl.output += 200;
//		}
//		/* 用固定加速度逼近终值， 0.8为积分裕量，用来削减积分值和实际值之间的偏差 */
//		if (motorPitch.posCtrl.err < 0.0f)
//			sign = -1.0f;
//		
//		refVel = sign * __sqrtf(2.0f * 0.8f * motorPitch.posCtrl.acc * sign * motorPitch.posCtrl.err);
//				
//		/* 如果接近终值则切换成PID控制 */
//		if (fabsf(refVel) < fabsf(motorPitch.posCtrl.output))
//		motorPitch.posCtrl.output = refVel;
		
		// 输出限幅 
		motorPitch.posCtrl.output = limit_output(motorPitch.posCtrl.output, motorPitch.posCtrl.outputMin, motorPitch.posCtrl.outputMax);
	}
}

/*	进行电机速度控制  */
void Motor_VeloCtrl(VeloPidCtrl_t *vel_t)
{
	float diff;
	
	// 速度PID 
	vel_t->errLast = vel_t->err;
	vel_t->err = vel_t->refVel - vel_t->rawVel;		//使用vel_t->refVel作为速度期望
	diff = vel_t->err - vel_t->errLast;
	vel_t->integ += vel_t->err;
	 
	//积分限幅 
	vel_t->integ = limit_output(vel_t->integ, -4000, 4000);
		
	vel_t->output = vel_t->kp * vel_t->err + vel_t->ki * vel_t->integ + vel_t->kd * diff;
	
	//输出限幅 
	vel_t->output = limit_output(vel_t->output, vel_t->outputMin, vel_t->outputMax);
}

static inline void MotorFliter_VeloCtrl(VeloPidCtrl_t *vel_t)  //速度反馈使用td滤波后的值
{
	float diff;
	
	// 速度PID 
	vel_t->errLast = vel_t->err;
	vel_t->err = vel_t->refVel - vel_t->filrawVel;		//使用vel_t->refVel作为速度期望
	diff = vel_t->err - vel_t->errLast;
	vel_t->integ += vel_t->err;
	 
	//积分限幅 
	vel_t->integ = limit_output(vel_t->integ, -4000, 4000);
		
	vel_t->output = vel_t->kp * vel_t->err + vel_t->ki * vel_t->integ + vel_t->kd * diff;
	
	//输出限幅 
	vel_t->output = limit_output(vel_t->output, vel_t->outputMin, vel_t->outputMax);
}

static inline void motor_pitch_veloCtrl(void)
{
	float diff;
	
	// 速度PID 
	motorPitch.veloCtrl.errLast = motorPitch.veloCtrl.err;
	
//	TD_Calculate(&td2_velo, motorPitch.veloCtrl.err);
	TD_Calculate(&td2_velo, motorPitch.veloCtrl.rawVel);
	
	motorPitch.veloCtrl.err = motorPitch.veloCtrl.refVel - td2_velo.v1;//motorPitch.veloCtrl.rawVel;		//使用vel_t->refVel作为速度期望
	diff = motorPitch.veloCtrl.err - motorPitch.veloCtrl.errLast;
	motorPitch.veloCtrl.integ += motorPitch.veloCtrl.err;
	
	//积分限幅 
	motorPitch.veloCtrl.integ = limit_output(motorPitch.veloCtrl.integ, -4000, 4000);
		
	motorPitch.veloCtrl.output = motorPitch.veloCtrl.kp * motorPitch.veloCtrl.err + motorPitch.veloCtrl.ki * motorPitch.veloCtrl.integ + motorPitch.veloCtrl.kd * (td2.v2 - td2_velo.v2);
	
	//输出限幅 
	motorPitch.veloCtrl.output = limit_output(motorPitch.veloCtrl.output, motorPitch.veloCtrl.outputMin, motorPitch.veloCtrl.outputMax);
}


