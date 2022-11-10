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
#include <cstdlib>

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
#define DEFAULT_ARM_LENGTH	 2.0
#define SHAFT_DIAMETER	 0.5
#define SHAFT_LENGTH	 1.0
#define DEFAULT_LEG_LENGTH	 9.0		// fixme make adjustable
double arm_length = DEFAULT_ARM_LENGTH;
double leg_length = DEFAULT_LEG_LENGTH;

#define PLATFORM_HEIGHT	 20.0

#if 0				// in 30x30 cm base
#define B0x -6.5
#define B0y  6.1
#define B1x -8.3
#define B1y  3.0
#define B2x -2.8
#define B2y -8.5
#define B3x  3.8 
#define B3y -8.5
#define B4x  8.3
#define B4y  3.0
#define B5x  6.5
#define B5y  6.1
#endif
#if 0				// current
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
#endif
#if 0				// old
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

#define B0_ANGLE  2.69159
#define B1_ANGLE -2.69159
#define B2_ANGLE -1.4972
#define B3_ANGLE -0.597197
#define B4_ANGLE  0.597197
#define B5_ANGLE  1.4972
#define B0_AI	  0.020
#define B1_AI	  0.021
#define B2_AI	  0.018
#define B3_AI	  0.022
#define B4_AI	  0.017
#define B5_AI	  0.006
#define DEFAULT_BASE_RADIUS	7.5
double base_radius = DEFAULT_BASE_RADIUS;



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

#if 0
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
#endif

#define P0_ANGLE  2.54
#define P1_ANGLE -2.54
#define P2_ANGLE -1.65
#define P3_ANGLE -0.44
#define P4_ANGLE  0.44
#define P5_ANGLE  1.65
#define DEFAULT_PLATFORM_RADIUS	1.75
double platform_radius = DEFAULT_PLATFORM_RADIUS;



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
  servo (double x, double y, double sa, double aa, double ai, double fk) {
    pos.x = x;
    pos.y = y;
    rotation_angle = atan2 (y, x);
    shaft_angle    = sa;
    shaft_vector.x = cos (sa);
    shaft_vector.y = sin (sa);
    alpha = aa;
    alpha_incr = ai;
    fake_angle = fk;
  }
  position_s pos;
  double     rotation_angle;
  double     shaft_angle;
  vector_s   shaft_vector;
  double     alpha;		// angle of servo arm wrt x axis,
  double     alpha_incr;
  double     fake_angle;
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

namespace sps {
  namespace colors {
    enum color {
      none    = 0x00,
      black   = 0x01,
      red     = 0x02,
      green   = 0x03,
      yellow  = 0x04,
      blue    = 0x05,
      magenta = 0x06,
      cyan    = 0x07,
      white   = 0x08
    };
  }
  namespace faces {
    enum face {
      normal    = 0x00,
      bold      = 0x01,
      dark      = 0x02,
      uline     = 0x04,
      invert    = 0x07,
      invisible = 0x08,
      cline     = 0x09
    };
  }

  std::string set_color(sps::colors::color foreground = sps::colors::none,
			sps::colors::color background = sps::colors::none) {
    std::stringstream s;
    s << "\033[";
    if (!foreground && ! background){
        s << "0"; // reset colors if no params
    }
    if (foreground) {
        s << 29 + foreground;
        if (background) s << ";";
    }
    if (background) {
        s << 39 + background;
    }
    s << "m";
    return s.str();
  }

  std::string set_face(sps::faces::face face = sps::faces::normal) {
    std::stringstream s;
    s << "\033[";
    if (!face) {
        s << "0"; // reset face
    }
    if (face) {
      s << face;
    }
    s << "m";
    return s.str();
  }
}

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

spherical location = spherical (16.9, D2R (0.0), D2R (45.0));
spherical centre   = spherical (0.0, 12.8, 0.0);


bool one_shot  = false;
bool do_motion = true;
bool demo_mode = false;

double h0;				// base height based on geometry

static void
set_h0 ()
{
  h0 = 0.0;					// Eq 10
  for (int i = 0; i < 6; i++) {
    double xp = platform->anchors[i].x;		// anchor
    double yp = platform->anchors[i].y;		// anchor
    double xb = servos[i]->pos.x;		// servo
    double yb = servos[i]->pos.y;		// servo
    h0 += sqrt (pow (leg_length, 2.0) +
		pow (arm_length, 2.0) -
		(pow ((xp - xb), 2.0) +
		pow ((yp - yb), 2.0)));

  }
  h0 /= 6.0;
}

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
    base_z = platform->delta_z;
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
      platform->delta_y = base_y + target_y * attack_stage / 10.0; 
      platform->delta_z = base_z + target_z * attack_stage / 10.0; 
      platform->phi     = base_p + target_p * attack_stage / 10.0;
      platform->theta   = base_t + target_t * attack_stage / 10.0; 
      platform->rho     = base_r + target_r * attack_stage / 10.0; 
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
      pow (l, 2.0) - (pow (leg_length, 2.0) - pow (arm_length, 2.0));
    double M  = 2.0 * arm_length * deltaPB.y;
    double N  = 2.0 * arm_length *
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
  set_h0 ();
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
	glutSolidCylinder(ARM_RADIUS, arm_length,  32,   32);
	glutSolidSphere (0.3, 32, 32);  // shaft dot
        glFlush ();
        glPopMatrix();
	interMatrix =
	  glm::translate (fMtx, glm::vec3 (0.0f, 0.0f, arm_length));  // 687
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
      glutSolidCylinder(ARM_RADIUS, arm_length,  32,   32);
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
set_base_radius ()
{
  servos[0]->pos.x = base_radius * cos (B0_ANGLE);
  servos[0]->pos.y = base_radius * sin (B0_ANGLE);
  servos[1]->pos.x = base_radius * cos (B1_ANGLE);
  servos[1]->pos.y = base_radius * sin (B1_ANGLE);
  servos[2]->pos.x = base_radius * cos (B2_ANGLE);
  servos[2]->pos.y = base_radius * sin (B2_ANGLE);
  servos[3]->pos.x = base_radius * cos (B3_ANGLE);
  servos[3]->pos.y = base_radius * sin (B3_ANGLE);
  servos[4]->pos.x = base_radius * cos (B4_ANGLE);
  servos[4]->pos.y = base_radius * sin (B4_ANGLE);
  servos[5]->pos.x = base_radius * cos (B5_ANGLE);
  servos[5]->pos.y = base_radius * sin (B5_ANGLE);;
  set_h0 ();
}

static void
set_platform_radius ()
{
  platform->anchors[0].x = platform_radius * cos (P0_ANGLE);
  platform->anchors[0].z = platform_radius * sin (P0_ANGLE);
  platform->anchors[1].x = platform_radius * cos (P1_ANGLE);
  platform->anchors[1].z = platform_radius * sin (P1_ANGLE);
  platform->anchors[2].x = platform_radius * cos (P2_ANGLE);
  platform->anchors[2].z = platform_radius * sin (P2_ANGLE);
  platform->anchors[3].x = platform_radius * cos (P3_ANGLE);
  platform->anchors[3].z = platform_radius * sin (P3_ANGLE);
  platform->anchors[4].x = platform_radius * cos (P4_ANGLE);
  platform->anchors[4].z = platform_radius * sin (P4_ANGLE);
  platform->anchors[5].x = platform_radius * cos (P5_ANGLE);
  platform->anchors[5].z = platform_radius * sin (P5_ANGLE);;
  set_h0 ();
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
      platform->delta_z += 0.2;
      break;
    case GLUT_KEY_UP:
      platform->delta_z -= 0.2;
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
show_help ()
{
  fprintf (stdout, "\nKeys:\n");
  fprintf (stdout, "\tr	platform roll right\n");
  fprintf (stdout, "\tR	platform roll left\n");
  fprintf (stdout, "\tp	platform pitch down\n");
  fprintf (stdout, "\tP	platform pitch up\n");
  fprintf (stdout, "\ty	platform yaw right\n");
  fprintf (stdout, "\tY	platform yaw left\n");
  fprintf (stdout, "\tleft arrow	platform shift left\n");
  fprintf (stdout, "\tright arrow	platform shift right\n");
  fprintf (stdout, "\tup arrow	platform shift forward\n");
  fprintf (stdout, "\tdown arrow	platform shift backward\n");
  fprintf (stdout, "\thome	platform reset to initial\n");
  fprintf (stdout, "\td	platform lower\n");
  fprintf (stdout, "\tu	platform raise\n");
  fprintf (stdout, "\tm	pause motion\n");
  fprintf (stdout, "\tM	resume motion\n");

  fprintf (stdout, "\nControl Keys:\n");
  fprintf (stdout, "\tctrl-d	zoom in\n");
  fprintf (stdout, "\tctrl-D	zoom out\n");
  fprintf (stdout, "\tctrl-a	view azimuth right\n");
  fprintf (stdout, "\tctrl-A	view azimuth left\n");
  fprintf (stdout, "\tctrl-e	view elevation down\n");
  fprintf (stdout, "\tctrl-x	view pan left\n");
  fprintf (stdout, "\tctrl-X	view pan right\n");
  fprintf (stdout, "\tctrl-y	view pan down\n");
  fprintf (stdout, "\tctrl-Y	view pan up\n");
  fprintf (stdout, "\tctrl-z	view pan nearer\n");
  fprintf (stdout, "\tctrl-Z	view pan farther\n");
  fprintf (stdout, "\tctrl-left arrow	view pan left\n");
  fprintf (stdout, "\tctrl-right arrow	view pan right\n");
  fprintf (stdout, "\tctrl-up arrow	view pan down\n");
  fprintf (stdout, "\tctrl-down arrow	view pan up\n");
  fprintf (stdout, "\tctrl-home	view origin\n");

  fprintf (stdout, "\nAlt Keys:\n");
  fprintf (stdout, "\talt-r	reduce base radius\n");
  fprintf (stdout, "\talt-R	increase base radius\n");
  fprintf (stdout, "\talt-p	reduce platform radius\n");
  fprintf (stdout, "\talt-P	increase platform radius\n");
  fprintf (stdout, "\talt-l	decrease leg length\n");
  fprintf (stdout, "\talt-L	increase leg length\n");
  fprintf (stdout, "\talt-a	decrease arm length\n");
  fprintf (stdout, "\talt-A	increase arm length\n");
  fprintf (stdout, "\talt-d	display anchor points\n");
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
      location.incLatitude (D2R (1));
      showeye = true;
      break;
    case 'E':
      location.decLatitude (D2R (1));
      showeye = true;
      break;
    case 'x':
      centre.x -= 0.1;
      showlook = true;
      break;
    case 'X':
      centre.x += 0.1;
      showlook = true;
      break;
    case 'y':
      centre.y -= 0.1;
      showlook = true;
      break;
    case 'Y':
      centre.y += 0.1;
      showlook = true;
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
    case 'h':
    case 'H':
      show_help ();
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
    case 'r': base_radius -= 0.1; set_base_radius (); break;
    case 'R': base_radius += 0.1; set_base_radius (); break;
    case 'p': platform_radius -= 0.1; set_platform_radius (); break;
    case 'P': platform_radius += 0.1; set_platform_radius (); break;
    case 'l': leg_length -= 0.1; set_h0 (); break;
    case 'L': leg_length += 0.1; set_h0 (); break;
    case 'a': arm_length -= 0.1; set_h0 (); break;
    case 'A': arm_length += 0.1; set_h0 (); break;
    case 'd':
      fprintf (stdout, "base: (cm)\n");
      for (int i = 0; i < servos.size (); i++) {
	fprintf (stdout, "%d %g %g\n", i,
		 servos[i]->pos.x,
		 servos[i]->pos.y);
      }
      fprintf (stdout, "\nplatform (cm):\n");
      for (int i = 0; i < platform->anchors.size (); i++) {
	fprintf (stderr, "%d %g %g\n",
		 i, platform->anchors[i].x, platform->anchors[i].z);
      }
      fprintf (stdout, "\nArm length: %g cm\n", arm_length);
      fprintf (stdout, "\nLeg length: %g cm\n", leg_length);
      break;
    case 'h':
    case 'H':
      show_help ();
      break;
    }
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
    case 'h':
    case 'H':
      show_help ();
      break;
    }
  }
}

int
main(int argc, char **argv)
{
  {
#define GET_HELP  1000
    static struct option long_options[] = {
      {"width",		required_argument, 0,  'w' },
      {"height",	required_argument, 0,  'h' },
      {"record",	optional_argument, 0,  'r' },
      {"once",		no_argument,       0,  'o' },
      {"demo",		no_argument, 	   0,  'd' },
      {"motion",	no_argument, 	   0,  'm' },
      {"help",		no_argument, 	   0,   GET_HELP },
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
      case GET_HELP:
	fprintf (stderr, "\t-w v\n");
	fprintf (stderr, "\t--width=v\tset window width\n");
	
	fprintf (stderr, "\t-h v\n");
	fprintf (stderr, "\t--height=v\tset window height\n");
	
	fprintf (stderr, "\t-r [s]\n");
	fprintf (stderr, "\t--record=[s]\tset recording and optionall filename\n");
	
	fprintf (stderr, "\t-o\n");
	fprintf (stderr, "\t--once\tstop after one iteration\n");
	
	fprintf (stderr, "\t-d\n");
	fprintf (stderr, "\t--demo\tstart in demo mode\n");
	
	return 1;
	break;
      }
    }
  }

  signal (SIGINT, enditall);
  srand48 (time (NULL));

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
  platform->set_anchor (platform_radius * cos (P0_ANGLE),
			platform_radius * sin (P0_ANGLE));
  platform->set_anchor (platform_radius * cos (P1_ANGLE),
			platform_radius * sin (P1_ANGLE));
  platform->set_anchor (platform_radius * cos (P2_ANGLE),
			platform_radius * sin (P2_ANGLE));
  platform->set_anchor (platform_radius * cos (P3_ANGLE),
			platform_radius * sin (P3_ANGLE));
  platform->set_anchor (platform_radius * cos (P4_ANGLE),
			platform_radius * sin (P4_ANGLE));
  platform->set_anchor (platform_radius * cos (P5_ANGLE),
			platform_radius * sin (P5_ANGLE));
#if 0
  {
    double srad = 0.0;
    for (int i = 0; i < platform->anchors.size (); i++) {
      double ang = atan2 (platform->anchors[i].z, platform->anchors[i].x);
      double rad = hypot (platform->anchors[i].z, platform->anchors[i].x);
      srad += rad;
      fprintf (stderr, "\n%d %g %g rad %g ang %g\n",
	       i, platform->anchors[i].x, platform->anchors[i].z,
	       rad, ang);
    }
    fprintf (stderr, "avg rad = %g\n", srad /6.0);
  }
#endif

#define M_270 (3.0 * M_PI_2)
  servos.push_back (new servo (base_radius * cos (B0_ANGLE),
			       base_radius * sin (B0_ANGLE),
			       SA0, M_PI_2, B0_AI, 25.0));
  servos.push_back (new servo (base_radius * cos (B1_ANGLE),
			       base_radius * sin (B1_ANGLE),
			       SA1, M_PI_2, B1_AI, -25.0));
  servos.push_back (new servo (base_radius * cos (B2_ANGLE),
			       base_radius * sin (B2_ANGLE),
			       SA2, M_PI_2, B2_AI, 25.0));
  servos.push_back (new servo (base_radius * cos (B3_ANGLE),
			       base_radius * sin (B3_ANGLE),
			       SA3, M_PI_2, B3_AI, -25.0));
  servos.push_back (new servo (base_radius * cos (B4_ANGLE),
			       base_radius * sin (B4_ANGLE),
			       SA4, M_PI_2, B4_AI, 25.0));
  servos.push_back (new servo (base_radius * cos (B5_ANGLE),
			       base_radius * sin (B5_ANGLE),
			       SA5, M_PI_2, B5_AI, -25.0));
#if 0
  for (int i = 0; i < servos.size (); i++) {
    fprintf (stderr, "\n%d %g %g rad %g ang %g\n",
	     i, servos[i]->pos.x, servos[i]->pos.y,
	     hypot (servos[i]->pos.x, servos[i]->pos.y),
	     atan2 (servos[i]->pos.y, servos[i]->pos.x));
  }
#endif

  // pcb   14.6 x 11.5
  // panel 14.6 x 11.5
#if 1
  // servo module
  fprintf (stdout, "module servo(ang, loc, name) {\n");
  fprintf (stdout, "  rotate([0, 0, ang]) {\n");
  fprintf (stdout, "    translate([-4, -1.0, 0]) cube([4.0, 2.0, 4.05]);\n");
  fprintf (stdout, "    rotate([0,90,0]) translate([-3.5, 0, 0.0]) \
cylinder(h=3,r=0.5);\n");
  fprintf (stdout, "    linear_extrude(.1) translate([-2.5, -2.2, 0]) \
color([1,0,0]) text(loc, size=1);\n");
  fprintf (stdout, "    linear_extrude(.1) translate([-2.5, 1.3, 0]) \
color([1,0,0]) text(name, size=1);\n");
  fprintf (stdout, "  }\n");
  fprintf (stdout, "}\n");
  // smoothness
  fprintf (stdout, "$fn = 32;\n");
  // anchor radius
  fprintf (stdout, "radius = %g;\n", base_radius);
  
  // base
  fprintf (stdout, "translate([-10, -15, -.5]) color(\"green\") \
	   cube([40, 30, .4]);\n");

  // pcb
  fprintf (stdout, "translate([18.5, -15, 0.2]) color(\"blue\")		\
	   cube([11.5, 14.6, .2]);\n");
  fprintf (stdout, "linear_extrude(.1) translate([18, -15, 5.14]) color([1,0,0]) rotate([0, 0, 90]) {text(\"PCB\", size=1);}\n");

  // panel
  fprintf (stdout, "translate([18.5, -0, 2.5]) color(\"pink\")		\
	   cube([11.5, 14.6, .2]);\n");
  fprintf (stdout, "linear_extrude(.1) translate([17.5, 11, 0]) color([1,0,0]) rotate([0, 0, 90]) {text(\"Panel\", size=1);}\n");

  //anchors
  for (int i = 0; i < servos.size (); i++) {
    char loc[64];
    char name[64];
    sprintf (loc, "%.3g, %.3g", servos[i]->pos.x,servos[i]->pos.y);
    sprintf (name, "Servo %d", i);
    fprintf (stdout,
	     "rotate([0, 0, %g]) translate ([radius, 0, 0]) \
  servo(%g, \"%s\", \"%s\");\n",
	     R2D (atan2 (servos[i]->pos.y,servos[i]->pos.x)),
	     servos[i]->fake_angle, loc, name);
  }
#endif
  
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

  //https://stackoverflow.com/questions/36314690/c-fprintf-with-colors

  if (0) {
    using namespace sps;
    std::cout << set_color (colors::red)
	      << set_face (faces::bold)
	      << set_face (faces::invert)
	      << "\n            Press h for help            \n\n"
	      << set_color() << std::endl;
  }
  
  glutMainLoop ();

  return 0;
}

