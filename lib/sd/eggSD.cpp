#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
// #include <string>
#include <Adafruit_BMP280.h>
#include "altimeter.hpp"
#include "state.hpp"



#define SD_CARD_CS 10
char filename[50] = {0};
File myFile;

void EggSD::setupSD() {
  SD.begin(SD_CARD_CS);
    Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CARD_CS)) {
    Serial.println("SD initialization failed!");
    while (1);
  }
  else{
    Serial.println("SD card initialised.");
    myFile = SD.open("test.txt", FILE_WRITE);

    if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("millis,state,temp,pressure,raw_altitude,altitude,velocity");
    myFile.close();
    
    } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
    }
  }
}

void EggSD::writeSD() {
  Serial.begin(115200);
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) // it opened OK
    {
    myFile.print(millis());
    myFile.print(",");

    myFile.print(eggState.stateMachine());
    myFile.print(",");

    myFile.print(altimeter.getTemperature());
    myFile.print(",");

    myFile.print(altimeter.getPressure());
    myFile.print(",");

    myFile.print(altimeter.getRawAltitude()); 
    myFile.print(",");

    myFile.print(altimeter.getFliteredAltitude());
    myFile.print(",");

    myFile.print(altimeter.getVelocity());
    myFile.print("\n");

    myFile.close();
    }
  else 
    Serial.println("Error opening simple.txt");
}

EggSD eggSD;