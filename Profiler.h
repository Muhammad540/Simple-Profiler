#pragma once 

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <chrono>
#include <thread>
#include <cinttypes>
#include <string>

#if defined(_MSC_VER)
    #include <intrin.h>
    #define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #include <x86intrin.h>
    #define FORCE_INLINE __attribute__((always_inline)) inline
#endif

using u32 = uint32_t;
using u64 = uint64_t;
using f64 = double;

/*
* Platform specific CPU timer.
* RDTSC is an instruction that reads the time stamp counter.
*/
FORCE_INLINE u64 ReadCPUTimer(void) {
    return __rdtsc();
}

/*
* Compile time string hashing
* This is important for Multiple Translation Units
*/
constexpr u32 hash_string(const char* str) {
    u32 hash = 2166136261u;
    while (*str) {
        hash ^= static_cast<u32>(*str++);
        hash *= 16777619u;
    }
    // this is to ensure that the hash is never 0, it is reserved for the root block
    return hash | 1;
}

// combine the file, line and label -> unique hash
#define PROFILE_HASH(label) (hash_string(__FILE__) ^ hash_string(label) ^ __LINE__)

struct ProfileStorage {
    u64 counter_elapsed;
    u64 children_counter_elapsed;
    u64 number_of_touches;
    const char* label;
    const char* function;
    const char* file;
    u32 line;
    u32 hash;
};

struct SimpleProfiler {
    static constexpr u32 MAX_STORAGE_SIZE = 4096;

    ProfileStorage storage[MAX_STORAGE_SIZE];
    u64 StartTimePoint;
    u64 EndTimePoint;
    u32 ParentIndex;

    /*
    * get a storage location by hash with
    * simple linear probing in case of collision
    */
    ProfileStorage* get_storage_location(u32 hash) {
        u32 index = hash % MAX_STORAGE_SIZE;
        // index 0 is reserved for the root block
        if (index == 0) {
            index = 1;
        }
        // attempts is used to avoid infinite loops and wrap around the storage array to find a free slot
        u32 attempts = 0;

        while (storage[index].label != nullptr) {
            if (storage[index].hash == hash) {
                return &storage[index];
            }
            index = (index + 1) % MAX_STORAGE_SIZE;
            if (index == 0) {
                index = 1;
            }
            attempts++;

            // we have checked all the slots 
            if (attempts > MAX_STORAGE_SIZE) {
                return nullptr;
            }
        }

        return &storage[index];
    }

    u32 get_storage_index(ProfileStorage* current_storage) {
        return static_cast<u32>(current_storage - storage);
    }
};

// a single instance of the profiler ( initialized to 0)
inline SimpleProfiler* global_profiler = nullptr;

static u64 EstimateCPUFreq() {
    auto start_time = std::chrono::high_resolution_clock::now();
    u64 start_tsc = ReadCPUTimer();
    
    // Sleep for known duration
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    u64 end_tsc = ReadCPUTimer();
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    double elapsed_seconds = elapsed.count() / 1e9;
    u64 elapsed_cycles = end_tsc - start_tsc;
    
    return (u64)(elapsed_cycles / elapsed_seconds);
}

static void EndAndPrintProfile(SimpleProfiler* profiler, const char* filename = "profile_results.txt") {
    profiler->EndTimePoint = ReadCPUTimer();
    u64 total_counter_elapsed = profiler->EndTimePoint - profiler->StartTimePoint;
    u64 cpu_freq = EstimateCPUFreq();

    FILE* file = fopen(filename, "w");
    if (!file) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    fprintf(file, "=== PROFILER RESULTS ===\n");
    time_t current_time = time(nullptr);
    fprintf(file, "Timestamp: %s", ctime(&current_time));

    if (cpu_freq > 0) {
        f64 total_time_ms = 1000.0 * (f64)total_counter_elapsed / (f64)cpu_freq;
        fprintf(file, "Total time: %.4fms (CPU freq ~%" PRIu64 " Hz)\n\n", total_time_ms, cpu_freq);
    } else {
        fprintf(file, "Total time: %" PRIu64 " cycles\n\n", total_counter_elapsed);
    }

    fprintf(file, "%-32s %-6s %-15s %-10s %-10s %-10s %-10s %s\n", 
        "Function/Block", "Hits", "Cycles", "ms (Total)", "ms (Exc)", "% Total", "% Excl.", "Location");
    fprintf(file, "%s\n", "--------------------------------"
                        "--------------------------------------------------------------------------------"
                        "----------------");
    for (u32 i = 0; i < SimpleProfiler::MAX_STORAGE_SIZE; i++) {
        ProfileStorage* storage = &profiler->storage[i];
        if (storage->counter_elapsed > 0) {
            u64 exclusive_time_cycles = storage->counter_elapsed - storage->children_counter_elapsed;
            
            f64 total_percent = 100.0 * (f64)storage->counter_elapsed / (f64)total_counter_elapsed;
            f64 exclusive_percent = 100.0 * (f64)exclusive_time_cycles / (f64)total_counter_elapsed;

            f64 total_ms = 0.0;
            f64 exclusive_ms = 0.0;
            if (cpu_freq > 0) {
                total_ms = 1000.0 * (f64)storage->counter_elapsed / (f64)cpu_freq;
                exclusive_ms = 1000.0 * (f64)exclusive_time_cycles / (f64)cpu_freq;
            }

            fprintf(file, "%-32s %-6" PRIu64 " %-15" PRIu64 " %-10.2f %-10.2f %-10.2f %-10.2f %s:%u\n",
                storage->label,
                storage->number_of_touches,
                storage->counter_elapsed,
                total_ms,
                exclusive_ms,
                total_percent,
                exclusive_percent,
                strrchr(storage->file, '/') ? strrchr(storage->file, '/') + 1 : storage->file,
                storage->line);
        }
    }

    fclose(file);   
    printf("Profile results saved to %s\n", filename);
}

class ProfilingSession {
    public:
        ProfilingSession(const std::string& filename = "profile_results.txt") : output_filename_(filename) {
            memset(&profiler_, 0, sizeof(profiler_));
            profiler_.StartTimePoint = ReadCPUTimer();
            global_profiler = &profiler_;
        }

        ~ProfilingSession() {
            EndAndPrintProfile(&profiler_, output_filename_.c_str());
            global_profiler = nullptr;
        }
    
    private:
        std::string output_filename_;
        SimpleProfiler profiler_;;
};

struct ProfileBlock {
    ProfileStorage* storage;
    u64 StartTimePoint;
    u32 parent_index;

    ProfileBlock(const char* label, const char* function, const char* file, u32 line) {
        if (global_profiler == nullptr) {
            // No active session, so this block should do nothing.
            storage = nullptr;
            return;
        }
        u32 hash = hash_string(file) ^ hash_string(label) ^ line;
        storage = global_profiler->get_storage_location(hash);

        if (storage == nullptr) {
            std::cerr << "Profiler storage is full, please increase MAX_STORAGE_SIZE" << std::endl;
            std::cerr << "Current size: " << SimpleProfiler::MAX_STORAGE_SIZE << std::endl;
            std::cerr << "This can be increased changing the value of MAX_STORAGE_SIZE in Profiler.h" << std::endl;
            return;
        }

        if (storage->label == nullptr) {
            storage->label = label;
            storage->function = function;
            storage->file = file;
            storage->line = line;
            storage->hash = hash;
        }
        
        parent_index = global_profiler->ParentIndex;
        global_profiler->ParentIndex = global_profiler->get_storage_index(storage);

        StartTimePoint = ReadCPUTimer();
    }

    ~ProfileBlock() {
        if (storage == nullptr) {
            return;
        }

        u64 elapsed = ReadCPUTimer() - StartTimePoint;
        // reset the parent index to the previous block
        global_profiler->ParentIndex = parent_index;

        // add the elapsed time to the children of the parent block
        global_profiler->storage[parent_index].children_counter_elapsed += elapsed;

        storage->counter_elapsed += elapsed;
        storage->number_of_touches++;
    }
};

// ============== MACROS ==============
#if PROFILING_ENABLED

#define PROFILE_CONCAT_IMPL(x, y) x##y
#define PROFILE_CONCAT(x, y) PROFILE_CONCAT_IMPL(x, y)

#define PROFILE_SESSION(name) ProfilingSession PROFILE_CONCAT(__prof_session_, __LINE__)(name)

#define PROFILE_SCOPE(name) \
    ProfileBlock PROFILE_CONCAT(__prof_block_, __LINE__)(name, __func__, __FILE__, __LINE__)

#define PROFILE_FUNCTION() \
    ProfileBlock PROFILE_CONCAT(__prof_block_, __LINE__)(__func__, __func__, __FILE__, __LINE__)

#else

#define PROFILE_SESSION(name)
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()

#endif