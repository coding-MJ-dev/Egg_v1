#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include <ArduinoJson.hpp>

class Peripheral {
  public:
    Peripheral() {}
    virtual ~Peripheral() {}
    virtual bool initialise() = 0;
    bool isInitialised() const {
      return this->initialised;
    }

  protected:
    bool initialised = false;
};

#endif
