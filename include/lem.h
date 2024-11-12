#ifndef LEM_H
#define LEM_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "rt_thread.h"
#include "space.h"

#define INIT_VEL_D      0
#define INIT_POS_X      320
#define INIT_POS_ANGLE  0
#define INIT_VEL_Y      0
#define INIT_FUEL       8000
#define INIT_MASS       10000
#define INIT_M_INERTIA  20000
#define LEM_DIM         10 // dimension in meter of the lem
#define INIT_THRUST     0

#define IMOM_MASS_RATIO	2
#define LOW_MASS		3000
#define LOW_M_CONTROL	0.1	// controller scale for low mass
#define NORM_M_CONTROL	1

#define ASC_THRUST      15000 // ascending thrust
#define DES_THRUST      45000 // descending thrust
#define NO_THRUST       0

#define R_2_D			180/M_PI // from radiant to degree
#define D_2_R			M_PI/180 // from gradiant to degree
#define MAX_DEGREE		10		
#define T_2_FUEL		0.000001 //torque to fuel coefficient
#define THRUST_2_FUEL	0.0001   // thrust to fuel

#define PRD_LEM			20	// Task period
#define DL_LEM			20	// Task deadline
#define PRIO_LEM		20	// Task priority

#define MAX_LAND_Y_VEL	5
#define MAX_LAND_X_VEL	2


typedef struct pos_lem {
	float 		x;		// [meters] x coordinate
	float 		y;		// [meters] y coordinate
	float 		angle;		// [degree]
} pos_lem;


typedef struct pixel_lem {
	int 		x;		// [pixel] x coordinate
	int 		y;		// [pixel] y coordinate
} pixel_lem;


typedef struct lem {

	int 		lem_tid;
	pos_lem 	pos;
	pixel_lem	pix;
	float		vel_x;
	float 		vel_y;
	float		fuel;
	unsigned int	mass;
	unsigned int 	m_inertia;
	unsigned int	dim;
	unsigned int	thrust;
	float 		torque;
	bool        dead;
	bool		landed;
	bool		rock;
} lem;

extern lem l;
extern pthread_mutex_t lem_mtx;
extern float vel_d;

int start_lem(void);
void * move_lem(void * arg);
void init_lem(lem *l,int lem_tid);
void turn_on();
void turn_off();
void go_right();
void go_left();
void stop_lem();


#endif
