#include <HCSR04.h>

#define d_threshold 40
UltraSonicDistanceSensor sensor1(A0, A1);  //
UltraSonicDistanceSensor sensor2(A2, A3);  //
int person_count = 0;
int s1_state = 0, s1_last_state = 0;   // 0 is open/not triggered, 1 is closed/triggered
int s2_state = 0, s2_last_state = 0;
unsigned long s1_fall_millis, s2_fall_millis;  // we keep track of of when the last time the sensor when from high to low

void process_sensors() {   // set the state and determine if we have a falling and if so record the time for each sensor
  s1_last_state = s1_state;
  s2_last_state = s2_state;
  s1_state = sensor1.measureDistanceCm() < d_threshold ? 1 : 0;
  s2_state = sensor2.measureDistanceCm() < d_threshold ? 1 : 0;
  if (s1_last_state == HIGH && s1_state == LOW) {   // falling edge for s1
    s1_fall_millis = millis();   // mark the time
    if (s2_fall_millis < s1_fall_millis && s1_fall_millis - s2_fall_millis < 1000 && s1_fall_millis - s2_fall_millis > 25 ) { // if the fall times are less then 2 seconds apart we have an exit
      if ( person_count > 0 )
      {
        person_count -= 1;
      } 
      else
      {
        Serial.println("TELEPORTATION DETECTED");
      }
      Serial.print("person_count: "); Serial.println(person_count);
    }
  }
  else if (s2_last_state == HIGH && s2_state == LOW) 
  {
    s2_fall_millis = millis();
    if ( s2_fall_millis > s1_fall_millis && s2_fall_millis - s1_fall_millis < 1000 && s2_fall_millis - s1_fall_millis > 25) 
    { /// we have an entry
      person_count += 1;
      Serial.print("person_count: "); Serial.println(person_count);
    }
  }


}

void setup () {
  Serial.begin(9600);  // We initialize serial connection so that we could print values from sensor.
}


void loop () {
  static long last_sensor_millis = millis();
  if ( millis() - last_sensor_millis > 50 ) {
    process_sensors();
    last_sensor_millis = millis();
  }
  delay(10);
}
