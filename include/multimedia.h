#ifndef MULTIMEDIA_H
#define MULTIMEDIA_H

#include "asteroid.h"
#include "lem.h"
#include "rt_thread.h"
#include "space.h"


#define PRD_GRAPHICS		20	// Task period
#define DL_GRAPHICS 		20	// Task deadline
#define PRIO_GRAPHICS 		30	// Task priority

#define PRD_KEYBOARD 		200	// Task period
#define DL_KEYBOARD   		200	// Task deadline
#define PRIO_KEYBOARD 		10	// Task priority

#define PRD_RADAR			40	// Task period
#define DL_RADAR 			40	// Task deadline
#define PRIO_RADAR 			20	// Task priority


#define SPACESHIP_X			220
#define SPACESHIP_Y			PIX_Y_INIT - 100
#define	SPACESHIP_DIM		200

#define BG_PATH				"background.bmp"
#define AST_PATH			"asteroid.bmp"
#define LEM_PIC_PATH		"lem_pic.bmp"
#define SHIP_PATH			"spaceship.bmp"
#define LANDING				"surface.bmp"

#define IMG_LEM_SIZE		DIM_LEM * SCALE
#define IMG_SHIP_SIZE		60

#define SCAN_DEG_RANGE		120	// radar scan angles
#define RMIN 				50	// minimum sensor distance
#define RSTEP 				1	// sensor resolutions
#define RMAX				300	// maximum sensor distance

#define SAFE_DESTR_X		10
#define SAFE_DESTR_Y		20

#define ANGLE_256 			((float)256/(float)360) //	conversion from float to fix

#define NUM_IM_EXP			9

// structure of asteroids detected by the radar
typedef struct ast_pix {

	int x_init, y_init;	// first pixel position detected by the radar
	int x_end, y_end;	// last pixel position detected by the radar
	int x_centro, y_centro;	//estimate of the asteroid center
	int vel_est;
} ast_pix;


int init_multimedia(void);
void change_screen(lem *l);
void destruction(lem *l);
void you_win(void);
int line_scan (lem *l);
void auto_response(lem *l, int ast_found);
void stop_multimedia(void);
void wait_for_termination(void);


#endif
