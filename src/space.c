#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "space.h"

pthread_mutex_t mass_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t screen_mtx = PTHREAD_MUTEX_INITIALIZER;

int screen_counter = 0;		//number of screen passed
double m_planet = M_PLANET;
float grav = (GRAV_CONST * M_PLANET)/(R_PLANET*R_PLANET);

/* reduce the mass of the landing planet, thus reducing gravity*/
void lessen_mass() {
	
	if (m_planet > 0) {
		pthread_mutex_lock(&mass_mtx);
		m_planet = m_planet - M_PLANET;
		grav = (GRAV_CONST * m_planet)/(R_PLANET*R_PLANET);
		pthread_mutex_unlock(&mass_mtx);
	}
}


/* increase the mass of the landing planet, thus increasing gravity*/
void increase_mass() {
	
	pthread_mutex_lock(&mass_mtx);
	m_planet = m_planet + M_PLANET;
	grav = (GRAV_CONST * m_planet)/(R_PLANET*R_PLANET);
	pthread_mutex_unlock(&mass_mtx);
}
