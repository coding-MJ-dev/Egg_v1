#include <Arduino.h>
#include "EggSystem.hpp"
#include <Wire.h>



void setup() {
  eggsystem.eggSystemSetup();
}

void loop() {
  eggsystem.eggFlight();
}
