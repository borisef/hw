#ifndef RESOURCE_USAGE_H
#define RESOURCE_USAGE_H


#include <iostream>
#include <chrono>
#include <fstream>
#include <string>
#include <sys/resource.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <numeric>
#include <thread>


// Runtime
void printRuntime(std::chrono::high_resolution_clock::time_point start,
                  std::chrono::high_resolution_clock::time_point end) {
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Runtime: " << elapsed.count() << " seconds\n";
}

// Memory usage from /proc/self/status
void printMemoryUsage() {
    std::ifstream status_file("/proc/self/status");
    std::string line;
    std::cout << "--- Memory Usage ---\n";
    while (std::getline(status_file, line)) {
        if (line.find("VmRSS:") == 0 || line.find("VmHWM:") == 0) {
            std::cout << line << std::endl;
        }
    }
}

// CPU time via getrusage
void printCPUUsage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    std::cout << "--- CPU Usage ---\n";
    std::cout << "User time: " << usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6 << " sec\n";
    std::cout << "System time: " << usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6 << " sec\n";
}

// GPU usage via nvidia-smi
void printGPUUsage() {
    std::cout << "--- GPU Usage (NVIDIA) ---\n";
    FILE* fp = popen("nvidia-smi --query-gpu=utilization.gpu,memory.used --format=csv,noheader,nounits", "r");
    if (!fp) {
        std::cerr << "Failed to run nvidia-smi\n";
        return;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), fp)) {
        std::cout << "GPU Load %, Memory Used MB: " << buffer;
    }
    pclose(fp);
}

#endif // RESOURCE_USAGE_H