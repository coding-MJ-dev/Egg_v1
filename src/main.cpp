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

// SD card file name
char filename[16] = {0};
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
void SD_Setup();
void writeToFile();

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
  altimeter();
  SD_Setup();
}


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
    Serial.print(raw_altitude); /* Adjusted to local forecast! */
    Serial.println(" m");

    Serial.print(F("/ filtered altitude = "));
    Serial.print(altitude); /* Adjusted to local forecast! */
    Serial.println(" m");

    altitude_index = i % FLINTERING_SIZE; 
    i++;
    save_raw_altitude();
    altitude_filter();
    save_filtered_altitude();
    velocity_checker();
    state = stateMachine();
    Serial.println();
    writeToFile();

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
    if (altitude > 30.0) {
      state++;
    }
    else {
      return state;
    }
  }
  //ascending = 1
  while (state == 1){
    if (velocity < 5.0) {
      state++;
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
      state++;
    }
    else {
      return state;
    }
  }

  //descending = 3
  while (state == 3){
    if (altitude < 30.0) {
      state++;
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
}



// SDcard setting

void SD_Setup(){
  Serial.begin(115200);
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
  // myFile.close();
  // Serial.println("done.");

  } else {
  // if the file didn't open, print an error:
  Serial.println("error opening test.txt");
  }


  // re-open the file for reading:
  // myFile = SD.open("test.txt");
  // if (myFile) {
  //   Serial.println("test.txt:");
   // read from the file until there's nothing else in it:
    // while (myFile.available()) {
    //   Serial.write(myFile.read());
    // }
    // close the file:
    // myFile.close();
  // } else {
  //   // if the file didn't open, print an error:
  //   Serial.println("error opening test.txt");
  // }
}


void writeToFile() {
  Serial.begin(115200);
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) // it opened OK
    {
    myFile.print(F("state = "));
    myFile.print(state);

    myFile.print(F("  / Micro = "));
    myFile.print(millis());
    myFile.print(" mSec    ");

    myFile.print(F("/ Temperature = "));
    myFile.print(bmp.readTemperature());
    myFile.print(" *C    ");

    myFile.print(F("/Pressure = "));
    myFile.print(bmp.readPressure());
    myFile.print(" Pa    ");

    myFile.print(F("/ raw altitude = "));
    myFile.print(raw_altitude); /* Adjusted to local forecast! */
    myFile.println(" m");

    myFile.print(F("/ filtered altitude = "));
    myFile.print(altitude); /* Adjusted to local forecast! */
    myFile.println(" m");

    myFile.close(); 
    }
  else 
    Serial.println("Error opening simple.txt");
}

// std::string GetFilenameByCurrentDate(const char* extension=".txt") {
//     // Get the current time
//     auto now = std::chrono::system_clock::now();
//     // Convert to local time
//     auto localTime = std::chrono::system_clock::to_time_t(now);
//     // Format the timestamp as a string
//     std::stringstream ss;
//     ss << std::put_time(std::localtime(&localTime), "%F_%H-%M-%S") << extension;
//     return ss.str();
// }