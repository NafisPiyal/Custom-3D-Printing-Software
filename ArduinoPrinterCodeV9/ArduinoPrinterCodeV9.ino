/*
 ** Common pins to attach SD card reader to:
 ** MOSI - pin 11 or 51
 ** MISO - pin 12 or 50
 ** CLK - pin 13 or 52
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN) or 53
*/

#include <SPI.h>
#include <SD.h>
#include <AccelStepper.h>
#include <Wire.h>

// Defines four AccelStepper profiles for each motor
// Change to reflect printer setup (step, dir)
//AccelStepper x_stepper(AccelStepper::DRIVER, 29, 28);
//AccelStepper y_stepper(AccelStepper::DRIVER, 31, 30);
//AccelStepper z_stepper(AccelStepper::DRIVER, 33, 32);
//AccelStepper r_stepper(AccelStepper::DRIVER, 36, 34);
AccelStepper x_stepper(AccelStepper::DRIVER, 29, 28);
AccelStepper y_stepper(AccelStepper::DRIVER, 31, 30);
AccelStepper z_stepper(AccelStepper::DRIVER, 33, 32);
AccelStepper r_stepper(AccelStepper::DRIVER, 36, 34);

// Variables for within arduino code
int arr_pos = 0;
int rpm = 200;
int current_mode = 0;
int previous_mode = 0;
bool isDone = false;
int mnlMv[8] = {0,0,0,0,0,0,0,0};
int xPos = 0;
int yPos = 0;
int zPos = 0;
int rPos = 0;
int ePos = 0;
int l1Pos = 0;
int l2Pos = 0;
int l3Pos = 0;
int xyFactor = 40;
int zFactor = 400;
float rFactor = 35.5555555555555;
int currentBatch = 0;
int currentBatchSize = 0;
int arraySize = 400;


// Variables used with the SD card, cords
//  Variables are accessed throughout the program
File myFile;
int RunCount = 0;

// Note, these values are about the max reasonable values I
// could make these arrarys. And while the program should be 
// designed to know if the file has more lines than each array
// has space I never got around to that and that functionality
// needs to be added.
int xCords[arraySize];
int yCords[arraySize];
int zCords[arraySize];
int rCords[arraySize];
int eCords[arraySize];
int l1Cords[arraySize];
int l2Cords[arraySize];
int l3Cords[arraySize];

void setup() {
  // These speeds can be adjusted and tested for specific machine that
  // the program is being implemented on.
  x_stepper.setMaxSpeed(2000);
  x_stepper.setSpeed(rpm);
  x_stepper.setAcceleration(1000);

  y_stepper.setMaxSpeed(2000);
  y_stepper.setSpeed(rpm);
  y_stepper.setAcceleration(1000);

  z_stepper.setMaxSpeed(2000);
  z_stepper.setSpeed(rpm);
  z_stepper.setAcceleration(1000);

  r_stepper.setMaxSpeed(2000);
  r_stepper.setSpeed(rpm);
  r_stepper.setAcceleration(1000);


  // Port used for CS by SD card. This can verify depending on installation
  SD.begin(53);
  Wire.begin();
  // Baud rate NEEDS to match baud rate specified in the GUI
  Serial.begin(9600);
  // Loads data from SD card immediately. Program does not flag bad data
  // in any way and may act unexpectedly if recieves bad data 
  GetSize();
  GetCords();
}

void loop() {
    recvData();
    prcsData();
    //Start Print
    if (current_mode == 1) {
      previous_mode = current_mode;
      if (!isDone) {
        autoPrint();
      }
      //Pause Print
      else {
        current_mode = 2;
      }
    }
    //Pause Print
    if (current_mode == 2) {
      previous_mode = current_mode;
    }
    //Stop Print
    if (current_mode == 3) {
      previous_mode = current_mode;
      arr_pos = 0;
      isDone = false;
      currentBatch = 0;  // Reset the batch count
      GetCords();  // Reload the first batch of coordinates
      current_mode = 2;
    }
    //Go Home
    if (current_mode == 4) {
      x_stepper.setCurrentPosition(0);
      y_stepper.setCurrentPosition(0);
      z_stepper.setCurrentPosition(0);
      r_stepper.setCurrentPosition(0);
      //e_stepper.setCurrentPosition(0);
      //l_stepper.setCurrentPosition(0);

      x_stepper.runToNewPosition(0-xPos);
      y_stepper.runToNewPosition(0-yPos);
      z_stepper.runToNewPosition(zPos-0);
      r_stepper.runToNewPosition(0-rPos);
      //e_stepper.runToNewPosition(0-ePos);
      //l_stepper.runToNewPosition(0-lPos);

      x_stepper.setCurrentPosition(0);
      y_stepper.setCurrentPosition(0);
      z_stepper.setCurrentPosition(0);
      r_stepper.setCurrentPosition(0);
      //e_stepper.setCurrentPosition(0);
      //l_stepper.setCurrentPosition(0);

      xPos = 0;
      yPos = 0;
      zPos = 0;
      rPos = 0;
      //ePos = 0;
      //lPos = 0;
      //Pause Print
      current_mode = 2;
    }
    //Set Home
    if (current_mode == 5) {
      x_stepper.setCurrentPosition(0);
      y_stepper.setCurrentPosition(0);
      z_stepper.setCurrentPosition(0);
      r_stepper.setCurrentPosition(0);
      xPos = 0;
      yPos = 0;
      zPos = 0;
      rPos = 0;
      //Pause Print
      current_mode = 2;
    }
    //Get Print Status
    if (current_mode == 6) {
      Serial.print(arr_pos + 1);
      Serial.print('/');
      Serial.print(RunCount); //Working
      if (previous_mode == 1) {
        //Start Print
        current_mode = 1;
      }
      else if (previous_mode == 3) {
        //Stop Print
        current_mode = 3;
      }
      else {
        //Pause Print
        current_mode = 2;
      }
    }
    //Manual Print
    if (current_mode == 7) {
      previous_mode = current_mode;
      recvData();
      prcsData();
      manualPrint();
      //Pause Print
      current_mode = 2;
    }
}

// manualPrint method handles recieved coordinates from
// the program and adjusts the printer by specified amount.
// I did not test doing this while in the middle of a print
// but the way I designed it, it should work. Although why would you.
void manualPrint() {
  while (true) {
    if (mnlMv[0]+mnlMv[1]+mnlMv[2]+mnlMv[3]+mnlMv[4]+mnlMv[5]+mnlMv[6]+mnlMv[7] == 0) {
      break;
    }
    if (mnlMv[4]!=0){
        Wire.beginTransmission(1);
        Wire.write(mnlMv[4]);
        Wire.endTransmission();
        mnlMv[4]=0;
    }
    if (mnlMv[5]!=0){
        Wire.beginTransmission(2);
        Wire.write(mnlMv[5]);
        Wire.endTransmission();
        mnlMv[5]=0;
    }
    if (mnlMv[6]!=0){
        Wire.beginTransmission(2);
        Wire.write(mnlMv[6]);
        Wire.endTransmission();
        mnlMv[6]=0;
    }
    if (mnlMv[7]!=0){
        Wire.beginTransmission(2);
        Wire.write(mnlMv[7]);
        Wire.endTransmission();
        mnlMv[7]=0;
    }
    if (x_stepper.distanceToGo() == 0 && y_stepper.distanceToGo() == 0 && z_stepper.distanceToGo() == 0 && r_stepper.distanceToGo() == 0) {
      xPos += mnlMv[0];
      yPos += mnlMv[1];
      zPos += mnlMv[2];
      rPos += mnlMv[3];
      //ePos += mnlMv[4];
      //lPos += mnlMv[5];
      x_stepper.setCurrentPosition(0);
      x_stepper.moveTo(mnlMv[0]);
      x_stepper.setSpeed(rpm);
      y_stepper.setCurrentPosition(0);
      y_stepper.moveTo(mnlMv[1]);
      y_stepper.setSpeed(rpm);
      z_stepper.setCurrentPosition(0);
      z_stepper.moveTo((mnlMv[2] * -1));
      z_stepper.setSpeed(rpm);
      r_stepper.setCurrentPosition(0);
      r_stepper.moveTo(mnlMv[3]);
      r_stepper.setSpeed(rpm);
    }
    else {
      x_stepper.runSpeedToPosition();
      y_stepper.runSpeedToPosition();
      z_stepper.runSpeedToPosition();
      r_stepper.runSpeedToPosition();
      //e_stepper.runSpeedToPosition();
      //l_stepper.runSpeedToPosition();
      if (x_stepper.distanceToGo() == 0 && y_stepper.distanceToGo() == 0 && z_stepper.distanceToGo() == 0 && r_stepper.distanceToGo() == 0) {
        break;
      }
    }
  }
  // Clears amount to move after finished printing
  mnlMv[0] = 0;
  mnlMv[1] = 0;
  mnlMv[2] = 0;
  mnlMv[3] = 0;
  mnlMv[4] = 0;
  mnlMv[5] = 0;
  mnlMv[6] = 0;
  mnlMv[7] = 0;
}

// Used as the main printing functionality and prints from the file initially loaded
// grabs the next coordinate whenever the printer finishes the previous movement
void autoPrint() {
  if (x_stepper.distanceToGo() == 0 && y_stepper.distanceToGo() == 0 && z_stepper.distanceToGo() == 0 && r_stepper.distanceToGo() == 0 && arr_pos < RunCount /*&& eCords[arr_pos]!=-1 && l1Cords[arr_pos]!=-1 && l2Cords[arr_pos]!=-1*/) {
    if (arr_pos >= currentBatchSize) {
      if (currentBatch * arraySize < RunCount) {
        // We have more lines to process
        GetCords();
        arr_pos = 0;
      } else {
        isDone = true;
        return;
      }
    }
    
    xPos += xCords[arr_pos];
    yPos += yCords[arr_pos];
    zPos += zCords[arr_pos];
    rPos += rCords[arr_pos];
    //ePos += eCords[arr_pos];
    //l1Pos += l1Cords[arr_pos];
    //l2Pos += l2Cords[arr_pos];
    //l3Pos += l3Cords[arr_pos];
    x_stepper.setCurrentPosition(0);
    x_stepper.moveTo(xCords[arr_pos]*xyFactor);
    x_stepper.setSpeed(rpm);
    y_stepper.setCurrentPosition(0);
    y_stepper.moveTo(yCords[arr_pos]*xyFactor);
    y_stepper.setSpeed(rpm);
    z_stepper.setCurrentPosition(0);
    z_stepper.moveTo(zCords[arr_pos]*zFactor);
    z_stepper.setSpeed(rpm);
    r_stepper.setCurrentPosition(0);
    r_stepper.moveTo(rCords[arr_pos]*rFactor);
    r_stepper.setSpeed(rpm);
    if (eCords[arr_pos]!=0){
        Wire.beginTransmission(1);
        Wire.write(eCords[arr_pos]);
        Wire.endTransmission();
        eCords[arr_pos]=0;
    }
    if (l1Cords[arr_pos]!=0){
        Wire.beginTransmission(2);
        Wire.write(l1Cords[arr_pos]+11);
        Wire.endTransmission();
        l1Cords[arr_pos]=0;
    }
    if (l2Cords[arr_pos]!=0){
        Wire.beginTransmission(2);
        Wire.write(l2Cords[arr_pos]+31);
        Wire.endTransmission();
        l2Cords[arr_pos]=0;
    }



    arr_pos++;
  }
  else if (arr_pos == RunCount) {
    isDone = true;
  }
  else {
    x_stepper.runSpeedToPosition();
    y_stepper.runSpeedToPosition();
    z_stepper.runSpeedToPosition();
    r_stepper.runSpeedToPosition();
    //e_stepper.runSpeedToPosition();
    //l_stepper.runSpeedToPosition();
  }
}

// Used to get count of lines in the file.
// Note, you can use this to adjust the program
// to work with files with 500+ lines but adjusting
// the program logic to go and get another set of 500
// lines whenever it finishes the first 500.
void GetSize() {
  myFile = SD.open("output.txt");
  char tempChar;
  RunCount = 0;
  while (myFile.available()) {
    tempChar = char(myFile.read());
    if (tempChar == '\n') {
      RunCount++;
    }
  }
  // close the file:
  myFile.close();
}

// After getting the length of the file goes through the 
// file and stores all the x, y, and z coordinates in their
// corresponding arrays
// Expects that the file on the sd card be named "output.txt"
void GetCords() {
  myFile = SD.open("output.txt");
  char tempChar;
  int tempIntVal;
  String tempVal;
  String tempInt;

  for (int i = 0; i < currentBatch * arraySize; i++) {
    myFile.readStringUntil('\n');
  }
  
  // Calculate the size of the current batch
  // This ensures we process up to 400 lines at a time, but no more than the remaining lines in the file
  // For example, if RunCount is 1250:
  // - First batch:  min(400, 1250 - (0 * 400)) = 400
  // - Second batch: min(400, 1250 - (1 * 400)) = 400
  // - Third batch:  min(400, 1250 - (2 * 400)) = 400
  // - Fourth batch: min(400, 1250 - (3 * 400)) = 50
  // This ensures we don't try to read more lines than are left in the file for the final batch
  currentBatchSize = min(arraySize, RunCount - (currentBatch * arraySize));
  
  for (int i = 0; i < currentBatchSize; i++) {
    bool x = true;
    bool y = true;
    bool z = true;
    bool r = true;
    bool e = true;
    bool l1 = true;
    bool l2 = true;
    //bool l3 = true;
    tempInt = "";
    tempVal = "";
    tempChar = 'a';
    while (true) {
      tempChar = char(myFile.read());
      if (tempChar == '\n') {
        break;
      }
      else {
        tempVal += tempChar;
      }
    }
    for (int j = 0; j < tempVal.length(); j++) {
      tempChar = tempVal[j];
      if (tempChar == ',' && x) {
        tempIntVal = tempInt.toInt();
        xCords[i] = tempIntVal;
        tempInt = "";
        x = false;
      }
      else if (tempChar == ',' && y) {
        tempIntVal = tempInt.toInt();
        yCords[i] = tempIntVal;
        tempInt = "";
        y = false;
      }
      else if (tempChar == ',' && z) {
        tempIntVal = tempInt.toInt();
        zCords[i] = tempIntVal;
        tempInt = "";
        z = false;
      }
      else if (tempChar == ',' && r) {
        tempIntVal = tempInt.toInt();
        rCords[i] = tempIntVal;
        tempInt = "";
        r = false;
      }
      else if (tempChar == ',' && e) {
        tempIntVal = tempInt.toInt();
        eCords[i] = tempIntVal;
        tempInt = "";
        e = false;
      }
      else if (tempChar == ',' && l1) {
        tempIntVal = tempInt.toInt();
        l1Cords[i] = tempIntVal;
        tempInt = "";
        l1 = false;
      }
      else if (tempChar == ',' && l2) {
        tempIntVal = tempInt.toInt();
        l2Cords[i] = tempIntVal;
        tempInt = "";
        l2 = false;
      }
      /* else if (tempChar == ',' && l3) {
        tempIntVal = tempInt.toInt();
        l3Cords[i] = tempIntVal;
        tempInt = "";
        l3 = false;
      } */
      else {
        tempInt += tempChar;
      }
    }
  }

  myFile.close();
  currentBatch++;
}

//////
////// Begin
//////

// recvData and prcsData used to prevent the program from 
// blocking while waiting for additional instructions from
// the GUI. This enables smooth printing without having to be
// provided additional communication from the host GUI to know
// what the program should currently be doing.

// Can recieve data up to 32 characters
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

// Starts when called in Loop
// If no data is waiting at port proceeds without blocking
void recvData() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

//If new data was flagged by the recvData reads
//in the new data. Otherwise exits without blocking
void prcsData() {
    if (newData == true) {
        char tempChar = 'a';
        int tempIntVal;
        String tempInt = "";
        bool x = true;
        bool y = true;
        bool z = true;
        bool r = true;
        bool e = true;
        bool l1 = true;
        bool l2 = true;
        bool l3 = true;
        if (strlen(receivedChars) > 1) {
          for (int j = 0; j < strlen(receivedChars); j++) {
            tempChar = receivedChars[j];
            if (tempChar == ',' && x) {
              tempIntVal = tempInt.toInt();
              mnlMv[0] = tempIntVal;
              tempInt = "";
              x = false;
            }
            else if (tempChar == ',' && y) {
              tempIntVal = tempInt.toInt();
              mnlMv[1] = tempIntVal;
              tempInt = "";
              y = false;
            }
            else if (tempChar == ',' && z) {
              tempIntVal = tempInt.toInt();
              mnlMv[2] = tempIntVal;
              tempInt = "";
              z = false;
            }
            else if (tempChar == ',' && r) {
              tempIntVal = tempInt.toInt();
              mnlMv[3] = tempIntVal;
              tempInt = "";
              r = false;
            }
            else if (tempChar == ',' && e) {
              tempIntVal = tempInt.toInt();
              mnlMv[4] = tempIntVal;
              tempInt = "";
              e = false;
            }
            else if (tempChar == ',' && l1) {
              tempIntVal = tempInt.toInt();
              mnlMv[5] = tempIntVal;
              tempInt = "";
              l1 = false;
            }
            else if (tempChar == ',' && l2) {
              tempIntVal = tempInt.toInt();
              mnlMv[6] = tempIntVal;
              tempInt = "";
              l2 = false;
            }
            else if (tempChar == ',' && l3) {
              tempIntVal = tempInt.toInt();
              mnlMv[7] = tempIntVal;
              tempInt = "";
              l3 = false;
            }
            else {
              tempInt += tempChar;
            }
          }
        }
        else {
          tempInt = receivedChars[0];
          tempIntVal = tempInt.toInt();
          current_mode = tempIntVal;
        }
        newData = false;
    }
}

//////
////// End
//////
