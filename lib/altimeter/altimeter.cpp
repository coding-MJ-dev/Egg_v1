#include <Adafruit_BMP280.h>
#include <Arduino.h>
#include <Wire.h>
#include <SimpleKalmanFilter.h>
#include "altimeter.hpp"
#include <SPI.h>




// Altimeter
Adafruit_BMP280 bmp; // I2C
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

//KalmanFilter
SimpleKalmanFilter pressureKalmanFilter(5, 5, 5);


void Altimeter::altimeterSetup() {
  Serial.println(F("BMP280 Test."));

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
  this->getGroundPressure();
}



float Altimeter::getGroundPressure() {
  groundPressure = (bmp.readPressure() / (100));
  return groundPressure;
}


float Altimeter::getPressure() {
  pressure = bmp.readPressure();
  return pressure;
}


float Altimeter::getRawAltitude() {
  rawAltitude = bmp.readAltitude(groundPressure);
  return rawAltitude;
}



float Altimeter::getFliteredAltitude() {
  altitude = pressureKalmanFilter.updateEstimate(rawAltitude);
  return altitude;
}

float Altimeter::getTemperature() {
  temperature = bmp.readTemperature();
  return temperature;
}

//velocity checker!

float Altimeter::getVelocity(){
  float currentAltitude = this->getFliteredAltitude();
  velocity = (currentAltitude - previousAltitude);
  previousAltitude = currentAltitude;
  return velocity;
}


Altimeter altimeter;