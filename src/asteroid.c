#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "asteroid.h"

int asteroid_counter = 0;
ast asteroids[AST_MAX];

pthread_mutex_t asteroids_mtx = PTHREAD_MUTEX_INITIALIZER;

/********UTILITY********/

double frand(double min, double max) {
	double r = rand()/(double)RAND_MAX;
	return min + (max - min) * r;
}

//Initialize the asteroid setting the position	
void init_ast(ast *a, int t_id, int x_pos_max) {

	a->t_id = t_id;
	set_position(a, x_pos_max);
	a->exist = true;
}

// Change the position of the asteoroid according to its physical laws
void * move_ast(void * arg) {
	
	double m;
	ast *a = (ast *)arg;

	pthread_mutex_lock(&mass_mtx);
	m = m_planet;
	pthread_mutex_unlock(&mass_mtx);

	pthread_mutex_lock(&a->mtx);
	a->pos.x = a->pos.x + (sqrt(GRAV_CONST * m / (R_PLANET + a->pos.y))) * DELTA_T;
	pthread_mutex_unlock(&a->mtx);
	
}

 
void set_position(ast *a, int x_pos_max) {

	int screen;
	a->dim = MIN_DIM + rand() % MIN_DIM; // a size between MIN_DIM and 2 * MIN_DIM is assigned to the asteroid
	a->pos.x = rand()%x_pos_max - a->dim; // randomly assigned a position in meters along the x axis to the asteroid

	pthread_mutex_lock(&screen_mtx);
	screen = screen_counter;
	pthread_mutex_unlock(&screen_mtx);

	a->pos.y = CSM_HEIGHT - (SCREEN_Y-PIX_Y_INIT)/SCALE - EARTH_POSITION + a->dim - frand((screen-1)*SCREEN_Y/SCALE, (screen )*SCREEN_Y/SCALE); // randomly assigned a position in meters along the y axis to the asteroid
}

/* Add a new asteroid to the current number.
*  Returns 0 if success, -1  the maximum number has been reached, -2 if thread pool is full. */

int add_ast(int x_pos_max) {

	int t_id;
	int ast_id;

	pthread_mutex_lock(&asteroids_mtx);
	ast_id = asteroid_counter;
	pthread_mutex_unlock(&asteroids_mtx);
	if (ast_id >= AST_MAX)
		return -1;

	pthread_mutex_init(&asteroids[ast_id].mtx, NULL);
	pthread_mutex_lock(&asteroids[ast_id].mtx);

	t_id = start_thread(move_ast, &asteroids[ast_id], SCHED_FIFO, PRD_AST, DL_AST, PRIO_AST);

	if (t_id < 0) {
		pthread_mutex_unlock(&asteroids[ast_id].mtx);
		return -2;
	} else {
		init_ast(&asteroids[ast_id], t_id, x_pos_max);
		pthread_mutex_unlock(&asteroids[ast_id].mtx);

		pthread_mutex_lock(&asteroids_mtx);
		++asteroid_counter;
		pthread_mutex_unlock(&asteroids_mtx);

		printf("Created ast with id %d and tid #%d\n", ast_id, t_id);
	}

	return 0;
}

int initial_asteroids() {

	for (int i = 0; i < INIT_AST; ++i) {
		if (add_ast((int)SCREEN_X/SCALE) < 0)
			return i;
	}
	return INIT_AST;
}

/* destroy the last asteroid added.
*  Returns 0 if success, -1 if no asteroid is left. */

int destroy_ast() {

	int ast_id;

	pthread_mutex_lock(&asteroids_mtx);
	ast_id = asteroid_counter - 1;
	pthread_mutex_unlock(&asteroids_mtx);

	if (ast_id < 0)
		return -1;

	pthread_mutex_lock(&asteroids[ast_id].mtx);

	if (stop_thread(asteroids[ast_id].t_id) == 0) {
		pthread_mutex_lock(&asteroids_mtx);
		--asteroid_counter;
		pthread_mutex_unlock(&asteroids_mtx);
		asteroids[ast_id].exist = false;
	}
	
	pthread_mutex_unlock(&asteroids[ast_id].mtx);

	return 0;
}


void stop_ast(void) {
	
	int i = 0;
	while(i <= asteroid_counter) {
		destroy_ast();
		i++;
	}
}