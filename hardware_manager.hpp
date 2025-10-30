#pragma once
#include <vector>
#include <string>
#include "ir.hpp"
struct CPUCore {
    int id;
    CoreType type;
    float freqGHz;
    bool available;
    double powerWatts; // <-- Add this
};


std::vector<CPUCore> detectCores();
