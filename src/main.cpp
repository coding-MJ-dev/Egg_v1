#include <Arduino.h>
#include <Wire.h>
#include <MS5607.h>
#include <iostream>

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

Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

void altimeter();
float* save_raw_altitude();
float filtered_altitude();
float* save_filtered_altitude();
float velocity_checker();
int stateMachine();

/// @brief 
int i = 0;
int altitude_index = 0; 
int state = 0;
float altitude_array[10];
float avarage_altitude_array[10];
float velocity = 0.0;
float groundPressure;
float altitude;
float avarage_altitude;

void setup() {
  altimeter();
}

float previousPressure = 0.0;
float currentPressure = 0.0;

// float comparisonPressure = {previousPressure, currentPressure};

void loop() {
  // must call this to wake sensor up and get new measurement data
  // it blocks until measurement is complete
  if (bmp.takeForcedMeasurement()) {
    // can now print out the new measurements

    Serial.print(F("state = "));
    Serial.println(state);

    Serial.print(F("Micro = "));
    Serial.print(millis());
    Serial.println(" mSec");

    Serial.print(F("Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

    Serial.print(F("Pressure = "));
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print(F("Approx altitude = "));
    
    altitude = bmp.readAltitude(groundPressure);
    Serial.print(altitude); /* Adjusted to local forecast! */
    Serial.println(" m");


    i++;
    save_raw_altitude();
    filtered_altitude();
    Serial.println(avarage_altitude);


    save_filtered_altitude();
    velocity_checker();
    stateMachine();
    Serial.println();

    delay(2000);

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
  altitude_index = i % 10;
  altitude_array[altitude_index] = Altitude;
  return altitude_array;
}

float filtered_altitude() {
  float avarage_altitude = 0;

  for (int n = 0; n < 10 ; n++)
  {
    avarage_altitude += altitude_array[n];
  }
  avarage_altitude = avarage_altitude / 10;
  return avarage_altitude;
}

float* save_filtered_altitude() {
  altitude_index = altitude_index % 10;
  avarage_altitude_array[altitude_index] = filtered_altitude();
  return avarage_altitude_array;
}

float velocity_checker() {
  altitude_index = altitude_index % 10;
  velocity = ((avarage_altitude_array[altitude_index] - avarage_altitude_array[(altitude_index + 1) % 10]) * -1);
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
    float current_altitude = filtered_altitude();
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
    delay(4000);
    state++;
  }    
  //descending = 3
  while (state == 3){
    if (filtered_altitude() < 30) {
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