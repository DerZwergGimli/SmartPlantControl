#ifndef Timer_h
#define Timer_h

#include <Arduino.h>

class Timer
{
private:
    int start_hour;
    int start_minute;
    int end_hour;
    int end_minute;

public:
    Timer(/* args */);
    ~Timer();
    void init(int start_hour, int start_minute, int end_hour, int end_minute);
    String getStart();
    String getEnd();

    int getStartHour();
    int getStartMinute();
    int getEndHour();
    int getEndMinute();

    void setStartHour(String value);
    void setStartMinute(String value);
    void setEndHour(String value);
    void setEndMinute(String value);
};

#endif