//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "ppm.h"
extern "C" {
    #include "fonts.h"
}

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 360

#define MAX_PARTICLES 40000
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;
static int savex = 0;
static int savey = 0;
static int n = 0;
//Structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

struct Game {
    bool bubbler;
    Shape box[5];
    Shape circle;
    Particle *particle;
    int n;
    Game() {
        particle = new Particle[MAX_PARTICLES];
        n=0;
        bubbler = false;
        //declare a box shape
        for (int i = 0; i < 5; i++) {
            box[i].width = 70;
            box[i].height = 12;
            box[i].center.x = 90 + i*65;
            box[i].center.y = 300 - i*50;
        }
        circle.radius = 100;
        circle.center.x = 475;
        circle.center.y = -30;
    }
    ~Game() {
        delete [] particle;
    }

};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);

int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;
    //start animation
    while(!done) {
        while(XPending(dpy)) {
            XEvent e;
            XNextEvent(dpy, &e);
            check_mouse(&e, &game);
            done = check_keys(&e, &game);
        }
        movement(&game);
        render(&game);
        glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    cleanup_fonts();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void) {
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
            InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

#define rnd() (float)rand() / (float)RAND_MAX

void makeParticle(Game *game, int x, int y) {
    if (game->n >= MAX_PARTICLES)
        return;
    std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = rnd() - 0.5;
    p->velocity.x = rnd() - 0.5;
    game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
    savex = 0;
    savey = 0;
    n = 0;

    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button was pressed
            int y = WINDOW_HEIGHT - e->xbutton.y;
            for(int i = 0; i < 10; i++) {
                makeParticle(game, e->xbutton.x, y);
            }
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed
            return;
        }
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        savex = e->xbutton.x;
        savey = e->xbutton.y;
        if (++n < 10)
            return;
        int y = WINDOW_HEIGHT - e->xbutton.y;
        for(int i = 0; i < 10; i++) {
            makeParticle(game, e->xbutton.x, y);
        }
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }
        //You may check other keys here.
        if (key == XK_b) {
            if(!game->bubbler) {
                game->bubbler = true;
            }
            else {
                game->bubbler = false;
            }
        }
    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;

    if (game->n <= 0)
        return;

    for (int i = 0; i < game->n; i++) {
        p = &game->particle[i];
        p->s.center.x += p->velocity.x;
        p->s.center.y += p->velocity.y;
        p->velocity.y -= GRAVITY;
        //check for collosion with circle...
        float dist, d0, d1;
        d0 = p->s.center.x - game->circle.center.x;
        d1 = p->s.center.y - game->circle.center.y;
        dist = sqrt((d0*d0) + (d1*d1));
        if(dist < game->circle.radius) {
            //p->s.center.x = (game->circle.radius - dist) + p->s.center.x + 0.1;
            p->s.center.y = (game->circle.radius - dist) + p->s.center.y + 0.1;
            p->velocity.x += d0/dist;
            p->velocity.y += d1/dist;
        }
        //check for collision with shapes...
        //Shape *s;
        for(int j = 0; j < 5; j++) {
            if(p->s.center.x >= game->box[j].center.x - game->box[j].width &&
                    p->s.center.x <= game->box[j].center.x + game->box[j].width &&
                    p->s.center.y < game->box[j].center.y + game->box[j].height &&
                    p->s.center.y > game->box[j].center.y - game->box[j].height) {
                p->s.center.y = game->box[j].center.y + game->box[j].height + 0.1;
                p->velocity.x = rnd();
                p->velocity.y *= -0.5;
            }
        }
        //check for off-screen
        if (p->s.center.y < 0.0) {
            memcpy(&game->particle[i], &game->particle[game->n-1], sizeof(Particle));
            std::cout << "off screen" << std::endl;
            game->n--;
        }
    }
}

void render(Game *game)
{
    Rect r;
    float w, h;
    glClear(GL_COLOR_BUFFER_BIT);
    
    if(game->bubbler) {
        for(int i = 0; i < 5; i++) {
            makeParticle(game, 110, 349);
        }
    }
    //Draw shapes...
    //draw circle...
    //Shape *c = &game->circle; 
    const int n = 40;
    static struct Vec vect[n];
    float rads = (2.0f * 3.1415926f) / n;
    for(int i = 0; i < n; i++) {
        vect[i].x = game->circle.radius * cos(rads * i);
        vect[i].y = game->circle.radius * sin(rads * i);
    }
    glColor3ub(0,100,0);
    glBegin(GL_TRIANGLE_FAN); 
    for(int i = 0; i < n; i++) {
        glVertex2f(vect[i].x + game->circle.center.x, vect[i].y + game->circle.center.y);
    } 
    glEnd();
    //draw box
    Shape *s;
    glColor3ub(90,140,90);
    for(int i = 0; i < 5; i++) {
        s = &game->box[i];
        glPushMatrix();
        glTranslatef(s->center.x, s->center.y, s->center.z);
        w = s->width;
        h = s->height;
        glBegin(GL_QUADS);
        glVertex2i(-w,-h);
        glVertex2i(-w, h);
        glVertex2i( w, h);
        glVertex2i( w,-h);
        glEnd(); 
        glPopMatrix();
    }
    //draw all particles here
    glPushMatrix();
    srand(time(NULL)); 
    for(int i = 0; i < game->n; i++) {
        glColor3ub(rand()%(147-113+1)+113,rand()%(217-196+1)+196,rand()%(250-226+1)+226);
        Vec *c = &game->particle[i].s.center;
        w = 2;
        h = 2;
        glBegin(GL_QUADS);
        glVertex2i(c->x-w, c->y-h);
        glVertex2i(c->x-w, c->y+h);
        glVertex2i(c->x+w, c->y+h);
        glVertex2i(c->x+w, c->y-h);
        glEnd();
    }
    glPopMatrix();
    //
    glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,0,WINDOW_HEIGHT,WINDOW_HEIGHT,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0,WINDOW_WIDTH,0,WINDOW_HEIGHT); 

    r.bot = WINDOW_HEIGHT - 72;
    r.left = 30;
    r.center = 0;
    unsigned int cref = 0xFFFF00;
    ggprint16(&r, 49, cref, "Requirements");
    r.left += 95;
    ggprint16(&r, 49, cref, "Design");
    r.left += 65;
    ggprint16(&r, 49, cref, "Coding");
    r.left += 62;
    ggprint16(&r, 49, cref, "Testing");
    r.bot = WINDOW_HEIGHT - 273;
    r.left += 45;
    ggprint16(&r, 49, cref, "Maintenance");
    r.bot = WINDOW_HEIGHT - 25;
    r.left = 5;
    ggprint8b(&r, 16, 0xffffff, "waterfall Model");
}
