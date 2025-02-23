#pragma once

#include "battery_meter.h"
#include <cmath> // for fmin

class MyBatteryMeter : public BatteryMeter
{
    double *battery; // Current battery level

public:
    virtual size_t getBatteryState() const override
    {
        return static_cast<size_t>(*battery);
    }

    // Constructor
    MyBatteryMeter(double &battery) : battery(&battery) {}
};