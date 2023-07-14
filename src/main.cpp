#include <Arduino.h>
#include <Wire.h>
#include <MS5607.h>
//#include <iostream>
#include <SPI.h>
#include <SD.h>


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
#define SD_CARD_CS 9

// CHECK before Launch !!!
#define FLINTERING_SIZE 10
#define APOGEE_DURATION 4000

File myFile;

Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

void altimeter();
float* save_raw_altitude();
float altitude_filter();
float* save_filtered_altitude();
float velocity_checker();
int stateMachine();


int i = 0;
int altitude_index = i % FLINTERING_SIZE; 
int state = 0;
float altitude_array[FLINTERING_SIZE];
float filtered_altitude_array[FLINTERING_SIZE];
float velocity;
float groundPressure;
float altitude;
float filter_altitude;

void setup() {
  altimeter();
}

float previousPressure = 0.0;
float currentPressure = 0.0;



void loop() {
  // must call this to wake sensor up and get new measurement data
  // it blocks until measurement is complete
  if (bmp.takeForcedMeasurement()) {
    // can now print out the new measurements

    Serial.print(F("state = "));
    Serial.print(state);

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
    Serial.print(altitude); /* Adjusted to local forecast! */
    Serial.println(" m");

    Serial.print(F("/ filtered altitude = "));
    Serial.print(filter_altitude); /* Adjusted to local forecast! */
    Serial.println(" m");


    i++;
    save_raw_altitude();
    altitude_filter();
    save_filtered_altitude();
    velocity_checker();
    stateMachine();
    Serial.println();

    delay(100);

  } else {
    Serial.println("Forced measurement failed!");
  }


}

void altimeter() {
  Serial.begin(115200);
  Serial.println(F("BMP280 Forced Mode Test."));

  //if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  groundPressure = (bmp.readPressure() / (100));
  // altitude = bmp.readAltitude(groundPressure);

}

float* save_raw_altitude() {
  altitude = bmp.readAltitude(groundPressure);
  altitude_array[altitude_index] = altitude;
  return altitude_array;
}

float altitude_filter() {
  float sum_altitude = 0;
  for (int n = 0; n < FLINTERING_SIZE ; n++) {
    sum_altitude += altitude_array[n];
  }
  filter_altitude = sum_altitude / FLINTERING_SIZE;
  return filter_altitude;
}

float* save_filtered_altitude() {
  filtered_altitude_array[altitude_index] = altitude_filter();
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
    float current_altitude = altitude_filter();
    if (current_altitude > 30.0) {
      state++;
    }
    else {
      return state;
    }
  }
  //ascending = 1
  while (state == 1){
    if (velocity_checker() < 5.0) {
      state++;
    }
    else {
      return state;
    }    
  }
  //apogee = 2
  while (state == 2){
    // drogue, or main parachuate ejection
    size_t oldTime = 0;
    size_t newTime = millis();
    if (newTime > oldTime + APOGEE_DURATION) {
      state++;
    }
    else {
      return state;
    }
  }

  //descending = 3
  while (state == 3){
    if (altitude_filter() < 30) {
      state++;
    }
    else {
      return state;
    }    
  }
  //landed = 4
  if (state == 4){
    //rocket is landed. Please save the data
    return state;
  }
}

void SD_Setup(){
  SD.begin(SD_CARD_CS);
    Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CARD_CS)) {
    Serial.println("SD initialization failed!");
    while (1);
  }
  Serial.println("SD card initialised.");

// open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  // if the file opened okay, write to it:

  if (myFile) {
  Serial.print("Writing to test.txt...");
  myFile.println("testing 1, 2, 3.");

  // close the file:
  myFile.close();
  Serial.println("done.");

  } else {
  // if the file didn't open, print an error:
  Serial.println("error opening test.txt");
  }


  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");
   // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}



/*
// put function declarations here:
float altitude;
Chrono altimeterTimer;

MS5607 altimeter(&altitude);


void setup() {
  Serial.begin(115200);//initialize serial communication
  Serial.println("Hello from the setup");
  Serial.print(F("\tSDA = ")); Serial.println(SDA);
  Serial.print(F("\tSCL = ")); Serial.println(SCL);
  altimeter.begin();
}


void loop() {

  if (altimeter.handleAltimeter() == 1) {
    Serial.println(altitude);
  }

}

*/