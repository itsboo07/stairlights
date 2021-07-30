#include <ShiftRegister74HC595.h>
// create a global shift register object
// parameters: <number of shift registers> (data pin, clock pin, latch pin)
ShiftRegister74HC595<2> sr(14, 16, 15);
#define e_s1 2 //echo pin
#define t_s1 3 //Trigger pin

#define e_s2 4 //echo pin
#define t_s2 5 //Trigger pin

#define TOUPON 1
#define TOUPOFF 2
#define TODOWNON 3
#define TODOWNOFF 4
#define NONE -1
int anim_state = NONE;
int last_anim_state = -1;
unsigned long last_millis = 0;
unsigned long last_count = 0;
long dis_a = 0, dis_b = 0;
int flag1 = 0, flag2 = 0;
int person = 0;
int counter = 0;

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
  //*************************
  ultra_read(t_s1, e_s1, dis_a); delay(30);
  ultra_read(t_s2, e_s2, dis_b); delay(30);
  //*************************

  Serial.print("da:"); Serial.println(dis_a);
  Serial.print("db:"); Serial.println(dis_b);

  if (dis_a < 80 && flag1 == 0) {
    flag1 = 1;
    if (flag2 == 0) {
      person = person + 1;
    }
  }

  if (dis_b < 80 && flag2 == 0) {
    flag2 = 1;
    if (flag1 == 0) {
      person = person - 1;
    }
  }

  if (dis_a > 80 && dis_b > 80 && flag1 == 1 && flag2 == 1 ) {
    flag1 = 0, flag2 = 0;
//    delay(4000);
 (millis() - last_count > 4000);
  }


  Serial.print("Have Person: ");
  Serial.print(person);
  Serial.print("  ");

  Serial.print("Light is ");
  if (person < 0) {
    person = 0;
  }
  if (person == 1 && dis_a < 80 && flag1 == 1 )
  {
    anim_state = TODOWNON;

  }
  if (person == 0 && dis_b < 80 && flag2 == 1)
  {
    anim_state = TOUPOFF;
  }

}

void animate_state() {
  
  int time0 = 500;
            

  
  if (anim_state == TOUPON){
  //last_millis = millis();
  if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
    counter = 0;
    sr.set(counter, HIGH);
    Serial.println("starting toupon ");
    last_millis = millis();
  } else if ( millis() > last_millis + time0 ) {   // continue the animation if the time interval is up
    counter++;
    sr.set(counter, HIGH); 
    Serial.print(" lights on ");
    Serial.println(counter);
    last_millis = millis();
  }
  if ( counter >= 15 ) anim_state = NONE;
  
}
  if (anim_state == TOUPOFF){
  //last_millis = millis();
  if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
    counter = 0;
    sr.set(counter, LOW);
    Serial.println("starting toupOff");
    last_millis = millis();
  } else if ( millis() > last_millis + time0 ) {   // continue the animation if the time interval is up
    counter++;
    sr.set(counter, LOW); 
    Serial.print(" lights off ");
    Serial.println(counter);
    last_millis = millis();
  }
  if ( counter >=15) anim_state = NONE;
  
}

if (anim_state == TODOWNON){
  //last_millis = millis();
  if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
    counter = 15;
    sr.set(counter, HIGH);
    Serial.println("starting todownon ");
    last_millis = millis();
  } else if ( millis() > last_millis + time0 ) {   // continue the animation if the time interval is up
    counter--;
    sr.set(counter, HIGH); 
    Serial.print(" lights on ");
    Serial.println(counter);
    last_millis = millis();
  }
  if ( counter == 0 ) anim_state = NONE;
  
}

if (anim_state == TODOWNOFF){
  //last_millis = millis();
  if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
    counter = 15;
    sr.set(counter, LOW);
    Serial.println("starting todownoff ");
    last_millis = millis();
  } else if ( millis() > last_millis + time0 ) {   // continue the animation if the time interval is up
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
