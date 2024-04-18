#include <AccelStepper.h>
#include <Wire.h>

AccelStepper stepper(AccelStepper::DRIVER, 4, 3);
int calcSpeed = 300;
int x;

void setup() {
  // Set the maximum speed and acceleration
  stepper.setMaxSpeed(10000);
  stepper.setAcceleration(500);

  Wire.begin(1);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
}

void receiveEvent(int howMany)
{
  x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer

}

void loop() {
  
   // Run the stepper motor
  //stepper.runSpeed();
  if(x==23)
  {
    reverseResin();
  }
  else if(x==22)
  {
    primeResin();
  }
  else if(x==21)
  {
    stopResin();
  }
  else
  {
    startResin();
  }

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
