#include "GrowLightInterface.h"

GrowLightInterface::GrowLightInterface(int pin, bool state, Mode mode, int duty_cycle, int frequency)
{
    this->duty_cycle = duty_cycle;
    this->state = state;
    this->mode = mode;
    this->duty_cycle = duty_cycle;
    this->frequency = frequency;
}

GrowLightInterface::~GrowLightInterface()
{
}

String GrowLightInterface::getState()
{
    return String(this->state);
}
String GrowLightInterface::getMode()
{
    switch (this->mode)
    {
    case AUTO:
        return String("auto");
    case MANUAL:
        return String("manual");
    default:
        return String("NONE");
    }
}
String GrowLightInterface::getDutyCycle()
{
    return String(this->duty_cycle);
}

String GrowLightInterface::getFrequency()
{
    return String(this->frequency);
}

void GrowLightInterface::setPin(String pin)
{
    this->pin = pin.toInt();
}

void GrowLightInterface::setState(String state)
{
    if (state == "0")
    {
        this->state = false;
    }
    else if (state == "1")
    {
        this->state = true;
    }
}
void GrowLightInterface::setMode(String mode)
{
    if (mode == "manual")
    {
        this->mode = MANUAL;
    }
    else if (mode == "auto")
    {
        this->mode = AUTO;
    }
}
void GrowLightInterface::setDutyCycle(String duty_cycle)
{
    this->duty_cycle = duty_cycle.toInt();
    if (this->duty_cycle > 100)
    {
        this->duty_cycle = 100;
    }
    else if (this->duty_cycle < 0)
    {
        this->duty_cycle = 0;
    }
}

void GrowLightInterface::setFerquency(String frequency)
{
    this->frequency = frequency.toInt();
}