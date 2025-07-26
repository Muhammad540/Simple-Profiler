#define PROFILING_ENABLED 1
#include "../Profiler.h"

int fib(int n){
    PROFILE_FUNCTION();
    return (n < 2) ? n : fib(n-1) + fib(n-2);
}

int main(){
    PROFILE_SESSION("recursion.txt");
    fib(30);
    return 0;
}