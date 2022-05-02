#include "GrowLightFAN.h"

GrowLightFAN::~GrowLightFAN()
{
}

// Getter
String GrowLightFAN::getTemperatureMax()
{
    return String(this->temperature_max);
}
String GrowLightFAN::getTemperatureIs()
{
    return String(this->temperature_is);
}
// Setter
void GrowLightFAN::setTemperatureMax(String value)
{
    Serial.print(value);
    this->temperature_max = value.toInt();
}
void GrowLightFAN::setTemperatureIs(String value)
{
    this->temperature_is = value.toInt();
}
