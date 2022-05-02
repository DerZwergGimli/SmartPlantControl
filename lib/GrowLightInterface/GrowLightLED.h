#ifndef GrowLightLED_h
#define GrowLightLED_h

#include <Arduino.h>
#include "GrowLightInterface.h"

class GrowLightLED : public GrowLightInterface
{
private:
public:
    GrowLightLED(int pin, bool state, Mode mode, int duty_cycle, int frequency) : GrowLightInterface(pin, state, mode, duty_cycle, frequency){};
    ~GrowLightLED();
};

#endif