#include <Wire.h>
int freq = 100;
int cycleTime = 1000/freq;

int x;
#define laser2 9
#define laser1 8
#define laser3 10

void setup() {
  Wire.begin(2);                // join i2c bus with address #2
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  pinMode(laser1, OUTPUT);
  pinMode(laser2, OUTPUT);
  pinMode(laser3, OUTPUT);
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
  else if(x>=31 && x<=41)
  {
    laser2on();
  }
  else if(x==30)
  {
    laser2off();
  }
  else if(x>=11 && x<=21)
  {
    laser1on();
  }
  else if(x==10)
  {
    laser1off();
  }
}

void laser3on() {

} 

void laser3off() {

}

void laser2on() {
  int dutycycle2=(x-31)*10;
  onTime2=cycleTime*dutycycle2/100;
  offtime2=cycleTime-onTime;
  

  }

}

void laser2off() {

}

void laser1on() {

}

void laser1off() {

}

void startResin() {
  stepper.setSpeed(x*calcSpeed);// Set the initial speed
  stepper.runSpeed();  // Start the motor
}

void stopResin() {
  stepper.setSpeed(0);
  stepper.stop();  // Stop the motor
}

void primeResin() {
  stepper.setSpeed(6000);  // Set the speed for priming
  stepper.runSpeed();  // Start the motor for priming
}

void reverseResin() {
  stepper.setSpeed(-6000);  // Set the speed for reversing
  stepper.runSpeed();  // Start the motor for reversing
}


