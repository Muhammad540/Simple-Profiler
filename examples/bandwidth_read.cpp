#include <iostream>
#include <stdio.h>
#include <sys/stat.h>

#define PROFILING_ENABLED 1
#include "../Profiler.h"

struct buffer{
    size_t size;
    uint8_t* data;
};

buffer AllocateBuffer(size_t size){
    buffer Result = {};
    Result.data = new uint8_t[size];
    if (Result.data){
        Result.size = size;
    } else {
        printf("Failed to allocate buffer\n");
    }
    return Result;
}

void ReadFile(const char* filename){
    PROFILE_FUNCTION();

    buffer Result = {};

    FILE* file = fopen(filename, "rb");
    if (file){
        struct stat file_stat;
        stat(filename, &file_stat);

        Result = AllocateBuffer(file_stat.st_size);
        if (Result.data){
            PROFILE_BANDWIDTH("ReadFile", file_stat.st_size);  
            printf("file_stat.st_size: %zu\n", file_stat.st_size);
            if (fread(Result.data, Result.size, 1, file) != 1){
                printf("Failed to read file\n");
                if (Result.data){
                    delete[] Result.data;
                }
                Result.data = nullptr;
                Result.size = 0;
            }
        }
        fclose(file);
    } else {
        printf("Failed to open file\n");
    }
}

int main(){
    PROFILE_SESSION("bandwidth_test.txt");

    ReadFile("sample.txt");

    return 0;
}