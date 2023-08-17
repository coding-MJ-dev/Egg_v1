#ifndef EGGSD_H
#define EGGSD_H
#include <Arduino.h>





class EggSD {
 public:
  void setupSD();
  void writeSD();

private:
  

};

extern EggSD eggSD;

#endif