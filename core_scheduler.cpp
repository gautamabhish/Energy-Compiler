#include "core_scheduler.hpp"
#include <iostream>
#include <unordered_map>
#include <limits>

// simulateExecution: compute total time & energy and print per-op lines
SimulationResult simulateExecution(
    const IRGraph &g,
    const std::unordered_map<std::string,int> &assign,
    const std::vector<CPUCore> &cores,
    bool optimized
) {
    // per-core accumulators (for parallel-model)
    std::unordered_map<int,double> coreTime;
    std::unordered_map<int,double> coreEnergy;

    double totalSequential = 0.0;
    double totalEnergySeq = 0.0;

    std::cout << "\n=== Simulated Execution (" 
              << (optimized ? "Energy-Aware" : "Baseline") 
              << ") ===\n";

    for (auto &node : g.nodes) {
        // some IR nodes (like Print) may have empty name; handle by using id-string
        std::string key = node->name.empty() ? ("op#" + std::to_string(node->id)) : node->name;
        auto it = assign.find(key);
        if (it == assign.end()) {
            // If not mapped, skip with a warning
            std::cout << "Warning: node '" << key << "' not scheduled — skipping\n";
            continue;
        }

        int cid = it->second;
        if (cid < 0 || cid >= (int)cores.size()) {
            std::cerr << "Invalid core id " << cid << " for node " << key << "\n";
            continue;
        }

        const auto &core = cores[cid];

        double timeMs = (node->flops > 0)
            ? (node->flops / (core.freqGHz * 1e9)) * 1000.0
            : 0.1; // tiny constant for control ops

        double energyJ = core.powerWatts * (timeMs / 1000.0); // P (W) * t (s)

        if (optimized) {
            coreTime[cid] += timeMs;
            coreEnergy[cid] += energyJ;
        } else {
            // baseline we accumulate sequentially (assume normal compiler runs sequentially on chosen core)
            totalSequential += timeMs;
            totalEnergySeq += energyJ;
        }

        std::cout << "Exec op#" << node->id << " (" << key << ") "
                  << "on Core#" << cid << " [" 
                  << (core.type==CoreType::HighPerf ? "HP" : "EE") << " @ " 
                  << core.freqGHz << "GHz] -> "
                  << timeMs << " ms, " << energyJ << " J\n";
    }

    double totalTime = 0.0;
    double totalEnergy = 0.0;
    if (optimized) {
        // assume per-core parallelism: total time = max(coreTime)
        for (auto &p : coreTime) {
            if (p.second > totalTime) totalTime = p.second;
        }
        for (auto &p : coreEnergy) totalEnergy += p.second;
    } else {
        totalTime = totalSequential;
        totalEnergy = totalEnergySeq;
    }

    std::cout << "\n=== Summary ===\n";
    std::cout << "Total time: " << totalTime << " ms\n";
    std::cout << "Total energy: " << totalEnergy << " J\n";

    return { totalTime, totalEnergy };
}

// Baseline schedule: map every node to the first HighPerf core if any, else core 0
std::unordered_map<std::string,int> scheduleBaseline(const IRGraph &g, const std::vector<CPUCore> &cores) {
    std::unordered_map<std::string,int> mapping;
    int chosen = -1;
    for (const auto &c : cores) {
        if (c.type == CoreType::HighPerf) { chosen = c.id; break; }
    }
    if (chosen == -1 && !cores.empty()) chosen = cores.front().id;

    for (auto &node : g.nodes) {
        std::string key = node->name.empty() ? ("op#" + std::to_string(node->id)) : node->name;
        mapping[key] = chosen;
    }
    std::cout << "\n=== Baseline mapping: all nodes -> Core#" << chosen << " ===\n";
    return mapping;
}

// Energy-aware schedule: assign nodes to cores that match their assignedCore preference.
// note: this function consumes 'cores' available flags (mutates vector) to avoid double-assign on same core.
std::unordered_map<std::string,int> scheduleEnergyAware(const IRGraph &g, std::vector<CPUCore> &cores) {
    std::unordered_map<std::string,int> mapping;

    // Reset availability flags if caller didn't
    for (auto &c : cores) c.available = true;

    // First pass: try to assign nodes to a core that matches preference and is free
    for (auto &node : g.nodes) {
        std::string key = node->name.empty() ? ("op#" + std::to_string(node->id)) : node->name;
        bool assigned = false;
        for (auto &core : cores) {
            if (core.available && core.type == node->assignedCore) {
                mapping[key] = core.id;
                core.available = false;
                assigned = true;
                break;
            }
        }
        // fallback: if not assigned, place on the least loaded core id (simple fallback to first)
        if (!assigned && !cores.empty()) {
            mapping[key] = cores.front().id;
        }
    }

    std::cout << "\n=== Energy-aware core assignment ===\n";
    for (auto &p : mapping) {
        std::cout << "Node " << p.first << " -> Core#" << p.second << "\n";
    }
    return mapping;
}
