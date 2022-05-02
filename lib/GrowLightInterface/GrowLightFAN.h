#ifndef GrowLightFAN_h
#define GrowLightFAN_h

#include <Arduino.h>
#include "GrowLightInterface.h"

class GrowLightFAN : public GrowLightInterface
{
public:
    int temperature_max;
    int temperature_is;

public:
    GrowLightFAN(int pin, bool state, Mode mode, int duty_cycle, int frequency, int temperature_max) : GrowLightInterface(pin, state, mode, duty_cycle, frequency)
    {
        this->temperature_max = temperature_max;
    };
    ~GrowLightFAN();

    // Getter
    String getTemperatureMax();
    String getTemperatureIs();
    // Setter
    void setTemperatureMax(String value);
    void setTemperatureIs(String value);
};

#endif