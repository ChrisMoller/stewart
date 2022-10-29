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

#include <Servo.h>
#include <BasicLinearAlgebra.h>

#define ARM_LENGTH	 2.0
#define LEG_LENGTH	 9.0

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

typedef struct {
  double x;
  double y;
} pos_s;

typedef struct {
  double x;
  double y;
  double z;
} pos3_s;


class Platform {
  public:
  Platform() {
    pitch = 0;
    roll  = 0;
    yaw   = 0;
    dx    = 0;
    dy    = 0;
    dz    = 0;
    anchor[0].x = P0x; anchor[0].y = 0.0; anchor[0].z = P0y;
    anchor[1].x = P1x; anchor[1].y = 0.0; anchor[1].z = P1y;
    anchor[2].x = P2x; anchor[2].y = 0.0; anchor[2].z = P2y;
    anchor[3].x = P3x; anchor[3].y = 0.0; anchor[3].z = P3y;
    anchor[4].x = P4x; anchor[4].y = 0.0; anchor[4].z = P4y;
    anchor[5].x = P5x; anchor[5].y = 0.0; anchor[5].z = P5y;
  }
  double pitch;
  double roll;
  double yaw;
  double dx;
  double dy;
  double dz;
  pos3_s anchor[6];
};

class Base {
  public:
  Base() {
    pos[0].x = B0x; pos[0].y = B0y;
    pos[1].x = B1x; pos[1].y = B1y;
    pos[2].x = B2x; pos[2].y = B2y;
    pos[3].x = B3x; pos[3].y = B3y;
    pos[4].x = B4x; pos[4].y = B4y;
    pos[5].x = B5x; pos[5].y = B5y;
  }
  pos_s pos[6];
};

#define NUM_SERVOS 6
Servo myServo[NUM_SERVOS];

Platform myPlatform;
Base myBase;

double h0;

 
// x-axis
static BLA::Matrix<4,4> rotatePitch(double r)
{
  BLA::Matrix<4,4> mtx = {
    1.0,     0.0,    0.0,        0.0,
    0.0,     cos(r), -sin(r),    0.0,
    0.0,     sin(r),  cos(r),    0.0,
    0.0,     0.0,     0.0,       1.0
  };
  return mtx;
}

// y-axis
static BLA::Matrix<4,4> rotateRoll(double r)
{
  BLA::Matrix<4,4> mtx = {
    cos(r),  0.0, sin(r), 0.0,
    0.0,     1.0, 0.0,    0.0,
    -sin(r), 0.0, cos(r), 0.0,
    0.0,     0.0, 0.0,    1.0
  };
  return mtx;
}

// z-axis
static BLA::Matrix<4,4> rotateYaw(double r)
{
  BLA::Matrix<4,4> mtx = {
    cos(r),  -sin(r), 0.0, 0.0,
    sin(r),   cos(r), 0.0, 0.0,
    0.0,      0.0,    1.0, 0.0,
    0.0,      0.0,    0.0, 1.0
  };
  return mtx;
}

#if 0
static void showVector(char *t, BLA::Matrix<4> v)
{
  Serial.println(t);
  Serial << v;
  Serial.print("\n");
}

static void showMatrix(char *t, BLA::Matrix<4,4> v)
{
  Serial.println(t);
  Serial << v;
  Serial.print("\n");
}
#endif

static double length(BLA::Matrix<4> v)
{
  return sqrt((v(0) * v(0)) +
                  (v(1) * v(1)) +
                  (v(2) * v(2)));
}

#define R2D(r) ((r / M_PI) * 180.0)

double alpha[6] = {180.0, 0.0, 180.0, 0.0, 180.0, 0.180};
double incr[8];
double pincr;

static void update_alpha()
{

  BLA::Matrix<4,4> pRot = rotatePitch(myPlatform.pitch);
  BLA::Matrix<4,4> rRot = rotatePitch(myPlatform.roll);
  BLA::Matrix<4,4> yRot = rotateYaw(myPlatform.yaw);
  BLA::Matrix<4,4> cRot = pRot * rRot * yRot;

  //double alpha_stage[6];
  bool is_valid = true;

  for (int i = 0; i < NUM_SERVOS; i++) {
    // Eq 3
    BLA::Matrix<4> P0 = {myPlatform.dx,
                        myPlatform.dy + h0,
                        myPlatform.dz, 1.0}; 
    
    BLA::Matrix<4> P1 = {myPlatform.anchor[i].x,
                         myPlatform.anchor[i].y,
                         myPlatform.anchor[i].z, 
                         1.0};
                                      
    BLA::Matrix<4> P  = P0 +  (cRot * P1);

    BLA::Matrix<4> B = {myBase.pos[i].x,
                        0.0,
                        myBase.pos[i].y, 
                        1.0};
    BLA::Matrix<4> deltaPB = P - B;
    
    // Eq 9
    double beta = (i & 1) ? (M_PI/6.0) : (-M_PI/6.0);
  
    double l = length (deltaPB);
    //Serial.print(i); Serial.print(" l = "); Serial.println(l);

    double L  =
      (l * l) - ((LEG_LENGTH * LEG_LENGTH) - 
                 (ARM_LENGTH * ARM_LENGTH));

    double M  = 2.0 * ARM_LENGTH * deltaPB(1);
    double N  = 2.0 * ARM_LENGTH *
      (deltaPB(0) * cos (beta) + deltaPB(2) * sin (beta));
    N = -fabs (N);
    double arg = L / sqrt ((M * M) + (N * N));
    double alphaX = asin (arg) - atan2 (N, M);
    if (isnan (alphaX)) {
      is_valid = false;
      //Serial.println(i);
      //Serial.println(" boom");
      break;
    }
    else {
      alpha[i] = R2D((i&1) ? alphaX : alphaX);
      //Serial.println(alpha[i]);
    }

#if 0
  if (!is_valid) {
  Serial.println("boom");

  //  for (int i = 0; i < servos.size (); i++)
  //    servos[i]->alpha = alpha_stage[i];
  }
#endif
  }
}


void 
setup() {
  Serial.begin(115200); // Any baud rate should work
  //Serial.println("Start stewart");
  
  myServo[0].attach( 11);
  myServo[1].attach( 10);
  myServo[2].attach( 9);
  myServo[3].attach( 6);
  myServo[4].attach( 5);
  myServo[5].attach( 3);
  pinMode(A0, INPUT);         // x
  pinMode(A1, INPUT);         // y
  //pinMode(A2, INPUT);       // z 
  pinMode(A3, INPUT);         // roll
  pinMode(A4, INPUT);         // pitch
  //pinMode(A5, INPUT);       // yaw
  pinMode(2, INPUT_PULLUP);   // r0n/stop
  pinMode(4, INPUT_PULLUP);   // position/rotation

  h0 = 0.0;					// Eq 10
  for (int i = 0; i < 6; i++) {
    incr[i] = ((double)random(-1000, 1000))/500.0;
    double xp = myPlatform.anchor[i].x;
    double yp = myPlatform.anchor[i].y;
    double xb = myBase.pos[i].x;
    double yb = myBase.pos[i].y;
    h0 += sqrt ( (LEG_LENGTH * LEG_LENGTH) +
		    (ARM_LENGTH * ARM_LENGTH) -
		  ( ((xp - xb) * (xp - xb)) +
		    ((yp - yb) * (yp - yb))));
  }
  h0 /= 6.0;
  update_alpha ();
  pincr = ((double)random(1000))/10000.0;
}

void 
loop() {
  static bool runState = true;
#if 0
  byte posrot = digitalRead(4);
  Serial.println(posrot);
  int movx = map(analogRead(A0), 0, 1023, -1000, 1000);
  int movy = map(analogRead(A1), 0, 1023, -1000, 1000);
  //int movz = map(analogRead(A2), 0, 1023, -1000, 1000);
  int movr = map(analogRead(A3), 0, 1023, -1000, 1000);
  int movp = map(analogRead(A4), 0, 1023, -1000, 1000);
  //int movy = map(analogRead(A5), 0, 1023, -1000, 1000);
#endif
  {
    static byte preveState = HIGH;
    byte btn = digitalRead(2);
    if (btn == LOW && preveState == HIGH) {
      runState = !runState;
    }
    preveState = btn;
  //Serial.println(btn);
  }

  for(int i=0; i< NUM_SERVOS; i++) {
    myServo[i].write( alpha[i]);
  }
  
  if (runState) {
    myPlatform.yaw += pincr;
    if (myPlatform.yaw > 10.0) {
      pincr = -pincr;
      myPlatform.yaw = 10.0;
    }
    else if (myPlatform.yaw < -10.0) {
      pincr = -pincr;
      myPlatform.yaw = -10.0;
   }
   update_alpha();
  }
 delay( 2);
}


