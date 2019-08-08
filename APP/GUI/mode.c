#include "mode.h"
#include "oled.h"
#include "adc.h"

uint8_t mode_flag;

void menu_init(void)
{
	oled_showstring(0, 1, "Mode:");
	oled_showstring(1, 1, "Dist:");
	oled_showstring(2, 1, "Fire:");
	oled_showstring(4, 1, "Reload:");
}

void mode_change(uint8_t mod)
{
	switch (mod)
	{
		case fire_mode:  //			mode_flag = 1;
		{
			oled_showstring(0, 10, " fire_mode      ");
			break;
		}
		
		case dist_mode:  //			mode_flag = 2;
		{
			oled_showstring(0, 10, " dist_mode      ");			
			break;
		}
		
		case dist_angle_mode:  //mode_flag = 3;
		{
			oled_showstring(0, 10, " dist_angle_mode");
			break;
		}
		
		case auto_mode:  //mode_flag = 4;
		{
			oled_showstring(0, 10, " auto_mode       ");
			break;
		}
		
		case auto_horizon_mode:  //mode_flag = 5;
		{		
			oled_showstring(0, 10, " auto_horizon_mode");
			break;
		}
		
		case track_mode:  //mode_flag = 6;
		{
			oled_showstring(0, 10, " track_mode       ");
			break;
		}
		
		default:
			break;
	}
	
}


