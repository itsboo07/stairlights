#include <ShiftRegister74HC595.h>
#include "HCSR04_Custom.h"

//#define D_THRESHOLD 100     // the minimum maximum distance to indicate a trigger
#define MEASURE_COUNT 3;   // the number of measurement below the threshold in a row required to indicate a person

#define BOOPINS 1           // change 0 for TAVIS to change hardware pins
#if BOOPINS == 1
  #define e_s1 2 //echo pin
  #define t_s1 3 //Trigger pin
  #define e_s2 4 //echo pin
#define t_s2 5 //Trigger pin
#else
  #define e_s1 A1 //echo pin
  #define t_s1 A0 //Trigger pin
  #define e_s2 A3 //echo pin
#define t_s2 A2 //Trigger pin
#endif

UltraSonicDistanceSensor sensor1(t_s1, e_s1);  //
UltraSonicDistanceSensor sensor2(t_s2, e_s2);  //
int person_count = 0;
const int m_count = MEASURE_COUNT ;

// create a global shift register object
// parameters: <number of shift registers> (data pin, clock pin, latch pin)
ShiftRegister74HC595<2> sr(14, 16, 15);

#define ANIM_DELAY 400  // 100 ms between each light turnon
#define DISTANCE_THRESHOLD 80 // the distance to trigger the sensor on

#define TOUPON 1
#define TOUPOFF 2
#define TODOWNON 3
#define TODOWNOFF 4
#define NONE -1
int anim_state = NONE;
int last_anim_state = -1;
unsigned long last_millis = 0;
long dis_a = 1000, dis_b = 1000;
int flag1 = 0, flag2 = 0;
int person = 0, last_person = 0;
int counter = 0;
const int sensor_sleep_mils = 1000;
unsigned long sensor_sleep_start = 0;
bool sensor_sleep = false;
bool last_sleep = false;
unsigned long last_flag_change;
unsigned long last_active = 0;

int read_sensor(UltraSonicDistanceSensor &sensor) {    /// take multiple reading to eliminate spurious triggers
  long d = 0;
  long last_d = sensor.measureDistanceCm();
  delay(10);
  for (int i = 0 ; i < m_count ; i++ ) 
  {
    d = sensor.measureDistanceCm();   // <---- actually the second measurement
    ////////// check for mis-matched measurements 
    if ((d > DISTANCE_THRESHOLD && last_d < DISTANCE_THRESHOLD) || (d < DISTANCE_THRESHOLD && last_d > DISTANCE_THRESHOLD) ) {return 1000;}
    delay(10);
    last_d = d;
  }
  if (last_d < DISTANCE_THRESHOLD && last_d != -1 ) 
  { 
    return last_d;   // if we get here then got consecutive readings below threshold
  }
  else {
    return 1000;
  }
}


void peopleCount();
void animate_state();

void setup() {
  Serial.begin(9600);// initialize serial communication at 9600 bits per second:
  pinMode(t_s1, OUTPUT);
  pinMode(e_s1, INPUT);
  pinMode(t_s2, OUTPUT);
  pinMode(e_s2, INPUT);


  Serial.println("BOO Staircase Started");
}

void loop() {

  peopleCount();
  animate_state();

}
void peopleCount() {

  ////// check for sensor sleep state and timeout condition
  if (sensor_sleep && millis() < sensor_sleep_start + sensor_sleep_mils )
  {
    if (sensor_sleep != last_sleep) {
      Serial.println("sleep start");
      last_sleep = sensor_sleep;
    }
    return;   // exit the function early thus disabling sensor reading
  }
  else
  {
    if (sensor_sleep != last_sleep) {
      Serial.println("sleep end");
      last_sleep = sensor_sleep;
    }
    sensor_sleep = false;  // sensor sleep is over so set the bool to false and let the function continue
  }

  //*************************
  // ultra_read(t_s1, e_s1, dis_a); delay(30);
  // ultra_read(t_s2, e_s2, dis_b); delay(30);
  // //*************************
  dis_a = read_sensor(sensor1);
  if (dis_a < DISTANCE_THRESHOLD ) {
    //Serial.print("s1 triggered: ");Serial.println(dis_a);
    //Serial.print("flag1 : "); Serial.println(flag1);
    //Serial.print("flag2 : "); Serial.println(flag2);
  }

  if (person > 0 || dis_b < DISTANCE_THRESHOLD) {   // if we have people to exit, or last reading was a trigger
    dis_b = read_sensor(sensor2);
    //if (dis_b < DISTANCE_THRESHOLD ) {Serial.print("s2 triggered: ");Serial.println(dis_b);}
  }
  //Serial.print("da:"); Serial.println(dis_a);
  //Serial.print("db:"); Serial.println(dis_b);
  
  if (dis_a < DISTANCE_THRESHOLD && flag1 == 0) {
    flag1 = 1;
    last_flag_change = millis();
    if (flag2 == 0) {
      person = person + 1;
      last_active = millis();
      //sensor_sleep = true;
      //sensor_sleep_start = millis();
    }
  }

  if (dis_b < DISTANCE_THRESHOLD && flag2 == 0) {
    flag2 = 1;
    last_flag_change = millis();
    if (flag1 == 0) {
      if (person > 0) person = person - 1;
      //sensor_sleep = true;
      //sensor_sleep_start = millis();
    }
  }
 // <------reset the flags after both have been triggered and also use a timeout in case the second sensor missed
  if (dis_a > DISTANCE_THRESHOLD && dis_b > DISTANCE_THRESHOLD && ((flag1 == 1 || flag2 == 1))) 
  {
      if ((flag1 == 1 && flag2 == 1) || millis() > last_flag_change+2000 ) 
      {
      flag1 = 0, flag2 = 0;
      //delay(2000);
      sensor_sleep = true;
      sensor_sleep_start = millis();
      Serial.println("flag reset");
    }
  }

  if (person != last_person) {
    //sensor_sleep = true;
    //sensor_sleep_start = millis();
    Serial.print("Person count: ");
    Serial.println(person);
    
    if (person == 1 && last_person == 0 )
    {
    Serial.println("TODOWNON");
    anim_state = TODOWNON;
    }
    if (person == 0 && last_person == 1)
    {
      Serial.println("TOUPOFF");
      anim_state = TOUPOFF;
    }
    last_person = person;
  }
  if (person>0 && (millis()> 15000 + last_active)){
    person=0;
    sr.setAllLow();
    
    
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
    } else if ( millis() > last_millis + ANIM_DELAY ) {   // continue the animation if the time interval is up
      counter++;
      sr.set(counter, HIGH);
      //Serial.print(" lights on ");
      //Serial.println(counter);
      last_millis = millis();
    }
    if ( counter >= 15 ) 
    {
      anim_state = NONE;
      Serial.println("stopped toupon");
    }

  }

  if (anim_state == TOUPOFF) {
    //last_millis = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
      counter = 0;
      sr.set(counter, LOW);
      Serial.println("starting toupOff ");
      last_millis = millis();
    } else if ( millis() > last_millis + ANIM_DELAY ) {   // continue the animation if the time interval is up
      counter++;
      sr.set(counter, LOW);
      //Serial.print(" lights off ");
      //Serial.println(counter);
      last_millis = millis();
    }
    if ( counter >= 15 ) 
    {
      anim_state = NONE;
      Serial.println("stopped toupoff");
    }

  }
    if (anim_state == TODOWNON) {
    //last_millis = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
      counter = 15;
      sr.set(counter, HIGH);
      Serial.println("starting todownon ");
      last_millis = millis();
    } else if ( millis() > last_millis + ANIM_DELAY ) {   // continue the animation if the time interval is up
      counter--;
      sr.set(counter, HIGH);
      //Serial.print(" lights on ");
      //Serial.println(counter);
      last_millis = millis();
    }
    if ( counter == 0 ) 
    {
      anim_state = NONE;
      Serial.println("stopped todownon");
    }

  }


  if (anim_state == TODOWNOFF) {
    //last_millis = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
      counter = 15;
      sr.set(counter, LOW);
      Serial.println("starting todownoff");
      last_millis = millis();
    } else if ( millis() > last_millis + ANIM_DELAY ) {   // continue the animation if the time interval is up
      counter--;
      sr.set(counter, LOW);
      //Serial.print(" lights off ");
      //Serial.println(counter);
      last_millis = millis();
    }
    if ( counter == 0 )
    {
      anim_state = NONE;
      Serial.println("stopped todownoff");
    }
  }
  last_anim_state = anim_state;
}
