#include "AlgoB.h"

REGISTER_ALGORITHM(AlgoB);

// Constructor
AlgoB::AlgoB() 
:   totalSteps(0), isExploring(true), lastStep(Step::Stay),
    batteryMeter(nullptr), dirtSensor(nullptr), wallsSensor(nullptr),
    maxSteps(0), maxBattery(0), pathToDock({}), 
    returningToDock(false), position(pair{0, 0}),
    dfsNumOfActualVisited(1), dfsBackPath({}),
    dfsMovingNewPos(false), dfsNewPosPath({}), 
    nextSpiralDir(Direction::North) ,gen(rd())
{
    // Initialize an empty stack
    dfsStack.push(pair{0, 0});
    dfsStack.pop();

    // Initialize an the sets
    visited.insert(pair{0, 0});

    // Intializing the house mapping to hold the docking station
    houseMapping.insert(pair{pair{0, 0}, std::make_unique<Point>(pair{0, 0})});

    vector<pair<string, bool*>> pairs = {{"spiralClockwise", &spiralClockwise}};
    MyUtils::loadConfig(CONFIG_NAME, pairs);
}


// pre: position is already in the houseMapping
void AlgoB::updateHouseMapping()
{
    // finding the dirt level of current position
    auto it = houseMapping.find(position);
    auto &pointPtr = it->second;
    pointPtr->dirtLevel = dirtSensor->dirtLevel();

    auto surrounding = fetchSurrounding(); // getting the surrounding in <coordinate, direction to coordinate> format

    // going through surrounding positions
    for (auto it = surrounding.begin(); it != surrounding.end(); ++it)
    {
        insertCoordinate(it->first); // inserting the coordinate to the house mapping
    }
}

void AlgoB::updateReachable()
{
    queue<pair<int, int>> bfsQueue;
    unordered_set<pair<int, int>, PairHash, PairEqual> bfsVisited;
    pair<int, int> dockPos = pair{0, 0};
    size_t maxRange = static_cast<size_t>(maxBattery) % 2 == 1 ? (maxBattery + 1) / 2 : (maxBattery / 2);

    bfsQueue.push(dockPos);
    bfsVisited.insert(dockPos);

    // i is the index of circles around the docking
    for (size_t i = 0; i < maxRange; i++)
    {
        size_t queueSize = bfsQueue.size();

        // j is the index of positions in circle i'th
        for (size_t j = 0; j < queueSize; j++)
        {
            pair<int, int> curPos = bfsQueue.front();

            bfsQueue.pop();

            auto surrounding = fetchSurrounding(curPos);
            for (auto it = surrounding.begin(); it != surrounding.end(); ++it)
            {
                auto neighbor = it->first;
                if (bfsVisited.find(neighbor) == bfsVisited.end()) // Not visited in the tmpPos yet
                {
                    bfsQueue.push(neighbor);
                    bfsVisited.insert(neighbor);
                }
            }
        }
    }

    while (!bfsQueue.empty())
    {
        auto unreachablePos = bfsQueue.front();
        bfsQueue.pop();
        auto unreachablePoint = houseMapping.find(unreachablePos);
        if (unreachablePoint != houseMapping.end())
        {
            unreachablePoint->second->reachable = false;
            visited.insert(unreachablePos);
        }
    }
}

size_t AlgoB::numOfReachable()
{
    size_t reachables = 0;

    for (auto it = houseMapping.begin(); it != houseMapping.end(); ++it)
    {
        if (it->second->reachable)
            reachables++;
    }

    return reachables;
}

// inserting a specific coordiante to the house mapping
// pre: pos is not a wall
void AlgoB::insertCoordinate(pair<int, int> pos)
{
    auto search = houseMapping.find(pos);
    if (search != houseMapping.end())
    {
        return; // coordinate already the house mapping
    }
    houseMapping.insert(pair{pos, std::make_unique<Point>(Point(pos))});
}


Step AlgoB::nextStep()
{

    Step step;

    if ((batteryMeter->getBatteryState() == 0) && !atDocking())
    {
        step = Step::Stay;
    }

    updateHouseMapping(); // first thing is to add the surrounding to the house mapping

    updatePathToDock(); // need to update the shortest path to the dock

    // can only happen in the docking station
    if (batteryMeter->getBatteryState() == maxBattery)
    {
        isExploring = true;
        dfsMovingNewPos = false;
    }

    bool stuck = isStuck();

    // We stuck but can go back to docking station
    if (stuck && !atDocking())
    {
        step = returnDocking();
    }

    // We stuck and already at docking station
    else if (stuck)
    {
        step = Step::Finish;
    }

    // We keep exploring and cleaning the house
    else if (isExploring)
    {
        size_t enoughBattery = batteryMeter->getBatteryState() - pathToDock.size();
        size_t enoughSteps = maxSteps - totalSteps - pathToDock.size();

        bool canClean = (enoughBattery >= 1) && (enoughSteps >= 1);       // 1 step to clean
        bool canKeepExplore = (enoughBattery >= 3) && (enoughSteps >= 3); // 2 steps to go forward and back + 1 step to clean something

        bool needToClean = isDirty();

        // inserting to visited all clean spots
        if (!atDocking() && !needToClean)
        {
            auto result = visited.insert(position);
            if (result.second) // the insertion took place, meaning the position wasnt there before, so need to increase the number of actual visited
                dfsNumOfActualVisited++;
        }

        if (returningToDock)
        {
            step = returnDocking();
        }

        // Need to clean the spot
        else if (canClean && needToClean)
        {
            step = Step::Stay;
        }

        // There is enough battery to move forward (and later on to go back)
        else if (canKeepExplore)
        {
            step = exploreStep();
        }

        // There isn't enough battery
        else
        {
            step = returnDocking();
        }
    }

    // The current location is at the docking station and need to charge
    else if (atDocking())
    {

        step = Step::Stay;

        if (returningToDock)
            step = Step::Finish;

        if (lastStep != Step::Stay)
        {
            updateReachable();

            if (isFinish())
            {
                step = Step::Finish;
            }
        }
    }

    // Need to go back to docking station
    else
    {
        step = returnDocking();
    }

    position = MyUtils::calcNewPosition(step, position); // calculating the new position after doing the step

    lastStep = step;

    if (step != Step::Finish)
        totalSteps++;

    return step;
}

bool AlgoB::isStuck()
{
    return fetchSurrounding().empty();
}

// searching the shortest path from current location to the docking station
// at pathToDock.back there is the first step to do in order to return to docking,
// and at the start of the vector there is the last step that leads to the docking.
void AlgoB::updatePathToDock()
{
    pair<int, int> dockPos = pair{0, 0};

    updateBFSPath(position, dockPos, pathToDock);
}

// update path to hold a path from src to dst. finding the path using BFS
void AlgoB::updateBFSPath(pair<int, int> src, pair<int, int> dst, vector<Direction> &path)
{
    path.clear(); // clearing the path from old paths that may be in it

    queue<pair<int, int>> bfsQueue;
    unordered_set<pair<int, int>, PairHash, PairEqual> bfsVisited;

    // the key is position
    // the value is a pair of a position that lead to the key and the Direction that lead to the key from the position in the value
    unordered_map<pair<int, int>, pair<pair<int, int>, Direction>, PairHash, PairEqual> parent; // To reconstruct the path

    bfsQueue.push(src);
    bfsVisited.insert(src);
    parent[src] = pair{src, Direction::East}; // initializing with something - not important what

    while (!bfsQueue.empty())
    {
        pair<int, int> curPos = bfsQueue.front();

        bfsQueue.pop();

        if (curPos == dst) // reached the destination
        {
            // updating path
            pair<int, int> tmpPos = dst;
            while (tmpPos != src)
            {
                path.push_back(parent[tmpPos].second); // inserting the direction we came from to tmpPos
                tmpPos = parent[tmpPos].first;
            }

            return;
        }

        auto surrounding = fetchSurrounding(curPos);
        for (auto it = surrounding.begin(); it != surrounding.end(); ++it)
        {
            auto tmpPos = it->first;
            if (bfsVisited.find(tmpPos) == bfsVisited.end()) // Not visited in the tmpPos yet
            {
                bfsQueue.push(tmpPos);
                bfsVisited.insert(tmpPos);
                parent[tmpPos] = pair{curPos, it->second};
            }
        }
    }
}

// returning a step on the way to the docking station
Step AlgoB::returnDocking()
{
    isExploring = false;
    Direction dir = pathToDock.back();
    pathToDock.pop_back();
    return MyUtils::directionToStep(dir);
}

pair<int, int> AlgoB::computeOptimalNextPos()
{
    // Collect unvisited positions from 'circlesLeft' circles,
    // stating from the first unvisited that was found.
    int circlesLeft = 1;
    bool unvisitedFound = false;

    queue<pair<int, int>> unvisitedQueue;
    queue<pair<int, int>> bfsQueue;
    unordered_set<pair<int, int>, PairHash, PairEqual> bfsVisited;
    pair<int, int> dockPos = pair{0, 0};

    bfsQueue.push(position);
    bfsVisited.insert(position);

    // find all unvisited positions for 'circlesLeft' circles
    while (circlesLeft > 0 && !bfsQueue.empty())
    {
        size_t queueSize = bfsQueue.size();

        // j is the index of positions in circle i'th
        for (size_t j = 0; j < queueSize; j++)
        {
            pair<int, int> curPos = bfsQueue.front();

            bfsQueue.pop();

            auto surrounding = fetchSurrounding(curPos);
            for (auto it = surrounding.begin(); it != surrounding.end(); ++it)
            {
                auto neighbor = it->first;

                if (bfsVisited.find(neighbor) == bfsVisited.end())
                {
                    bfsQueue.push(neighbor);
                    bfsVisited.insert(neighbor);
                }

                if (visited.find(neighbor) ==  visited.end() && !(houseMapping.find(neighbor)->second->isWall))
                {
                    unvisitedFound = true;
                    unvisitedQueue.push(neighbor);
                }
            }
        }
        if (unvisitedFound)
            circlesLeft--;
    }

    // Choose optimal position from the found unvisited positions
    vector<pair<int, int>> optimalPositions;
    size_t optimalObjective = -1; // maximum size_t value

    while (!unvisitedQueue.empty())
    {
        pair<int, int> pos = unvisitedQueue.front();
        unvisitedQueue.pop();

        size_t objective = computeDistance(pos);
        if (objective == optimalObjective)
        {
            optimalPositions.push_back(pos);
        }

        else if (objective < optimalObjective)
        {
            optimalPositions.clear();
            optimalPositions.push_back(pos);
            optimalObjective = objective;
        }
    }

    if (optimalPositions.empty()) //|| optimalObjective >= batteryMeter->getBatteryState())
        return dockPos;
    size_t randIdx = decideUniformIndex(optimalPositions.size());
    return optimalPositions[randIdx];
}

size_t AlgoB::decideUniformIndex(size_t range)
{
    std::uniform_int_distribution<> dist(0, range - 1);
    return dist(gen);
}

// distance = (#steps){position -> dst} + (#steps){dst -> docking}
size_t AlgoB::computeDistance(pair<int, int> dst)
{
    vector<Direction> currToDst;
    vector<Direction> dstToDock;
    updateBFSPath(position, dst, currToDst);
    updateBFSPath(dst, {0, 0}, dstToDock);
    return currToDst.size() + dstToDock.size();
}

Step AlgoB::exploreStep()
{
    size_t reachableSize = numOfReachable();

    if (dfsMovingNewPos)
    {
        if (dfsNewPosPath.empty())
            dfsMovingNewPos = false;
        else
            return popNewPathStep();
    }

    if (dfsNumOfActualVisited < reachableSize)
    {
        pair<int, int> nextPos = MyUtils::calcNewPosition(nextSpiralDir, position);
        bool isSpiralDirValid = !(wallsSensor->isWall(nextSpiralDir)) && visited.find(nextPos) == visited.end();
        
        Direction straight;
        if(spiralClockwise)
            straight = static_cast<Direction>((static_cast<int>(nextSpiralDir) + 4 - 1) % 4);
        else
            straight = static_cast<Direction>((static_cast<int>(nextSpiralDir) + 1) % 4);
        
        nextPos = MyUtils::calcNewPosition(straight, position);
        bool isStraightDirValid = !(wallsSensor->isWall(straight)) && visited.find(nextPos) == visited.end();
        if(!isStraightDirValid && !isSpiralDirValid)
        {
            pair<int, int> targetPos = computeOptimalNextPos();
            updateBFSPath(position, targetPos, dfsNewPosPath);
            
            dfsMovingNewPos = true;

            Step step = popNewPathStep();
            if(dfsNewPosPath.empty())
                dfsMovingNewPos = false;

            return step;
        }

        if (isSpiralDirValid)
        {
            return fetchSpiralDir();
        }

        if(isStraightDirValid)
            return MyUtils::directionToStep(straight);

        return returnDocking();
    }

    if (dfsNumOfActualVisited == reachableSize)
    {
        
        // need to go back to docking
        if (!atDocking())
        {
            returningToDock = true;  // returning to the docking station
            dfsMovingNewPos = false; // if we returning to dock, then we dont move to new position

            return returnDocking();
        }

        // we are at docking station
        return Step::Finish;
    }

    return Step::Stay;
}

Step AlgoB::fetchSpiralDir()
{
    Direction dir = nextSpiralDir;
    if(spiralClockwise)
        // right hand
        nextSpiralDir = static_cast<Direction>((static_cast<int>(nextSpiralDir) + 1) % 4);
    else
        // left hand
        nextSpiralDir = static_cast<Direction>((static_cast<int>(nextSpiralDir) + 4 - 1) % 4);
    return MyUtils::directionToStep(dir);
}

Step AlgoB::popNewPathStep()
{
    Direction dir = dfsNewPosPath.back();
    dfsNewPosPath.pop_back();

    return MyUtils::directionToStep(dir);
}

unordered_map<pair<int, int>, Direction, AlgoB::PairHash, AlgoB::PairEqual> AlgoB::fetchSurrounding()
{
    unordered_map<pair<int, int>, Direction, PairHash, PairEqual> surr;

    // going through surrounding positions
    for (int i = 0; i < 4; i++)
    {
        bool posReachable = true;
        Direction dir = static_cast<Direction>(i);
        auto nextPos = MyUtils::calcNewPosition(dir, position);
        auto nextPosInHouse = houseMapping.find(nextPos);
        if (nextPosInHouse != houseMapping.end())
        {
            posReachable = nextPosInHouse->second->reachable;
            nextPosInHouse->second->isWall = wallsSensor->isWall(dir);
        }
        if (!(wallsSensor->isWall(dir)) && posReachable)
        {
            surr.insert(pair{nextPos, dir});
        }
    }
    return surr;
}

unordered_map<pair<int, int>, Direction, AlgoB::PairHash, AlgoB::PairEqual> AlgoB::fetchSurrounding(pair<int, int> pos)
{
    unordered_map<pair<int, int>, Direction, PairHash, PairEqual> surr;

    // going through surrounding positions
    for (int i = 0; i < 4; i++)
    {
        Direction dir = static_cast<Direction>(i);
        auto tempPos = MyUtils::calcNewPosition(dir, pos);
        auto tempPoint = houseMapping.find(tempPos);
        if (tempPoint != houseMapping.end() && tempPoint->second->reachable)
        {
            surr.insert(pair{tempPos, dir});
        }
    }
    return surr;
}

bool AlgoB::isFinish()
{
    size_t sumDirt = 0;
    bool discoveredReachable = true;
    for (auto it = houseMapping.begin(); it != houseMapping.end(); ++it)
    {

        if (it->second->reachable)
        {
            discoveredReachable &= it->second->dirtLevel != UNDISCOVERED_CODE;

            if (it->second->dirtLevel <= MAX_DIRT)
                sumDirt += it->second->dirtLevel;
        }
    }

    return sumDirt == 0 && discoveredReachable;
}
