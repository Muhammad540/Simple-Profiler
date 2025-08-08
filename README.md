# ![Logo](logo.png)

# A Simple C++ Profiler

A header only, RAII based, low overhead profiler for C++ projects. It's designed for ease of use, requiring no complex build steps and providing a clear, hierarchical view of your application's performance.

## Features

-   [x] **Header Only:** No complicated build steps. Just `#include "profiler.h"` in your project.
-   [x] **RAII Based:** Timing starts and stops automatically managed automatically by object lifetimes. No need to manually manage timers.
-   [x] **Zero Overhead When Disabled:** Compiles to nothing in release builds for zero performance impact.
-   [x] **Multi Translation Unit Support:** Works across multiple `.cpp` files.
-   [x] **Simple API:** Use `PROFILE_FUNCTION()` and `PROFILE_SCOPE("name")` to instrument your code.
-   [x] **Function Nesting Accuracy:** Accurately calculates Total (Inclusive) vs. Exclusive time by tracking the call hierarchy.
-   [x] **Output File:** Stores profiling results in a text file whose name can be specified at runtime. The output is organized in a clear, tabular format for easy analysis.
-   [x] **Bandwidth Profiling:** Track the bandwidth of data transfers for scoped regions.
-   [x] **Recursive Function Support:** Correctly handle timing for recursive calls.

## How to Use

### 1. Enable Profiling
Define `PROFILING_ENABLED` to `1` **before** including the header. It is highly recommended to set this in your build system (e.g., in `CMakeLists.txt` or via a compiler flag like `-DPROFILING_ENABLED=1`) for debug builds only.

### 2. Start a Session
Create a `ProfilingSession` object at the beginning of your application entry point (typically `main`). The session will automatically start timing when it's created and write the results to a file when it's destroyed at the end of the scope.

The easiest way to do this is with the `PROFILE_SESSION` macro.

```cpp
#define PROFILING_ENABLED 1
#include "Profiler.h"

int main() {
    // Starts the session. Results will be saved to "profile_results.txt" when main exits.
    PROFILE_SESSION("profile_results.txt");

    // Your application code here...
    return 0;
}
```

### 3. Instrument Your Code
Use these macros to mark functions and code blocks for timing:

-   `PROFILE_FUNCTION()`: Place this at the top of a function to time its entire scope. The function's name is used as the label automatically.
-   `PROFILE_SCOPE("My Custom Name")`: Use this to time any specific, smaller block of code within a function.

### 4. Example
For a complete, runnable example, please see the `main.cpp` file inside the `/examples` directory. You can also find a README in the `examples` directory with more details.

## Understanding the Output

The profiler generates a clean, tabulated text file that helps you quickly identify bottlenecks. A (.txt) profiler output can look like this:

### Profiling Output:
```text
=== PROFILER RESULTS ===
Timestamp: Wed Jul 23 03:01:12 2025
Total time: 41.6512ms (CPU freq ~3193819522 Hz)

Function/Block                   Hits   Cycles          ms (Total) ms (Exc)   % Total    % Excl.    Location
------------------------------------------------------------------------------------------------------------------
another_function                 1      80121952        25.09      0.00       60.23      0.00       main.cpp:37
main                             1      133022336       41.65      0.00       100.00     0.00       main.cpp:52
long_work                        1      36721760        11.50      10.09      27.60      24.22      main.cpp:20
short_work                       101    4574240         1.43       1.43       3.44       3.44       main.cpp:9
Inner Loop                       1      4509120         1.41       0.00       3.39       0.01       main.cpp:24
Final Cleanup                    1      16176352        5.06       5.06       12.16      12.16      main.cpp:60
Waiting                          1      80040480        25.06      25.06      60.17      60.17      main.cpp:40
```

### Bandwidth Profiling Output:

```text
=== PROFILER RESULTS ===
Timestamp: Fri Aug  8 23:46:23 2025
Total time: 0.0476 ms (CPU freq ~3193855563 Hz)

Function/Block                   Hits   Cycles          ms (Total) ms (Exc)   % Total    % Incl.    B/s (MB)   B/s (GB)   Location
---------------------------------------------------------------------------------------------------------------------------------------------------
ReadFile                         1      148864          0.05       0.01       30.37      97.92      0.00       0.00       bandwidth_read.cpp:25
ReadFile                         1      102688          0.03       0.03       67.54      67.54      1128.03    1.10       bandwidth_read.cpp:36
```

-   **Hits:** The number of times this block was executed. Note `short_work` was hit 101 times and the data was aggregated automatically.
-   **Cycles:** The raw number of CPU cycles spent in a block.
-   **ms (Total) / % Total:** The *total* time spent in a block, **including** the time of any other profiled blocks it called (its "children"). Also known as **Inclusive Time**.
-   **ms (Exc) / % Excl.:** The time spent *only* within a block, **excluding** the time of its profiled children. Also known as **Exclusive Time**. This is the key metric for finding the actual source of a slowdown.
-   **Location:** The source file and line number where the profile block was defined.

## Disabling in Release Builds
To compile out all profiling code, simply ensure `PROFILING_ENABLED` is **not** defined as `1` (or is defined as `0`). All profiling macros will expand to nothing, resulting in zero performance impact.


## Future Improvements

-   [ ] **Thread Safety:** Add support for profiling multi-threaded applications.
-   [ ] **Recursive Function Support:** Correctly handle timing for recursive calls.
-   [ ] **Sorted Output:** Add an option to sort results by time to easily spot bottlenecks.
-   [ ] **Configurable Storage:** Allow the maximum number of profile points (default: 4096) to be configured easily.
-   [ ] **Alternative Output Formats:** Support for JSON or CSV for easier integration with other tools.
-   [ ] **Hierarchical Output:** Shows parent child relationships