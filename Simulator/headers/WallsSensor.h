#pragma once

#include "wall_sensor.h"
#include "Utils.h"

using MyUtils::calcNewLocation;

class MyWallsSensor : public WallsSensor
{
    vector<vector<size_t>> &houseStructure;
    vector<size_t> &currentLocation;

public:
    MyWallsSensor(vector<vector<size_t>> &houseStructure, vector<size_t> &currentLocation) : houseStructure(houseStructure), currentLocation(currentLocation) {}

    virtual bool isWall(Direction d) const override
    {
        vector<size_t> newLocation = calcNewLocation(d, currentLocation);

        return houseStructure[newLocation[0]][newLocation[1]] == WALL_CODE;
    }
};
