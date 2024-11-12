#include <allegro.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <allegro/keyboard.h>
#include <time.h>

#include "multimedia.h"


pthread_cond_t terminate = PTHREAD_COND_INITIALIZER;
pthread_mutex_t terminate_mtx = PTHREAD_MUTEX_INITIALIZER;

int graphics_tid, keyboard_tid, auto_tid = 0;

char current_message[80];
ast_pix ast_estimate[AST_MAX];
int Dist_x[6], Dist_y[6];
int z = 1, d = 0, ast_found;

BITMAP	*buf;
BITMAP  *night;
BITMAP	*asteroid;
BITMAP  *lem_pic, *lem_thruster;
BITMAP  *lem_left_rocket, *lem_right_rocket, *lem_thrust_left, *lem_thrust_right;
BITMAP  *spaceship;
BITMAP  *surface;
BITMAP	*explosion[NUM_IM_EXP];
BITMAP	*gameover;
BITMAP  *win;

SAMPLE 	*sample;



int init_graphics(void) {
	
	char s1[] = "explosion0.bmp";
	int i;

	if (allegro_init()!=0)
		return 1;

	//play piece of music
	install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, 0);
	sample = load_sample("Zarathustra.wav");
	if (sample == NULL) {
		printf("ERROR ON LOADING WAVE FILE\n");
		exit(1);
	}
	play_sample(sample, 255, 128, 1000, 0);

	set_color_depth(32);
	if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, SCREEN_X + STATS_PANEL_W, SCREEN_Y + TOOLBAR_H, 0, 0))
		return 2;

	buf = create_bitmap(SCREEN_X , SCREEN_Y );
	clear_bitmap(buf);

	asteroid = load_bitmap("asteroid.bmp", NULL);
	lem_pic = load_bitmap("lem_pic.bmp", NULL);
	lem_thruster = load_bitmap("lem_thruster.bmp", NULL);
	night = load_bitmap("background.bmp", NULL);
	spaceship = load_bitmap("spaceship.bmp", NULL);
    surface = load_bitmap("surface.bmp", NULL);
	gameover = load_bitmap("gameover.bmp",NULL);
	lem_left_rocket = load_bitmap("lem_left_rocket.bmp",NULL);
	lem_right_rocket = load_bitmap("lem_right_rocket.bmp",NULL);
	lem_thrust_left = load_bitmap("lem_thrust_left.bmp", NULL);
	lem_thrust_right = load_bitmap("lem_thrust_right.bmp", NULL);
	win = load_bitmap("WIN.bmp",NULL);
	
	for(i = 1; i<=NUM_IM_EXP; i++){
		s1[9] = i + '0';	
		explosion[i-1] = load_bitmap(s1, NULL);

	}
	
	blit(buf, screen, 0, 0, 0, 0, SCREEN_X, SCREEN_Y);

	return 0;
}


void clear_field() {
	
	blit(night, buf, 0, 0, 0, 0, SCREEN_X, SCREEN_Y);
}


/********************/
/******DRAW LEM******/
/********************/


void draw_lem(void) {

	lem *L = &l;
    pthread_mutex_lock(&lem_mtx);

   	L->pix.y = SCREEN_Y - ((int)(EARTH_POSITION + L->pos.y)*SCALE)%SCREEN_Y;
    L->pix.x = (int)(L->pos.x*SCALE)%SCREEN_X;

	change_screen(L);
	destruction(L);
	/* choose the image of the lem considering the ignition of both the main thrust engine and the side rockets */

	if (L->thrust == 0){
		if(L->torque > 1.0)
			rotate_scaled_sprite(buf, lem_right_rocket, L->pix.x, L->pix.y, ftofix((float)(ANGLE_256*L->pos.angle)), ftofix(0.13333333));
		else if(abs(L->torque) < 1.0)
			rotate_scaled_sprite(buf, lem_pic, L->pix.x, L->pix.y, ftofix((float)(ANGLE_256*L->pos.angle)), ftofix(0.13333333));
		else 	
			rotate_scaled_sprite(buf, lem_left_rocket, L->pix.x, L->pix.y, ftofix((float)(ANGLE_256*L->pos.angle)), ftofix(0.13333333));
	} else {
		if(L->torque > 1.0)
        	rotate_scaled_sprite(buf, lem_thrust_right, L->pix.x, L->pix.y, ftofix((float)(ANGLE_256*L->pos.angle)), ftofix(0.13333333));
		else if(abs(L->torque) < 1.0)
			rotate_scaled_sprite(buf, lem_thruster, L->pix.x, L->pix.y, ftofix((float)(ANGLE_256*L->pos.angle)), ftofix(0.13333333));
		else
			rotate_scaled_sprite(buf, lem_thrust_left, L->pix.x, L->pix.y, ftofix((float)(ANGLE_256*L->pos.angle)), ftofix(0.13333333));
	}
	
	//conditions of win
	if(L->pos.y >= CSM_HEIGHT && L->vel_y <= MAX_LAND_Y_VEL && abs(L->vel_x) <= MAX_LAND_X_VEL && L->landed == true) {
		L->pix.x = SPACESHIP_X;
		L->pix.y = SPACESHIP_Y;
		L->vel_y = 0;
		L->vel_x = 0;
		stop_lem();
		you_win();
	}

    pthread_mutex_unlock(&lem_mtx);
}


// destruction of the lem due to the collision with an asteroid
void destruction(lem *L) {

	int j;

	if(screen_counter >= INIT_AST_SCREEN && screen_counter <= END_AST_SCREEN) {
		pthread_mutex_lock(&asteroids_mtx);
			for(j = 0; j < asteroid_counter; j++) {
				if(((L->pix.y + (LEM_DIM * SCALE)) >= asteroids[j].pix.y) && (L->pix.y <= asteroids[j].pix.y + asteroids[j].dim + SAFE_DESTR_Y))
					if((L->pix.x + LEM_DIM * SCALE >= asteroids[j].pix.x) && (L->pix.x <= asteroids[j].pix.x + asteroids[j].dim + SAFE_DESTR_X))
						L->dead = TRUE;
		}

		pthread_mutex_unlock(&asteroids_mtx);
	}
}


// crossing of a screen by the lem, with consequent updating of the variable screen_counter
void change_screen(lem *L) {

	if ((L->pos.y*SCALE - (SCREEN_TOT - 2 - screen_counter)*SCREEN_Y - SCREEN_Y + EARTH_POSITION*SCALE)<= 0) {	//distance in pixel - number of screen
        pthread_mutex_lock(&screen_mtx);
        screen_counter++;
        pthread_mutex_unlock(&screen_mtx);

        for (int i = 0; i < AST_MAX; ++i) {
            ast *a = &asteroids[i];
            pthread_mutex_lock(&a->mtx);
            if (a->exist) set_position(a, (int)SCREEN_X/SCALE);
            pthread_mutex_unlock(&a->mtx);
        }
    }

    if ((L->pos.y*SCALE - (SCREEN_TOT - 2 - screen_counter)*SCREEN_Y - SCREEN_Y + EARTH_POSITION*SCALE) >= SCREEN_Y) {
        pthread_mutex_lock(&screen_mtx);
        screen_counter--;
        pthread_mutex_unlock(&screen_mtx);

        for (int i = 0; i < AST_MAX; ++i) {
            ast *a = &asteroids[i];
            pthread_mutex_lock(&a->mtx);
            if (a->exist) set_position(a, (int)SCREEN_X/SCALE);
            pthread_mutex_unlock(&a->mtx);
        }
    }
}



/********************/
/******AUTOPILOT*****/
/********************/

void* autopilot (void *arg) {

	lem *l = (lem *)arg;
	int ast_found = line_scan (l);
	auto_response(l, ast_found);
}

/* function that detects the center of each asteroid 
	making the midpoint between the first and the last point detected */
int line_scan (lem *l) {

	int ast_found = 0;
	bool scanning = false, white_found = false; // pixel diversi da 0 in una singola linea di scansione
	int black = 0;
	float alpha, d, deg;
	int xr, yr, col;

	for (deg = 0; deg < SCAN_DEG_RANGE; deg++) {

		d = RMIN;

		if (l->landed == true)
			alpha = ((deg + 60)*3.14/180);	
		else
			alpha = ((deg + 200)*3.14/180);	

		do {
			pthread_mutex_lock(&lem_mtx);
			xr = l->pix.x  + d*cos(alpha); 
			yr = l->pix.y  - d*sin(alpha);
			pthread_mutex_unlock(&lem_mtx);
			// allows to consider the starry background completely black, so that the only thing that the radar reveals are asteroids
			col = getpixel(screen, xr, yr) - getpixel(night, xr, yr); //

			if (col != black && scanning == false) {	
				ast_estimate[ast_found].x_init = xr;
				ast_estimate[ast_found].y_init = yr;
				scanning = true;
			}

			if (col != black && scanning == true) {
				ast_estimate[ast_found].x_end = xr;
				ast_estimate[ast_found].y_end = yr;
			}

			if (col == black && scanning == true && d == RMAX) {
				scanning = false; 
				ast_found++;
			}

			d = d + RSTEP;	
		} while (col == black  && d <= RMAX && yr != SCREEN_Y -1 && screen_counter >= INIT_AST_SCREEN && screen_counter <= END_AST_SCREEN); // yr != SCREEN_Y -1 per non considerare 
	}
	// drawing of the circles to indicate how good is the detection of the asteroids
	if (ast_found > 0 )	{	
			for (int c = 0; c < ast_found; c++) {
				ast_estimate[c].x_centro = (ast_estimate[c].x_init + ast_estimate[c].x_end) / 2;
				ast_estimate[c].y_centro = (ast_estimate[c].y_init + ast_estimate[c].y_end) / 2;
				if(ast_estimate[c].x_centro > 0 && ast_estimate[c].y_centro > 0)			
					circle(screen, ast_estimate[c].x_centro, ast_estimate[c].y_centro, SCALE * MIN_DIM, COLOR_RED);
			}
	}

	return ast_found;
}


/* actions that the lem must automatically perform, to avoid the collision, 
	based on its vertical speed and the distance from each detected asteroid */
void auto_response(lem *l, int ast_found) {

	pthread_mutex_lock(&lem_mtx);	
	
	if (l->landed == false){
		if(screen_counter >= INIT_AST_SCREEN && screen_counter <= END_AST_SCREEN) {
			if(l->vel_y < -20)
				l->thrust = DES_THRUST;

			if(l->vel_y >= -15)
				l->thrust = NO_THRUST;
		} else {
			if(l->vel_y < -5)
				l->thrust = DES_THRUST;
			
			if( l->vel_y > -2)
				l->thrust = NO_THRUST;
		}
		
			
		for (int c = 0; c < ast_found; c++) {

			Dist_x[c] = abs(ast_estimate[c].x_centro - l->pix.x);
			Dist_y[c] = abs(ast_estimate[c].y_centro - l->pix.y);
				
			if(Dist_y[c] <= 140 && Dist_x[c] < 350 && Dist_y[c] > 80 && l->vel_y < 0)
				l->thrust = DES_THRUST;
			else if(Dist_y[c] < 80 && l->vel_y < -13)
				l->thrust = NO_THRUST;
			else if(Dist_y[c] < 80 && l->vel_y > -13)
				l->thrust = DES_THRUST;
		}

		if(l->vel_y > -0.5)
			l->thrust = NO_THRUST;
	} else {	//fase di risalita
		l->rock = true;

		if(screen_counter >= INIT_AST_SCREEN && screen_counter <= SCREEN_TOT) {
			if(l->vel_y > 10)
				l->thrust = NO_THRUST;
			if(l->vel_y <= 5)
				l->thrust = ASC_THRUST;
		} else {
			if(l->vel_y > 5 )
				l->thrust = NO_THRUST;
			if( l->vel_y <= 0)
				l->thrust = ASC_THRUST;
		}

		for (int c = 0; c < ast_found; c++){
			Dist_x[c] = abs(ast_estimate[c].x_centro - l->pix.x);
			Dist_y[c] = abs(ast_estimate[c].y_centro - l->pix.y);
			
			if(Dist_y[c] <= 140 && Dist_x[c]<350 && Dist_y[c]>80 && l->vel_y>0)
				l->thrust = NO_THRUST;
		}

		if(l->vel_y < 0.5)
			l->thrust = ASC_THRUST;
	}

	pthread_mutex_unlock(&lem_mtx);
}


int start_auto(void) {

    auto_tid = start_thread(autopilot, &l, SCHED_FIFO, PRD_RADAR, DL_RADAR, PRIO_RADAR);
    if (auto_tid < 0) {
        printf("Failed to initialize the autopilot thread!\n");
        return 1;
    } else {
        printf("Initialized the autopilot thread with id #%d.\n", auto_tid);
    }

    return 0;
}


void stop_auto(void) {

    stop_thread(auto_tid);
    printf("Autopilot thread stopped.\n");
}


/********************/
/*****DRAW ITEMS*****/
/********************/

void draw_spaceship(void) {

    stretch_sprite(buf, spaceship, SPACESHIP_X, SPACESHIP_Y, SPACESHIP_DIM, SPACESHIP_DIM);
}


void draw_asteroids(void) {

	int pix_dim;

    for (int i = 0; i < AST_MAX; ++i) {
        ast *a = &asteroids[i];
        pthread_mutex_lock(&a->mtx);

        if (a->exist) {
            a->pix.x = (int) (a->pos.x * SCALE);
            if (a->pix.x >= SCREEN_X)
                set_position(a, 1);
            a->pix.y = SCREEN_Y - (int) (a->pos.y * SCALE) % SCREEN_Y;
            pix_dim = SCALE * a->dim;

            stretch_sprite(buf, asteroid, a->pix.x, a->pix.y, pix_dim, pix_dim);
        }
        pthread_mutex_unlock(&a->mtx);
    }
}


void draw_surface(void) {

    blit(surface, buf, 0, 0, 0, 0, SCREEN_X, SCREEN_Y);
}


void game_over(void) {

	blit(gameover,buf, 0, 0, 0, 0, SCREEN_X, SCREEN_Y);
}


void you_win(void) {

	blit(win,buf, 0, 0, 0, 0, SCREEN_X, SCREEN_Y);
}


//	explosion sequence 
void draw_explosion(void) {

	int death_y = SCREEN_Y - ((int)(+ EARTH_POSITION + l.pos.y)*SCALE)%SCREEN_Y;
    int death_x = (int)(l.pos.x*SCALE)%SCREEN_X;
		
	stretch_sprite(buf, explosion[z-1], death_x,death_y, LEM_DIM*SCALE, LEM_DIM*SCALE);
	
	d++;
	if (d == 10){
		d = 0;
		z++;
	}	
	if (z>=10)
		z= 10;
}



/********************/
/*****DRAW BARS******/
/********************/

void draw_toolbar(void) {

	//	Draw background and border
    rectfill(screen, 0, SCREEN_Y , SCREEN_X , SCREEN_Y + TOOLBAR_H -1, COLOR_TOOLBAR_BORDER);
    rectfill(screen, 2, SCREEN_Y + 2, SCREEN_X - 2, SCREEN_Y + TOOLBAR_H - 3, COLOR_TOOLBAR);
	//	Print istruction
    textout_ex(screen, font, "LEM COMMANDS", 70, SCREEN_Y + 20, COLOR_TEXT, COLOR_STATS);
    textout_ex(screen, font, "W/S       go up/down", 70, SCREEN_Y + 50, COLOR_TEXT, COLOR_STATS);
    textout_ex(screen, font, "A/D       go left/right", 70, SCREEN_Y + 80, COLOR_TEXT, COLOR_STATS);

    textout_ex(screen, font, "SPACE COMMANDS", 330, SCREEN_Y + 20, COLOR_TEXT, COLOR_STATS);
    textout_ex(screen, font, "N/M       less/more gravity", 330, SCREEN_Y + 50, COLOR_TEXT, COLOR_STATS);
    textout_ex(screen, font, "UP/DOWN   add/delete asteroid", 330, SCREEN_Y + 80, COLOR_TEXT, COLOR_STATS);

}


void draw_stats(void) {
    char text[60];
	
	// Draw background and border
    rectfill(screen, SCREEN_X, 0,SCREEN_X + STATS_PANEL_W - 1, SCREEN_X + TOOLBAR_H - 1, COLOR_STATS_BORDER);
    rectfill(screen, SCREEN_X + 2, 2, SCREEN_X + STATS_PANEL_W - 3, SCREEN_Y + TOOLBAR_H - 3, COLOR_STATS);

	// Print stats
    textout_centre_ex(screen, font, "DEADLINES STAT", SCREEN_X + (STATS_PANEL_W)/2, 20, COLOR_TEXT, COLOR_STATS);
    sprintf(text, "%s    %d", "Graphics Missed: ", how_many_dl_missed(graphics_tid));
    textout_centre_ex(screen, font, text, SCREEN_X + (STATS_PANEL_W)/2, 50, COLOR_TEXT, COLOR_STATS);
    sprintf(text, "%s    %d", "Keyboard Missed: ", how_many_dl_missed(keyboard_tid));
    textout_centre_ex(screen, font, text, SCREEN_X + (STATS_PANEL_W)/2, 80, COLOR_TEXT, COLOR_STATS);

    textout_centre_ex(screen, font, "LEM VARIABLES", SCREEN_X + (STATS_PANEL_W)/2, 110, COLOR_TEXT, COLOR_STATS);
    sprintf(text, "%s    %.1f", "Vertical Speed: ", l.vel_y);
    textout_centre_ex(screen, font, text, SCREEN_X + (STATS_PANEL_W)/2, 130, COLOR_TEXT, COLOR_STATS);
    sprintf(text, "%s    %.1f", "Horizontal Speed:", l.vel_x);
    textout_centre_ex(screen, font, text, SCREEN_X + (STATS_PANEL_W)/2, 150, COLOR_TEXT, COLOR_STATS);
    sprintf(text, "%s    %.1f%s", "Remaining Fuel:", l.fuel*100/INIT_FUEL, " %");
    textout_centre_ex(screen, font, text, SCREEN_X + (STATS_PANEL_W)/2, 170, COLOR_TEXT, COLOR_STATS);

    textout_centre_ex(screen, font, "WORLD VARIABLES", SCREEN_X + (STATS_PANEL_W)/2, 200, COLOR_TEXT, COLOR_STATS);
    sprintf(text, "%s    %.2f", "Gravity: ", grav);
    textout_centre_ex(screen, font, text, SCREEN_X + (STATS_PANEL_W)/2, 220, COLOR_TEXT, COLOR_STATS);

	if (l.rock == false){
		sprintf(text, " %s", " Rock taken : FALSE");
    	textout_centre_ex(screen, font, text, SCREEN_X + (STATS_PANEL_W)/2, 240, COLOR_TEXT, COLOR_STATS);
	} else {
		sprintf(text, "%s","Rock taken : TRUE");
    	textout_centre_ex(screen, font, text, SCREEN_X + (STATS_PANEL_W)/2, 240, COLOR_TEXT, COLOR_STATS);
	}

    rectfill(screen, SCREEN_X + (STATS_PANEL_W)/2, 260, SCREEN_X + (STATS_PANEL_W)/2 + 6, 410, COLOR_RED);
    //150:CSM = x:pos_y
    circlefill(screen, SCREEN_X + (STATS_PANEL_W)/2 + 3, 410 - 150*l.pos.y/CSM_HEIGHT + 6 ,6, COLOR_BLUE);
    sprintf(text, "%.1f%s", l.pos.y, " m");
    textout_centre_ex(screen, font, text, SCREEN_X + (STATS_PANEL_W)/2 + 50, 410 - 150*l.pos.y/CSM_HEIGHT, COLOR_TEXT, COLOR_STATS);
 
    //  PATH OF THE LEM
    line(screen, SCREEN_X + (STATS_PANEL_W)/2 - 15, 260 + 150*(((SCREEN_Y-PIX_Y_INIT)+(INIT_AST_SCREEN-1)*SCREEN_Y)/SCALE)/CSM_HEIGHT  , SCREEN_X + (STATS_PANEL_W)/2 + 20,
         260 +150*((SCREEN_Y-PIX_Y_INIT+(INIT_AST_SCREEN -1)*SCREEN_Y)/SCALE)/CSM_HEIGHT, COLOR_WHITE);

    line(screen, SCREEN_X + (STATS_PANEL_W)/2 - 15, 260 + 150*((SCREEN_Y-PIX_Y_INIT+(END_AST_SCREEN )*SCREEN_Y)/SCALE)/CSM_HEIGHT  , SCREEN_X + (STATS_PANEL_W)/2 + 20,
         260 +150*((SCREEN_Y-PIX_Y_INIT+(END_AST_SCREEN)*SCREEN_Y)/SCALE)/CSM_HEIGHT, COLOR_WHITE);

    //  HORIZONTAL SPEED
    line(screen, SCREEN_X + 50, 495  , SCREEN_X +STATS_PANEL_W  -60, 495, COLOR_WHITE); 
    line(screen, SCREEN_X + 50, 570  , SCREEN_X + 50, 420, COLOR_WHITE); 
	
	//	DESIRED HORIZONTAL SPEED
    line(screen, SCREEN_X + 50 , 495 - 150*(vel_d)/16,SCREEN_X + STATS_PANEL_W - 60, 495 - 150*(vel_d)/16, COLOR_BLUE);
	sprintf(text, "%.f ", vel_d);
    textout_centre_ex(screen, font, text, SCREEN_X + 26, 495 - 150*(vel_d)/16, COLOR_TEXT, COLOR_STATS);
	
	//	REAL HORIZONTAL SPEED
	line(screen, SCREEN_X + 50 , 495 - 150*(l.vel_x)/16,SCREEN_X + STATS_PANEL_W  -60, 495 - 150*(l.vel_x)/16, COLOR_RED); // asse y lungo 150
	sprintf(text,"vel_des");
	textout_centre_ex(screen, font, text, SCREEN_X +STATS_PANEL_W-50,420, COLOR_BLUE, COLOR_STATS);

}


/**************************/
/*****GRAPHICS THREAD******/
/**************************/

void * graphics_behaviour(void *arg) {

	clear_field();

    pthread_mutex_lock(&screen_mtx);
    int counter = screen_counter;
    pthread_mutex_unlock(&screen_mtx);
	
	// Draw toolbar and stats
	draw_toolbar();
    draw_stats();
	
	//	Draw asteroids
    if ((counter >= INIT_AST_SCREEN) && (counter <= (END_AST_SCREEN)))
         draw_asteroids();
	
	//	Draw the surface of the landing planet
    if(counter == SCREEN_TOT-1 )
        draw_surface();
	
	//	Draw lem
    if(l.dead == false)
         draw_lem();
	
	//	Draw spaceship
    if(counter == 0)
        draw_spaceship();
	
	//	Draw  game over
    if (counter < 0){
		game_over();	
		stop_lem();
	}
	
	//	Draw explosion sequence 
	if (l.dead == true && z<=10){
		stop_lem();
		draw_explosion();
			if(z >=10)
				game_over();	
	}

	blit(buf, screen, 0, 0, 0, 0, SCREEN_X, SCREEN_Y);
}



/* ======================================
*  ============== KEYBOARD ==============
*  ====================================== */

// Get ASCII code and scan code of the pressed key
void get_keycodes(char *scan, char *ascii) {

    int k;
    k = readkey();  // read the key
    *ascii = k;     // extract ascii code
    *scan = k >> 8; // extract scan code
}


// Keyboard thread routine
void * keyboard_behaviour(void *arg) {

	int key;
	char ascii, scan;
	char text[60];
	lem *L = &l;

	// Execute the actions corresponding to the pressed keys
	while (keypressed()) {
		get_keycodes(&scan, &ascii);

		switch (scan) {
		case KEY_ESC:
			pthread_mutex_lock(&terminate_mtx);
			pthread_cond_signal(&terminate);
			pthread_mutex_unlock(&terminate_mtx);
		break;

		//space commands
		case KEY_UP:
			add_ast(1);
		break;
		case KEY_DOWN:
			destroy_ast();
		break;
		case KEY_M:
			increase_mass();
		break;
		case KEY_N:
			lessen_mass();
		break;

		//LEM commands
		case KEY_W:
			if(L->fuel > 0)
				turn_on();
		break;
		case KEY_S:
			turn_off();
		break;
        case KEY_A:
            go_left();
        break;
        case KEY_D:
            go_right();
        break;

        //autopilot
		case KEY_SPACE:
			if (auto_tid != 0) {
				stop_auto(); 
				L->thrust = NO_THRUST;
				auto_tid = 0;
			} else
				start_auto();
		break;

		case KEY_R:
            if(L->landed == true) 
            	L->rock = true;
        break;
		
		default:
			printf("Press ESC to quit!\n");
		}
	}
}


/* ======================================
*  ============== THREADS ==============
*  ====================================== */


/* Start the graphics thread */
int start_graphics(void) {

    graphics_tid = start_thread(graphics_behaviour, NULL, SCHED_FIFO, PRD_GRAPHICS, DL_GRAPHICS, PRIO_GRAPHICS);
    if (graphics_tid < 0) {
        printf("Failed to initialize the graphics thread!\n");
        return 1;
    } else {
        printf("Initialized the graphics thread with id #%d.\n", graphics_tid);
    }

    return 0;
}


/* Gracefully stop the graphics thread */
void stop_graphics(void) {

    stop_thread(graphics_tid);
    printf("Graphics thread stopped.\n");
}


// Start the keyboard thread
unsigned int start_keyboard(void) {

    keyboard_tid = start_thread(keyboard_behaviour, NULL, SCHED_FIFO, PRD_KEYBOARD, DL_KEYBOARD, PRIO_KEYBOARD);
    if (keyboard_tid < 0) {
        printf("Failed to initialize the keyboard thread!\n");
        return 1;
    } else {
        printf("Initialized the keyboard thread with id #%d.\n", keyboard_tid);
    }
    return 0;
}


// Gracefully stop the keyboard thread
void stop_keyboard(void) {

    stop_thread(keyboard_tid);
    printf("Keyboard thread stopped.\n");
}



/* Initialize multimedia (graphics, mouse, keyboard) data structures */
int init_multimedia() {

    if (init_graphics())
        return 1;

    install_keyboard();

    if (start_graphics() || start_keyboard())
        return 1;

    return 0;
}


//Stop all multimedia threads
void stop_multimedia() {

	destroy_bitmap(buf);
	destroy_bitmap(night);
	destroy_bitmap(asteroid);
	destroy_bitmap(lem_pic);
	destroy_bitmap(lem_thruster);
	destroy_bitmap(spaceship);
	destroy_bitmap(surface);
	destroy_bitmap(gameover);
	for(int i = 0; i< NUM_IM_EXP; i++)
		destroy_bitmap(explosion[i]);	

    stop_keyboard();
    stop_graphics();

}


//Wait until the application should close (i.e. user pressed ESC)
void wait_for_termination(void) {

    pthread_mutex_lock(&terminate_mtx);
    pthread_cond_wait(&terminate, &terminate_mtx);
    pthread_mutex_unlock(&terminate_mtx);
}




