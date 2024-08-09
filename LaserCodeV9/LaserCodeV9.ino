#include <Wire.h>

int x;
#define laser1 9
#define laser2 10
#define laser3 11

void setup() {
  Wire.begin(2);                // join i2c bus with address #2
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  pinMode(laser1, OUTPUT);
  pinMode(laser2, OUTPUT);
  pinMode(laser3, OUTPUT);
  digitalWrite(laser1, LOW);
  digitalWrite(laser2, LOW);
  digitalWrite(laser3, LOW);
}

void receiveEvent(int howMany)
{
  x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}

void loop() {
  
  if(x==51)
  {
    laser3on();
  }
  else if(x==50)
  {
    laser3off();
  }
  else if(x>=31 && x<=40)
  {
    laser2on();
  }
  else if(x==30)
  {
    laser2off();
  }
  else if(x>=11 && x<=20)
  {
    laser1on();
  }
  else if(x==10)
  {
    laser1off();
  }
}

void laser3on() {
  analogWrite(laser3, 10);
} 

void laser3off() {
  analogWrite(laser3, 0);
}

void laser2on() {
  analogWrite(laser2, (x-30)*25.5);
}

void laser2off() {
  analogWrite(laser2, 0);
}

void laser1on() {
  analogWrite(laser1, (x-10)*25.5);
}

void laser1off() {
  analogWrite(laser1, 0);
}
