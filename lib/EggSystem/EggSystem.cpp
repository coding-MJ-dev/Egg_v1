#include "EggSystem.hpp"
#include "altimeter.hpp"
#include "state.hpp"
#include "eggSD.hpp"
#include <Arduino.h>


void EggSystem::eggSystemSetup() {
  Serial.begin(115200);
  altimeter.altimeterSetup();
  eggSD.setupSD();
}

void EggSystem::eggFlight() {
  

  Serial.print(("Micro:"));
  Serial.print(millis());
  Serial.print(",");

  Serial.print(("state:"));
  Serial.print(eggState.stateMachine());
  Serial.print((","));

  Serial.print(("Temperature:"));
  Serial.print(altimeter.getTemperature());
  Serial.print(",");

  Serial.print(("Pressure:"));
  Serial.print(altimeter.getPressure());
  Serial.print(",");

  Serial.print(("raw_altitude:"));
  Serial.print(altimeter.getRawAltitude());
  Serial.print(",");

  Serial.print(("altitude:"));
  Serial.print(altimeter.getFliteredAltitude()); 
  Serial.print(",");

  Serial.print(("velocity:"));
  Serial.println(altimeter.getVelocity());
  Serial.println();

  eggSD.writeSD();
  delay(100);
}

EggSystem eggsystem;