#include "Utils.h"


Step MyUtils::strToStep(const string &str)
{
    for(size_t i = 0; i < 6; i++)
    {
        if(str == stepLabels[i])
            return static_cast<Step>(i);
    }

    return Step::Finish;
}

char MyUtils::charToSize_t(const char c)
{
    if (c == ' ')
        return 0;

    if (c >= '0' && c <= '9')
        return static_cast<size_t>(c - '0');

    if (c == DOCKING_SGN)
        return DOCKING_CODE;

    if (c == WALL_SGN)
        return WALL_CODE;

    else
        // Invalid char is considered as clean space
        return 0;
}

// pre: location + step is a valid location in house structure
vector<size_t> MyUtils::calcNewLocation(Direction dir, vector<size_t> curLocation)
{
    // location = (rows, cols) = (y, x)
    switch (dir)
    {
    case Direction::North:
        curLocation[0]--;
        break;
    case Direction::East:
        curLocation[1]++;
        break;
    case Direction::South:
        curLocation[0]++;
        break;
    case Direction::West:
        curLocation[1]--;
        break;
    }
    return curLocation;
}

vector<size_t> MyUtils::calcNewLocation(Step step, vector<size_t> curLocation)
{
    if(step == Step::Stay || step == Step::Finish)
        return curLocation;
    
    return calcNewLocation(static_cast<Direction>(step), curLocation);
}

pair<int,int> MyUtils::calcNewPosition(Direction dir, pair<int,int> curPosition)
{
    // poistion = (y, x)
    switch (dir)
    {
    case Direction::North:
        curPosition.first--;
        break;
    case Direction::East:
        curPosition.second++;
        break;
    case Direction::South:
        curPosition.first++;
        break;
    case Direction::West:
        curPosition.second--;
        break;
    }
    return curPosition;
}

pair<int,int> MyUtils::calcNewPosition(Step step, pair<int,int> curPosition)
{
    if(step == Step::Stay || step == Step::Finish)
        return curPosition;
    
    return calcNewPosition(static_cast<Direction>(step), curPosition);
}


char MyUtils::size_tToChar(size_t code) noexcept(false)
{
    if (code == 0)
        return ' ';
    if (code <= MAX_DIRT)
        return static_cast<char>(code + '0');
    if (code == WALL_CODE)
        return WALL_SGN;
    if (code == DOCKING_CODE)
        return DOCKING_SGN;

    throw std::logic_error("Invalid char code in data structure");
}

// pre: some direction exists from -> to
Step MyUtils::calcStep(pair<int,int> from, pair<int,int> to)
{
    if(from.first == to.first && from.second == to.second)
        return Step::Stay;
        
    for(int i = 0; i<4; i++) 
    {
        Direction dir = static_cast<Direction>(i);
        if(calcNewPosition(dir, from) == to)
            return directionToStep(dir);
    }
    return Step::Stay;
}

template <typename T>
void MyUtils::loadConfig(const char* configName, vector<pair<string, T*>> &pairs)
{

    // path finding for "configs" directory
    fs::path startPath = fs::current_path();
    fs::path currentPath = startPath;
    while (!currentPath.empty()) {
        if (fs::exists(currentPath / "configs/")) {
            break;
        }
        currentPath = currentPath.parent_path();
    }

    fs::path configPath = currentPath / "configs" / configName;

    
    std::ifstream file(configPath.string().c_str());
    if (!file.is_open())
    {
        string errMsg = string("Error: Invalid config file (") + configPath.string() + ")";
        throw std::invalid_argument(errMsg);
    }
    string line, param;
    size_t argsFound = 0;
    while(std::getline(file, line))
    {
        std::istringstream ss(line);
        ss >> param;
        for(auto& currPair: pairs)
        {
            if (param == currPair.first)
            {
                ss >> *(currPair.second);

                argsFound++;
                break;
            }
        }
    }

    if(argsFound != pairs.size())
    {
        string msg = "Error: Found fewer argument in config file than required. (" + configPath.string() + ")";
        throw std::invalid_argument(msg);
    }
}

template void MyUtils::loadConfig<bool>(const char*, vector<pair<string,  bool*>>&);
template void MyUtils::loadConfig<uint>(const char*, vector<pair<string,  uint*>>&);
