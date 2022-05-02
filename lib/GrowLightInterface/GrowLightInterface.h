#ifndef GrowLightInterface_h
#define GrowLightInterface_h

#include <Arduino.h>

enum Mode
{
    AUTO,
    MANUAL
};

class GrowLightInterface
{
public:
    uint8_t pin;
    bool state;
    Mode mode;
    int duty_cycle;
    int frequency;

public:
    explicit GrowLightInterface(int pin, bool state, Mode mode, int duty_cycle, int frequency);
    ~GrowLightInterface();
    // Getter
    String getState();
    String getMode();
    String getDutyCycle();
    String getFrequency();

    // Setter
    void setPin(String pin);
    void setState(String state);
    void setMode(String mode);
    void setDutyCycle(String duty_cycle);
    void setFerquency(String frequency);
};

#endif