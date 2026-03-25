#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>

using namespace std;
using namespace std::chrono;

std::chrono::steady_clock::time_point g_StartTime;

inline void Timer_Start() { g_StartTime = high_resolution_clock::now(); }
inline void elapsed_time() {
	auto d = high_resolution_clock::now() - g_StartTime;
	std::cout << duration_cast<milliseconds>(d).count() << " msecs\n";
}