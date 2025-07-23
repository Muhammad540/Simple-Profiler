// IMPORTANT: Define this BEFORE including the profiler to enable it.
#define PROFILING_ENABLED 1
#include "../Profiler.h"

#include <cmath>

void short_work() {
     // Time this entire function
    PROFILE_FUNCTION();

    volatile double result = 0.0;
    for (int i = 0; i < 5000; ++i) {
        result += sqrt((double)i);
    }
}

// A function that calls another profiled function within a loop.
void long_work() {
    // Time this entire function
    PROFILE_FUNCTION(); 

    {
        // Time just this specific block
        PROFILE_SCOPE("Inner Loop"); 
        for (int i = 0; i < 100; ++i) {
            // This call will be part of long_work's inclusive time
            short_work(); 
        }
    }

    // This section's time will contribute to long_work's *exclusive* time.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// Another top-level function to test.
void another_function() {
    PROFILE_FUNCTION();

    {
        PROFILE_SCOPE("Waiting");
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    // Call short_work again from a different place
    short_work(); 
}


int main() {
    // 1. Start the profiling session
    PROFILE_SESSION("Testing Profiler.txt");
    // Profile the main function itself
    PROFILE_FUNCTION(); 

    // 2. Run our test functions
    long_work();
    another_function();

    // 3. A final scope in main
    {
        PROFILE_SCOPE("Final Cleanup");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    return 0;
}