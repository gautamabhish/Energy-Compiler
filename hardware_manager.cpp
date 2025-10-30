#include "hardware_manager.hpp"
#include <thread>
#include <iostream>
std::vector<CPUCore> setupHardware() {
    std::vector<CPUCore> cores = {
        {0, CoreType::HighPerf, 3.5, true},
        {1, CoreType::EnergyEfficient, 1.8, true}
    };
    return cores;
}
std::vector<CPUCore> detectCores() {
    unsigned int n = std::thread::hardware_concurrency();
    if (n == 0) n = 4; // fallback
    std::vector<CPUCore> cores;

    for (unsigned int i = 0; i < n; i++) {
        CPUCore c;
        c.id = i;
        c.type = (i % 2 == 0) ? CoreType::HighPerf : CoreType::EnergyEfficient;
        c.freqGHz = (c.type == CoreType::HighPerf) ? 3.2 : 2.0;
        c.available = true;
        c.powerWatts = (c.type == CoreType::HighPerf) ? 25.0 : 8.0; //watt per core
        cores.push_back(c);
    }

    std::cout << "Detected " << cores.size() << " logical cores.\n";
    return cores;
}
