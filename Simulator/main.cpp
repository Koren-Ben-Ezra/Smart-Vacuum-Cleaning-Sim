#include "AlgorithmRegistrar.h"
#include "Simulator.h"

#include <dlfcn.h>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>

#define ERROR_DIR_PATH "./errors/"
const string houseExt = ".house";
const string algoExt = ".so";

using std::mutex;
using std::thread;

mutex mtx; // mutex to protect share resources
std::atomic<int> availableThreads(0);
std::queue<thread> threadQueue;

void writeErrFile(string filename, const string &content)
{
    fs::create_directories(ERROR_DIR_PATH); // creates the directory if doesn't exists
    string errFilename = ERROR_DIR_PATH + filename + ".error";
    ofstream file;
    file.open(errFilename, std::ios::out | std::ios::trunc);
    if (!file)
    {
        return;
    }

    file << content << std::endl;

    file.close();
}

size_t execAlgo(std::unique_ptr<AbstractAlgorithm> algorithm, string housePath, string algoName, bool summaryOnly, bool writeLog)
{
    try
    {
        MySimulator sim(algoName, !summaryOnly, writeLog);

        sim.readHouseFile(housePath.c_str());
        sim.setAlgorithm(*algorithm);
        sim.run();
        return sim.getScore();
    }
    catch (const CustomError &e)
    {
        string filename;
        if (e.owner == ErrOwnership::House)
        {
            string houseName(housePath);                                        // converting from const char * to string
            fs::path filePath(houseName);                                       // converting fron string to fs::path
            houseName = filePath.filename().string();                           // getting the house name without the path to it
            filename = houseName.substr(0, houseName.size() - houseExt.size()); // removing the .house suffix
        }

        else if (e.owner == ErrOwnership::Algorithm)
        {
            filename = algoName;
        }

        else if (e.owner == ErrOwnership::Simulator)
        {
            filename = "Simulator";
        }

        else
        {
            filename = "General";
        }

        writeErrFile(filename, e.content);

        return e.score;
    }

    catch (const std::exception &e)
    {
        writeErrFile(algoName, e.what());
    }

    catch (...)
    {
        writeErrFile("General", "An error occured in simulator");
    }
    return 0;
}

void executeThread(vector<vector<string>> *scores, size_t algoIndex, size_t houseIndex,
                   std::unique_ptr<AbstractAlgorithm> algorithm, string houseName, string algoName,
                   bool summaryOnly, bool writeLog)
{
    auto res = execAlgo(std::move(algorithm), houseName, algoName, summaryOnly, writeLog);

    // update scores
    mtx.lock();
    (*scores)[algoIndex][houseIndex] = std::to_string(res);
    availableThreads++;
    mtx.unlock();
}

void writeCSV(const string &filename, const vector<vector<string>> &data)
{
    std::ofstream file(filename); // open the file

    if (!file.is_open())
    {
        writeErrFile("csv", "Failed to open csv file");
        return;
    }

    for (size_t i = 0; i < data.size(); ++i)
    {
        const auto &row = data[i];
        for (size_t j = 0; j < row.size(); ++j)
        {
            file << row[j];
            if (j < row.size() - 1)
            {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

void handleCLIArguments(int argc, char **argv, string *housePath, string *algoPath, size_t *numThreads, bool *summaryOnly, bool *writeLog)
{
    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];
        size_t pos = arg.find('=');

        if (pos != string::npos)
        {
            string key = arg.substr(0, pos);
            string value = arg.substr(pos + 1);

            if (key.compare("-house_path") == 0)
            {
                *housePath = value;
            }

            if (key.compare("-algo_path") == 0)
            {
                *algoPath = value;
            }

            if (key.compare("-num_thread") == 0)
            {
                if (std::stoul(value) > 0)
                {
                    *numThreads = std::stoul(value);
                }
            }
        }

        if (arg.compare("-summary_only") == 0)
        {
            *summaryOnly = true;
        }

        if (arg.compare("-log") == 0)
        {
            *writeLog = true;
        }
    }
}

void loadLibs(vector<void *> *libsHandle, vector<std::filesystem::path> &algoLibNames)
{
    for (const auto &lib : algoLibNames)
    {
        void *handle = dlopen(lib.c_str(), RTLD_GLOBAL | RTLD_NOW);
        if (handle == nullptr)
        {
            string filename = lib.filename().string();
            filename = filename.substr(0, filename.size() - algoExt.size());
            string errMsg = "Error in filename: '" + filename + "' - Failed loading: " + string(dlerror());
            writeErrFile(filename, errMsg);
            continue;
        }

        libsHandle->push_back(handle);
    }
}

// return vector of files from directort that has specific extension
// pre: directory is an existing directory
vector<fs::path> fetchFiles(string directory, string extension)
{
    vector<fs::path> files;

    for (const auto &entry : fs::directory_iterator(directory))
    {
        if (entry.is_regular_file() && entry.path().extension() == extension)
        {
            files.push_back(entry.path());
        }
    }

    return files;
}

// return vector of files that has .house extension
vector<fs::path> fetchHouseNames(string housePath)
{
    if (!fs::exists(housePath))
    {
        writeErrFile("houses", "Location for houses doesn't exists");
        return {};
    }
    if (!fs::is_directory(housePath))
    {
        writeErrFile("houses", "Location for houses isn't a directory");
        return {};
    }

    return fetchFiles(housePath, houseExt);
}

// return vector of files that has .so extension
vector<fs::path> fetchAlgoLibraries(string algoPath)
{
    if (!fs::exists(algoPath))
    {
        writeErrFile("algprithms", "Location for algorithms doesn't exists");
        return {};
    }
    if (!fs::is_directory(algoPath))
    {
        writeErrFile("algprithms", "Location for algorithms isn't a directory");
        return {};
    }

    return fetchFiles(algoPath, algoExt);
}

void removeInvalidHouseFromScores(vector<vector<string>> &data)
{
    size_t numRows = data.size();
    size_t numCols = data[0].size();

    // Iterate over each column (from last to first to avoid issues with removing)
    for (size_t col = numCols; col > 0; col--)
    {
        bool isEmpty = true;

        // Iterate over the rows to check if the column has only empty strings
        for (size_t row = 1; row < numRows; ++row)
        {
            if (data[row][col - 1] != "0")
            {
                isEmpty = false;
                break;
            }
        }

        // If the column has only empty strings, remove it from all rows
        if (isEmpty)
        {
            for (size_t row = 0; row < numRows; ++row)
            {
                data[row].erase(data[row].begin() + (col - 1));
            }
        }
    }
}

int main(int argc, char **argv)
{
    vector<vector<string>> scores; // vector to write to CSV

    string housePath = "./";
    string algoPath = "./";
    size_t numThreads = 10;
    bool summaryOnly = false;
    bool writeLog = false;
    string csvFileName = "summary.csv";

    // handling command line arguments
    handleCLIArguments(argc, argv, &housePath, &algoPath, &numThreads, &summaryOnly, &writeLog);
    availableThreads = numThreads;

    auto houseNames = fetchHouseNames(housePath);
    auto algoLibNames = fetchAlgoLibraries(algoPath);

    // adding the house names to the scores as headlines of the columns
    vector<string> vec;
    vec.push_back("Algorithms");
    for (const auto &housePath : houseNames)
    {
        string houseName = housePath.filename().string();
        vec.push_back(houseName.substr(0, houseName.size() - houseExt.size()));
    }
    scores.push_back(vec);

    // Loading the libraries and register the algorithms
    vector<void *> libsHandle;
    loadLibs(&libsHandle, algoLibNames);

    // running each algo on each house
    auto algos = AlgorithmRegistrar::getAlgorithmRegistrar();

    // Initializing the scores vector to empty strings
    for (const auto &algo : algos)
    {
        vector<string> vec(houseNames.size() + 1, "");
        vec[0] = algo.name();
        scores.push_back(vec);
    }

    size_t i = 0;
    for (const auto &algo : algos)
    {
        // going through the houses and running the simulation
        for (size_t j = 0; j < houseNames.size(); ++j)
        {
            auto algoFactory = algo;     // store the factory to create multiple instances
            auto algoName = algo.name(); // get the algorithm's name

            while (availableThreads == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            availableThreads--;
            try
            {
                auto algorithm = algoFactory.create();
                thread t(executeThread, &scores, i + 1, j + 1, std::move(algorithm),
                         houseNames[j].string(), algoName, summaryOnly, writeLog);
                threadQueue.push(std::move(t));
            }
            catch (const std::exception &e)
            {
                writeErrFile(algoName, e.what());
            }
        }

        i++;
    }

    // Waiting for all threads to finish
    while (!threadQueue.empty())
    {
        thread t = std::move(threadQueue.front());
        t.join();
        threadQueue.pop();
    }

    algos.clear();
    AlgorithmRegistrar::getAlgorithmRegistrar().clear();
    houseNames.clear();
    algoLibNames.clear();

    // closing the loaded libraries
    for (auto handle : libsHandle)
    {
        dlclose(handle);
    }

    // remove from scores the invalid houses if there are any
    removeInvalidHouseFromScores(scores);

    // writing the csv summary file
    writeCSV(csvFileName, scores);

    return EXIT_SUCCESS;
}