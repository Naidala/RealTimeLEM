#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "lem.h"

pthread_mutex_t lem_mtx = PTHREAD_MUTEX_INITIALIZER;

lem l;
int lem_tid;

//data history of angle, torque and error (desired - real speed)
// cell [0] contains the lathest data th(k-1), cell [1] is th(k-2)
float th[2] = {0, 0};
float err[2] = {0, 0};
float torque[2] = {0, 0};

float vel_d = INIT_VEL_D;	//desired velocity


void update_data(float new_data, float * array) {

    int i;
    
    for (i = 1; i > 0; --i) {
        *(array + i) = *(array + i - 1);
    }
    *array = new_data;
}

// initial condition of the lem
void init_lem(lem *l, int lem_tid) {

	l->lem_tid = lem_tid;
	l->pos.x = INIT_POS_X;
	l->pos.y = CSM_HEIGHT;
	l->pos.angle = INIT_POS_ANGLE;
	l->vel_y = INIT_VEL_Y;
	l->fuel = INIT_FUEL;
	l->mass = INIT_MASS;
	l->m_inertia = INIT_M_INERTIA;
	l->dim = LEM_DIM;
	l->thrust = NO_THRUST;
	l->dead = false;
	l->landed = false;
	l->rock = false;
}


// lem movement based on its physical laws, considering also the mass variation given by the fuel used
void * move_lem(void * arg) {

	float g, torque_now, err_now, acc_y, K_controller, angle_rad;
	float fuel_used;
    lem *l = (lem *)arg;

    pthread_mutex_lock(&mass_mtx);
    g = grav;
    pthread_mutex_unlock(&mass_mtx);

    pthread_mutex_lock(&lem_mtx);


    /***VERTICAL***/

    acc_y = (l->thrust - l->mass * g)/l->mass;  // m*a = F - m*g, F verso alto
    l->vel_y = l->vel_y + acc_y * DELTA_T;
    l->pos.y = l->pos.y + l->vel_y * DELTA_T + (acc_y * DELTA_T * DELTA_T)/2;


    /***ORIZONTAL***/

    err_now = vel_d - l->vel_x; // error between the desired speed and the real speed of the lem, along the x axis

    K_controller = NORM_M_CONTROL;
   	/* since lem varies its mass, it is good to vary the gain of the controller when mass is below a certain threshold*/
	if (l->mass <= LOW_MASS) 
        K_controller = LOW_M_CONTROL;

	l->torque = 1.4*torque[0] - 0.5*torque[1] + K_controller*(err_now*990 - 1960*err[0] + 970*err[1]);
	angle_rad = 2*th[0] - th[1] + (torque[0] + torque[1])*0.03/l->m_inertia;
	
	if (angle_rad >= MAX_DEGREE * D_2_R) 
        angle_rad = MAX_DEGREE * D_2_R;
	if (angle_rad <= -MAX_DEGREE * D_2_R) 
        angle_rad = -MAX_DEGREE * D_2_R;
    
	l->pos.angle = angle_rad * R_2_D; // from deg to radiant
    
	// update speeds and position along the axes
	l->vel_x = l->vel_x + 0.25*th[0]*l->thrust/l->mass;
    l->pos.x = l->pos.x + l->vel_x * DELTA_T;
	
	// update the remaining fuel
    fuel_used = (l->thrust*THRUST_2_FUEL + abs(l->torque*T_2_FUEL));
    l->fuel = l->fuel - fuel_used;
	if(l->fuel <= 0) {
		l->fuel = 0;
		l->thrust = NO_THRUST;
	}
    // update the total mass of the lem	
	l->mass = l->mass - fuel_used;
    l->m_inertia = IMOM_MASS_RATIO*l->mass;

    update_data(l->torque, torque);
    update_data(err_now, err);
    update_data(angle_rad, th);

   	
	/***LANDING***/

    // conditions that allow the landing of the lem
	if(l->pos.y <= 0  && abs(l->vel_x) <= MAX_LAND_X_VEL && l->vel_y > - MAX_LAND_Y_VEL) {
		l->pos.y = 0;
		l->vel_y = 0;
		l->vel_x = 0;
		l->landed = true;
	}

	// conditions that do not allow the landing of the lem
	if (l->pos.y <= 0 && l->vel_y < - MAX_LAND_Y_VEL || screen_counter < 0) 
    	l->dead = TRUE;
	
	// conditions that do not allow the return of the lem to the spaceship
    if(l->pos.y >= CSM_HEIGHT && l->vel_y >= MAX_LAND_Y_VEL)
    	l->dead = TRUE;

    pthread_mutex_unlock(&lem_mtx);
}


int start_lem(void) {

	pthread_mutex_lock (&lem_mtx);
	lem_tid = start_thread(move_lem, &l, SCHED_FIFO, PRD_LEM, DL_LEM, PRIO_LEM);
	if (lem_tid < 0) {
		pthread_mutex_unlock (&lem_mtx);
		return 1;
	} else {
		init_lem(&l, lem_tid);
		pthread_mutex_unlock (&lem_mtx);
		printf("Created lem with tid #%d\n", lem_tid);
	}
	return 0;
}


/* functions that establish the movement of the lem along the 4 directions:
	turn_on 
	turn_off
	go_right
	go_left */

void turn_on() {

    pthread_mutex_lock(&(lem_mtx));

	if (l.landed == false)        
		l.thrust = DES_THRUST;
	else 
		l.thrust = ASC_THRUST;
	
    pthread_mutex_unlock(&(lem_mtx));
}


void turn_off() {

	pthread_mutex_lock(&(lem_mtx));
	l.thrust = NO_THRUST;
	pthread_mutex_unlock(&(lem_mtx));
}

void go_right() {

    pthread_mutex_lock(&lem_mtx);
    ++vel_d;
    pthread_mutex_unlock(&lem_mtx);
}

void go_left() {

    pthread_mutex_lock(&lem_mtx);
    --vel_d;
    pthread_mutex_unlock(&lem_mtx);
}

void stop_lem() {

   	stop_thread(l.lem_tid);
}


