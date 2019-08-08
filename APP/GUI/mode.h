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


extern void menu_init(void);


#endif

