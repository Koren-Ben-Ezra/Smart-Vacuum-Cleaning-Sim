#pragma once

#include "enums.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

using std::pair;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

namespace fs = std::filesystem;
using namespace std::string_literals;

#define CONFIG_PATH "../Utils.config"

#define MAX_DIRT 9
#define DOCKING_SGN 'D'
#define WALL_SGN 'W'
#define ROBOT_SGN 'X'
#define CLEAN_CODE 0
#define DOCKING_CODE 10
#define WALL_CODE 11
#define UNDISCOVERED_CODE 12

enum class LogCode
{
    Finished,
    Cleaning,
    Charging,
    Exploring,
    BatteryExhuasted,
    OutOfSteps,
    Stuck,
    NoLog
};

enum class Status
{
    Working,
    Finished,
    Dead
};

namespace MyUtils
{
    const string directionLabels[4] = {"north", "east", "south", "west"};
    const string stepFullLabels[6] = {"north", "east", "south", "west", "stay", "finish"};
    const string stepLabels[6] = {"N", "E", "S", "W", "s", "F"};
    const string boolLabels[2] = {"true", "false"};
    
    Step strToStep(const string &);

    char charToSize_t(const char);
    vector<size_t> calcNewLocation(Direction dir, vector<size_t> curLocation);
    vector<size_t> calcNewLocation(Step step, vector<size_t> curLocation);

    pair<int, int> calcNewPosition(Direction dir, pair<int, int> curPosition);
    pair<int, int> calcNewPosition(Step step, pair<int, int> curPosition);

    char size_tToChar(size_t code) noexcept(false);
    Step calcStep(pair<int, int> from, pair<int, int> to);

    template <typename T>
    void loadConfig(const char* configName, vector<pair<string, T*>> &pairs);

    inline string stepToStr(Step step)
    {
        return stepLabels[static_cast<int>(step)];
    }

    inline string stepToFullStr(const Step step)
    {
        return stepFullLabels[static_cast<size_t>(step)];
    }

    inline string directionToStr(Direction dir)
    {
        return stepLabels[static_cast<int>(dir)];
    }
    inline Step directionToStep(Direction dir)
    {
        return static_cast<Step>(dir);
    }

    // converts bool to string by the format:
    // true -> "true"
    // false -> "false"
    inline string boolToStr(bool b)
    {
        return boolLabels[static_cast<int>(b)];
    }
};
