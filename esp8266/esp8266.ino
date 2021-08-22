#include<SoftwareSerial.h>
SoftwareSerial abc(3,1);

void setup(){
  abc.begin(9600);
}
 void loop(){
  abc.write("1");
  delay(3000);
  abc.write("2");
  delay(3000);
  abc.write("3");
  delay(3000);
  abc.write("4");
  delay(3000);
 }
