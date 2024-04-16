#include <Wire.h>
#include <AccelStepper.h>

AccelStepper stepper(AccelStepper::DRIVER, 29, 28);
int calcSpeed = 300;

void setup() {
  // Set the maximum speed and acceleration
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);

  // Initialize I2C communication as a slave with address 9
  Wire.begin(9);
  Wire.onReceive(receiveEvent);

  // Wait for I2C connection to be established
  while (!Wire.available()) {
    delay(1000);
  }
}

void loop() {
  // Run the stepper motor
  stepper.runSpeed();
}

void startResin() {
  stepper.setSpeed(calcSpeed);// Set the initial speed
  stepper.runSpeed();  // Start the motor
}

void stopResin() {
  stepper.setSpeed(0);
  stepper.stop();  // Stop the motor
}

void primeResin() {
  stepper.setSpeed(600);  // Set the speed for priming
  stepper.runSpeed();  // Start the motor for priming
}

void reverseResin() {
  stepper.setSpeed(-600);  // Set the speed for reversing
  stepper.runSpeed();  // Start the motor for reversing
}

void setSpeed(int speed) {
  calcSpeed = 300 + (100 * speed);
}

void receiveEvent(int numBytes) {
  while (Wire.available()) {
    char command = Wire.read();
    switch (command) {
      case '1':  // Start resin
        startResin();
        break;
      case '0':  // Stop resin
        stopResin();
        break;
      case '2':  // Prime resin
        primeResin();
        break;
      case '3':  // Reverse resin
        reverseResin();
        break;
      //case 'V':  // Set speed
      //  int speed = Wire.read();
      //  setSpeed(speed);
      //  break;
    }
  }
}
