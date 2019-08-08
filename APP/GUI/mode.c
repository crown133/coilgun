#include "mode.h"
#include "oled.h"
#include "adc.h"

uint8_t first_menu = 1;
uint8_t second_menu;

uint8_t mode_flag = 1;
extern uint8_t CHL, CHR;

void menu_init(void)
{
	oled_showstring(0, 0, "Mode:");
	oled_showstring(1, 0, "Dist:");
	oled_showstring(2, 0, "Fire:");
	oled_showstring(3, 0, "Reload:");
}

void mode_change(void)
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

void item_selection(void)
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
			oled_show_invert_string(0, 0, "Mode");
			oled_showstring(1, 0, "Dist:");
			oled_showstring(2, 0, "Fire:");
			oled_showstring(3, 0, "Reload:");
			break;
		}
		case dist:
		{
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
}

