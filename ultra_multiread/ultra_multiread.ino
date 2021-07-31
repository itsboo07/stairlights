#include <ShiftRegister74HC595.h>
#include "HCSR04_Custom.h"

//#define D_THRESHOLD 100     // the minimum maximum distance to indicate a trigger
#define MEASURE_COUNT 3;   // the number of measurement below the threshold in a row required to indicate a person

#define BOOPINS 1           // change 0 for TAVIS to change hardware pins
#if BOOPINS == 1
  // lower group
  #define e_sA 2 //echo pin
  #define t_sA 3 //Trigger pin
  #define e_sB 4 //echo pin
  #define t_sB 5 //Trigger pin
  // upper group 
  #define e_sC 6 //echo pin
  #define t_sC 7 //Trigger pin
  #define e_sD 8 //echo pin
  #define t_sD 9 //Trigger pin
#else
  #define e_sA A1 //echo pin
  #define t_sA A0 //Trigger pin
  #define e_sB A3 //echo pin
  #define t_sB A2 //Trigger pin
  #define e_sC 6 //echo pin
  #define t_sC 7 //Trigger pin
  #define e_sD 8 //echo pin
  #define t_sD 9 //Trigger pin
#endif

UltraSonicDistanceSensor sensor_a(t_sA, e_sA);  //
UltraSonicDistanceSensor sensor_b(t_sB, e_sB);  //
UltraSonicDistanceSensor sensor_c(t_sC, e_sC);  //
UltraSonicDistanceSensor sensor_d(t_sD, e_sD);  //
int person_count = 0;
const int m_count = MEASURE_COUNT ;

// create a global shift register object
// parameters: <number of shift registers> (data pin, clock pin, latch pin)
ShiftRegister74HC595<2> sr(14, 16, 15);

#define ANIM_DELAY 400  // 100 ms between each light turnon
#define DISTANCE_THRESHOLD 80 // the distance to trigger the sensor on


/// Animation states
#define TOUPON 1
#define TOUPOFF 2
#define TODOWNON 3
#define TODOWNOFF 4
#define NONE -1

int anim_state = NONE;                // state variable for current animation state
int last_anim_state = -1;             // last state of above
unsigned long anim_start = 0;         // millis for when animation has started

int person = 0, last_person = 0;      // person counter and last person -- used to detect when person count has changed
int anim_counter = 0;                 // animation counter -- used to keep track of animation progress
const int sensor_sleep_mils = 2000;   // amount of time to sleep a sensor pair after both get triggered

////////////////////////////////////////// Lower floor sensor group
long dis_a = 1000, dis_b = 1000;        // distance return in cm for each sensor of lower group
int flag_a = 0, flag_b = 0;             // flags for lower sensors
unsigned long l_sensor_sleep_start = 0; // the time the sensor_pair started sleeping 
bool l_sensor_sleep = false;            // boolean to indicated that a sensor pair is sleeping
bool l_last_sleep = false;              // boolean to detect a change in sleep status
unsigned long l_last_flag_change;       // the time when a flag was last changed in lower group

////////////////////////////////////////// Upper floor sensor group
long dis_c = 1000, dis_d = 1000;        // distance return in cm for each sensor of upper group
int flag_c = 0, flag_d = 0;             // flags for upper sensors
unsigned long u_sensor_sleep_start = 0; // the time the sensor_pair started sleeping 
bool u_sensor_sleep = false;            // boolean to indicated that a sensor pair is sleeping
bool u_last_sleep = false;              // boolean to detect a change in sleep status
unsigned long u_last_flag_change;       // the time when a flag was last changed in upper group


unsigned long last_active = 0;        // the time when we last had activity -- used to timeout a lights on condition if the senosr missed the exit

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
  
  return last_d;
}

// forward declaration.
void people_count_lower();
void people_count_upper();

void animate_state();

void setup() {
  Serial.begin(9600);// initialize serial communication at 9600 bits per second:
  pinMode(t_sA, OUTPUT);
  pinMode(e_sA, INPUT);
  pinMode(t_sB, OUTPUT);
  pinMode(e_sB, INPUT);
  pinMode(t_sC, OUTPUT);
  pinMode(e_sC, INPUT);
  pinMode(t_sD, OUTPUT);
  pinMode(e_sD, INPUT);


  Serial.println("BOO Staircase Started");
}

void loop() {

  people_count_lower();
  people_count_upper();
  animate_state();

}

void people_count_lower() {       // function to detect people count at lower sensor group

  ////// check for sensor sleep state and timeout condition
  if (l_sensor_sleep && millis() < l_sensor_sleep_start + sensor_sleep_mils )
  {
    if (l_sensor_sleep!= l_last_sleep) {
      Serial.println("lower sleep start");
      l_last_sleep = l_sensor_sleep;
    }
    return;   // exit the function early thus disabling sensor reading
  }
  else
  {
    if (l_sensor_sleep!= l_last_sleep) {
      Serial.println("lower sleep end");
      l_last_sleep = l_sensor_sleep;
    }
    l_sensor_sleep= false;  // sensor sleep is over so set the bool to false and let the function continue
  }

  dis_a = read_sensor(sensor_a);      // read sensor a
//  if (dis_a < DISTANCE_THRESHOLD ) {
    //Serial.print("s1 triggered: ");Serial.println(dis_a);
    //Serial.print("flag_a : "); Serial.println(flag_a);
    //Serial.print("flag_b : "); Serial.println(flag_b);
//  }

  if (person > 0 || dis_b < DISTANCE_THRESHOLD) {   // if we have people to exit, or last reading was a trigger
    dis_b = read_sensor(sensor_b);
    //if (dis_b < DISTANCE_THRESHOLD ) {Serial.print("s2 triggered: ");Serial.println(dis_b);}
  }
  //Serial.print("da:"); Serial.println(dis_a);
  //Serial.print("db:"); Serial.println(dis_b);
  
  if (dis_a < DISTANCE_THRESHOLD && flag_a == 0) {
    flag_a = 1;
    l_last_flag_change = millis();
    if (flag_b == 0) {
      person = person + 1;
      last_active = millis();
      //l_sensor_sleep= true;
      //l_sensor_sleep_start = millis();
    }
  }

  if (dis_b < DISTANCE_THRESHOLD && flag_b == 0) {
    flag_b = 1;
    l_last_flag_change = millis();
    if (flag_a == 0) {
      if (person > 0) person = person - 1;
      //l_sensor_sleep= true;
      //l_sensor_sleep_start = millis();
    }
  }
 // <------reset the flags after both have been triggered and also use a timeout in case the second sensor missed
  if (dis_a > DISTANCE_THRESHOLD && dis_b > DISTANCE_THRESHOLD && ((flag_a == 1 || flag_b == 1))) 
  {
      if ((flag_a == 1 && flag_b == 1) || millis() > l_last_flag_change+2000 ) 
      {
      flag_a = 0, flag_b = 0;
      //delay(2000);
      l_sensor_sleep= true;
      l_sensor_sleep_start = millis();
      Serial.println("flag reset");
    }
  }
}

void people_count_upper() {       // function to detect people count at lower sensor group

  ////// check for sensor sleep state and timeout condition
  if (u_sensor_sleep && millis() < u_sensor_sleep_start + sensor_sleep_mils )
  {
    if (u_sensor_sleep!= u_last_sleep) {
      Serial.println("upper sleep start");
      u_last_sleep = u_sensor_sleep;
    }
    return;   // exit the function early thus disabling sensor reading
  }
  else
  {
    if (u_sensor_sleep!= u_last_sleep) {
      Serial.println("upper sleep end");
      u_last_sleep = u_sensor_sleep;
    }
    u_sensor_sleep= false;  // sensor sleep is over so set the bool to false and let the function continue
  }

  dis_c = read_sensor(sensor_c);      // read sensor a
//  if (dis_c < DISTANCE_THRESHOLD ) {
    //Serial.print("s1 triggered: ");Serial.println(dis_c);
    //Serial.print("flag_c : "); Serial.println(flag_c);
    //Serial.print("flag_d : "); Serial.println(flag_d);
//  }

  if (person > 0 || dis_d < DISTANCE_THRESHOLD) {   // if we have people to exit, or last reading was a trigger
    dis_d = read_sensor(sensor_d);
    //if (dis_d < DISTANCE_THRESHOLD ) {Serial.print("s2 triggered: ");Serial.println(dis_d);}
  }
  //Serial.print("da:"); Serial.println(dis_c);
  //Serial.print("db:"); Serial.println(dis_d);
  
  if (dis_c < DISTANCE_THRESHOLD && flag_c == 0) {
    flag_c = 1;
    u_last_flag_change = millis();
    if (flag_d == 0) {
      person = person + 1;
      last_active = millis();
      //u_sensor_sleep= true;
      //u_sensor_sleep_start = millis();
    }
  }

  if (dis_d < DISTANCE_THRESHOLD && flag_d == 0) {
    flag_d = 1;
    u_last_flag_change = millis();
    if (flag_c == 0) {
      if (person > 0) person = person - 1;
      //u_sensor_sleep= true;
      //u_sensor_sleep_start = millis();
    }
  }
 // <------reset the flags after both have been triggered and also use a timeout in case the second sensor missed
  if (dis_c > DISTANCE_THRESHOLD && dis_d > DISTANCE_THRESHOLD && ((flag_c == 1 || flag_d == 1))) 
  {
      if ((flag_c == 1 && flag_d == 1) || millis() > u_last_flag_change+2000 ) 
      {
      flag_c = 0, flag_d = 0;
      //delay(2000);
      u_sensor_sleep= true;
      u_sensor_sleep_start = millis();
      Serial.println("flag reset");
    }
  }
}

void animate_state() {

  if (person != last_person)              // Set the anim state based on people count change.  
  {
    Serial.print("Person count: ");
    Serial.println(person);

    if (person == 1 && last_person == 0 )     // NEED a last activity variable to know which sensor group activity came from
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

  if (person>0 && (millis()> 15000 + last_active))
  {
    person=0;
    sr.setAllLow();
  }

  if (anim_state == TOUPON) {
    //anim_start = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the anim_counter to 0
      anim_counter = 0;
      sr.set(anim_counter, HIGH);
      Serial.println("starting toupon ");
      anim_start = millis();
    } else if ( millis() > anim_start + ANIM_DELAY ) {   // continue the animation if the time interval is up
      anim_counter++;
      sr.set(anim_counter, HIGH);
      //Serial.print(" lights on ");
      //Serial.println(anim_counter);
      anim_start = millis();
    }
    if ( anim_counter >= 15 ) 
    {
      anim_state = NONE;
      Serial.println("stopped toupon");
    }

  }

  if (anim_state == TOUPOFF) {
    //anim_start = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the anim_counter to 0
      anim_counter = 0;
      sr.set(anim_counter, LOW);
      Serial.println("starting toupOff ");
      anim_start = millis();
    } else if ( millis() > anim_start + ANIM_DELAY ) {   // continue the animation if the time interval is up
      anim_counter++;
      sr.set(anim_counter, LOW);
      //Serial.print(" lights off ");
      //Serial.println(anim_counter);
      anim_start = millis();
    }
    if ( anim_counter >= 15 ) 
    {
      anim_state = NONE;
      Serial.println("stopped toupoff");
    }

  }
    if (anim_state == TODOWNON) {
    //anim_start = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the anim_counter to 0
      anim_counter = 15;
      sr.set(anim_counter, HIGH);
      Serial.println("starting todownon ");
      anim_start = millis();
    } else if ( millis() > anim_start + ANIM_DELAY ) {   // continue the animation if the time interval is up
      anim_counter--;
      sr.set(anim_counter, HIGH);
      //Serial.print(" lights on ");
      //Serial.println(anim_counter);
      anim_start = millis();
    }
    if ( anim_counter == 0 ) 
    {
      anim_state = NONE;
      Serial.println("stopped todownon");
    }

  }


  if (anim_state == TODOWNOFF) {
    //anim_start = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the anim_counter to 0
      anim_counter = 15;
      sr.set(anim_counter, LOW);
      Serial.println("starting todownoff");
      anim_start = millis();
    } else if ( millis() > anim_start + ANIM_DELAY ) {   // continue the animation if the time interval is up
      anim_counter--;
      sr.set(anim_counter, LOW);
      //Serial.print(" lights off ");
      //Serial.println(anim_counter);
      anim_start = millis();
    }
    if ( anim_counter == 0 )
    {
      anim_state = NONE;
      Serial.println("stopped todownoff");
    }
  }
  last_anim_state = anim_state;
}
