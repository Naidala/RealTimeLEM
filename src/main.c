#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "multimedia.c"
#include "rt_thread.c"
#include "asteroid.c"
#include "space.c"
#include "lem.c"

int main(void)
{
	srand(time(NULL));
	init_rt_thread_manager();
	printf("Thread manager successfully initialized.\n");

	initial_asteroids();
	printf("Ast initialized.\n");
	start_lem();

	printf("Lem initialized.\n");
	// Initialize graphics and keyboard 
	init_multimedia();

	// Wait for the end of simulation (ESC) 
	wait_for_termination();

	stop_ast();
	stop_lem();
	stop_multimedia();
	return 0;
	
	
}
