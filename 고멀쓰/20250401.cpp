#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <Windows.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

volatile int sum;
volatile int Lock;

std::chrono::steady_clock::time_point g_StartTime;

inline void Timer_Start() { g_StartTime = high_resolution_clock::now(); }
inline void elapsed_time() {
	auto d = high_resolution_clock::now() - g_StartTime;
	std::cout << duration_cast<milliseconds>(d).count() << " msecs\n";
}

void CAS_LOCK()
{
	//InterlockedCompareExchange
}

void CAS_UNLOCK()
{

}

void threadFunc(int threadnum)
{
	for (int i = 0; i < 50000000 / threadnum; ++i)
	{
		CAS_LOCK();
		sum += 2;
		CAS_UNLOCK();
	}
}

int main()
{

}