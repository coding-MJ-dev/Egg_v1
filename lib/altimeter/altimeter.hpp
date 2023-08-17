#ifndef ALTIMETER_H
#define ALTIMETER_H


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
#include <Arduino.h>
#include <SimpleKalmanFilter.h>


//extern 

class Altimeter {
 public:
   Altimeter() {};
   ~Altimeter(){};

   void altimeterSetup();

   float getGroundPressure();
   float getPressure();

   float getRawAltitude();
   float getFliteredAltitude();
   float getTemperature();
   float getVelocity();

 private:
   float groundPressure;
   float pressure;
   float rawAltitude;
   float altitude;
   float velocity = 0.0;
   float temperature = 0.0;
   float previousAltitude = 0.0f;
};

extern Altimeter altimeter;


#endif
