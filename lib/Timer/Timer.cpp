#include "Timer.h"
#include <Arduino.h>

Timer::Timer(/* args */)
{
}

Timer::~Timer()
{
}

void Timer::init(int start_hour, int start_minute, int end_hour, int end_minute)
{

    this->start_hour = start_hour;
    this->start_minute = start_minute;
    this->end_hour = end_hour;
    this->end_minute = end_minute;
}

String Timer::getStart()
{
    char time[6];
    snprintf(time, sizeof(time), "%02d:%02d", this->start_hour, this->start_minute);
    return String(time);
}

String Timer::getEnd()
{
    char time[6];
    if (this->end_hour == 24)
    {
        this->end_hour = 0;
    }
    snprintf(time, sizeof(time), "%02d:%02d", this->end_hour, this->end_minute);
    return String(time);
}

int Timer::getStartHour()
{
    return this->start_hour;
}
int Timer::getStartMinute()
{
    return this->start_minute;
}
int Timer::getEndHour()
{
    return this->end_hour;
}
int Timer::getEndMinute()
{
    return this->end_minute;
}

void Timer::setStartHour(String value)
{
    this->start_hour = value.toInt();
}
void Timer::setStartMinute(String value)
{
    this->start_minute = value.toInt();
}
void Timer::setEndHour(String value)
{
    this->end_hour = value.toInt();
}
void Timer::setEndMinute(String value)
{
    this->end_minute = value.toInt();
}