#pragma once

#include <boost/asio.hpp>
// #include <boost/bind/bind.hpp>

#include "abstract_algorithm.h"
#include "Utils.h"
#include "BatteryMeter.h"
#include "DirtSensor.h"
#include "WallsSensor.h"
#include <chrono>

#include <iostream> // error output
#include <fstream>	// file I/O ops
#include <sstream>	// file reading
#include <math.h>

#define SIM_CONFIG_NAME "Simulator.config" 
#define LOG_DIR_PATH "./logs/"
#define OUTPUT_DIR_PATH "./outputs/"

using std::endl;
using std::invalid_argument;
using std::logic_error;
using std::ofstream;
using namespace MyUtils;
using namespace std::string_literals;

enum class FaultCode
{
    FOUT_OF_STEPS,
    FBATTERY_EXHAUSTED,
	FROBOT_IN_WALL
};

enum class ErrOwnership
{
	House,
	Algorithm,
	Simulator
};


struct CustomError
{
	ErrOwnership owner;
	string content;
	size_t score;
	CustomError(ErrOwnership owner, string content, size_t score = 0): owner(owner), content(content), score(score) {}
};

class MySimulator
{
	string houseDescription;
	size_t rows;						   // House structure's dimensions
	size_t cols;						   // House structure's dimensions
	vector<vector<size_t>> houseStructure; // data structure for input house file
	vector<size_t> dockingLocation;		   // Docking station location in the house (y,x)
	vector<size_t> currLocation;		   // Robot current location in the house (y,x)
	size_t initDirt;						// Total amount of dirt in the house at the beginning
	size_t dirtLeft;			   			// Total amount of dirt left in the house 
	size_t maxSteps;					   // Max steps to accomplish the target
	double maxBattery;					   // The maximum battery. Always happening: battery <= max_battery
	double curBattery;					   // The current battey level
	AbstractAlgorithm *pAlgo;
	string houseName;
	string algoName;
	size_t algoScore;
	bool writeOutput;
	bool writeLog;

	MyBatteryMeter batteryMeter;
	MyDirtSensor dirtSensor;
	MyWallsSensor wallsSensor;

	ofstream logFile;
	LogCode currLog;
	LogCode prevLog;

	// Members for tracking after the algorithm
	size_t numSteps;
	vector<Step> steps;
	Status status;

	uint timeoutCoefficient;
    boost::asio::io_context io;
    boost::asio::steady_timer timer;
	bool timeoutOccoured;

private:
	void houseWallPadding();
	void writeOutputFile();
	void tryChargeRobot();
	void tryToClean();
	bool isValidLocation(vector<size_t> location) const;
	void updateTotalDirt();
	void initLogFile();
	void handleStep(Step step);
	void handleFault(const FaultCode e);
	void finalize();
	void updateHouseName(const char* inputName);
	bool inWall();
	void activateTimer();
	void handleErrors();

	// battery handling
	size_t getMaxBattery() const { return maxBattery; }
	void chargeBattery() // change battery with max_battery / 20.  if full: doesn't charge
	{
		curBattery = fmin(curBattery + maxBattery / 20, maxBattery);

		// round up to 5 digits after the point
		curBattery *= pow(10, 5);
		curBattery = ceil(curBattery);
		curBattery /= pow(10, 5); 
	}

	void reduceBattery()
	{
		if (curBattery <= 0)
			// can't reduce battery
			throw FaultCode::FBATTERY_EXHAUSTED;

		curBattery--;
	}

	// returns true iff
	// the robot's battery==0 and the robot isn't at the docking station.
	inline bool isBatteryExhausted() const { return batteryMeter.getBatteryState() == 0 && !robotAtDocking(); }

	// return true iff the house is clean and the robot at the docking station.
	inline bool isMissionSucceed() const { return dirtLeft == 0 && robotAtDocking(); }

	// returns true iff robot is at the docking station
	inline bool robotAtDocking() const { return (currLocation[0] == dockingLocation[0] && currLocation[1] == dockingLocation[1]); }

	//
	void updateLogFile(Step currStep);
	
	LogCode fetchLogCode(Step currStep);
	void calcScore();

public:
	// Constructor
	MySimulator(string algoName, bool writeOutput, bool writeLog);

	// Deconstructor
	~MySimulator();

	void readHouseFile(const char *filename);
	void setAlgorithm(AbstractAlgorithm &algo);
	void run();

	size_t getScore() const { return algoScore; };
};
