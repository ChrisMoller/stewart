/***
    This file is part of the Stewart platform simulator.

    Copyright 2022 C. H. L. Moller

Stewart is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

Stewart is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
Stewart. If not, see <https://www.gnu.org/licenses/>.
 ***/

#include <errno.h>
#include <getopt.h>
#include <malloc.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_INTRINSICS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#if 0
#include <glm/gtc/bitfield.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/integer.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_integer.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp> 
#include <glm/gtc/reciprocal.hpp>
#include <glm/gtc/round.hpp>
//#include <glm/gtc/aligned.hpp>
#include <glm/gtc/type_precision.hpp>
#endif

#include "popen2.h"

#define D2R(d) ((d / 180.0) * M_PI)
#define R2D(r) ((r / M_PI) * 180.0)

/************* platform description ***************/


#define ARM_RADIUS	 0.2
#define ARM_LENGTH	 2.0
#define SHAFT_DIAMETER	 0.5
#define SHAFT_LENGTH	 1.0
#define LEG_LENGTH	 9.0

#define PLATFORM_HEIGHT	 20.0

/***
         5
             4
    0

    1
             3
          2



	  
                 +z
	  +y      ^
	  ^      /
	  |     /
	  |    /
	  |   /
	  |  /
	  | /
	  |/
	  o ------------> +x
 ***/
#if 1
#define B0x -5.40268
#define B0y  2.60979
#define B1x -5.40268
#define B1y -2.60979
#define B2x  0.441194
#define B2y -5.98376
#define B3x  4.96149
#define B3y -3.37396
#define B4x  4.96149
#define B4y  3.37396
#define B5x  0.441194
#define B5y  5.98376
#else
#define B0x	-5.2		// cm coords wrt world origin
#define B0y	 2.5
#define B1x	-5.2
#define B1y	-2.5
#define B2x	 0.5
#define B2y	-5.8
#define B3x	 4.8
#define B3y	-3.3
#define B4x	 4.8
#define B4y	 3.3
#define B5x	 0.5
#define B5y	 5.8
#endif

#if 1
#define SA0	M_PI_2			// radians shaft angle
#define SA1	M_PI_2
#define SA2	(-M_PI / 6.0)
#define SA3	(-M_PI / 6.0)
#define SA4	(7.0 * M_PI / 6.0)
#define SA5	(7.0 * M_PI / 6.0)
#else
#define SA0	(-2.0 * M_PI / 6.0)			// radians shaft angle
#define SA1	(-4.0 * M_PI / 6.0)
#define SA2	(-2.0 * M_PI / 6.0)
#define SA3	(-4.0 * M_PI / 6.0)
#define SA4	(-2.0 * M_PI / 6.0)
#define SA5	(-4.0 * M_PI / 6.0)
#endif

#if 0
#define ARBITRARY_PLATFORM_SCALE	0.3
#define oP0x	B0x * ARBITRARY_PLATFORM_SCALE	// cm coords wrt platform
#define oP0y	B0y * ARBITRARY_PLATFORM_SCALE
#define oP1x	B1x * ARBITRARY_PLATFORM_SCALE
#define oP1y	B1y * ARBITRARY_PLATFORM_SCALE
#define oP2x	B2x * ARBITRARY_PLATFORM_SCALE
#define oP2y	B2y * ARBITRARY_PLATFORM_SCALE
#define oP3x	B3x * ARBITRARY_PLATFORM_SCALE
#define oP3y	B3y * ARBITRARY_PLATFORM_SCALE
#define oP4x	B4x * ARBITRARY_PLATFORM_SCALE
#define oP4y	B4y * ARBITRARY_PLATFORM_SCALE
#define oP5x	B5x * ARBITRARY_PLATFORM_SCALE
#define oP5y	B5y * ARBITRARY_PLATFORM_SCALE
#endif

#define P0x -1.430
#define P0y  0.976
#define P1x -1.432
#define P1y -1.000
#define P2x -0.137
#define P2y -1.743
#define P3x  1.577
#define P3y -0.752
#define P4x  1.582
#define P4y  0.740
#define P5x -0.130
#define P5y  1.726



/************* end platform description ***************/


/**************** classes and typdefs  ****************/

typedef struct {
  double x;
  double y;
} position_s;

typedef struct {
  double x;
  double y;
} vector_s;

typedef enum {
  UP_X,
  UP_Y,
  UP_Z
} up_e;
up_e upi = UP_Y;

class servo {
public:
  servo (double x, double y, double sa, double aa, double ai) {
    pos.x = x;
    pos.y = y;
    rotation_angle = atan2 (y, x);
    shaft_angle    = sa;
    shaft_vector.x = cos (sa);
    shaft_vector.y = sin (sa);
    alpha = aa;
    alpha_incr = ai;
  }
  position_s pos;
  double     rotation_angle;
  double     shaft_angle;
  vector_s   shaft_vector;
  double     alpha;		// angle of servo arm wrt x axis,
  double     alpha_incr;
  glm::mat4  servo_mtx;
};

class _platform {
public:
  _platform () {
    delta_x = 0.0;
    delta_y = 0.0;
    delta_z = 0.0;
    phi    = 0.0;   
    theta  = 0.0; 
    rho    = 0.0;   
  }
  void set_anchor (double x, double y) {
    anchors.push_back (glm::vec3 ((float)x, 0.0f, (float)y));
    anchor_mtx.push_back  (glm::mat4 (0.0f));
    anchor_loc.push_back  (glm::mat4 (0.0f));
  }
  std::vector<glm::vec3>anchors;  
  std::vector<glm::mat4>anchor_mtx;
  std::vector<glm::mat4>anchor_loc;
  double delta_x;
  double delta_y;
  double delta_z;
  double phi;		// yaw		opengl y, real z
  double theta;		// pitch	opengl z, real y
  double rho;		// roll		opengl x, real x
};

class spherical {
public:
  double distance;
  double theta;		// longitude
  double phi;		// latitude
  double x;
  double y;
  double z;

private:
  void s2r () {
    double pp = /*M_PI_2 - */phi;
    x = distance * cos (theta) * sin (pp);
    z = distance * sin (theta) * sin (pp);
    y = distance * cos (pp);
  }

  void r2s () {
    distance	= sqrt (pow (x, 2.0) + pow (y, 2.0) + pow (z, 2.0));
    phi		= M_PI_2 + acos (z / distance);
    theta	= acos (x / (distance * sin (phi)));
  }

  void norm (double *ang) {
    if (*ang > 2.0 * M_PI) *ang -= 2.0 * M_PI;
    else if (*ang < 2.0 * M_PI) *ang += 2.0 * M_PI;
  }
  
public:
  spherical (double d, double t, double p)
  {
    distance = d;
    theta    = t;
    phi      = p;
    s2r ();
  }

  void setDistance  (double d) { distance = d;	s2r (); }
  void decDistance  (double d) { distance -= d;	s2r (); }
  void incDistance  (double d) { distance += d;	s2r (); }
  void setLongitude (double t) { theta = t;	norm (&theta); s2r (); }
  void decLongitude (double d) { theta -= d;	norm (&theta); s2r (); }
  void incLongitude (double d) { theta += d;	norm (&theta); s2r (); }
  void setLatitude  (double p) { phi = p;	norm (&phi);   s2r (); }
  void decLatitude  (double d) { phi -= d;	norm (&phi);   s2r (); }
  void incLatitude  (double d) { phi += d;	norm (&phi);   s2r (); }
};

/**************** end classes  ****************/

#define DEFAULT_WIDTH  500
#define DEFAULT_HEIGHT 500
#define DEFAULT_FILENAME "stewart.mp4"
#define READOUT_VIEW_X	-0.9f
#define READOUT_VIEW_Y	-0.9f
#define READOUT_PLATFORM_X	-0.9f
#define READOUT_PLATFORM_Y	 0.9f

int width  = DEFAULT_WIDTH;
int height = DEFAULT_HEIGHT;

pid_t ffmpeg_pid = -1;
char* filename = NULL;
FILE* ffmpeg = NULL;

std::vector<servo *> servos;
_platform *platform;

glm::vec3 textloc = glm::vec3 (0.0f, 0.0f, 0.0f);
float testAng = 0.0f;

spherical location = spherical (16.9, D2R (0.0), D2R (45.0));
spherical centre   = spherical (0.0, 12.8, 0.0);


bool one_shot  = false;
bool do_motion = true;
bool demo_mode = false;

double h0;				// base height based on geometry

static void
set_colours (int i)
{
  float red, green, blue;
  switch(i) {
  case 0:
    red = 1.0; green = 0.0, blue = 0.0;		// red
    break;
  case 1:
    red = 1.0; green = 1.0, blue = 0.0;		// yellow
    break;
  case 2:
    red = 0.0; green = 1.0, blue = 0.0;		// green
    break;
  case 3:
    red = 0.0; green = 1.0, blue = 1.0;		// cyan
    break;
  case 4:
    red = 0.0; green = 0.0, blue = 1.0;		// blue
    break;
  case 5:
    red = 1.0; green = 0.0, blue = 1.0;		// magenta
    break;
  }
  glColor3f (red, green, blue);
}

static void
showCurrentXform (int which, const char *title)
{
  fprintf (stderr, "\n%s\n", title);
  float m[16];
  glGetFloatv (which, m);
  int off = 0;
  for (int row = 0; row < 4; row++, off+=4) {
    fprintf (stderr, "%g %g %g %g\n",
	     (double)m[off],
	     (double)m[off+1],
	     (double)m[off+2],
	     (double)m[off+3]);
  }
}

static void
showVector (const char *title, glm::vec4 & vx)
{
  fprintf (stderr, "%s ", title);
  float *v = glm::value_ptr (vx);
  fprintf (stderr, "%g %g %g %g\n",
	   (double)v[0],
	   (double)v[1],
	   (double)v[2],
	   (double)v[3]);
}

static void
showVector3 (const char *title, glm::vec3 & vx)
{
  fprintf (stderr, "%s ", title);
  float *v = glm::value_ptr (vx);
  fprintf (stderr, "%g %g %g\n",
	   (double)v[0],
	   (double)v[1],
	   (double)v[2]);
}

static void
showMatrix (const char *title, glm::mat4 & mx)
{
  fprintf (stderr, "\n%s\n", title);
  float *m = glm::value_ptr (mx);
  int off = 0;
  for (int row = 0; row < 4; row++, off+=4) {
    fprintf (stderr, "%g %g %g %g\n",
	     (double)m[off],
	     (double)m[off+1],
	     (double)m[off+2],
	     (double)m[off+3]);
  }
}

void
enditall (int sig)
{
  if (ffmpeg && ffmpeg_pid >= 0) pclose2 (ffmpeg,  ffmpeg_pid);
  ffmpeg = NULL;
  write (fileno (stdout), "\n", 1);
  exit (0);
}

enum {
  JITTER_QUIET,
  JITTER_ATTACK,
  JITTER_ATTACK_CONTINUE,
  JITTER_DECAY
};

int jitter_mode = JITTER_ATTACK;

static unsigned int
do_jitter ()
{
  unsigned int sleep_time = 10000;
  static double target_x;
  static double target_y;
  static double target_z;
  static double target_p;
  static double target_t;
  static double target_r;
  static double attack_stage;
  static double base_x;
  static double base_y;
  static double base_z;
  static double base_p;
  static double base_t;
  static double base_r;
  switch (jitter_mode) {
  case JITTER_QUIET:		// do nothing
    break;
  case JITTER_ATTACK:
    base_x = platform->delta_x;
    base_y = platform->delta_y;
    base_z = platform->delta_y;
    base_p = platform->phi;
    base_t = platform->theta;
    base_r = platform->rho;
	
    target_x = 2.0 * (drand48 () - 0.5);
    target_y = 2.0 * (drand48 () - 0.5);
    target_z = 2.0 * (drand48 () - 0.5);
    target_p= 0.2 * (drand48 () - 0.5);
    target_t = 0.2 * (drand48 () - 0.5);
    target_r   = 0.2 * (drand48 () - 0.5);
    attack_stage = 0.0;
    jitter_mode = JITTER_ATTACK_CONTINUE;
    //break;					no break
  case JITTER_ATTACK_CONTINUE:
    if (attack_stage < 10.0) {
      platform->delta_x = base_x + target_x * attack_stage / 10.0;
      platform->delta_y = base_x + target_y * attack_stage / 10.0; 
      platform->delta_z = base_x + target_z * attack_stage / 10.0; 
      platform->phi     = base_x + target_p * attack_stage / 10.0;
      platform->theta   = base_x + target_t * attack_stage / 10.0; 
      platform->rho     = base_x + target_r * attack_stage / 10.0; 
      attack_stage += 0.2;
      sleep_time = 1000;
    }
    else jitter_mode = JITTER_DECAY;
    break;
  case JITTER_DECAY:
    if (attack_stage > 0.0) {
      platform->delta_x = target_x * attack_stage / 10.0;
      platform->delta_y = target_y * attack_stage / 10.0; 
      platform->delta_z = target_z * attack_stage / 10.0; 
      platform->phi     = target_p * attack_stage / 10.0;
      platform->theta   = target_t * attack_stage / 10.0; 
      platform->rho     = target_r * attack_stage / 10.0; 
      attack_stage -= 0.1;
      sleep_time = 50000;
    }
    else {
      jitter_mode = JITTER_ATTACK;
      sleep_time = 3000000;
    }
    break;
  }
  return sleep_time;
}

static void
update_alpha ()
{
  /***

      Based on the paper "The Mathematics of the Stewart Platform" from
https://content.instructables.com/ORIG/FFI/8ZXW/I55MMY14/FFI8ZXWI55MMY14.pdf

  ***/
  
  // Eq 1
  glm::mat4 xrot =	// pitch
    glm::rotate ((float)platform->theta, glm::vec3 (1.0f, 0.0f, 0.0f));
  glm::mat4 yrot =	// roll
    glm::rotate ((float)platform->rho, glm::vec3 (0.0f, 1.0f, 0.0f));
  glm::mat4 zrot =	// yaw
    glm::rotate ((float)platform->phi, glm::vec3 (0.0f, 0.0f, 1.0f));
  glm::mat4 compositeRotation = xrot * yrot * zrot;

  double alpha_stage[6];
  bool is_valid = true;
  
  for (int i = 0; i < servos.size (); i++) {
    // Eq 3
    glm::vec4 P =
      glm::vec4 ((float)platform->delta_x,
		 (float)(platform->delta_y + h0),
		 (float)platform->delta_z, 1.0f) +
      (compositeRotation *
       glm::vec4 ((float)platform->anchors[i].x,
		  (float)platform->anchors[i].y,
		  (float)platform->anchors[i].z, 1.0f));

    glm::vec3 B = glm::vec3 (servos[i]->pos.x, 0.0, servos[i]->pos.y);

    glm::vec3 deltaPB = glm::vec3 (P) - B;

    // Eq 9
    double beta = (i & 1) ? (M_PI/6.0) : (-M_PI/6.0);
    double l = (double)glm::length (deltaPB);
    double L  =
      pow (l, 2.0) - (pow (LEG_LENGTH, 2.0) - pow (ARM_LENGTH, 2.0));
    double M  = 2.0 * ARM_LENGTH * deltaPB.y;
    double N  = 2.0 * ARM_LENGTH *
      (deltaPB.x * cos (beta) + deltaPB.z * sin (beta));
    N = -fabs (N);
    double arg = L / sqrt (pow (M, 2.0) + pow (N, 2.0));
    double alpha = asin (arg) - atan2 (N, M);
    if (isnan (alpha)) {
      is_valid = false;
      break;
    }
    else alpha_stage[i] = (i&1) ? -alpha : alpha;
  }
  if (is_valid) {
    for (int i = 0; i < servos.size (); i++)
      servos[i]->alpha = alpha_stage[i];
  }
}

static void
update_positions ()
{
  unsigned int sleep_time = 10000;
  if (do_motion) {
    if (demo_mode) {
      sleep_time = do_jitter ();
    }
    else {		// simple automation
      for (int i = 0; i < servos.size (); i++) {
	servos[i]->alpha += servos[i]->alpha_incr;
	if (servos[i]->alpha >  3.0 * M_PI_2 ||
	    servos[i]->alpha <  M_PI_2)
	  servos[i]->alpha_incr = -servos[i]->alpha_incr;
      }
    }

    update_alpha ();
    
    if (one_shot) enditall (0);
  }
  usleep (sleep_time);
}

static void
init(void)
{

  /***
      page 4
      
      Ai are the points of the arm/leg joint on the servo with coordinates

      a = [xa, ya, za] in the base framework.

      Bi are the points of rotation of the servo arms with the coordinates

      b = [xb, yb, zb] in the base framework.

      Pi are the points the joints between the operating rods and the platform,

      p = [xp, yp, zp] with coordinates in the platform framework

      z                 P = [platform->anchors[i].x, platform->anchors[i].y, 0]
      o------------------o
      |                   \
      |                    \
      |                     \
      |                      \  
      |                       \
      |                        \
      |    y                    \
      |   /                      o A = [sqrt(xb^2 + AL^2), yb + AL, 0]
      |  /                      /                     
      | /                      /
      o--------------------->o B = [servos[i]->pos.x, servos[i]->pos.y, 0]
                 x

   ***/
  h0 = 0.0;					// Eq 10
  for (int i = 0; i < 6; i++) {
    double xp = platform->anchors[i].x;		// anchor
    double yp = platform->anchors[i].y;		// anchor
    double xb = servos[i]->pos.x;		// servo
    double yb = servos[i]->pos.y;		// servo
    h0 += sqrt (pow (LEG_LENGTH, 2.0) +
		pow (ARM_LENGTH, 2.0) -
		(pow ((xp - xb), 2.0) +
		pow ((yp - yb), 2.0)));

  }
  h0 /= 6.0;
  update_alpha ();
	     
  GLfloat white[]       = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat black[]       = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat direction0[]   = { 15.0, 15.0, -15.0, 0.0 };
  GLfloat direction1[]   = { -15.0, 15.0, -15.0, 0.0 };
  GLfloat direction2[]   = { 0.0, -10.0, 0.0, 0.0 };
  GLfloat direction3[]   = { 0.0, 20.0, 0.0, 0.0 };

  glLightfv(GL_LIGHT0, GL_AMBIENT, black);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
  glLightfv(GL_LIGHT0, GL_POSITION, direction0);

  glLightfv(GL_LIGHT1, GL_AMBIENT, black);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
  glLightfv(GL_LIGHT1, GL_POSITION, direction1);

  glLightfv(GL_LIGHT2, GL_AMBIENT, black);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, white);
  glLightfv(GL_LIGHT2, GL_POSITION, direction2);

  glLightfv(GL_LIGHT3, GL_AMBIENT, black);
  glLightfv(GL_LIGHT3, GL_DIFFUSE, white);
  glLightfv(GL_LIGHT3, GL_POSITION, direction3);

  glClearColor (0.0, 0.0, 0.0, 0.0);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glEnable (GL_LIGHT1);
  glEnable (GL_LIGHT2);
  glEnable (GL_LIGHT3);
  glEnable(GL_COLOR_MATERIAL);


}

static void
spin (void)
{
 
  update_positions ();
  glutPostRedisplay();
}

static void
draw_platform ()
{
#define FACE_NEAR	-3.5
#define FACE_FAR	 3.5
#define FACE_LEFT	-3.5
#define FACE_RIGHT	 3.5
#define FACE_BOTTOM	-0.25
#define FACE_TOP	 0.25
  
  //Multi-colored side - FRONT
  glBegin(GL_POLYGON);
  glVertex3f (FACE_LEFT,	FACE_BOTTOM,	FACE_NEAR);       // P1
  glVertex3f (FACE_LEFT,	FACE_TOP,	FACE_NEAR);       // P2
  glVertex3f (FACE_RIGHT,	FACE_TOP,	FACE_NEAR);       // P3
  glVertex3f (FACE_RIGHT,	FACE_BOTTOM,	FACE_NEAR);       // P4
  glEnd();

  // White side - BACK
  glBegin(GL_POLYGON);
  glColor3f(   0.5,  0.5, 0.5 );
  glVertex3f (FACE_RIGHT,	FACE_BOTTOM,	FACE_FAR );
  glVertex3f (FACE_RIGHT,	FACE_TOP,	FACE_FAR );
  glVertex3f (FACE_LEFT,	FACE_TOP,	FACE_FAR );
  glVertex3f (FACE_LEFT,	FACE_BOTTOM,	FACE_FAR );
  glEnd();

  // Purple side - RIGHT
  glBegin(GL_POLYGON);
  glColor3f(  0.5,  0.0,  0.5 );
  glVertex3f (FACE_FAR, FACE_BOTTOM,	FACE_NEAR );
  glVertex3f (FACE_FAR, FACE_TOP,	FACE_NEAR );
  glVertex3f (FACE_FAR, FACE_TOP, 	FACE_FAR );
  glVertex3f (FACE_FAR, FACE_BOTTOM, 	FACE_FAR );
  glEnd();

// Green side - LEFT
  glBegin(GL_POLYGON);
  glColor3f(   0.0,  0.5,  0.0 );
  glVertex3f (FACE_LEFT,	FACE_BOTTOM, 	FACE_FAR );
  glVertex3f (FACE_LEFT,	FACE_TOP, 	FACE_FAR );
  glVertex3f (FACE_LEFT,	FACE_TOP,	FACE_NEAR );
  glVertex3f (FACE_LEFT,	FACE_BOTTOM,	FACE_NEAR );
  glEnd();

  // Blue side - TOP
  glBegin(GL_POLYGON);
  glColor3f(   0.0,  0.0,  0.5 );
  glVertex3f (FACE_RIGHT,	FACE_TOP, 	FACE_FAR );
  glVertex3f (FACE_RIGHT,	FACE_TOP,	FACE_NEAR );
  glVertex3f (FACE_LEFT,	FACE_TOP,	FACE_NEAR );
  glVertex3f (FACE_LEFT,	FACE_TOP, 	FACE_FAR );
  glEnd();

  // Red side - BOTTOM
  glBegin(GL_POLYGON);
  glColor3f(   0.5,  0.0,  0.0 );
  glVertex3f (FACE_RIGHT,	FACE_BOTTOM,	FACE_NEAR );
  glVertex3f (FACE_RIGHT,	FACE_BOTTOM, 	FACE_FAR );
  glVertex3f (FACE_LEFT,	FACE_BOTTOM, 	FACE_FAR );
  glVertex3f (FACE_LEFT,	FACE_BOTTOM,	FACE_NEAR );
  glEnd();
}

void
renderString(float x, float y, void *font, const unsigned char* string,
		  float red, float green, float blue)
{  
  char *c;


  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glRasterPos3f (x, y, 0.0f);

  glutBitmapString(font, string);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

static void
show_platform (glm::mat4 &baseXform)
{
  glm::vec3 transVec = glm::vec3 ((float)platform->delta_x,
				  (float)(h0 + platform->delta_y),
				  (float)platform->delta_z);

  glm::mat4 transMatrix = glm::translate (glm::mat4(1.0f), transVec);

  glm::mat4 xrot =	// pitch
    glm::rotate ((float)platform->theta, glm::vec3 (1.0f, 0.0f, 0.0f));
  glm::mat4 yrot =	// roll
    glm::rotate ((float)platform->rho, glm::vec3 (0.0f, 1.0f, 0.0f));
  glm::mat4 zrot =	// yaw
    glm::rotate ((float)platform->phi, glm::vec3 (0.0f, 0.0f, 1.0f));
  glm::mat4 compositeRotation = xrot * yrot * zrot;
      
  glm::mat4 compositeMatrix = transMatrix * compositeRotation;

  {
    glPushMatrix();
    glLoadMatrixf (glm::value_ptr (baseXform * compositeMatrix));

    draw_platform ();
  
    glFlush ();
    glPopMatrix();
  }

  for (int i = 0; i < platform->anchors.size (); i++) {
    glm::vec3 pos = platform->anchors[i];
    glm::mat4 anchorMatrix = glm::translate (glm::mat4(1.0f), pos);
    glm::mat4 localAnchorMatrix = compositeRotation * anchorMatrix;
    glm::mat4 compositeAnchorMatrix = transMatrix * localAnchorMatrix;
    {
      glPushMatrix();
      glLoadMatrixf (glm::value_ptr (baseXform * compositeAnchorMatrix));
      platform->anchor_mtx[i]  = compositeAnchorMatrix;
      
      set_colours (i);
      glutSolidSphere (0.4, 32, 32);
  
      glFlush ();
      glPopMatrix();
    }
  }
}

static void
show_servos (glm::mat4 &baseXform)
{
  for (int i = 0; i < servos.size (); i++) {    // servos
    set_colours (i);

    glPushMatrix();

    glm::vec3 transVec = glm::vec3 (servos[i]->pos.x, 0.0, servos[i]->pos.y);
    glm::mat4 transMatrix = glm::translate (glm::mat4(1.0f), transVec);
    glLoadMatrixf (glm::value_ptr (baseXform * transMatrix));
    
    glutSolidSphere (0.3, 32, 32);		  // shaft dot

    {
      glPushMatrix();  // shaft and actuator arm		
      glm::mat4 arm_angle =
	glm::rotate ((float)servos[i]->shaft_angle, glm::vec3 (0.0, 1.0, 0.0));
      glm::mat4 currentMtx = transMatrix * arm_angle;
      glLoadMatrixf (glm::value_ptr (baseXform *currentMtx));
      glutSolidCylinder(SHAFT_DIAMETER / 2.0, -SHAFT_LENGTH,  32,   32); //683
      glFlush ();
      {
	glm::mat4 interMatrix =
	  glm::translate (currentMtx, glm::vec3 (0.0f, 0.0f, -1.0f));  // 687
	glm::mat4 x90Mtx   =
	  glm::rotate ((float)M_PI_2, glm::vec3 (1.0f, 0.0f, 0.0f));
	double adjAlpha = (i&1) ? -M_PI_2 : M_PI_2;
	glm::mat4 alphaMtx = 
	  glm::rotate ((float)(servos[i]->alpha +adjAlpha),
		       glm::vec3 (0.0f, 1.0f, 0.0f));
	glPushMatrix();
	glm::mat4 fMtx = interMatrix * x90Mtx * alphaMtx;
	glLoadMatrixf (glm::value_ptr (baseXform * fMtx));
	glutSolidCylinder(ARM_RADIUS, ARM_LENGTH,  32,   32);
	glutSolidSphere (0.3, 32, 32);  // shaft dot
        glFlush ();
        glPopMatrix();
	interMatrix =
	  glm::translate (fMtx, glm::vec3 (0.0f, 0.0f, ARM_LENGTH));  // 687
	servos[i]->servo_mtx = interMatrix;
      }
      glFlush ();
      glPopMatrix();
    }
    glFlush ();
    glPopMatrix();
  }
}

static void
show_links (glm::mat4 &baseXform)
{
  for (int i = 0; i < servos.size (); i++) {    // servos
    set_colours (i);
    glm::vec4 anc_loc =
      platform->anchor_mtx[i] *
      glm::vec4 (0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 svo_loc =
      servos[i]->servo_mtx *
      glm::vec4 (0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 delta4 = anc_loc - svo_loc;
    glm::vec3 delta = glm::vec3 (delta4);

    //    fprintf (stderr, "%d length %g\n", i, (double)glm::length (delta));

    float dotprod = glm::dot (delta, glm::vec3 (0.0f, 0.0f, 1.0f));
    float ang = (float)(acos (dotprod / glm::length (delta)));
    glm::vec3 rotAxis =
      glm::normalize (glm::cross (delta, glm::vec3 (0.0f, 0.0f, 1.0f)));
    {
      glm::mat4 rotMtx = glm::rotate (-ang, rotAxis);
      glPushMatrix();
      glm::mat4 tx = glm::translate (glm::mat4 (1.0f), glm::vec3 (svo_loc));
      glLoadMatrixf (glm::value_ptr ( baseXform * tx * rotMtx));
      glutSolidCylinder(ARM_RADIUS/2.0f, glm::length (delta),  32,   32);
      glFlush ();
      glPopMatrix();
    }
    
    
    {					// servo ball
      glPushMatrix();
      glLoadMatrixf (glm::value_ptr (baseXform * servos[i]->servo_mtx));
      glutSolidSphere (0.3, 32, 32);
      glFlush ();
      glPopMatrix();
    }

#if 0			// anchor balls just for test
    {
      glPushMatrix();
      glLoadMatrixf (glm::value_ptr ( baseXform * platform->anchor_mtx[i]));
      glutSolidSphere (0.6, 32, 32);
      glutSolidCylinder(ARM_RADIUS, ARM_LENGTH,  32,   32);
      glFlush ();
      glPopMatrix();
    }
#endif
  }
}


static void
display(void)
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glShadeModel (GL_SMOOTH);

  glLoadIdentity ();
  double ux, uy, uz;

  switch(upi) {
  case UP_X: ux =  1.0; uy =  0.0; uz =  0.0; break;
  case UP_Y: ux =  0.0; uy =  1.0; uz =  0.0; break;
  case UP_Z: ux =  0.0; uy =  0.0; uz =  1.0; break;
  }

  gluLookAt (location.x, location.y, location.z,       // eye
	     centre.x, centre.y, centre.z,             // ctr
             ux, uy, uz);               // up

  char *string;
  asprintf (&string, "location %#0.3g\t%#0.3g\t%#g\nlookat %#0.3g\t%#0.3g\t%#g\n",
	    location.x, location.y, location.y,
	    centre.x, centre.y, centre.z);
  renderString (READOUT_VIEW_X, READOUT_VIEW_Y, GLUT_BITMAP_HELVETICA_18,
	       (const unsigned char*)string,
	       1.0f, 1.0f, 0.0f);
  free (string);
  asprintf (&string, "platform\nroll %#0.3g\tpitch %#0.3g\tyaw %#g\noffset %#0.3g\t%#0.3g\t%#g\n",
	    platform->rho, platform->theta, platform->phi,
	    platform->delta_x, h0 + platform->delta_y, platform->delta_z);
  renderString (READOUT_PLATFORM_X, READOUT_PLATFORM_Y,
		GLUT_BITMAP_HELVETICA_18,
	       (const unsigned char*)string,
	       1.0f, 1.0f, 0.0f);

  glm::mat4 baseXform;
  {
    float m[16];
    glGetFloatv (GL_MODELVIEW_MATRIX, m);
    baseXform = glm::make_mat4(m);
  }

  show_platform (baseXform);

  show_servos (baseXform);

  show_links (baseXform);


  if (ffmpeg) {
    void *buffer = malloc (sizeof(int) * width * height);
    glReadBuffer (GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    fwrite(buffer, sizeof(int) * width * height, 1, ffmpeg);
    free (buffer);
  }
  glutSwapBuffers();
}

static void
reshape (int w, int h)
{	
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (60, (GLfloat) w / (GLfloat) h, 1.0, 100.0);
  glMatrixMode (GL_MODELVIEW);
}

static void
specialkeys (int key, int x, int y)
{
  // https://www.opengl.org/resources/libraries/glut/spec3/node54.html#SECTION00089000000000000000
  int mod = glutGetModifiers ();
  if ((mod & ~GLUT_ACTIVE_SHIFT) == GLUT_ACTIVE_CTRL) {	// move camera
    bool showeye  = false;
    bool showlook = false;
    switch (key) {
    case GLUT_KEY_LEFT:
      centre.x -= 0.1;
      showlook = true;
      break;
    case GLUT_KEY_RIGHT:
      centre.x += 0.1;
      showlook = true;
      break;
    case GLUT_KEY_DOWN:
      centre.y -= 0.1;
      showlook = true;
      break;
    case GLUT_KEY_UP:
      centre.y += 0.1;
      showlook = true;
      break;
    case GLUT_KEY_HOME:
      centre.x = 0.0;
      centre.y = 0.0;
      centre.z = 0.0;
      break;
    }
#if 0
    if (showeye)
      fprintf (stderr, "eye %#0.3g\t%#0.3g\t%#g\n",
	       location.distance, location.theta, location.phi);
    if (showlook)
      fprintf (stderr, "lookat %#0.3g\t%#0.3g\t%#g\n",
	       centre.x, centre.y, centre.z);
#endif
  }
  else {	// move platform un-ctrled, un-alted
    switch (key) {
    case GLUT_KEY_LEFT:
      platform->delta_x -= 0.2;
      break;
    case GLUT_KEY_RIGHT:
      platform->delta_x += 0.2;
      break;
    case GLUT_KEY_DOWN:
      platform->delta_z -= 0.2;
      break;
    case GLUT_KEY_UP:
      platform->delta_z += 0.2;
      break;
    case GLUT_KEY_HOME:
      platform->rho   = 0.0;
      platform->phi   = 0.0;
      platform->theta = 0.0;
      platform->delta_x = 0.0;
      platform->delta_y = 0.0;
      platform->delta_z = 0.0;
      break;
    }
  }
}

static void
keyboard (unsigned char key, int x, int y)
{
  if (key == 27 || key == 'q') {
    if (ffmpeg && ffmpeg_pid >= 0) pclose2 (ffmpeg,  ffmpeg_pid);
    ffmpeg = NULL;
    write (fileno (stdout), "\n", 1);
    glutLeaveMainLoop ();
  }
  
  int mod = glutGetModifiers ();
  if ((mod & ~GLUT_ACTIVE_SHIFT) == GLUT_ACTIVE_CTRL) {	// move camera
    bool showeye  = false;
    bool showlook = false;
    key += (mod & GLUT_ACTIVE_SHIFT) ? 0x40 : 0x60;
    switch (key) {
    case 'd':				// distance
      location.decDistance (0.1);
      showeye = true;
      break;
    case 'D':
      location.incDistance (0.1);
      showeye = true;
      break;
    case 'a':				// azimuth
      location.decLongitude (D2R (1));
      showeye = true;
      break;
    case 'A':
      location.incLongitude (D2R (1));
      showeye = true;
      break;	
    case 'e':				// elevation
      location.decLatitude (D2R (1));
      showeye = true;
      break;
    case 'E':
      location.incLatitude (D2R (1));
      showeye = true;
      break;
    case 'z':
      centre.z -= 0.1;
      showlook = true;
      break;
    case 'Z':
      centre.z += 0.1;
      showlook = true;
      break;
    case 'u':
      upi = (up_e)(((int)upi+1)%3);
      break;
    case '?':
      fprintf (stderr, "ctrl-d/D - eye distance\n");
      fprintf (stderr, "ctrl-a/A - eye azimuth");
      fprintf (stderr, "ctrl-e/E - eye elevation");
      fprintf (stderr, "ctrl-z/Z - lookat z");
      break;
    }
#if 0
    if (showeye)
      fprintf (stderr, "eye %#0.3g\t%#0.3g\t%#g\n",
	       location.distance, location.theta, location.phi);
    if (showlook)
      fprintf (stderr, "lookat %#0.3g\t%#0.3g\t%#g\n",
	       centre.x, centre.y, centre.z);
#endif
  }
  else if ((mod & ~GLUT_ACTIVE_SHIFT) == GLUT_ACTIVE_ALT) {
    bool showit = false;
    switch(key) {
    case 'x': textloc.x -= 0.1f; showit = true; break;
    case 'X': textloc.x += 0.1f; showit = true; break;
    case 'y': textloc.y -= 0.1f; showit = true; break;
    case 'Y': textloc.y += 0.1f; showit = true; break;
    case 'z': textloc.z -= 0.1f; showit = true; break;
    case 'Z': textloc.z += 0.1f; showit = true; break;
    case 'a': testAng -= 0.01f; break;
    case 'A': testAng += 0.01f; break;
    }
#if 0
    if (showit)
      fprintf (stderr, "txt %g %g %g\n", textloc.x, textloc.y, textloc.z);
#endif
  }
  else {	// move platform un-ctrled, un-alted
    switch (key) {
    case 'r':				// roll
      platform->phi -= 0.02;
      break;
    case 'R':
      platform->phi += 0.02;
      break;
    case 'p':				// pitch
      platform->theta -= 0.02;
      break;
    case 'P':
      platform->theta += 0.02;
      break;
    case 'y':				// yaw
      platform->rho -= 0.02;
      break;
    case 'Y':
      platform->rho += 0.02;
      break;
    case 'd':				// down
      platform->delta_y -= 0.2;
      break;
    case 'u':				// up
      platform->delta_y += 0.2;
      break;
    case '?':
      fprintf (stderr, "y/Y - platform yaw\n");
      fprintf (stderr, "p/P - platform pitch\n");
      fprintf (stderr, "r/R - platform roll\n");
      fprintf (stderr, "z/Z - platform in/out\n");
      fprintf (stderr, "m   - motion on/off\n");
      break;
    case 'm':
      do_motion = false;
      break;
    case 'M':
      do_motion = true;
      break;
    }
  }
}

int
main(int argc, char **argv)
{
  signal (SIGINT, enditall);
  srand48 (time (NULL));

  {
    static struct option long_options[] = {
      {"width",		required_argument, 0,  'w' },
      {"height",	required_argument, 0,  'h' },
      {"record",	optional_argument, 0,  'r' },
      {"once",		no_argument,       0,  'o' },
      {"demo",		no_argument, 	   0,  'd' },
      {"motion",	no_argument, 	   0,  'm' },
      {0, 0, 0, 0 }
    };

    int c = 0;
    int option_index = 0;
    while (c != -1) {
      c = getopt_long(argc, argv, "h:w:r::odm", long_options, &option_index);
      switch(c) {
      case 'w':
	if (optarg) width = atoi (optarg);
	break;
      case 'h':
	if (optarg) height = atoi (optarg);
	break;
      case 'r':
	filename = strdup (optarg ?: DEFAULT_FILENAME);
	break;
      case 'o':
	one_shot = true;
	break;
      case 'd':
	demo_mode = true;
	do_motion = true;
	break;
      case 'm':
	do_motion = true;
	break;
      }
    }
  }

  // https://computergraphics.stackexchange.com/questions/5606/opengl-animation-turn-into-mp4-movie

#if 0
  const char* cmd = "ffmpeg -r 60 -f rawvideo -pix_fmt rgba -s 500x500 -i - "
    "-threads 0 -preset fast -y -pix_fmt yuv420p -crf 21 -vf vflip output.mp4";
#endif

#define FFMPEF_CMD "ffmpeg -r 60 -f rawvideo -pix_fmt rgba -s %dx%d -i - \
    -threads 0 -preset fast -y -pix_fmt yuv420p -crf 21 -vf vflip %s"

  if (filename) {
    char* ffmpeg_cmd;
    asprintf (&ffmpeg_cmd,  FFMPEF_CMD, width, height, filename);

    ffmpeg = popen2 (ffmpeg_cmd, "w", &ffmpeg_pid);
    if (errno != 0)
      perror ("Opening mpeg");
    free (ffmpeg_cmd);
  }

 
  platform = new _platform ();
  platform->set_anchor (P0x, P0y);
  platform->set_anchor (P1x, P1y);
  platform->set_anchor (P2x, P2y);
  platform->set_anchor (P3x, P3y);
  platform->set_anchor (P4x, P4y);
  platform->set_anchor (P5x, P5y);

#define M_270 (3.0 * M_PI_2)
  servos.push_back (new servo (B0x, B0y, SA0,	M_PI_2,	 0.020));
  servos.push_back (new servo (B1x, B1y, SA1,   M_270,	-0.021));
  servos.push_back (new servo (B2x, B2y, SA2,   M_PI_2,	 0.018));
  servos.push_back (new servo (B3x, B3y, SA3,  	M_270,	-0.022));
  servos.push_back (new servo (B4x, B4y, SA4,   M_PI_2,	 0.017));
  servos.push_back (new servo (B5x, B5y, SA5,   M_270,	-0.006));
  
  glutInit(&argc, argv);

  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize (width, height);
  glutCreateWindow ("Stewart");

  init ();
  glutDisplayFunc (display);
  glutReshapeFunc (reshape);
  glutKeyboardFunc (keyboard);
  glutSpecialFunc (specialkeys);
  glutIdleFunc (spin);
  glutMainLoop ();

  return 0;
}

