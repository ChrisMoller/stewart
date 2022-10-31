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
#include <LiquidCrystal.h>

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

double jitterDX = 0.002;  // cm = .2mm
double jitterDY = 0.002;
double jitterDZ = 0.002;
double jitterAR = 5.0;    // degrees
double jitterAP = 5.0;
double jitterAY = 5.0;
 
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

#define RS_BUTTON 44
#define POS_BUTTON 45
#define TIME_BUTTON 46
#define JS1_BUTTON 12 // upper, xyz
#define JS2_BUTTON 13 // lower, rpy

#define SERVO0_PIN 11
#define SERVO1_PIN 10
#define SERVO2_PIN 9
#define SERVO3_PIN 8
#define SERVO4_PIN 7
#define SERVO5_PIN 6

//                rs   e  d4  d5  d6  d7
LiquidCrystal lcd(14, 15, 16, 17, 18, 19);


static void
cvtFlt(bool do_scale, char *buf, double val)
{
#define BUFLEN 64
  char lclbuf[BUFLEN];
  long int ivl = lrint((do_scale ? 1000.0 : 1.0) * val);
  int src_len = sprintf(lclbuf, "%d", ivl);
  int src; 
  int dst = 0;
  if (src_len < 5) 
    for (; dst < 5-src_len; dst++) buf[dst] = ' ';
  for (src = 0; dst < BUFLEN && lclbuf[src]; ) {
    buf[dst++] = lclbuf[src++];
    if (src == src_len - 1) 
      buf[dst++] = '.';
  }
  buf[dst] = 0;
}

enum {
  DISPLAY_NOTHING,
  DISPLAY_POSITION,
  DISPLAY_JITTER,
  DISPLAY_TIME
};
 
 bool jitter_dirty = true;
 bool position_dirty = true;

static void updateDisplay(int which)
{
#define LBUFLEN 64
  char buf[LBUFLEN];
  memset(buf, 0, LBUFLEN);

  switch(which) {
  case DISPLAY_NOTHING:
    lcd.clear();
    break;
  case DISPLAY_POSITION:
    if (position_dirty) {
      cvtFlt (true, buf,    myPlatform.dx);
      cvtFlt (true, buf+5,  myPlatform.dy + h0);
      cvtFlt (true, buf+10, myPlatform.dz);
      lcd.setCursor(0, 0);
      lcd.print(buf);

      cvtFlt (true, buf,    myPlatform.roll);
      cvtFlt (true, buf+5,  myPlatform.pitch);
      cvtFlt (true, buf+10, myPlatform.yaw);
      lcd.setCursor(0, 1);
      lcd.print(buf);
      position_dirty = false;
    }
    break;

  case DISPLAY_JITTER:
    if (jitter_dirty) {
      cvtFlt (true, buf,    10.0 * jitterDX);
      cvtFlt (true, buf+5,  10.0 * jitterDY);
      cvtFlt (true, buf+10, 10.0 * jitterDZ);
      lcd.setCursor(0, 0);
      lcd.print(buf);

      cvtFlt (true, buf,    jitterAR);
      cvtFlt (true, buf+5,  jitterAP);
      cvtFlt (true, buf+10, jitterAY);
      lcd.setCursor(0, 1);
      lcd.print(buf);
      jitter_dirty = false;
    }
    break;

  case DISPLAY_TIME:
    lcd.setCursor(0, 0);
    lcd.print("not yet implemented");
    lcd.setCursor(0, 1);
    lcd.print("                   ");
    break;
  }
}


void setup() {
  Serial.begin(115200); // Any baud rate should work
  //Serial.println("Start stewart");

  lcd.begin(16,2);
  //lcd.setRowOffsets(0, 64, 16, 68);
  
  myServo[0].attach( SERVO0_PIN);
  myServo[1].attach( SERVO1_PIN);
  myServo[2].attach( SERVO2_PIN);
  myServo[3].attach( SERVO3_PIN);
  myServo[4].attach( SERVO4_PIN);
  myServo[5].attach( SERVO5_PIN);
  pinMode(A0, INPUT);         // x
  pinMode(A1, INPUT);         // y
  //pinMode(A2, INPUT);       // z 
  pinMode(A3, INPUT);         // roll
  pinMode(A4, INPUT);         // pitch
  //pinMode(A5, INPUT);       // yaw
  pinMode(RS_BUTTON, INPUT_PULLUP);  // run/stop
  pinMode(POS_BUTTON, INPUT_PULLUP); // position/rotation
  pinMode(TIME_BUTTON, INPUT_PULLUP); // time
  pinMode(JS1_BUTTON, INPUT_PULLUP); // js1 sel
  pinMode(JS2_BUTTON, INPUT_PULLUP); // js2 sel

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
  updateDisplay(DISPLAY_JITTER);
  pincr = ((double)random(1000))/10000.0;
}


void loop() {
  static bool runState = true;
  // positive down, negative up
  // js 1, upper, xyz
  // js 2, lower, rpy
  int movdx = map(analogRead(A0), 0, 1023, -1000, 1000);
  int movdy = map(analogRead(A1), 0, 1023, -1000, 1000);
  //int movdz = map(analogRead(A2), 0, 1023, -1000, 1000);
  int movar = map(analogRead(A3), 0, 1023, -1000, 1000);
  int movap = map(analogRead(A4), 0, 1023, -1000, 1000);
  //int movay = map(analogRead(A5), 0, 1023, -1000, 1000);

  int which_display = DISPLAY_NOTHING;
  byte posjit  = digitalRead(POS_BUTTON);
  byte settime = digitalRead(TIME_BUTTON);
  byte xyzsel  = digitalRead(JS1_BUTTON);
  byte rpysel  = digitalRead(JS2_BUTTON);
  static bool non_jitter = false;
  if (posjit == HIGH) { // jitter or time
    if (settime == HIGH) { // jitter
       if (non_jitter) {
        non_jitter = false;
        jitter_dirty = true;
       }
      if (xyzsel == HIGH) {
        if (movdx > 0)      {jitterDX -= 0.1; jitter_dirty = true;}
        else if (movdx < 0) {jitterDX += 0.1; jitter_dirty = true;}
        if (movdy > 0)      {jitterDY -= 0.1; jitter_dirty = true;}
        else if (movdy < 0) {jitterDY += 0.1; jitter_dirty = true;}
        //if (movdz > 0)      {jitterDZ -= 0.1; jitter_dirty = true;}
        //else if (movdz < 0) {jitterDZ += 0.1; jitter_dirty = true;}
      }
      else {
        jitterDX = 0.0;
        jitterDY = 0.0;
        jitterDZ = 0.0;
        jitter_dirty = true;
      }
      which_display = DISPLAY_JITTER;
    }
    else {                 // time
      updateDisplay(DISPLAY_TIME);
      non_jitter = true;
    }
  }
  else {              // position
    which_display = DISPLAY_POSITION;
    position_dirty = true;
    non_jitter = true;
  }
  if (jitter_dirty |
      position_dirty) updateDisplay(which_display);


  {     // debounce
#define BOUNCE_COUNT 10
    static byte preveState = HIGH;
    static int count = 0;
    static byte oldBtn = HIGH;
    byte btn = digitalRead(RS_BUTTON);
    if (btn == oldBtn) count++;
    else oldBtn = btn;
    if (count >= BOUNCE_COUNT) {
      count = 0;
      if (btn == LOW && preveState == HIGH) {
        runState = !runState;
      }
      preveState = btn;
    }
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
