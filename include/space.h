#ifndef SPACE_H
#define SPACE_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "rt_thread.h"


#define DELTA_T			0.02

/*everything is scaled by a factor of 1e2*/

#define GRAV_CONST		6.67e-13	
#define R_PLANET		1e4
#define M_PLANET		1e20
#define CSM_HEIGHT		5e2

#define EARTH_POSITION 	10	//meter
#define SCALE			3	//1 meter = 3 pixel
#define SCREEN_X		640
#define SCREEN_Y		480
#define PIX_Y_INIT  	(SCREEN_Y - ((int)(EARTH_POSITION + CSM_HEIGHT)*SCALE)%SCREEN_Y)	// position of the lem in the first screen varies according to the distance from the ground
#define SCREEN_TOT  	(int)ceil((CSM_HEIGHT - EARTH_POSITION) * SCALE / SCREEN_Y)		// number of screens that must be crossed by the lem.


#define COLOR_RED 		makecol(0xFF,0x00,0x00)
#define COLOR_BLACK		makecol(0x00,0x00,0x00)
#define COLOR_WHITE		makecol(0xFF,0xFF,0xFF)
#define COLOR_BLUE	 	makecol(0x00,0x00,0xFF)

#define TOOLBAR_H 	                110	// [pixel] height of menu area
#define STATS_PANEL_W               250	// [pixel] width of status window

#define COLOR_TOOLBAR 		        COLOR_BLACK
#define COLOR_TOOLBAR_BORDER 	    COLOR_WHITE
#define COLOR_STATS_BORDER          COLOR_WHITE
#define COLOR_STATS                 COLOR_BLACK
#define COLOR_TEXT                  COLOR_WHITE

extern pthread_mutex_t mass_mtx;
extern pthread_mutex_t screen_mtx;

extern int screen_counter;
extern double m_planet;
extern float grav;

void lessen_mass();
void increase_mass();


#endif
