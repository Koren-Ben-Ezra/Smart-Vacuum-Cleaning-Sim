#include "AlgoA.h"
#include <iostream>

REGISTER_ALGORITHM(AlgoA);

// Constructor
AlgoA::AlgoA() : totalSteps(0), isExploring(true), lastStep(Step::Stay),
                             batteryMeter(nullptr), dirtSensor(nullptr), wallsSensor(nullptr),
                             maxSteps(0), maxBattery(0), pathToDock({}), returningToDock(false), position(pair{0, 0}),
                             dfsNumOfActualVisited(1), dfsBacktracking(false), dfsBackPath({}),
                             dfsMovingNewPos(false), dfsNewPosPath({})
{
    // Initialize an empty stack
    dfsStack.push(pair{0, 0});
    dfsStack.pop();

    // Initialize an the sets
    dfsVisited.insert(pair{0, 0});

    // Intializing the house mapping to hold the docking station
    houseMapping.insert(pair{pair{0, 0}, std::make_unique<Point>(pair{0, 0})});
}

// pre: position is already in the houseMapping
void AlgoA::updateHouseMapping()
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

void AlgoA::updateReachable()
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
            dfsVisited.insert(unreachablePos);
        }
    }
}

bool AlgoA::isFinish()
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

size_t AlgoA::numOfReachable()
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
void AlgoA::insertCoordinate(pair<int, int> pos)
{
    auto search = houseMapping.find(pos);
    if (search != houseMapping.end())
    {
        return; // coordinate already the house mapping
    }
    houseMapping.insert(pair{pos, std::make_unique<Point>(Point(pos))});
}

Step AlgoA::nextStep()
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
        dfsBacktracking = false;
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

        // inserting to dfsVisited all clean spots
        if (!atDocking() && !needToClean)
        {
            auto result = dfsVisited.insert(position);
            if (result.second) // the insertion took place, meaning the position wasnt there before, so need to increase the number of actual visited
                dfsNumOfActualVisited++;
        }

        if (returningToDock)
        {
            step = returnDocking();
        }

        // Need to clean the spot
        else if (canClean && needToClean && !dfsBacktracking)
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

        if(returningToDock)
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

bool AlgoA::isStuck()
{
    return fetchSurrounding().empty();
}

// searching the shortest path from current location to the docking station
// at pathToDock.back there is the first step to do in order to return to docking,
// and at the start of the vector there is the last step that leads to the docking.
void AlgoA::updatePathToDock()
{
    pair<int, int> dockPos = pair{0, 0};

    updateBFSPath(position, dockPos, pathToDock);
}

// update path to hold a path from src to dst. finding the path using BFS
void AlgoA::updateBFSPath(pair<int, int> src, pair<int, int> dst, vector<Direction> &path)
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
Step AlgoA::returnDocking()
{
    isExploring = false;
    Direction dir = pathToDock.back();
    pathToDock.pop_back();
    return MyUtils::directionToStep(dir);
}

// doing a DFS step
Step AlgoA::exploreStep()
{
    // if dfsStack.top() == position then we enter the position at previous time but didnt visit it back them
    // and now we have visit the position so we just need to pop it for the stack and continue the exploration
    while (!dfsStack.empty() && dfsStack.top() == position)
    {
        dfsStack.pop();
    }

    size_t reachableSize = numOfReachable();

    // searching for a new position to do dfs from
    if (dfsStack.empty() && dfsNumOfActualVisited < reachableSize)
    {
        if (dfsBacktracking)
        {
            // we are backtracking, so going to according to the dfsBackPath
            Direction dir = dfsBackPath.back();
            dfsBackPath.pop_back();

            // reached the destination of backtracking
            if (dfsBackPath.empty())
                dfsBacktracking = false;

            return MyUtils::directionToStep(dir);
        }

        // we already found a new position and we are going on the path to it
        if (dfsMovingNewPos)
        {
            // reached the new position
            if (dfsNewPosPath.empty())
            {
                dfsMovingNewPos = false;
                dfsBacktracking = false;
                // not doing return statement in order to get the next section of checking if the stack is not empty
            }

            else
            {
                Direction dir = dfsNewPosPath.back();
                dfsNewPosPath.pop_back();

                return MyUtils::directionToStep(dir);
            }
        }

        else
        {
            return fetchNewDFSStep(true);
        }
    }

    // dfsStack isn't empty so there are more positions to go to
    if (!dfsStack.empty())
    {
        if (atDocking())
        {
            return fetchNewDFSStep(false);
        }

        // we are NOT backtracking
        if (!dfsBacktracking)
        {
            // checking if the that the top of the stack has been already visited
            auto prevPos = dfsStack.top();

            // if the dfsStack.pop is already been visited then backtrack
            if (dfsVisited.find(prevPos) != dfsVisited.end())
            {
                dfsBacktracking = true;
                dfsMovingNewPos = false; // if we are backtracking, then we dont move to new position
                // not doing return statement in order to get the next section of checking if we are backtracking
            }

            // no need to backtrack
            if (!needToBacktrack())
            {
                auto step = fetchNextDFSStep();
                if (step == Step::Stay) // fetchNextDFSStep return stay iff we need to backtrack
                {
                    dfsBacktracking = true;
                    dfsMovingNewPos = false; // if we are backtracking, then we dont move to new position
                }
                else
                    return step;
            }

            // need to backtrack
            // this is if and not else, so that if flag turned on in previous section, it will be catched here
            if (needToBacktrack())
            {
                dfsBacktracking = true;
                dfsMovingNewPos = false; // if we are backtracking, then we dont move to new position

                pair<int, int> backtrackTo;

                if (dfsStack.empty())
                {
                    backtrackTo = pair{0, 0}; // docking station
                }
                else
                    backtrackTo = dfsStack.top();

                updateBFSPath(position, backtrackTo, dfsBackPath); // finding path from current position to backtrackTo using BFS
            }
        }

        // we are backtracking, so going to according to the dfsBackPath
        Direction dir = dfsBackPath.back();
        dfsBackPath.pop_back();

        // reached the destination of backtracking
        if (dfsBackPath.empty())
            dfsBacktracking = false;

        return MyUtils::directionToStep(dir);
    }

    // this can only happen if we finished cleaning the house
    if (dfsNumOfActualVisited == reachableSize)
    {
        // need to go back to docking
        if (!atDocking())
        {
            returningToDock = true;  // returning to the docking station
            dfsBacktracking = false; // if we returning to dock, then we dont backtrack
            dfsMovingNewPos = false; // if we returning to dock, then we dont move to new position

            return returnDocking();
        }

        // we are at docking station
        return Step::Finish;
    }

    // ---- NOT SUPPOSE TO HAPPEN ---- //
    return Step::Stay; // no more position to explore
}

unordered_map<pair<int, int>, Direction, AlgoA::PairHash, AlgoA::PairEqual> AlgoA::fetchSurrounding()
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
            posReachable = nextPosInHouse->second->reachable;

        if (!(wallsSensor->isWall(dir)) && posReachable)
        {
            surr.insert(pair{nextPos, dir});
        }
    }
    return surr;
}

unordered_map<pair<int, int>, Direction, AlgoA::PairHash, AlgoA::PairEqual> AlgoA::fetchSurrounding(pair<int, int> pos)
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

// pre: it stackEmpty == true -> there is at least one position in house mapping that was not visited yet
// pre: if stackEmpty == false -> there is at least one position in dfsStack
Step AlgoA::fetchNewDFSStep(bool stackEmpty)
{
    // if we are at the start of the algorithm, no need to search for a new position
    if (totalSteps == 0)
    {
        auto surrounding = fetchSurrounding();

        for (auto it = surrounding.begin(); it != surrounding.end(); ++it)
        {
            pair<int, int> goToPos = it->first;
            if (dfsVisited.find(goToPos) == dfsVisited.end()) // Not visited in the location
            {
                auto goToPoint = houseMapping.find(goToPos);

                // if the goToPos is clean -> insert it to the dfsVisited
                if (goToPoint->second->dirtLevel == 0)
                {
                    dfsVisited.insert(goToPos);
                    dfsNumOfActualVisited++;
                }

                // the goToPos is not clean -> need to visit it sometime
                else
                    dfsStack.push(goToPos); // push the position the algo is going to go to
            }
        }

        return MyUtils::calcStep(position, dfsStack.top());
    }

    dfsMovingNewPos = true;
    dfsBacktracking = false; // if we are moving to new position, then we dont backtrack

    pair<int, int> newPos;

    // if stackEmpty, then searching for a new position that is not visited yet
    if (stackEmpty)
    {
        newPos = pair{0, 0};
        for (auto it = houseMapping.begin(); it != houseMapping.end(); ++it)
        {
            // found a position that was not visited
            if (dfsVisited.find(it->first) == dfsVisited.end())
            {
                if (it->second->dirtLevel == 0) // if the position is clean and not in the dfsVisited, then insert it
                {
                    dfsVisited.insert(it->first);
                    dfsNumOfActualVisited++;
                }
                else // the position is not clean so it's the chosen one
                {
                    newPos = it->first;
                    break; // going out of the loop
                }
            }
        }

        // newPos didnt change so we covered the house and finshed
        if (newPos == pair{0, 0})
        {
            return Step::Finish;
        }
    }

    // stckEmpty == false, then new position is the top of the stack
    else
    {
        newPos = dfsStack.top();
        dfsStack.pop();
    }

    // finding path for current position to the new position
    updateBFSPath(position, newPos, dfsNewPosPath);

    // doing first step in the path
    Direction dir = dfsNewPosPath.back();
    dfsNewPosPath.pop_back();

    // reached the new position
    if (dfsNewPosPath.empty())
    {
        dfsMovingNewPos = false;
        dfsBacktracking = false;
    }

    return MyUtils::directionToStep(dir);
}

// pre: fetchSurrounding().size() != 1
// pre: dfsStack not empty
Step AlgoA::fetchNextDFSStep()
{
    auto currTop = dfsStack.top();
    dfsStack.pop();
    auto surrounding = fetchSurrounding();

    for (auto it = surrounding.begin(); it != surrounding.end(); ++it)
    {
        pair<int, int> goToPos = it->first;
        if (dfsVisited.find(goToPos) == dfsVisited.end()) // Not visited in the location
        {
            auto goToPoint = houseMapping.find(goToPos);

            // goToPos is clean so inserting it to the dfsVisited
            if (goToPoint->second->dirtLevel == 0)
            {
                dfsVisited.insert(goToPos);
                dfsNumOfActualVisited++;
            }

            // goToPos is not clean so inserting it to dfsStack
            else
                dfsStack.push(goToPos); // push the position the algo is going to go to
        }
    }

    if (currTop == dfsStack.top()) // we didnt enter any new position to the stack
    {
        return Step::Stay; // return stay so the algorith, would know to start backtracking
    }

    return MyUtils::calcStep(position, dfsStack.top());
}

bool AlgoA::needToBacktrack()
{
    if (dfsBacktracking)
        return true;

    auto surrounding = fetchSurrounding();

    for (auto it = surrounding.begin(); it != surrounding.end(); ++it)
    {
        pair<int, int> curPos = it->first;

        // if there is at least one neighbor that we havent visited yet -> no need to backtrack
        if (dfsVisited.find(curPos) == dfsVisited.end())
        {
            return false;
        }
    }

    return true;
}