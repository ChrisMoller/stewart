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
#include <ezButton.h>

/***  Operational parameters ***/

#define MAX_POSITIONAL_JITTER  20    // millimetres
#define MAX_ROTATIONAL_JITTER 10   // degrees




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
double onset_time    = 0.2;
double relax_time    = 1.0;
double interval_time = 3.2;
 
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
      break;
    }
    else {
      alpha[i] = R2D((i&1) ? alphaX : alphaX);
    }
  }
}

#define RS_BUTTON   44
#define POS_BUTTON  45
#define TIME_BUTTON 46
#if 1
#define JS1_BUTTON  12 // upper, xyz
#define JS2_BUTTON  13 // lower, rpy
#endif

#define SERVO0_PIN 11
#define SERVO1_PIN 10
#define SERVO2_PIN 9
#define SERVO3_PIN 8
#define SERVO4_PIN 7
#define SERVO5_PIN 6
#define JITTER_LED_PIN   5
#define POSITION_LED_PIN 4
#define TIME_LED_PIN     3
#define LCD_RS_PIN 14
#define LCD_E_PIN  15
#define LCD_D4_PIN 16
#define LCD_D5_PIN 17
#define LCD_D6_PIN 18
#define LCD_D7_PIN 19

LiquidCrystal lcd(LCD_RS_PIN,
                  LCD_E_PIN,
                  LCD_D4_PIN,
                  LCD_D5_PIN,
                  LCD_D6_PIN,
                  LCD_D7_PIN);
ezButton rs_button(RS_BUTTON);
ezButton pos_button(POS_BUTTON);
ezButton time_button(TIME_BUTTON);

enum {
  SCALE_NONE,
  SCALE_CM_TO_MM,
  SCALE_TENTHS
};

static void
cvtFlt(int scale, char *buf, double val)
{
#define BUFLEN 64
  char lclbuf[BUFLEN];
  long int ivl;
  bool do_dec = false;
  switch(scale)
  {
  case SCALE_NONE:
    ivl = lrint(val);
    break;
  case SCALE_TENTHS:
    ivl = lrint(val);
    do_dec = true;
    break;
  case SCALE_CM_TO_MM:
    ivl = lrint(10.0 * val);
    break;
  }
  int src_len = sprintf(lclbuf, "%d", ivl);
  int src; 
  int dst = 0;
  memset(buf, ' ', 5);
  buf[5] = 0;
  for (src = src_len - 1, dst = 4; dst >= 0 && src >= 0; ) {
    buf[dst--] = lclbuf[src--];
    if (do_dec && dst  == 3) {
      buf[dst--] = '.';
      buf[dst] = '0';
    }
  }
}

enum {
  DISPLAY_NOTHING,
  DISPLAY_POSITION,
  DISPLAY_JITTER,
  DISPLAY_TIME
};
 
 bool jitter_dirty = true;
 bool position_dirty = true;
 bool time_dirty = true;

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
      cvtFlt (SCALE_NONE, buf,    myPlatform.dx);
      cvtFlt (SCALE_NONE, buf+5,  myPlatform.dy + h0);
      cvtFlt (SCALE_NONE, buf+10, myPlatform.dz);
      lcd.setCursor(0, 0);
      lcd.print(buf);

      cvtFlt (SCALE_NONE, buf,    myPlatform.roll);
      cvtFlt (SCALE_NONE, buf+5,  myPlatform.pitch);
      cvtFlt (SCALE_NONE, buf+10, myPlatform.yaw);
      lcd.setCursor(0, 1);
      lcd.print(buf);
      position_dirty = false;
    }
    break;

  case DISPLAY_JITTER:
    if (jitter_dirty) {
      cvtFlt (SCALE_NONE, buf,    10.0 * jitterDX);
      cvtFlt (SCALE_NONE, buf+5,  10.0 * jitterDY);
      cvtFlt (SCALE_NONE, buf+10, 10.0 * jitterDZ);
      lcd.setCursor(0, 0);
      lcd.print(buf);

      cvtFlt (SCALE_NONE, buf,    jitterAR);
      cvtFlt (SCALE_NONE, buf+5,  jitterAP);
      cvtFlt (SCALE_NONE, buf+10, jitterAY);
      lcd.setCursor(0, 1);
      lcd.print(buf);
      jitter_dirty = false;
    }
    break;

  case DISPLAY_TIME:
    if (time_dirty) {
      cvtFlt (SCALE_TENTHS, buf,    10.0 * onset_time);
      cvtFlt (SCALE_TENTHS, buf+5,  10.0 * relax_time);
      cvtFlt (SCALE_TENTHS, buf+10, 10.0 * interval_time);
      lcd.setCursor(0, 0);
      lcd.print(buf);
      lcd.setCursor(0, 1);
      lcd.print("                   ");
      time_dirty = false;
    }
    break;
  }
}


void setup() {
  Serial.begin(115200); // Any baud rate should work
  //Serial.println("Start stewart");

  lcd.begin(16,2);
  rs_button.setDebounceTime(50);
  pos_button.setDebounceTime(50);
  time_button.setDebounceTime(50);
  
  myServo[0].attach( SERVO0_PIN);
  myServo[1].attach( SERVO1_PIN);
  myServo[2].attach( SERVO2_PIN);
  myServo[3].attach( SERVO3_PIN);
  myServo[4].attach( SERVO4_PIN);
  myServo[5].attach( SERVO5_PIN);
  pinMode(A0, INPUT);         // x
  pinMode(A1, INPUT);         // y
  pinMode(A2, INPUT);         // z 
  pinMode(A3, INPUT);         // roll
  pinMode(A4, INPUT);         // pitch
  pinMode(A5, INPUT);         // yaw
  pinMode(JITTER_LED_PIN,    OUTPUT);  
  pinMode(POSITION_LED_PIN,  OUTPUT);  
  pinMode(TIME_LED_PIN,      OUTPUT);  
  pinMode(LCD_E_PIN,         OUTPUT);      
  pinMode(LCD_D4_PIN,        OUTPUT);      
  pinMode(LCD_D5_PIN,        OUTPUT);      
  pinMode(LCD_D6_PIN,        OUTPUT);      
  pinMode(LCD_D7_PIN,        OUTPUT);   
  #if 1
  pinMode(JS1_BUTTON, INPUT_PULLUP); // js1 sel
  pinMode(JS2_BUTTON, INPUT_PULLUP); // js2 sel
  #endif
  
  digitalWrite(JITTER_LED_PIN,   HIGH);
  digitalWrite(POSITION_LED_PIN, LOW);
  digitalWrite(TIME_LED_PIN,     LOW);

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
enum {
  JITTER_QUIET,
  JITTER_ONSET,
  JITTER_ONSET_CONTINUE,
  JITTER_RELAX
};

#define TO_US(s) ((unsigned int)((s) * 1000000))
int jitter_mode = JITTER_ONSET;

static void
do_jitter()
{
  unsigned int sleep_time = TO_US(interval_time);
  static double target_dx;
  static double target_dy;
  static double target_dz;
  static double target_ap;
  static double target_ay;
  static double target_ar;
  static double attack_stage;
  static double base_dx;
  static double base_dy;
  static double base_dz;
  static double base_ap;
  static double base_ay;
  static double base_ar;
  
  switch (jitter_mode) {
  case JITTER_QUIET:		// do nothing
    break;
  case JITTER_ONSET:
    base_dx = myPlatform.dx;
    base_dy = myPlatform.dy;
    base_dz = myPlatform.dz;
    base_ap = myPlatform.pitch;
    base_ay = myPlatform.yaw;
    base_ar = myPlatform.roll;
	
    target_dx = random ((double)(-2000, 2000))/ 1000.0;
    target_dy = random ((double)(-2000, 2000))/ 1000.0;
    target_dz = random ((double)(-2000, 2000))/ 1000.0;
    target_ap = random ((double)(-2000, 2000))/ 10000.0;
    target_ay = random ((double)(-2000, 2000))/ 10000.0;
    target_ar = random ((double)(-2000, 2000))/ 10000.0;
    attack_stage = 0.0;
    jitter_mode = JITTER_ONSET_CONTINUE;
    //break;					no break
  case JITTER_ONSET_CONTINUE:
    if (attack_stage < 10.0) {
      myPlatform.dx    = base_dx + target_dx * attack_stage / 10.0;
      myPlatform.dy    = base_dy + target_dy * attack_stage / 10.0; 
      myPlatform.dz    = base_dz + target_dz * attack_stage / 10.0; 
      myPlatform.pitch = base_ap + target_ap * attack_stage / 10.0;
      myPlatform.yaw   = base_ay + target_ay * attack_stage / 10.0; 
      myPlatform.roll  = base_ar + target_ar * attack_stage / 10.0; 
      attack_stage += 0.2;
    }
    else jitter_mode = JITTER_RELAX;
    break;
  case JITTER_RELAX:
    if (attack_stage > 0.0) {
      myPlatform.dx    = target_dx * attack_stage / 10.0;
      myPlatform.dy    = target_dy * attack_stage / 10.0; 
      myPlatform.dz    = target_dz * attack_stage / 10.0; 
      myPlatform.pitch = target_ap * attack_stage / 10.0;
      myPlatform.yaw   = target_ay * attack_stage / 10.0; 
      myPlatform.roll  = target_ar * attack_stage / 10.0; 
      attack_stage -= 0.1;
      sleep_time = TO_US(relax_time/10.0);
    }
    else {
      jitter_mode = JITTER_ONSET;
      sleep_time = TO_US(interval_time);
    }
    break;
  }
  update_alpha();
  updateDisplay(DISPLAY_POSITION);  // fixme
  delayMicroseconds(sleep_time);
}

void loop() {
  rs_button.loop();
  pos_button.loop();
  time_button.loop();

  static bool runState = true;

  {
    int which_display = DISPLAY_NOTHING;
    int posBtnState = pos_button.getState();
    int timeBtnState = time_button.getState();
    byte xyzsel  = digitalRead(JS1_BUTTON);
    byte rpysel  = digitalRead(JS2_BUTTON);
    static bool non_jitter = false;
    if (posBtnState == HIGH) { // jitter or time
      if (timeBtnState == HIGH) { // jitter
        digitalWrite(JITTER_LED_PIN,   HIGH);
        digitalWrite(POSITION_LED_PIN, LOW);
        digitalWrite(TIME_LED_PIN,     LOW);
        if (non_jitter) {
         non_jitter = false;
         jitter_dirty = true;
        }
        if (xyzsel == HIGH) {
#if 1
          int movdx = map(analogRead(A0), 0, 1023, 
             -100, 100);
          int movdy = map(analogRead(A1), 0, 1023, 
             -100, 100);
          int movdz = map(analogRead(A2), 0, 1023, 
             -100, 100);
          int movar = map(analogRead(A3), 0, 1023, 
             -100, 100);
          int movap = map(analogRead(A4), 0, 1023, 
             -100, 100);
          int movay = map(analogRead(A5), 0, 1023, 
             -100, 100);
          double jdx = (((double)movdx) / 5000.0);
          double jdy = (((double)movdy) / 5000.0);
          double jdz = (((double)movdz) / 5000.0);
          double jar = (((double)movar) / 5000.0);
          double jap = (((double)movap) / 5000.0);
          double jay = (((double)movay) / 5000.0);
          static double odx = 0.0/0.0;
          static double ody = 0.0/0.0;
          static double odz = 0.0/0.0;
          static double oar = 0.0/0.0;
          static double oap = 0.0/0.0;
          static double oay = 0.0/0.0;
          if (jdx != odx && fabs(jdx) > 0.006) 
               {odx = (jitterDX += jdx); jitter_dirty = true;}
          if (jdy != ody && fabs(jdy) > 0.006)
               {ody = (jitterDY += jdy); jitter_dirty = true;}
          if (jdz != odz && fabs(jdz) > 0.006) 
               {odz = (jitterDZ += jdz); jitter_dirty = true;}
          if (jar != oar && fabs(jar) > 0.006) 
               {oar = (jitterAR += jar); jitter_dirty = true;}
          if (jap != oap && fabs(jap) > 0.006) 
               {oap = (jitterAP += jap); jitter_dirty = true;}
          if (jay != oay && fabs(jay) > 0.006) 
               {oay = (jitterAY += jay); jitter_dirty = true;}
#else
          int movdx = map(analogRead(A0), 0, 1023, 
             0, MAX_POSITIONAL_JITTER);
          int movdy = map(analogRead(A1), 0, 1023, 
             0, MAX_POSITIONAL_JITTER);
          int movdz = map(analogRead(A2), 0, 1023, 
             0, MAX_POSITIONAL_JITTER);
          int movar = map(analogRead(A3), 0, 1023, 
             0, MAX_ROTATIONAL_JITTER);
          int movap = map(analogRead(A4), 0, 1023, 
             0, MAX_ROTATIONAL_JITTER);
          int movay = map(analogRead(A5), 0, 1023, 
             0, MAX_ROTATIONAL_JITTER);

          double jdx = (((double)movdx) / 10.0);
          double jdy = (((double)movdy) / 10.0);
          double jdz = (((double)movdz) / 10.0);
          double jar = (double)movar;
          double jap = (double)movap;
          double jay = (double)movay;
          static double odx = 0.0/0.0;
          static double ody = 0.0/0.0;
          static double odz = 0.0/0.0;
          static double oar = 0.0/0.0;
          static double oap = 0.0/0.0;
          static double oay = 0.0/0.0;
          if (jdx != odx) 
               {jitterDX = odx = jdx; jitter_dirty = true;}
          if (jdy != ody)
               {jitterDY = ody = jdy; jitter_dirty = true;}
          if (jdz != odz) 
               {jitterDZ = odz = jdz; jitter_dirty = true;}
          if (jar != oar) 
               {jitterAR = oar = jar; jitter_dirty = true;}
          if (jap != oap) 
               {jitterAP = oap = jap; jitter_dirty = true;}
          if (jay != oay) 
               {jitterAY = oay = jay; jitter_dirty = true;}
#endif
        }
        else {
          jitterDX = 0.0;
          jitterDY = 0.0;
          jitterDZ = 0.0;
          jitterAR = 0.0;
          jitterAP = 0.0;
          jitterAY = 0.0;
          jitter_dirty = true;
        }
        which_display = DISPLAY_JITTER;
      }
      else {                 // time
        if (xyzsel == HIGH) {
          digitalWrite(JITTER_LED_PIN,   LOW);
          digitalWrite(POSITION_LED_PIN, LOW);
          digitalWrite(TIME_LED_PIN,     HIGH);
#if 1
          int movdx = map(analogRead(A0), 0, 1023, 
             -100, 100);
          int movdy = map(analogRead(A1), 0, 1023, 
             -100, 100);
          int movdz = map(analogRead(A2), 0, 1023, 
             -100, 100);
          double jdx = (((double)movdx) / 5000.0);
          double jdy = (((double)movdy) / 5000.0);
          double jdz = (((double)movdz) / 5000.0);
          static double odx = 0.0/0.0;
          static double ody = 0.0/0.0;
          static double odz = 0.0/0.0;
          if (jdx != odx && fabs(jdx) > 0.006) 
               {odx = (onset_time += jdx); time_dirty = true;}
          if (jdy != ody && fabs(jdy) > 0.006)
               {ody = (relax_time += jdy); time_dirty = true;}
          if (jdz != odz && fabs(jdz) > 0.006) 
               {odz = (interval_time += jdz); time_dirty = true;}
#else
          int movdx = map(analogRead(A0), 0, 1023,  0, 20);
          int movdy = map(analogRead(A1), 0, 1023,  0, 20);
          int movdz = map(analogRead(A2), 0, 1023,  0, 70);
          static double odx = 0.0/0.0;
          static double ody = 0.0/0.0;
          static double odz = 0.0/0.0;
          double jdx = ((double)movdx) / 10.0;
          double jdy = ((double)movdy) / 10.0;
          double jdz = ((double)movdz) / 10.0;
          if (jdx != odx) 
               {onset_time    = odx = jdx; time_dirty = true;}
          if (jdy != ody) 
               {relax_time    = ody = jdy; time_dirty = true;}
          if (jdz != odz) 
               {interval_time = odz = jdz; time_dirty = true;}
#endif
          time_dirty = true;
          non_jitter = true;
        }
        which_display = DISPLAY_TIME;
      }
    }
    else {              // position
      if (xyzsel == HIGH) {
        digitalWrite(JITTER_LED_PIN,   LOW);
        digitalWrite(POSITION_LED_PIN, HIGH);
        digitalWrite(TIME_LED_PIN,     LOW);
#if 1
          int movdx = map(analogRead(A0), 0, 1023, 
             -100, 100);
          int movdy = map(analogRead(A1), 0, 1023, 
             -100, 100);
          int movdz = map(analogRead(A2), 0, 1023, 
             -100, 100);
          int movar = map(analogRead(A3), 0, 1023, 
             -100, 100);
          int movap = map(analogRead(A4), 0, 1023, 
             -100, 100);
          int movay = map(analogRead(A5), 0, 1023, 
             -100, 100);
          double jdx = (((double)movdx) / 500.0);
          double jdy = (((double)movdy) / 500.0);
          double jdz = (((double)movdz) / 500.0);
          double jar = (((double)movar) / 5.0);
          double jap = (((double)movap) / 5.0);
          double jay = (((double)movay) / 5.0);
          static double odx = 0.0/0.0;
          static double ody = 0.0/0.0;
          static double odz = 0.0/0.0;
          static double oar = 0.0/0.0;
          static double oap = 0.0/0.0;
          static double oay = 0.0/0.0;
          if (jdx != odx && fabs(jdx) > 0.006) 
               {odx = (myPlatform.dx += jdx);  position_dirty = true;}
          if (jdy != ody && fabs(jdy) > 0.006)
               {ody = (myPlatform.dy += jdy); position_dirty = true;}
          if (jdz != odz && fabs(jdz) > 0.006) 
               {odz = (myPlatform.dz += jdz); position_dirty = true;}
          if (jar != oar && fabs(jar) > 0.006) 
               {oar = (myPlatform.roll += jar); position_dirty = true;}
          if (jap != oap && fabs(jap) > 0.006) 
               {oap = (myPlatform.pitch += jap); position_dirty = true;}
          if (jay != oay && fabs(jay) > 0.006) 
               {oay = (myPlatform.yaw += jay); position_dirty = true;}
#else
        int movdx = map(analogRead(A0), 0, 1023,  40, 70);
        int movdy = map(analogRead(A1), 0, 1023,  40, 70);
        int movdz = map(analogRead(A2), 0, 1023,  40, 70);
        int movar = map(analogRead(A3), 0, 1023, -30, 30);
        int movap = map(analogRead(A4), 0, 1023, -30, 30);
        int movay = map(analogRead(A5), 0, 1023, -30, 30);
        static double odx = 0.0/0.0;
        static double ody = 0.0/0.0;
        static double odz = 0.0/0.0;
        static double oar = 0.0/0.0;
        static double oap = 0.0/0.0;
        static double oay = 0.0/0.0;

        double jdx = (double)movdx;
        double jdy = (double)movdy;
        double jdz = (double)movdz;
        double jar = (double)movar;
        double jap = (double)movap;
        double jay = (double)movay;
        if (fabs(jdx) > 0.01) 
               {myPlatform.dx = odx = jdx; position_dirty = true;}
        if (fabs(jdy) > 0.01)
               {myPlatform.dy = ody = jdy; position_dirty = true;}
        if (fabs(jdz) > 0.01) 
               {myPlatform.dz = odz = jdz; position_dirty = true;}
        if (fabs(jar) > 1.0) 
              {myPlatform.roll =  odx = jar; position_dirty = true;}
        if (fabs(jap) > 1.0) 
              {myPlatform.pitch = oap = jap; position_dirty = true;}
        if (fabs(jay) > 1.0) 
              {myPlatform.yaw =   oay = jay; position_dirty = true;}
#endif
      }
      else {
        myPlatform.dx = 0.0;
        myPlatform.dy = 0.0;
        myPlatform.dz = 0.0;
        myPlatform.roll = 0.0;
        myPlatform.pitch = 0.0;
        myPlatform.yaw = 0.0;
        position_dirty = true;
      }
      which_display = DISPLAY_POSITION;
      position_dirty = true;
      non_jitter = true;
    }
    if (jitter_dirty |
        position_dirty |
        time_dirty) {
        update_alpha();
          updateDisplay(which_display);
        }
  }

  if (rs_button.isPressed())
    runState = !runState;

  for(int i=0; i< NUM_SERVOS; i++) {
    myServo[i].write( alpha[i]);
  }
    
  if (runState) {
#if 0
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
   delay( 2);
#else
    do_jitter();
#endif
  }
}
