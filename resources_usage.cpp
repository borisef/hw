#include "resources_usage.h"


// Dummy workload
void my_function() {
    std::vector<int> data(1e7, 1);
    long long sum = std::accumulate(data.begin(), data.end(), 0LL);
    std::cout << "Work result (sum): " << sum << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1)); // simulate delay
}


// Test wrapper
void testFunction() {
    printMemoryUsage();
    printCPUUsage();
    printGPUUsage();

    auto start = std::chrono::high_resolution_clock::now();

    my_function();

    auto end = std::chrono::high_resolution_clock::now();

    printRuntime(start, end);
    printMemoryUsage();
    printCPUUsage();
    printGPUUsage();
}

int main() {
    testFunction();
    return 0;
}
