#include "AlgorithmRegistrar.h"

struct AlgorithmRegistration {
    AlgorithmRegistration(const std::string& name, AlgorithmFactory algorithmFactory) {
        AlgorithmRegistrar::getAlgorithmRegistrar().registerAlgorithm(name, std::move(algorithmFactory));
    }
};

#define REGISTER_ALGORITHM(ALGO) AlgorithmRegistration ALGO##_obj(#ALGO, []{return std::make_unique<ALGO>();})