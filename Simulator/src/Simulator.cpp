#include "Simulator.h"

MySimulator::MySimulator(string algoName, bool writeOutput, bool writeLog)
    : houseDescription(""), rows(0), cols(0),
      houseStructure({}), dockingLocation({}), currLocation({}),
      initDirt(0), dirtLeft(0), maxSteps(0),
      maxBattery(0), curBattery(0),
      pAlgo(nullptr), algoName(algoName), algoScore(0), 
      writeOutput(writeOutput), writeLog(writeLog),
      batteryMeter(MyBatteryMeter(curBattery)),
      dirtSensor(MyDirtSensor(houseStructure, currLocation)),
      wallsSensor(MyWallsSensor(houseStructure, currLocation)),
      logFile(""), currLog(LogCode::NoLog), prevLog(LogCode::NoLog),
      numSteps(0), steps({}), status(Status::Working), 
      timeoutCoefficient(0), timer(io), timeoutOccoured(false)
{
    dockingLocation.resize(2);
    currLocation.resize(2);
    
    vector<pair<string, uint*>> pairs = 
        {{"timeoutCoefficient", &timeoutCoefficient}};

    try{
        loadConfig(SIM_CONFIG_NAME, pairs);
    }
    catch(const std::invalid_argument &e)
    {
        throw CustomError(ErrOwnership::Simulator, e.what());
    }
}

MySimulator::~MySimulator()
{
    logFile.close();
}

void MySimulator::setAlgorithm(AbstractAlgorithm &algo)
{
    algo.setMaxSteps(maxSteps);
    algo.setWallsSensor(wallsSensor);
    algo.setDirtSensor(dirtSensor);
    algo.setBatteryMeter(batteryMeter);
    pAlgo = &algo;
}

void MySimulator::activateTimer()
{
    uint timeout = timeoutCoefficient * maxSteps;
    timer.expires_after(std::chrono::milliseconds(timeout));
    timer.async_wait([&](const boost::system::error_code& error){
        if(!error)
        {
            timeoutOccoured = true;
        }
    });
}

void MySimulator::run()
{
    activateTimer();

    Step nextStep = Step::Stay;
    try
    {
        do
        {
            if(inWall())
                throw FaultCode::FROBOT_IN_WALL;

            nextStep = pAlgo->nextStep();

            handleStep(nextStep);

            updateLogFile(nextStep);

            io.poll();

        } while (nextStep != Step::Finish && !timeoutOccoured);
    }
    catch (const FaultCode &e)
    {
        handleFault(e);
        updateLogFile(nextStep);
    }

    finalize();
}

void MySimulator::finalize()
{
    calcScore();
    writeOutputFile();
    handleErrors();
    timer.cancel();
}

void MySimulator::handleErrors()
{
    if(inWall())
        throw CustomError(ErrOwnership::Algorithm, "Algorithm tried to move into a wall", algoScore);

    if(timeoutOccoured)
    {
        string msg = "Timeout reached at "s + std::to_string(timeoutCoefficient * maxSteps) + "ms";
        throw CustomError(ErrOwnership::Algorithm, msg, algoScore);
    }
}


bool MySimulator::inWall()
{
    if(currLocation[0] < houseStructure.size() && currLocation[1] < houseStructure[0].size() &&
        houseStructure[currLocation[0]][currLocation[1]] != WALL_CODE)
        return false;
    
    return true;
}

void MySimulator::handleStep(Step step)
{
    if (numSteps == maxSteps)
    {
        steps.push_back(Step::Finish);
        throw FaultCode::FOUT_OF_STEPS;
    }

    steps.push_back(step);

    if (step == Step::Finish)
    {
        status = Status::Finished;
        return;
    }

    numSteps++;

    if (step == Step::Stay)
    {
        tryToClean();
        tryChargeRobot();
    }

    vector<size_t> prevLocation = currLocation;
    currLocation = calcNewLocation(step, currLocation);

    // if we stayed at the docking station -> we are chraging -> no need to reduce battery
    if (!(prevLocation == dockingLocation && prevLocation == currLocation))
        reduceBattery();
}

void MySimulator::handleFault(const FaultCode e)
{
    if (e == FaultCode::FBATTERY_EXHAUSTED)
        status = Status::Dead;

    if (e == FaultCode::FOUT_OF_STEPS)
        status = Status::Finished;
}   


void MySimulator::updateHouseName(const char* inputName)
{
    string str = inputName;
    size_t pos = str.find_last_of('/');

    if (pos != string::npos)
        houseName = str.substr(pos + 1);

    houseName = houseName.substr(0, houseName.size() - 6); // remove '.house'
}

void MySimulator::readHouseFile(const char *filename)
{
    updateHouseName(filename);

    // read input file into house structures
    std::ifstream file(filename);
    if (!file)
        throw CustomError(ErrOwnership::House, "Invalid input file in readHouseFile method"s);
    

    string line, word;
    std::stringstream ss;

    getline(file, houseDescription);

    getline(file, line, '=');
    getline(file, line);
    ss.str(line);
    ss >> maxSteps;
    ss.clear();

    getline(file, line, '=');
    getline(file, line);
    ss.str(line);
    ss >> maxBattery;
    curBattery = maxBattery;
    ss.clear();

    getline(file, line, '=');
    getline(file, line);
    ss.str(line);
    ss >> rows;
    ss.clear();

    getline(file, line, '=');
    getline(file, line);
    ss.str(line);
    ss >> cols;
    ss.clear();

    // read house structure from file.
    size_t letterCode;
    bool dockingFound = false;

    houseStructure.resize(rows + 2, vector<size_t>(cols + 2));
    // +2 for house wall padding for both sides.
    for (size_t i = 0; i < rows; i++)
    {
        // get current line from file and allocate memory for it
        if (!file.eof())
            getline(file, line);
        else
            line = " ";

        if (line.empty())
            line = " ";

        for (size_t j = 0; j < cols; j++)
        {

            if (j < line.length())
                letterCode = charToSize_t(line[j]);

            else
                letterCode = CLEAN_CODE;

            if (letterCode == DOCKING_CODE)
            {
                if (dockingFound)
                {
                    // found more then 1 docking stations overall, the file is invalid
                    throw CustomError(ErrOwnership::House, "Invalid input file, more than 1 docking station was found"s);
                }
                dockingFound = true;
                dockingLocation[0] = i + 1; // +1 for wall padding
                dockingLocation[1] = j + 1; // +1 for wall padding
            }

            houseStructure[i + 1][j + 1] = letterCode; // +1 for wall padding
        }
    }

    file.close();

    if (!dockingFound)
        throw CustomError(ErrOwnership::House, "Docking station not found"s);

    currLocation[0] = dockingLocation[0];
    currLocation[1] = dockingLocation[1];

    houseWallPadding();
    updateTotalDirt();
    initLogFile();
}

void MySimulator::initLogFile() noexcept(false)
{
    if(!writeLog)
        return;
    
    fs::create_directories(LOG_DIR_PATH); // creates the directory if doesn't exists

    const string logPath = LOG_DIR_PATH + houseName + "-" + algoName + ".log";
    logFile.open(logPath.c_str());
    if (!logFile)
        throw CustomError(ErrOwnership::House, "Invalid log file path"s);

    logFile << "Log File: " << houseDescription << endl
            << endl;
}

void MySimulator::houseWallPadding()
{
    rows += 2;
    cols += 2;

    for (size_t i = 0; i < rows; i++)
    {
        houseStructure[i][0] = WALL_CODE;
        houseStructure[i][cols - 1] = WALL_CODE;
    }
    for (size_t j = 0; j < cols; j++)
    {
        houseStructure[0][j] = WALL_CODE;
        houseStructure[rows - 1][j] = WALL_CODE;
    }
}

void MySimulator::writeOutputFile()
{
    if(!writeOutput)
        return;

    ofstream file;

    fs::create_directories(OUTPUT_DIR_PATH); // creates the directory if doesn't exists

    const string path = OUTPUT_DIR_PATH + houseName + "-" + algoName + ".txt";

    file.open(path.c_str());
    if (!file)
        throw CustomError(ErrOwnership::House, "Invalid output file path"s);

    file << "NumSteps = " << numSteps << '\n';
    file << "DirtLeft = " << dirtLeft << '\n';

    string outputStatus;
    switch (status)
    {
    case Status::Finished:
        outputStatus = "FINISHED";
        break;
    case Status::Dead:
        outputStatus = "DEAD";
        break;
    case Status::Working:
        outputStatus = "WORKING";
        break;
    }

    file << "Status = " << outputStatus << '\n';
    string inDockStr = robotAtDocking() ? "TRUE" : "FALSE";
    file << "InDock = " << inDockStr << '\n';
    file << "Score = " << algoScore << '\n';

    file << "Steps" << '\n';
    for (size_t i = 0; i < steps.size(); i++)
        file << stepToStr(steps[i]);

    file << endl;
    file.close();
}

LogCode MySimulator::fetchLogCode(Step currStep)
{

    if (curBattery == 0 && !robotAtDocking())
        return LogCode::BatteryExhuasted;

    if (numSteps == maxSteps)
        return LogCode::OutOfSteps;

    if (currStep == Step::Finish)
        return LogCode::Finished;

    bool isStuck = true;
    for (size_t dir = 0; dir < 4; dir++)
    {
        vector<size_t> neighbor = calcNewLocation(static_cast<Direction>(dir), currLocation);
        isStuck &= (houseStructure[neighbor[0]][neighbor[1]] == WALL_CODE);
    }

    if (isStuck)
        return LogCode::Stuck;

    if (currStep == Step::Stay && robotAtDocking())
        return LogCode::Charging;

    if (currStep == Step::Stay && !robotAtDocking())
        return LogCode::Cleaning;

    return LogCode::Exploring;
}

void MySimulator::updateLogFile(Step currStep)
{
    if(!writeLog)
        return;

    prevLog = currLog;
    currLog = fetchLogCode(currStep);

    if (currLog == LogCode::NoLog)
        return;

    vector<LogCode> nonRepetetives =
        {LogCode::Charging,
         LogCode::Stuck};

    for (LogCode nonRepetetive : nonRepetetives)
    {
        if (currLog == nonRepetetive && currLog == prevLog)
            return;
    }

    logFile << "[" << numSteps << "] ";
    string stepStr;

    switch (currLog)
    {
    case LogCode::Finished:
        logFile << "Mission accomplished!\nThe robot at the docking station and the house is clean";
        break;
    case LogCode::Cleaning:
        logFile << "Cleaned at location (" << currLocation[0] << "," << currLocation[1] << ")";
        break;
    case LogCode::Exploring:
        stepStr = stepToFullStr(currStep);
        logFile << "Exploring towards " << stepStr;
        break;
    case LogCode::Charging:
        logFile << "Arrived to docking station, starts charging";
        break;
    case LogCode::BatteryExhuasted:
        logFile << "Battery exhausted";
        break;
    case LogCode::OutOfSteps:
        logFile << "Out of steps";
        break;
    case LogCode::Stuck:
        logFile << "Surrounded by walls, can't move anywhere";
        break;
    case LogCode::NoLog:
        // don't need log message for that.
        break;
    }
    logFile << endl;
}

void MySimulator::tryChargeRobot()
{
    if (robotAtDocking())
        chargeBattery();
}

void MySimulator::tryToClean()
{
    size_t curLocationDirt = houseStructure[currLocation[0]][currLocation[1]];
    if (curLocationDirt > 0 && curLocationDirt <= MAX_DIRT)
    {
        // Clean one dirt at a step
        houseStructure[currLocation[0]][currLocation[1]]--;
        dirtLeft--;
    }
}

// Checking if it's not a wall or the docking station
bool MySimulator::isValidLocation(vector<size_t> location) const
{
    return location[0] < rows && location[1] < cols &&
           houseStructure[location[0]][location[1]] <= MAX_DIRT;
}

void MySimulator::updateTotalDirt()
{
    initDirt = 0;
    for (auto vec : houseStructure)
    {
        for (size_t element : vec)
        {
            if (element <= MAX_DIRT)
                initDirt += element;
        }
    }
    dirtLeft = initDirt;
}

void MySimulator::calcScore()
{
    algoScore = dirtLeft * 300;

    switch (status)
    {
    case Status::Finished:
        algoScore += robotAtDocking() ? numSteps : (maxSteps + 3000);
        break;
    case Status::Dead:
        algoScore += maxSteps + 2000;
        break;
    case Status::Working:
        algoScore += numSteps + (robotAtDocking() ? 0 : 1000);
        break;
    }

    if(timeoutOccoured)
        algoScore = maxSteps * 2 + initDirt * 300 + 2000;
}