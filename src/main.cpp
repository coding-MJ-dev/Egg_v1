#include <Arduino.h>
#include "altimeter.hpp"
#include "state.hpp"
#include "EggSystem.hpp"
#include "eggSD.hpp"
#include <Wire.h>



/*
 SimpleKalmanFilter(e_mea, e_est, q);
 e_mea: Measurement Uncertainty 
 e_est: Estimation Uncertainty 
 q: Process Noise
 */
// SimpleKalmanFilter pressureKalmanFilter(2, 2, 0.01);


void setup() {
  eggsystem.eggSystemSetup();
}

void loop() {
  eggsystem.eggFlight();
}
