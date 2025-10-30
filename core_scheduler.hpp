#pragma once
#include "ir.hpp"
#include "hardware_manager.hpp"
#include <unordered_map>
#include <vector>
#include <string>

struct SimulationResult {
    double timeMs;
    double energyJ;
};

// Simulate execution and return time & energy
SimulationResult simulateExecution(
    const IRGraph &g,
    const std::unordered_map<std::string,int> &assign,
    const std::vector<CPUCore> &cores,
    bool optimized
);

// Two scheduling APIs:
// - Baseline: naive mapping (normal compiler) — everything -> first high-perf core (or core 0 fallback)
// - Energy-aware: use node->assignedCore preference (your current policy)
std::unordered_map<std::string,int> scheduleBaseline(const IRGraph &g, const std::vector<CPUCore> &cores);
std::unordered_map<std::string,int> scheduleEnergyAware(const IRGraph &g, std::vector<CPUCore> &cores);
