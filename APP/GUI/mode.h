#ifndef __MODE_H_
#define __MODE_H_

typedef enum
{
	fire_mode		  = 1,
	dist_mode 		  = 2,
	dist_angle_mode  = 3,
	auto_mode 		  = 4,
	auto_horizon_mode = 5,
	track_mode 	  = 6
} Mode;

typedef enum
{
	mode = 1,
	dist = 2,
	fire = 3,
	reload =4	
} Item;

extern void menu_init(void);

extern void mode_change(void);
extern void item_selection(void);

#endif

