#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <pthread.h>

/* set here your screen resolution */
#define WIDTH                1920
#define HEIGHT               1080
#define BPP                  32

/* particle count */
#define N 100000


/* acceleration limiter */
#define MM 5.0f

/* speed limiter */
#define MAXMAX 80.0f

/* fluid friction */
#define ATTR 0.03f

/* particles transparency: 0.10 if you enable USE_LINES,
 * otherwise keep > 0.30 */
#define ALPHA 0.50f

/* draw lines instead of pixels */
/*#define USE_LINES*/

/* number of threads: how many cores do you have? */
#define NT 4

/* particle size */
#define PSIZE 1.0f

/* gravity center weight */
#define M 1000000.0f

#define RANDF               (rand()/(float)RAND_MAX)

typedef struct
{
	GLfloat ox,oy,x,y,xSpeed,ySpeed,xAccel,yAccel;
	GLfloat xBaseSpeed, yBaseSpeed;
	GLfloat r,g,b,a;
} particle_s;

typedef struct
{
	int from;
	int to;
} thread_s;

static int xM, yM, cM = 0;
static int stop = 0;

SDL_Surface *surface;
particle_s particles[N];

void init_particles()
{
	int i;
	particle_s *p;

	for(i=0;i<N;i++)
	{
		p = &particles[i];

		p->x = RANDF * WIDTH;
		p->y = RANDF * HEIGHT;

		p->xBaseSpeed = p->xSpeed = -1.0f + RANDF * 2.0f;
		p->yBaseSpeed = p->ySpeed = -1.0f + RANDF * 2.0f;

		p->r = 0.2+RANDF*0.1;
		p->g = 0.2+RANDF*0.1;
		p->b = 0.8+RANDF*0.2;
		p->a = ALPHA;
	}
}

void loop_particles(int from, int to)
{
	int i;
	float x1, y1, distance, mAccel, ang;
	particle_s *p;

	static const float gamma = 66.6667f;

	for(i=from;i<to;i++)
	{
		p = &particles[i];
		if(cM)
		{
			x1 = p->x-xM;
			y1 = p->y-yM;

			distance = y1*y1 + x1*x1;

			/* gravitation formula: -y * Mm/(d^2) */
			mAccel = -gamma*M/distance;

			/* acceleration limiter */
			if(mAccel<-MM) mAccel=-MM;
			if(mAccel>MM) mAccel=MM;

			/* get angle between particle and gravity center */
			ang = atan2(y1,x1);

			p->xAccel = mAccel * cos(ang);
			p->yAccel = mAccel * sin(ang);
		}
		else
			p->xAccel = p->yAccel = 0;

		/* fluid friction */
		p->xAccel -= ATTR * p->xSpeed;
		p->yAccel -= ATTR * p->ySpeed;

		/* update speeds */
		p->xSpeed += p->xAccel;
		p->ySpeed += p->yAccel;

		/* speed limiter */
		if(p->xSpeed>MAXMAX) p->xSpeed=MAXMAX;
		if(p->xSpeed<-MAXMAX) p->xSpeed=-MAXMAX;
		if(p->ySpeed>MAXMAX) p->ySpeed=MAXMAX;
		if(p->ySpeed<-MAXMAX) p->ySpeed=-MAXMAX;

#ifdef USE_LINES
		p->ox = p->x;
		p->oy = p->y;
#endif
		p->x += p->xSpeed;
		p->y += p->ySpeed;

		if(p->x<0) {p->ox+=WIDTH; p->x+=WIDTH;}
		if(p->x>=WIDTH) {p->ox-=WIDTH; p->x-=WIDTH;}
		if(p->y<0) {p->oy+=HEIGHT; p->y+=HEIGHT;}
		if(p->y>=HEIGHT) {p->oy-=HEIGHT; p->y-=HEIGHT;}
	}
}


void init_graphics()
{
	if(SDL_Init(SDL_INIT_EVERYTHING)<0)
	{
		fprintf(stderr, "SDL?\n");
		stop = 1;
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);

	surface = SDL_SetVideoMode(WIDTH,HEIGHT,BPP,SDL_HWSURFACE|SDL_GL_DOUBLEBUFFER|SDL_OPENGL|SDL_HWPALETTE|SDL_HWACCEL);
	if(!surface)
	{
		fprintf(stderr, "Cannot set video mode: %s\n", SDL_GetError());
		stop = 1;
		return;
	}

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearAccum(0.0f,0.0f,0.0f,1.0f);
	glPointSize(1.0f);

	glViewport(0,0,(GLsizei)WIDTH,(GLsizei)HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0,WIDTH,HEIGHT,0,0,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void event(SDL_Event *e)
{
	int x, y, s;

	if(e->type==SDL_QUIT || (e->type==SDL_KEYDOWN && 
				e->key.keysym.sym==SDLK_ESCAPE))
		stop = 1;

	s = SDL_GetMouseState(&x,&y);
	if(s & SDL_BUTTON(1))
	{
		xM = x;
		yM = y;
		cM = 1;
	}
	if(!(s & SDL_BUTTON(1)))
		cM = 0;
}

void render()
{
	int i;
	particle_s *p;

	glClear(GL_COLOR_BUFFER_BIT);
	glAccum(GL_RETURN, 0.99f);

	glClear(GL_ACCUM_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.375,0.375,0);

	glBlendFunc(GL_SRC_ALPHA,GL_DST_ALPHA);
	glPointSize(PSIZE);

#ifdef USE_LINES
	glBegin(GL_LINES);
#else
	glBegin(GL_POINTS);
#endif
	for(i=0;i<N;i++)
	{
		p = &particles[i];
		glColor4f(p->r,p->g,p->b,p->a);
		glVertex2f(p->x,p->y);
#ifdef USE_LINES
		glVertex2f(p->ox,p->oy);
#endif
	}
	glEnd();
	SDL_GL_SwapBuffers();

	glAccum(GL_ACCUM,0.9f);
}

long msec()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec/1000;
}

void *particles_thread(void *arg)
{
	long tmr;
	thread_s *t = (thread_s *) arg;

	while(!stop)
	{
		tmr = msec();
		loop_particles(t->from, t->to);
		tmr = msec() - tmr;

		if(tmr<30)
			SDL_Delay(30-tmr);
	}
	
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	pthread_t threads[NT];
	long tmr;
	SDL_Event e;
	thread_s tmp[NT];

	srand(time(NULL));

	init_particles();
	init_graphics();

	for(tmr=0;tmr<NT;tmr++)
	{
		tmp[tmr].from = (N/NT)*tmr;
		tmp[tmr].to = (N/NT)*(tmr+1);

		pthread_create(&threads[tmr], NULL, particles_thread, (void *)&tmp[tmr]);
	}

	while(!stop)
	{
		tmr = msec();

		while(SDL_PollEvent(&e))
			event(&e);

		render();

		tmr = msec() - tmr;
		if(tmr<30)
			SDL_Delay(30-tmr);
	}

	SDL_Quit();
	return 0;
}
