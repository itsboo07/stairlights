#define e_s1 2 //echo pin
#define t_s1 3 //Trigger pin

#define e_s2 4 //echo pin
#define t_s2 5 //Trigger pin


long dis_a=0,dis_b=0;
int flag1=0, flag2=0;
int person = 0; 
int counter =0;

void peopleCount();

//**********************ultra_read****************************
void ultra_read(int pin_t,int pin_e,long &ultra_time){
long time;
pinMode(pin_t,OUTPUT);
pinMode(pin_e,INPUT);
digitalWrite(pin_t,LOW);
delayMicroseconds(2);
digitalWrite(pin_t,HIGH);
delayMicroseconds(10);
time=pulseIn (pin_e,HIGH);
ultra_time =  time / 29 / 2; 
}


void setup(){
Serial.begin(9600);// initialize serial communication at 9600 bits per second:
Serial.println("BOO Staircase Started");



}

void loop(){ 
  
  peopleCount();
  
}
void peopleCount(){
//*************************
ultra_read(t_s1,e_s1,dis_a);delay(30);
ultra_read(t_s2,e_s2,dis_b);delay(30);
//*************************

Serial.print("da:");Serial.println(dis_a);
Serial.print("db:");Serial.println(dis_b);   

if(dis_a<90 && flag1==0){flag1=1;
if(flag2==0){person = person+1;}
}

if(dis_b<90 && flag2==0){flag2=1;
if(flag1==0){person = person-1;}
}

if(dis_a>90 && dis_b>90 && flag1==1 && flag2==1){
flag1=0, flag2=0;
delay(2000); 
}


Serial.print("Have Person: ");
Serial.print(person);
Serial.print("  ");
 
Serial.print("Light is ");
if (person < 0){person = 0;}
if(person == 1 && flag1 == 1)
{
        sr.setAllHigh();
      }
if(person == 0 && flag2 == 1)
{
        sr.setAllLow();
      }
}
