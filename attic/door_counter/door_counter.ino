//Adjustable Variables:-
int timeout=150; // (msx10) max time for a person to cross distance between 2 sensors.
int door_end=60; //should be less than 310 and more than 8.
int num=1;  //number of zeros that can be ignored between string of 1's.(value is 1 for ideal condition)
int vindael=900;

//ultrasonic sensor pins.
const int trig1=A0;
const int echo1=A1;
const int trig2=A2;
const int echo2=A3;

int c=0;
int s1(){
  return chk(trig1,echo1);
  }

int s2(){
  return chk(trig2,echo2);
  }


//function to print c on lcd.
void lcdp(){
 
  }

  

//function to return (1)/(0) for (obstacle after going out off way)/(no-obstacle) with error filter.(it ignores 'zeros by error' between 1s)
int chk(int trig, int echo){
 int s=ultra(trig,echo);
 int a=0;
if(s!=404){
  while(1){
    s=ultra(trig,echo);
    if(s==404){a++;}
    if(s!=404){a=0;}
    if(a>=num){return 1;}
    }  
  }
 else{return 0;}
}

//function to get distance of obstacle.
int ultra(int trig, int echo){
  
  long duration;
  int distance;
 
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  duration = pulseIn(echo, HIGH);

  distance = duration*0.0343/2; //veolcity of sound.
  
  if(distance>door_end || distance<8){
   return 404; 
  }
  else{
    return distance;
  }
}
void setup()
{
  Serial.begin(9600);
  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);
}
  int g=0;
void loop()
{
  if(s1()==1 && s2()==0){
    for(int i=0;i<timeout;i++){
      if(s2()==1 && s1()==0){c++;lcdp();Serial.println(c);break;}
      delay(10);
      }
      delay(vindael);
    }
 
  if(s2()==1 && s1()==0){
    for(int i=0;i<timeout;i++){
      if(s1()==1 && s2()==0){c--;lcdp();Serial.println(c);break;}
      delay(10);
      }
      delay(vindael);
    }
}
