#include <iostream>
#include <future>
#include <vector>
#include <chrono>
#include <thread> // für hardware_concurrency()

constexpr unsigned int MAX_START = 100000000;

unsigned int collatz_length(unsigned long long n) {
    unsigned int length = 1;
    while (n != 1) {
        if (n % 2 == 0)
            n /= 2;
        else
            n = 3 * n + 1;
        length++;
    }
    return length;
}

std::pair<unsigned int, unsigned int> collatz_worker(unsigned int start, unsigned int end) {
    unsigned int local_max_length = 0;
    unsigned int local_max_start = 0;

    for (unsigned int i = start; i <= end; ++i) {
        unsigned int len = collatz_length(i);
        if (len > local_max_length) {
            local_max_length = len;
            local_max_start = i;
        }
    }

    return {local_max_start, local_max_length};
}

int main() {
    // Ermittlung der Anzahl verfügbarer Kerne
    unsigned int NUM_TASKS = std::thread::hardware_concurrency();
    if (NUM_TASKS == 0) NUM_TASKS = 4;  // Fallback, falls 0 zurückgegeben wird

    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::future<std::pair<unsigned int, unsigned int>>> futures;
    unsigned int chunk_size = MAX_START / NUM_TASKS;

    for (unsigned int i = 0; i < NUM_TASKS; ++i) {
        unsigned int range_start = i * chunk_size + 1;
        unsigned int range_end = (i == NUM_TASKS - 1) ? MAX_START : (i + 1) * chunk_size;
        futures.emplace_back(std::async(std::launch::async, collatz_worker, range_start, range_end));
    }

    unsigned int global_max_start = 0;
    unsigned int global_max_length = 0;

    for (auto& fut : futures) {
        auto [start, length] = fut.get();
        if (length > global_max_length) {
            global_max_length = length;
            global_max_start = start;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "Tasks verwendet:       " << NUM_TASKS << std::endl;
    std::cout << "Längste Collatz-Folge: Start = " << global_max_start
              << ", Länge = " << global_max_length << std::endl;
    std::cout << "Laufzeit:              " << elapsed.count() << " Sekunden\n";

    return 0;
}
//g++ -std=c++17 -O3 -pthread -o collatz_async collatz_async.cpp
