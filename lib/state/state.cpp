#include "state.hpp"
#include "altimeter.hpp"
#include "EggSystem.hpp"


int State::stateMachine() {
  //idle = 0
  while (state == 0){
    // arming altitude
    if (altimeter.getFliteredAltitude() > ARM_ALTITUDE) {
      state = 1;
      // record what time you launch at
    }
    else {
      return state;
    }
  }
  //ascending = 1
  while (state == 1){
    if (altimeter.getVelocity() < APOGEE_VELOCITY) {
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
    if (altimeter.getFliteredAltitude() < ARM_ALTITUDE) {
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

int State::getState()
{
  return state;
}

State eggState;