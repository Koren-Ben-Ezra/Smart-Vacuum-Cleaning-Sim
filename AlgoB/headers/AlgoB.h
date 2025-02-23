#pragma once

#include "AlgorithmRegistration.h"
#include "abstract_algorithm.h"
#include "battery_meter.h"
#include "dirt_sensor.h"
#include "wall_sensor.h"
#include "Utils.h"
#include <filesystem>

#include <array>
#include <unordered_map>
#include <stack>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <random>

#include <iostream>

using std::array;
using std::queue;
using std::stack;
using std::unordered_map;
using std::unordered_set;

#define CONFIG_NAME "AlgoB.config"

class AlgoB : public AbstractAlgorithm
{

    struct Point
    {
        const pair<int, int> coordinate; // = {y, x}
        size_t dirtLevel;
        bool reachable;
        bool isWall;

        Point(pair<int, int> coor) : coordinate(coor), dirtLevel(UNDISCOVERED_CODE), reachable(true), isWall(false) {}
    };

    // Hash function for std::pair<int, int>
    struct PairHash
    {
        template <typename T1, typename T2>
        size_t operator()(const pair<T1, T2> &pair) const
        {
            // Combine hashes of first and second using a simple hash function
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }
    };

    // Equality function for std::pair<int, int>
    struct PairEqual
    {
        template <typename T1, typename T2>
        bool operator()(const std::pair<T1, T2> &lhs, const std::pair<T1, T2> &rhs) const
        {
            return lhs.first == rhs.first && lhs.second == rhs.second;
        }
    };

    size_t totalSteps; // Total number of steps that the robot has done in the house
    bool isExploring;  // robot can keep moving forward (or does he need to go back and charge)
    Step lastStep;     // The robot's last step in the path
    const BatteryMeter *batteryMeter;
    const DirtSensor *dirtSensor;
    const WallsSensor *wallsSensor;
    size_t maxSteps;
    double maxBattery;
    vector<Direction> pathToDock; // caching for shortest path to dock
    bool returningToDock;
    pair<int, int> position; // current location relative to docking station that is in coordinate (0, 0)
    unordered_map<pair<int, int>, unique_ptr<Point>, PairHash> houseMapping;

    // members for dfs using
    stack<pair<int, int>> dfsStack;                                // Stack for performing DFS
    unordered_set<pair<int, int>, PairHash, PairEqual> visited; // Set to hold the visited point in the house mapping for DFS
    size_t dfsNumOfActualVisited;                                  // Since we inserting to visited position that are unreachable as well, then need to know how many position we actually visited
    bool dfsBacktracking;                                          // flag to know if we backtracking in the DFS run
    vector<Direction> dfsBackPath;                                 // holds the backtracking path
    bool dfsMovingNewPos;                                          // flag to know if we are on the way to a new position after the stack emptied
    vector<Direction> dfsNewPosPath;                               // holds the path to new position
    Direction nextSpiralDir;
    std::random_device rd;                                          // seed
    std::mt19937 gen;                                               // random engine

    bool spiralClockwise;

private:
    void updateHouseMapping();                                                        // adding mapping of surrounding points of position
    void insertCoordinate(pair<int, int> pos);                                        // inserting a specific coordiante to the house mapping
    bool isStuck();                                                                   // checking if there is a direction that is not a wall
    void updatePathToDock();                                                          // searching the shortest path from current location to the docking station
    Step returnDocking();                                                             // returning a step on the way to the docking station
    Step exploreStep();                                                               // doing a DFS step
    unordered_map<pair<int, int>, Direction, PairHash, PairEqual> fetchSurrounding(); // returning a set of surrounding coordinates and the step to take to get to that coordinate
    unordered_map<pair<int, int>, Direction, PairHash, PairEqual> fetchSurrounding(pair<int, int> pos);
    void updateBFSPath(pair<int, int> src, pair<int, int> dst, vector<Direction> &path); // updating path to hold a path from src to dst using BFS
    void updateReachable();                                                              // search for reachable positions and add it to the reachable set
    bool isFinish();
    size_t numOfReachable();

    bool isDirty() const { return (dirtSensor->dirtLevel() > 0 && dirtSensor->dirtLevel() <= MAX_DIRT); }
    bool atDocking() const { return position == pair{0, 0}; } // returns true iff robot at the docking station
    pair<int, int> computeOptimalNextPos();
    size_t computeDistance(pair<int,int> dst);
    size_t decideUniformIndex(size_t range);
    Step popNewPathStep();
    Step fetchSpiralDir();
    void loadConfigs();
    
public:
    AlgoB();
    virtual Step nextStep() override;
    virtual void setMaxSteps(size_t maxSteps) override { this->maxSteps = maxSteps; }
    virtual void setWallsSensor(const WallsSensor &wallsSensor) override { this->wallsSensor = &wallsSensor; }
    virtual void setDirtSensor(const DirtSensor &dirtSensor) override { this->dirtSensor = &dirtSensor; }
    virtual void setBatteryMeter(const BatteryMeter &batteryMeter) override
    {
        this->batteryMeter = &batteryMeter;
        maxBattery = this->batteryMeter->getBatteryState();
    }
};