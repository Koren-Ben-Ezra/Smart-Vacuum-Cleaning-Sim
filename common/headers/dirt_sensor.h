#ifndef DIRT_SENSOR_H_
#define DIRT_SENSOR_H_

class DirtSensor {
public:
	virtual ~DirtSensor() {}
	virtual int dirtLevel() const = 0;
};

#endif  // DIRT_SENSOR_H_
