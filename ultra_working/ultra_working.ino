#include <ShiftRegister74HC595.h>
// create a global shift register object
// parameters: <number of shift registers> (data pin, clock pin, latch pin)
ShiftRegister74HC595<2> sr(14, 16, 15);
#define e_s1 2 //echo pin
#define t_s1 3 //Trigger pin

#define e_s2 4 //echo pin
#define t_s2 5 //Trigger pin

#define TOUPON 1
#define TODOWNOFF 2
#define NONE -1
int anim_state = NONE;
int last_anim_state = -1;
unsigned long last_millis = 0;
long dis_a = 0, dis_b = 0;
int flag1 = 0, flag2 = 0;
int person = 0;
int counter = 0;
const int sensor_sleep_mils = 2000;
unsigned long sensor_sleep_start = 0;
bool sensor_sleep = false;

void peopleCount();
void animate_state();
//**********************ultra_read****************************
void ultra_read(int pin_t, int pin_e, long &ultra_time) {
  long time;
  pinMode(pin_t, OUTPUT);
  pinMode(pin_e, INPUT);
  digitalWrite(pin_t, LOW);
  delayMicroseconds(2);
  digitalWrite(pin_t, HIGH);
  delayMicroseconds(10);
  time = pulseIn (pin_e, HIGH);
  ultra_time =  time / 29 / 2;
}


void setup() {
  Serial.begin(9600);// initialize serial communication at 9600 bits per second:
  Serial.println("BOO Staircase Started");
}

void loop() {

  peopleCount();
  animate_state();

}
void peopleCount() {

  ////// check for sensor sleep state and timeout condition
  if (sensor_sleep && millis() > sensor_sleep_start + sensor_sleep_mils )
  {
    return;   // exit the function early thus disabling sensor reading
  }
  else
  {
    sensor_sleep = false;  // sensor sleep is over so set the bool to false and let the function continue
  }

  //*************************
  ultra_read(t_s1, e_s1, dis_a); delay(30);
  ultra_read(t_s2, e_s2, dis_b); delay(30);
  //*************************

  Serial.print("da:"); Serial.println(dis_a);
  Serial.print("db:"); Serial.println(dis_b);

  if (dis_a < 90 && flag1 == 0) {
    flag1 = 1;
    if (flag2 == 0) {
      person = person + 1;
    }
  }

  if (dis_b < 90 && flag2 == 0) {
    flag2 = 1;
    if (flag1 == 0) {
      person = person - 1;
    }
  }

  if (dis_a > 90 && dis_b > 90 && flag1 == 1 && flag2 == 1) {
    flag1 = 0, flag2 = 0;
    //delay(2000);
    sensor_sleep = true;
    sensor_sleep_start = millis();
  }


  Serial.print("Have Person: ");
  Serial.print(person);
  Serial.print("  ");

  Serial.print("Light is ");
  if (person < 0) {
    person = 0;
  }
  if (person == 1 && dis_a < 90 && flag1 == 1 )
  {
    anim_state = TOUPON;
    // for (int i = 0; i < 16; i++) {
    //   sr.set(i, HIGH);
    //   delay(400);
    //   Serial.print("turn on lights ");
    //   Serial.println(i);
    }
  }
  if (person == 0 && dis_b < 90 && flag2 == 1)
  {
    anim_state = TODOWNOFF;
    // for (int i = 15; i >= 0; i--) {
    //   sr.set(i, LOW);
    //   delay(400);
    //   Serial.print("turn off lights ");
    //   Serial.println(i);
    }
  }
}

void animate_state() {

  if (anim_state == TOUPON) {
    //last_millis = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
      counter = 0;
      sr.set(counter, HIGH);
      Serial.println("starting toupon ");
      last_millis = millis();
    } else if ( millis() > last_millis + 800 ) {   // continue the animation if the time interval is up
      counter++;
      sr.set(counter, HIGH);
      Serial.print(" lights on ");
      Serial.println(counter);
      last_millis = millis();
    }
    if ( counter >= 15 ) anim_state = NONE;

  }




  if (anim_state == TODOWNOFF) {
    //last_millis = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
      counter = 15;
      sr.set(counter, LOW);
      Serial.println("starting todownoff ");
      last_millis = millis();
    } else if ( millis() > last_millis + 800 ) {   // continue the animation if the time interval is up
      counter--;
      sr.set(counter, LOW);
      Serial.print(" lights off ");
      Serial.println(counter);
      last_millis = millis();
    }
    if ( counter == 0 ) anim_state = NONE;

  }
  last_anim_state = anim_state;
}
