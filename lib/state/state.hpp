#ifndef STATE_H
#define STATE_H

#include "altimeter.hpp"
#include "EggSystem.hpp"
#include "eggSD.hpp"

// CHECK before Launch !!!
#define FLINTERING_SIZE 10
#define APOGEE_DURATION 4000
#define ARM_ALTITUDE 30
#define APOGEE_VELOCITY 0.5



class State {
 public:
   State(){};
   ~State(){};
   
   int stateMachine();
   int getState();

 private:
   size_t enterApogee; // millis at change to apogee state
   int state = 0;
};

extern State eggState;

#endif
