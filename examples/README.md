# Simple Profiler: Example Usage

This directory contains a sample C++ application (`main.cpp`) to demonstrate how to use the Simple Profiler and interpret its output.

The `main.cpp` file showcases several key features:
*   Profiling the `main` function itself.
*   Profiling individual functions and blocks of code using macros like `PROFILE_FUNCTION` and `PROFILE_SCOPE`.
*   How parent/child time is calculated when profiled functions call each other.
*   How the profiler aggregates data for functions that are called multiple times from different places.

## How to Run the Example

You can compile and run the example with a simple command. From within the `Simple-Profiler/examples` directory, execute the following command in your terminal:

```bash
g++ -std=c++17 -O2 -o profiler_test main.cpp && ./profiler_test
```

After running, the profiler will generate a file named `Testing Profiler.txt` in this directory with the performance results.

## Example Output

Running the test will produce the following output file similar to this:

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

## Understanding the Results

The output table gives you a detailed breakdown of where your program spent its time. Here is an explanation of each column and what the results tell us.

### Key Terminology

*   **Hits:** The total number of times a specific function or scope was executed.
*   **Cycles:** The raw number of CPU cycles spent in a block, as measured by the `RDTSC` instruction. 
*   **ms (Total) / % Total:** The *total* time spent in a block, **including** the time of any other profiled blocks it called (its "children"). This is also known as **Inclusive Time**.
*   **ms (Exc) / % Excl.:** The time spent *only* within a block, **excluding** the time of its profiled children. This is also known as **Exclusive Time**. 
*   **Location:** The source file and line number where the profiled block was defined.

### Analysis of the Example Output

*   **`main` is the Root:** The `main` block ran once, took `41.65ms` (`100% Total`), and had `0ms` of exclusive time. This is correct, as all of its time was spent calling its children (`long_work`, `another_function`, etc.).

*   **Inclusive vs. Exclusive Time:** Look at `long_work`.
    *   Its **Total** time is `11.50ms`.
    *   Its **Exclusive** time is only `10.09ms`.
    *   The difference (`1.41ms`) is the time spent in its child, `"Inner Loop"`. This shows how you can distinguish between time spent *in* a function versus time spent in other functions it calls.

*   **Finding Hotspots:** The `Waiting` block and `another_function` take up over `60%` of the total runtime. The exclusive time for `Waiting` is `25.06ms`, telling us that the `std::this_thread::sleep_for` call on line 40 is the source of that time.

*   **Aggregated Calls:** Notice `short_work` has **101 hits**. The profiler correctly detected that it was called 100 times from inside `long_work` and once from `another_function`, and aggregated all the timing data into a single, convenient entry.
