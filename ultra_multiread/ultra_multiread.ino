#include <EEPROM.h>   //We need this library
#include <Arduino.h>  // for type definitions

#include <ShiftRegister74HC595.h>
#include "HCSR04_Custom.h"

//#define D_THRESHOLD 100     // the minimum maximum distance to indicate a trigger
#define MEASURE_COUNT 3;   // the number of measurement below the threshold in a row required to indicate a person

#define buttonPin A3
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

UltraSonicDistanceSensor sensor_a(t_sA, e_sA);  //
UltraSonicDistanceSensor sensor_b(t_sB, e_sB);  //
UltraSonicDistanceSensor sensor_c(t_sC, e_sC);  //
UltraSonicDistanceSensor sensor_d(t_sD, e_sD);  //
int person_count = 0;
const int m_count = MEASURE_COUNT ;

// create a global shift register object
// parameters: <number of shift registers> (data pin, clock pin, latch pin)
ShiftRegister74HC595<2> sr(14, 16, 15);


//#define ANIM_DELAY 4000  // 100 ms between each light turnon 
//replaced with potVal
#define DISTANCE_THRESHOLD 80 // the distance to trigger the sensor on

int potPin = A4;
int potVal = 0; //to control the animations speed with the pot

/// Animation states
#define UPPER_ON 1
#define UPPER_OFF 2
#define LOWER_ON 3
#define LOWER_OFF 4
#define NONE -1
#define LOWER 0
#define UPPER 1

int anim_state = NONE;                // state variable for current animation state
int last_anim_state = -1;             // last state of above
unsigned long anim_start = 0;         // millis for when animation has started

int person = 0, last_person = 0;      // person counter and last person -- used to detect when person count has changed
int anim_counter = 0;                 // animation counter -- used to keep track of animation progress
const int sensor_sleep_mils = 1000;   // amount of time to sleep a sensor pair after both get triggered
const unsigned long lights_out_timeout = 60000; // 1min with no activity we reset
int last_active_group = LOWER;
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

int buttonPushCounter = 0;              // counter for the number of button presses
int buttonState = 0;                    // current state of the button
int lastButtonState = 0;                // previous state of the button
bool sensor_control = false;



unsigned long last_active = 0;        // the time when we last had activity -- used to timeout a lights on condition if the senosr missed the exit

int read_sensor(UltraSonicDistanceSensor &sensor) {    /// take multiple reading to eliminate spurious triggers
  long d = 0;
  long last_d = sensor.measureDistanceCm();
  delay(10);
  for (int i = 0 ; i < m_count ; i++ )
  {
    d = sensor.measureDistanceCm();   // <---- actually the second measurement
    ////////// check for mis-matched measurements
    if ((d > DISTANCE_THRESHOLD && last_d < DISTANCE_THRESHOLD) || (d < DISTANCE_THRESHOLD && last_d > DISTANCE_THRESHOLD) ) {
      return 1000;
    }
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
  Serial.println("BOO Staircase Started");
  
  pinMode(t_sA, OUTPUT);
  pinMode(e_sA, INPUT);
  pinMode(t_sB, OUTPUT);
  pinMode(e_sB, INPUT);
  pinMode(t_sC, OUTPUT);
  pinMode(e_sC, INPUT);
  pinMode(t_sD, OUTPUT);
  pinMode(e_sD, INPUT);
  pinMode (buttonPin, INPUT);
  buttonPushCounter = EEPROM.read(0);
  Serial.println(EEPROM.read(0));
//  Serial.println(potVal);

}

void loop() {

  detect_button();      // detect button will set sensor_control = true if we at buttoncounter 3
  if (sensor_control) {
    people_count_lower();
    people_count_upper();

  }
  animate_state();


}

void people_count_lower() {       // function to detect people count at lower sensor group

  ////// check for sensor sleep state and timeout condition
  if (l_sensor_sleep && millis() < l_sensor_sleep_start + sensor_sleep_mils )
  {
    if (l_sensor_sleep != l_last_sleep) {
      Serial.println("lower sleep start");
      l_last_sleep = l_sensor_sleep;
    }
    return;   // exit the function early thus disabling sensor reading
  }
  else
  {
    if (l_sensor_sleep != l_last_sleep) {
      Serial.println("lower sleep end");
      l_last_sleep = l_sensor_sleep;
    }
    l_sensor_sleep = false; // sensor sleep is over so set the bool to false and let the function continue
  }

  dis_a = read_sensor(sensor_a);      // read sensor a
  if (dis_a < DISTANCE_THRESHOLD ) {
    Serial.print("sA triggered: "); Serial.println(dis_a);
    //Serial.print("flag_a : "); Serial.println(flag_a);
    //Serial.print("flag_b : "); Serial.println(flag_b);
  }

  if (person > 0 || dis_b < DISTANCE_THRESHOLD) {   // if we have people to exit, or last reading was a trigger
    dis_b = read_sensor(sensor_b);
    if (dis_b < DISTANCE_THRESHOLD ) {
      Serial.print("sB triggered: ");
      Serial.println(dis_b);
    }
  }
  //Serial.print("da:"); Serial.println(dis_a);
  //Serial.print("db:"); Serial.println(dis_b);

  if (dis_a < DISTANCE_THRESHOLD && flag_a == 0) {
    flag_a = 1;
    l_last_flag_change = millis();
    if (flag_b == 0) {
      person = person + 1;
      last_active_group = LOWER;
      last_active = millis();
      //l_sensor_sleep= true;
      //l_sensor_sleep_start = millis();
    }
  }

  if (dis_b < DISTANCE_THRESHOLD && flag_b == 0) {
    flag_b = 1;
    l_last_flag_change = millis();
    if (flag_a == 0) {
      if (person > 0) {
        person = person - 1;
        last_active_group = LOWER;
      }
      //l_sensor_sleep= true;
      //l_sensor_sleep_start = millis();
    }
  }
  // <------reset the flags after both have been triggered and also use a timeout in case the second sensor missed
  if (dis_a > DISTANCE_THRESHOLD && dis_b > DISTANCE_THRESHOLD && ((flag_a == 1 || flag_b == 1)))
  {
    if ((flag_a == 1 && flag_b == 1) || millis() > l_last_flag_change + 2000 )
    {
      flag_a = 0, flag_b = 0;
      //delay(2000);
      l_sensor_sleep = true;
      l_sensor_sleep_start = millis();
      Serial.println("flag reset");
    }
  }
}

void people_count_upper() {       // function to detect people count at lower sensor group

  ////// check for sensor sleep state and timeout condition
  if (u_sensor_sleep && millis() < u_sensor_sleep_start + sensor_sleep_mils )
  {
    if (u_sensor_sleep != u_last_sleep) {
      Serial.println("upper sleep start");
      u_last_sleep = u_sensor_sleep;
    }
    return;   // exit the function early thus disabling sensor reading
  }
  else
  {
    if (u_sensor_sleep != u_last_sleep) {
      Serial.println("upper sleep end");
      u_last_sleep = u_sensor_sleep;
    }
    u_sensor_sleep = false; // sensor sleep is over so set the bool to false and let the function continue
  }

  dis_d = read_sensor(sensor_d);      // read sensor a
  if (dis_d < DISTANCE_THRESHOLD ) {
    Serial.print("sD triggered: "); Serial.println(dis_d);
    //Serial.print("flag_c : "); Serial.println(flag_c);
    //Serial.print("flag_d : "); Serial.println(flag_d);
  }

  if (person > 0 || dis_c < DISTANCE_THRESHOLD) {   // if we have people to exit, or last reading was a trigger
    dis_c = read_sensor(sensor_c);
    if (dis_c < DISTANCE_THRESHOLD ) {
      Serial.print("sC triggered: ");
      Serial.println(dis_c);
    }
  }
  //Serial.print("da:"); Serial.println(dis_c);
  //Serial.print("db:"); Serial.println(dis_d);

  if (dis_d < DISTANCE_THRESHOLD && flag_d == 0) {
    flag_d = 1;
    u_last_flag_change = millis();
    if (flag_c == 0) {
      person = person + 1;
      last_active = millis();
      last_active_group = UPPER;
      //u_sensor_sleep= true;
      //u_sensor_sleep_start = millis();
    }
  }

  if (dis_c < DISTANCE_THRESHOLD && flag_c == 0) {
    flag_c = 1;
    u_last_flag_change = millis();
    if (flag_d == 0) {
      if (person > 0) {
        person = person - 1;
        last_active_group = UPPER;
      }
      //u_sensor_sleep= true;
      //u_sensor_sleep_start = millis();
    }
  }
  // <------reset the flags after both have been triggered and also use a timeout in case the second sensor missed
  if (dis_c > DISTANCE_THRESHOLD && dis_d > DISTANCE_THRESHOLD && ((flag_c == 1 || flag_d == 1)))
  {
    if ((flag_c == 1 && flag_d == 1) || millis() > u_last_flag_change + 2000 )
    {
      flag_c = 0, flag_d = 0;
      //delay(2000);
      u_sensor_sleep = true;
      u_sensor_sleep_start = millis();
      Serial.println("flag reset");
    }
  }
}

void animate_state() {
  potVal  = analogRead(potPin);       // Read the analog value of the potmeter (0-1023)
  
  if (person != last_person)              // Set the anim state based on people count change.
  {
    Serial.print("Person count:  ----------------------> ");
    Serial.println(person);

    if (person == 1 && last_person == 0 )     // NEED a last activity variable to know which sensor group activity came from
    {
      if (last_active_group == LOWER) {
        //Serial.println("LOWER_ON");
        anim_state = LOWER_ON;
      } else {
        anim_state = UPPER_ON;
      }
    }
    if (person == 0 && last_person == 1)
    {
      if (last_active_group == UPPER) {
        anim_state = LOWER_OFF;
      } else {
        anim_state = UPPER_OFF;
      }
    }
    last_person = person;
  }

  if (person > 0 && millis() > lights_out_timeout + last_active)
  {
    Serial.println("lights out timeout");
    person = 0;
    sr.setAllLow();
  }

  if (anim_state == LOWER_ON) {
    //anim_start = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the anim_counter to 0
      anim_counter = 0;
      sr.set(anim_counter, HIGH);
      Serial.println("--------------> Start LOWER_ON ");
      anim_start = millis();
    } else if ( millis() > anim_start + potVal ) {   // continue the animation if the time interval is up
      anim_counter++;
      sr.set(anim_counter, HIGH);
      //Serial.print(" lights on ");
      //Serial.println(anim_counter);
      anim_start = millis();
    }
    if ( anim_counter >= 15 )
    {
      anim_state = NONE;
      Serial.println("--------------> Start LOWER_ON");
    }

  }

  if (anim_state == LOWER_OFF) {
    //anim_start = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the anim_counter to 0
      anim_counter = 0;
      sr.set(anim_counter, LOW);
      Serial.println("--------------> Start LOWER_OFF ");
      anim_start = millis();
    } else if ( millis() > anim_start + potVal ) {   // continue the animation if the time interval is up
      anim_counter++;
      sr.set(anim_counter, LOW);
      //Serial.print(" lights off ");
      //Serial.println(anim_counter);
      anim_start = millis();
    }
    if ( anim_counter >= 15 )
    {
      anim_state = NONE;
      Serial.println("--------------> Stop LOWER_OFF");
    }

  }
  if (anim_state == UPPER_ON) {
    //anim_start = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the anim_counter to 0
      anim_counter = 15;
      sr.set(anim_counter, HIGH);
      Serial.println("--------------> Start UPPER_ON ");
      anim_start = millis();
    } else if ( millis() > anim_start + potVal ) {   // continue the animation if the time interval is up
      anim_counter--;
      sr.set(anim_counter, HIGH);
      //Serial.print(" lights on ");
      //Serial.println(anim_counter);
      anim_start = millis();
    }
    if ( anim_counter == 0 )
    {
      anim_state = NONE;
      Serial.println("--------------> Stop UPPER_ON");
    }

  }


  if (anim_state == UPPER_OFF) {
    //anim_start = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the anim_counter to 0
      anim_counter = 15;
      sr.set(anim_counter, LOW);
      Serial.println("--------------> Start UPPER_OFF");
      anim_start = millis();
    } else if ( millis() > anim_start + potVal ) {   // continue the animation if the time interval is up
      anim_counter--;
      sr.set(anim_counter, LOW);
      //Serial.print(" lights off ");
      //Serial.println(anim_counter);
      anim_start = millis();
    }
    if ( anim_counter == 0 )
    {
      anim_state = NONE;
      Serial.println("--------------> Stop UPPER_OFF");
    }
  }
  last_anim_state = anim_state;
}


void detect_button()
{

  buttonState = digitalRead(buttonPin);
  uint8_t pinValues[] = { B00000001, B01000000};
  uint8_t pinValues1[] = { B01010101, B01010101};   
      

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
     EEPROM.write(0, buttonPushCounter);
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter++;
      Serial.println("on");
      Serial.print("number of button pushes: ");
      Serial.println(buttonPushCounter);
      if (buttonPushCounter == 4 ) {
        Serial.println("setting count = 0");
        person = 0;
        sr.setAll(pinValues);
        delay(3000);
        sr.setAllLow();
       } 
      else {
      sensor_control = false;
      }
    }
      
     
    
    lastButtonState = buttonState;
  }

  if (buttonPushCounter == 1) {
     anim_state = NONE;
    sr.setAll(pinValues1);
  }
  if (buttonPushCounter == 2) {
     sr.setAllHigh();     
  }
  if (buttonPushCounter == 3) {
   sr.setAllLow();
  }
   if (buttonPushCounter == 4) {
      sensor_control = true;
    } else {
      sensor_control = false;
    }
  if (buttonPushCounter > 4)
      {
       buttonPushCounter = 1;
      }


}
