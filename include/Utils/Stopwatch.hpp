#ifndef UTILS_STOPWATCH_HPP
#define UTILS_STOPWATCH_HPP

#include <chrono>

class StopWatch {
private:
    static inline std::chrono::high_resolution_clock::time_point s_startTime;
    static inline bool s_Running = false;

public:
    static void Start() {
        s_startTime = std::chrono::high_resolution_clock::now();
        s_Running = true;
    }

    // returns the elapsed time in milliseconds
    static double Stop() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(endTime - s_startTime);

        s_Running = false;

        return static_cast<double>(elapsed.count()) / 1000.0;
    }

    static bool Running() {
        return s_Running;
    }
};

#endif // !UTILS_STOPWATCH_HPP