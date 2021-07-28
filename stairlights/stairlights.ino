#include <EEPROM.h>   //We need this library
#include <Arduino.h>  // for type definitions

#include <ShiftRegister74HC595.h>
// create a global shift register object
// parameters: <number of shift registers> (data pin, clock pin, latch pin)
ShiftRegister74HC595<2> sr(14, 16, 15);

#define sensorPin1 8        //ir sensor 1
#define sensorPin2 7        //ir sensor 2
#define sensorPin3 6        //ir sensor 3
#define sensorPin4 5       //ir sensor 4
#define buttonPin 9

#define TOUPON 1
#define TOUPOFF 2
#define TODOWNON 3
#define TODOWNOFF 4
#define NONE -1
int potPin = A4;
int potVal = 0;
int anim_state = NONE;
int last_anim_state = -1;
unsigned long last_millis = 0;
unsigned long last_count = 0;
unsigned long last_count1 = 0;
int sensorState1 = 0;
int sensorState2 = 0;
int sensorState3 = 0;
int sensorState4 = 0;
int lastsensorState1 = HIGH;
int lastsensorState2 = HIGH;
int lastsensorState3 = HIGH;
int lastsensorState4 = HIGH;



int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

int count = 0;
int counter = 0;
int time1 = 200;

// forward declarations
void peopleCount();
void animate_state();
void button_switch();


void setup()
{
  Serial.begin(9600);
  Serial.println("BOO Staircase Started");
  pinMode (sensorPin1, INPUT);
  pinMode (sensorPin2, INPUT);
  pinMode (sensorPin3, INPUT);
  pinMode (sensorPin4, INPUT);
  pinMode (buttonPin, INPUT);
  buttonPushCounter = EEPROM.read(0);
  Serial.print(EEPROM.read(0));

}

void loop() {
  button_switch();

}

void button_switch()
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
    }
    if (buttonPushCounter == 4 ) {
      Serial.println("setting count = 0");
      count = 0; (anim_state = NONE);
      sr.setAll(pinValues);
      delay(1500);
      sr.setAllLow();
    }


    lastButtonState = buttonState;
  }

  if (buttonPushCounter == 1) {
    sr.setAll(pinValues1);
  }
  if (buttonPushCounter == 2) {
    sr.setAllHigh();
  }
  if (buttonPushCounter == 3) {
    sr.setAllLow();
  }
  if (buttonPushCounter == 4) {

    peopleCount();
    animate_state();
  }
  if (buttonPushCounter > 4)
  {
    buttonPushCounter = 1;
  }


}

void peopleCount()
{


  sensorState1 = digitalRead(sensorPin1);
  sensorState2 = digitalRead(sensorPin2);
  sensorState3 = digitalRead(sensorPin3);
  sensorState4 = digitalRead(sensorPin4);
  int timex = 3000;


  if (sensorState1 != lastsensorState1)   //sensor 1
  {
    if (sensorState1 == LOW && (millis() - last_count > timex) )
    {
      count++;
      Serial.println(count);
      last_count = millis();
    }
    lastsensorState1 = sensorState1;
  }

  if (sensorState4 != lastsensorState4)   //sensor 4
  {
    if (sensorState4 == LOW && (millis() - last_count1 > timex) )
    {
      count++;
      Serial.println(count);
      last_count1 = millis();
    }
    lastsensorState4 = sensorState4;
  }


  if (sensorState2 != lastsensorState2)   //sensor 2
  {
    if (sensorState2 == LOW && (millis() - last_count > timex) )
    {
      count--;
      Serial.println(count);
      last_count = millis();
    }
    lastsensorState2 = sensorState2;
  }
  if (sensorState3 != lastsensorState3)   //sensor 3
  {
    if (sensorState3 == LOW && (millis() - last_count1 > timex) )
    {
      count--;
      Serial.println(count);
      last_count1 = millis();
    }
    lastsensorState3 = sensorState3;
  }
  if (count == 0 && sensorState2 == LOW)
  {
    anim_state = TODOWNOFF;
  }
  if (count == 0 && sensorState3 == LOW)
  {
    anim_state = TOUPOFF;

  }
  if (count == 1  &&  sensorState1 == LOW)
  {
    anim_state = TOUPON;
  }
  if (count == 1 &&  sensorState4 == LOW)
  {
    anim_state = TODOWNON;
  }
  if (count < 0)
  {
    count = 0;
  }
}

void animate_state() {

  potVal = analogRead(potPin);       // Read the analog value of the potmeter (0-1023)



  if (anim_state == TOUPON) {
    //last_millis = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
      counter = 0;
      sr.set(counter, HIGH);
      Serial.println("starting toupon ");
      last_millis = millis();
    } else if ( millis() > last_millis + potVal ) {   // continue the animation if the time interval is up
      counter++;
      sr.set(counter, HIGH);
      Serial.print(" lights on ");
      Serial.println(counter);
      last_millis = millis();
    }
    if ( counter >= 15 ) anim_state = NONE;

  }
  if (anim_state == TOUPOFF) {
    //last_millis = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
      counter = 0;
      sr.set(counter, LOW);
      Serial.println("starting toupOff");
      last_millis = millis();
    } else if ( millis() > last_millis + potVal ) {   // continue the animation if the time interval is up
      counter++;
      sr.set(counter, LOW);
      Serial.print(" lights off ");
      Serial.println(counter);
      last_millis = millis();
    }
    if ( counter >= 15) anim_state = NONE;

  }

  if (anim_state == TODOWNON) {
    //last_millis = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
      counter = 15;
      sr.set(counter, HIGH);
      Serial.println("starting todownon ");
      last_millis = millis();
    } else if ( millis() > last_millis + potVal ) {   // continue the animation if the time interval is up
      counter--;
      sr.set(counter, HIGH);
      Serial.print(" lights on ");
      Serial.println(counter);
      last_millis = millis();
    }
    if ( counter == 0 ) anim_state = NONE;

  }

  if (anim_state == TODOWNOFF) {
    //last_millis = millis();
    if (anim_state != last_anim_state) { // state just changed, lets set the counter to 0
      counter = 15;
      sr.set(counter, LOW);
      Serial.println("starting todownoff ");
      last_millis = millis();
    } else if ( millis() > last_millis + potVal ) {   // continue the animation if the time interval is up
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
