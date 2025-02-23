#pragma once

#include "dirt_sensor.h"

#include <vector>

using std::vector;

class MyDirtSensor : public DirtSensor
{
    vector<vector<size_t>> &houseStructure;
    vector<size_t> &currentLocation;

public:
    MyDirtSensor(vector<vector<size_t>> &houseStructure, vector<size_t> &currentLocation) : houseStructure(houseStructure), currentLocation(currentLocation) {}

    virtual int dirtLevel() const override { return static_cast<int>(houseStructure[currentLocation[0]][currentLocation[1]]); }
};
