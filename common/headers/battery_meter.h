#ifndef BATTERY_METER_H_
#define BATTERY_METER_H_

#include <cstddef>


class BatteryMeter {
public:
	virtual ~BatteryMeter() {}
	virtual std::size_t getBatteryState() const = 0;
};

#endif  // BATTERY_METER_H_
