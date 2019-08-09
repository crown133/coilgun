#include "mode.h"
#include "oled.h"
#include "adc.h"

#include "Motor_Ctrl.h"

#include "math.h"

#define abs(x)  (((x)>0)?(x):-(x))  //����ֵ����
#define limit_num(x, min, max)	( (x) <= (min) ? (min) : (x) >= (max) ? (max) : (x) )//�޷�����
 
/**********  ״̬��־λ **********/ 
uint8_t first_menu = 1;
uint8_t second_menu;

uint8_t mode_flag = 1;

#define auto_step_unit 5 //�Զ��������ڲ���
uint8_t auto_flag = 0;
uint8_t CCW_flag = 0;  //�Զ�ģʽ��ת���־λ
uint8_t CW_flag = 1;  //�Զ�ģʽ��ת���־λ
uint8_t auto_init_flag = 1; //�ȴ����ת��-30��λ��

#define fire_voltage  100  //����ѹ ��λv
#define charge		PEout(2)=1
#define discharge   PEout(2)=0
#define fire_start  PEout(4)=1
#define fire_stop   PEout(4)=0
uint8_t fire_flag = 0;  //�����־
uint8_t shoot = 0;  //�����־

extern uint8_t CHL, CHR;

void menu_init(void)
{
	oled_showstring(0, 0, "Mode:");
	oled_showstring(1, 0, "Dist:");
	oled_showstring(2, 0, "Fire:");
	oled_showstring(3, 0, "Reload:");
}


/*********************************/
void mode_change(void)   //ģʽ�л�
{
	if(first_menu == 1)
	{
		if(CHR == 1)
		{
			mode_flag++;
			CHR = 0;
		}
		else if(CHR == 2)
		{
			mode_flag--;
			CHR = 0;
		}
		if(mode_flag > 6)
		{
			mode_flag = 1;
		}
		if(mode_flag < 1)
		{
			mode_flag = 6;
		}
	}
	
	switch (mode_flag)
	{
		case fire_mode:  //			mode_flag = 1;
		{
			oled_showstring(0, 5, "fire_mode       ");
			break;
		}
		
		case dist_mode:  //			mode_flag = 2;
		{
			oled_showstring(0, 5, "dist_mode       ");			
			break;
		}
		
		case dist_angle_mode:  //mode_flag = 3;
		{
			oled_showstring(0, 5, "distAngle_mode  ");
			break;
		}
		
		case auto_mode:  //mode_flag = 4;
		{
			oled_showstring(0, 5, "auto_mode       ");
			break;
		}
		
		case auto_horizon_mode:  //mode_flag = 5;
		{		
			oled_showstring(0, 5, "autoHorizon_mode");
			break;
		}
		
		case track_mode:  //mode_flag = 6;
		{
			oled_showstring(0, 5, "track_mode      ");
			break;
		}
		
		default:
			break;
	}
	
}

void dist_set(void)   //��������
{
	static uint16_t distance = 200;
	if(first_menu == dist)
	{
		if(((GPIOE->IDR >> 9) & 0x01) == 0)  //key �� PE9
		{
			if(CHR  == 1)
			{
				distance -=10;
			}
			else if(CHR == 2)
			{
				distance += 10;
			}
			CHR = 0;
		}
		else
		{
			if(CHR  == 1)
			{
				distance -=1;
				CHR = 0;
			}
			else if(CHR == 2)
			{
				distance += 1;
				CHR = 0;
			}
		}
		distance = limit_num(distance, 0, 450);			
	}
	oled_shownum(1, 6, distance, 0, 8);
}


/************************/

void reload_enter(void)
{
	if(first_menu == reload)
	{
		if(key == 1)
		{
			oled_showstring(3, 7, "reloading ...");
			key = 0;
		}
	}
	else 
	{
			oled_showstring(3, 7, "  loaded     ");
	}
}

/*******************************/
void fire_fire(void)
{
	static float voltage;
	voltage = voltage_get();
	if(((voltage - fire_voltage) < 2) && fire_flag)
	{
		charge;
	}
	else
	{
		discharge;
	}
	if(shoot)
	fire_start;
}
/*****************************************/
void auto_mode_run(void)  //�����Զ�ģʽ���ڴ˺���������
{
	if(auto_flag)
	{
		if(auto_init_flag)
		{
			motorYaw.posCtrl.refPos = -685;
			auto_init_flag = 0;
		}
		
		if((abs(motorYaw.posCtrl.relaPos + 685) < 40) && CW_flag == 1)
		{
			CCW_flag = 1;
			CW_flag = 0;
		}
		else if((abs(motorYaw.posCtrl.relaPos - 685) < 40) && CCW_flag == 1)
		{
			CCW_flag = 0;
			CW_flag = 1;
		}
	//	if(����Ŀ��)
	//	{
	//		CCW_flag = 0;
	//		CW_flag = 0;
	//	}

		Motor_IncPos(&(motorYaw.posCtrl), (CCW_flag-CW_flag) * auto_step_unit, 685, -685);   //��Ӧ +-30�� ��΢����һЩ����
	}	
}

void item_selection(void)  //���л�����
{
	if(CHL == 1)
	{
		first_menu--;
		CHL = 0;
	}
	else if(CHL == 2)
	{
		first_menu++;
		CHL = 0;
	}
	if(first_menu > 4)
	{
		first_menu = 1;
	}
	if(first_menu < 1)
	{
		first_menu = 4;
	}
	
	switch (first_menu)
	{
		case mode:
		{
			mode_change();
			
			oled_show_invert_string(0, 0, "Mode");
			oled_showstring(1, 0, "Dist:");
			oled_showstring(2, 0, "Fire:");
			oled_showstring(3, 0, "Reload:");
			break;
		}
		case dist:
		{
			dist_set();
			oled_showstring(0, 0, "Mode:");
			oled_showstring(2, 0, "Fire:");
			oled_showstring(3, 0, "Reload:");
			oled_show_invert_string(1, 0, "Dist");
			break;
		}
		case fire:
		{
			oled_showstring(0, 0, "Mode:");
			oled_showstring(1, 0, "Dist:");
			oled_showstring(3, 0, "Reload:");
			oled_show_invert_string(2, 0, "Fire");
			break;
		}
		case reload:
		{
			oled_showstring(0, 0, "Mode:");
			oled_showstring(1, 0, "Dist:");
			oled_showstring(2, 0, "Fire:");
			oled_show_invert_string(3, 0, "Reload");
			break;
		}
		
		default:
			break;
	}
	reload_enter();  //װ��
	
	{  //�Զ�ģʽ�л�
		if((mode_flag == auto_mode) || (mode_flag == auto_horizon_mode))
		{
			auto_flag = 1;
		}
		else 
		{
			auto_flag = 0;
			auto_init_flag = 1;
		}
		
		auto_mode_run();  //auto 
	}
	key = 0;
}




