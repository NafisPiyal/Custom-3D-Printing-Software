#include <AccelStepper.h>

AccelStepper stepper(AccelStepper::DRIVER, 29, 28);
int calcSpeed = 300;

void setup() {
  // Set the maximum speed and acceleration
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);

  // Initialize serial communication
  Serial.begin(9600);

  // Wait for serial connection to be established
  while (!Serial) {
    delay(1000);
  }
}

void loop() {
  // Check for incoming serial data
  if (Serial.available() > 0) {
    char command = Serial.read();

    // Perform actions based on the received command
    switch (command) {
      case 'S':  // Start resin
        startResin();
        break;
      case 'T':  // Stop resin
        stopResin();
        break;
      case 'P':  // Prime resin
        primeResin();
        break;
      case 'R':  // Reverse resin
        reverseResin();
        break;
      case 'V':  // Set speed
        setSpeed();
        break;
    }
  }

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

void setSpeed() {
  // Read the new speed value from serial
  int speed = Serial.parseInt();

  // Set the new speed for the stepper motor
  //stepper.setSpeed(300+(100*speed));
  calcSpeed = 300+(100*speed);
}


