#include <ShiftRegister74HC595.h>
#include <HCSR04.h>

#define D_THRESHOLD 80     // the minimum maximum distance to indicate a trigger
#define MEASURE_COUNT 3;   // the number of measurement below the threshold in a row required to indicate a person
UltraSonicDistanceSensor sensor1(A0, A1);  //
UltraSonicDistanceSensor sensor2(A2, A3);  //
int person_count = 0;
const int m_count = MEASURE_COUNT ;

// create a global shift register object
// parameters: <number of shift registers> (data pin, clock pin, latch pin)
ShiftRegister74HC595<2> sr(14, 16, 15);
#define e_s1 A1 //2 //echo pin
#define t_s1 A0 //3 //Trigger pin

#define e_s2 A3 //4 //echo pin
#define t_s2 A2 //5 //Trigger pin

#define ANIM_DELAY 100  // 100 ms between each light turnon
#define DISTANCE_THRESHOLD 60 // the distance to trigger the sensor on

#define TOUPON 1
#define TODOWNOFF 2
#define NONE -1
int anim_state = NONE;
int last_anim_state = -1;
unsigned long last_millis = 0;
long dis_a = 0, dis_b = 0;
int flag1 = 0, flag2 = 0;
int person = 0, last_person = 0;
int counter = 0;
const int sensor_sleep_mils = 2000;
unsigned long sensor_sleep_start = 0;
bool sensor_sleep = false;
unsigned long last_flag_change;
void peopleCount();
void animate_state();

int read_sensor(UltraSonicDistanceSensor &sensor) {    /// take multiple reading to eliminate spurious triggers
  long d = 0;
  long last_d = sensor.measureDistanceCm();
  delay(10);
  for (int i = 0 ; i < m_count ; i++ ) 
  {
    d = sensor.measureDistanceCm();   // <---- actually the second measurement
    ////////// check for mis-matched measurements 
    if ((d > D_THRESHOLD && last_d < D_THRESHOLD) || (d < D_THRESHOLD && last_d > D_THRESHOLD) ) {return 1000;}
    delay(10);
    last_d = d;
  }
  if (last_d < D_THRESHOLD ) 
  { 

    return last_d;   // if we get here then got consecutive readings below threshold
  }
  else {
    return last_d;
  }
}


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
    return;   // exit the function early thus disabling sensor reading
  }
  else
  {
    sensor_sleep = false;  // sensor sleep is over so set the bool to false and let the function continue
  }

  //*************************
  // ultra_read(t_s1, e_s1, dis_a); delay(30);
  // ultra_read(t_s2, e_s2, dis_b); delay(30);
  // //*************************
  dis_a = read_sensor(sensor1);
  if (dis_a < DISTANCE_THRESHOLD ) Serial.println("s1 triggered");
  dis_b = read_sensor(sensor2);
  if (dis_b < DISTANCE_THRESHOLD ) Serial.println("s2 triggered");

  //Serial.print("da:"); Serial.println(dis_a);
  //Serial.print("db:"); Serial.println(dis_b);

  if (dis_a < DISTANCE_THRESHOLD && flag1 == 0) {
    flag1 = 1;
    last_flag_change = millis();
    if (flag2 == 0) {
      person = person + 1;
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
                                        // reset the flags after both have been triggered and also use a timeout in case the second sensor missed
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
    Serial.println("TOUPON");
    anim_state = TOUPON;
    // for (int i = 0; i < 16; i++) {
    //   sr.set(i, HIGH);
    //   delay(400);
    //   Serial.print("turn on lights ");
    //   Serial.println(i);
    //}
    }
    if (person == 0 && last_person == 1)
    {
      Serial.println("TODOWNOFF");
      anim_state = TODOWNOFF;
      // for (int i = 15; i >= 0; i--) {
      //   sr.set(i, LOW);
      //   delay(400);
      //   Serial.print("turn off lights ");
      //   Serial.println(i);
      //}
    }
    last_person = person;
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