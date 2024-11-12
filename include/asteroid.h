#ifndef ASTEROID_H
#define ASTEROID_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "rt_thread.h"
#include "space.h"


#define AST_MAX 	5	// Maximum number of asteroids on one screen
#define INIT_AST 	1	// Starting number of asteroids present
#define MIN_DIM		10	// minimum size of an asteroid

#define INIT_AST_SCREEN ((SCREEN_TOT * 40) / 100)	// first screen where the asteroids appear
#define END_AST_SCREEN ((SCREEN_TOT * 50) / 100)	// last screen showing asteroids
													// it was decided to take a percentage of the total bands

#define PRD_AST			20	// Task period
#define DL_AST			20	// Task deadline
#define PRIO_AST		20	// Task priority


typedef struct position {
	int 		x;		// [meters] x coordinate
	int 		y;		// [meters] y coordinate
} position;


typedef struct pixel {
	int 		x;		// [pixel] x coordinate
	int 		y;		// [pixel] y coordinate
} pixel;


typedef struct ast {

	int 		t_id;		// thread id
	int 		dim;
	position 	pos;		// asteroid position
	pixel 		pix;
	pthread_mutex_t mtx;		// mutex to protect the struct
	bool 		exist;		
} ast;


extern ast asteroids[AST_MAX];
extern pthread_mutex_t asteroids_mtx;
extern int asteroid_counter;

void init_ast(ast *a, int t_id, int x_pos_max);
void* move_ast(void *arg);
int add_ast(int x_pos_max);
int initial_asteroids();
int destroy_ast();
void set_position(ast *a, int x_pos_max);


#endif
