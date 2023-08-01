#include <Arduino.h>
#include <Wire.h>
#include <MS5607.h>
//#include <iostream>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>



/***************************************************************************
  This is a library for the BMP280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BMP280 Breakout
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Adafruit_BMP280.h>

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)
#define SD_CARD_CS 10

// CHECK before Launch !!!
#define FLINTERING_SIZE 10
#define APOGEE_DURATION 4000
#define ARM_ALTITUDE 30
#define APOGEE_VELOCITY 0.5

// SD card file name
char filename[50] = {0};
File myFile;
// RTC_DS3231 rtc;

Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

void altimeter();
float* save_raw_altitude();
float altitude_filter();
float* save_filtered_altitude();
float velocity_checker();
int stateMachine();
void SD_Setup();
void writeToFile();
String setFileName();

float groundPressure;

int i = 0;
int altitude_index; 
int state = 0;
float altitude_array[FLINTERING_SIZE];
float filtered_altitude_array[FLINTERING_SIZE];
float velocity;

float raw_altitude;
float altitude;


size_t enterApogee; // millis at change to apogee state

// float temperature = bmp.readTemperature();
// float pressure = bmp.readPressure();

void setup() {
  Serial.begin(115200);
  altimeter();
  SD_Setup();
}


void loop() {
  // must call this to wake sensor up and get new measurement data
  // it blocks until measurement is complete
  //if (bmp.takeForcedMeasurement()) {
    // can now print out the new measurements


    // Consider putting EVERYTHING in the state machine

    Serial.print(F("state ="));
    Serial.print(state);
    Serial.print(F(","));

    Serial.print(F("  / Micro = "));
    Serial.print(millis());
    Serial.print(" mSec    ");

    Serial.print(F("/ Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.print(" *C    ");

    Serial.print(F("/Pressure = "));
    Serial.print(bmp.readPressure());
    Serial.print(" Pa    ");

    Serial.print(F("/ raw altitude = "));
    Serial.print(raw_altitude);
    Serial.print(" m");

    Serial.print(F("/ altitude = "));
    Serial.print(altitude); 
    Serial.print(" m");

    Serial.print(F("/ virtical velocity = "));
    Serial.print(velocity); 
    Serial.print(" m/s");
    
    Serial.println();
    Serial.println();

    altitude_index = i % FLINTERING_SIZE; 
    i++;
    save_raw_altitude();
    altitude_filter();
    save_filtered_altitude();
    velocity_checker();
    state = stateMachine();
  
    writeToFile();

    delay(100);

  // } else {
  //   Serial.println("Forced measurement failed!");
  // }


}

void altimeter() {
  Serial.println(F("BMP280 Test."));

  //if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_250); /* Standby time. */

  // can take multiple readings and get the average
  groundPressure = (bmp.readPressure() / (100));
  // setFileName();
}

float* save_raw_altitude() {
  raw_altitude = bmp.readAltitude(groundPressure);
  altitude_array[altitude_index] = raw_altitude;
  return altitude_array;
}

float altitude_filter() {
  float sum_altitude = 0;
  for (int n = 0; n < FLINTERING_SIZE ; n++) {
    sum_altitude += altitude_array[n];
  }
  altitude = sum_altitude / FLINTERING_SIZE;
  return altitude;
}

float* save_filtered_altitude() {
  filtered_altitude_array[altitude_index] = altitude;
  return filtered_altitude_array;
}

float velocity_checker() {
  velocity = ((filtered_altitude_array[altitude_index] - filtered_altitude_array[(altitude_index - 1 + FLINTERING_SIZE) % 10]));
  return velocity;
}

/*
0 = idle = altitude less than 30 m
1 = ascending = velocity > 5 m/s
2 = apogee = 5 sec
3 = descending = velocity < -5 m/s
4 = landed = altitude less than 30 m

int state = 0;
*/

int stateMachine() {

  //idle = 0
  while (state == 0){
    // arming altitude
    if (altitude > ARM_ALTITUDE) {
      state = 1;
      // record what time you launch at
    }
    else {
      return state;
    }
  }
  //ascending = 1
  while (state == 1){
    if (velocity < APOGEE_VELOCITY) {
      //consider changing state++ to state = .....
      state = 2;
      //subtract launch time from millis to get actual apogee time
      enterApogee = millis();
    }
    else {
      return state;
    }    
  }
  
  //apogee = 2
  while (state == 2){
    // drogue, or main parachuate ejection
    size_t maintainApogee = millis();
    if (maintainApogee > enterApogee + APOGEE_DURATION) {
      state = 3;
    }
    else {
      return state;
    }
  }

  //descending = 3
  while (state == 3){
    if (altitude < ARM_ALTITUDE) {
      state = 4;
    }
    else {
      return state;
    }    
  }
  //landed = 4
  if (state == 4){
    //rocket is landed
    return state;
  }
  return state;
}



// SDcard setting

void SD_Setup(){
  SD.begin(SD_CARD_CS);
    Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CARD_CS)) {
    Serial.println("SD initialization failed!");
    while (1);
    

  }
  else{
    Serial.println("SD card initialised.");
    // Serial.println(filename);

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    myFile = SD.open("test.txt", FILE_WRITE);
  // if the file opened okay, write to it:

    if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("millis,state,temp,pressure,raw_altitude,altitude,velocity");

    // close the file:
    myFile.close();
    // Serial.println("done.");

    } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
    }
  }

}


void writeToFile() {
  Serial.begin(115200);
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) // it opened OK
    {
    myFile.print(millis());
    myFile.print(",");

    myFile.print(state);
    myFile.print(",");

    myFile.print(bmp.readTemperature());
    myFile.print(",");

    myFile.print(bmp.readPressure());
    myFile.print(",");

    myFile.print(raw_altitude); /* Adjusted to local forecast! */
    myFile.print(",");

    myFile.print(altitude); /* Adjusted to local forecast! */
    myFile.print(",");

    myFile.print(velocity); /* Adjusted to local forecast! */
    myFile.print("\n");

    myFile.close(); 
    }
  else 
    Serial.println("Error opening simple.txt");
}


// String setFileName() {
//   // Set the time
//   DateTime now = rtc.now();
//   int year = now.year();
//   int month = now.month();
//   int day = now.day();
//   int second = now.second();
//   int minute = now.minute();
//   int hour = now.hour();

//   snprintf(filename, 50, "%d-%02d-%02d-%02d-%02d-%02d.csv", year, month, day, hour, minute, second);

//   return filename;
// }
    