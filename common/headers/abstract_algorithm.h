#ifndef ROBOT_ALGORITHM_H__
#define ROBOT_ALGORITHM_H__

#include <cstddef>

#include "enums.h"
#include "battery_meter.h"
#include "wall_sensor.h"
#include "dirt_sensor.h"


class AbstractAlgorithm {
public:
	virtual ~AbstractAlgorithm() {}
	virtual void setMaxSteps(std::size_t maxSteps) = 0;
	virtual void setWallsSensor(const WallsSensor&) = 0;
	virtual void setDirtSensor(const DirtSensor&) = 0;
	virtual void setBatteryMeter(const BatteryMeter&) = 0;
	virtual Step nextStep() = 0;
};

#endif  // ROBOT_ALGORITHM_H__
