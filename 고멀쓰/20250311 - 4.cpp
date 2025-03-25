#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>

using namespace std;
using namespace chrono;

volatile int sum = 0;

atomic<int> a_sum = 0;

mutex am;

void worker(int num) {
	for (auto i = 0; i < 50000000 / num; ++i)
	{
		am.lock();
		sum += 2;
		am.unlock();
	}
}

void a_worker(int num) {
	for (auto i = 0; i < 50000000 / num; ++i)
	{
		a_sum += 2;
	}
}


void Optimistic_worker(int num) {
	volatile int local_sum = 0;

	for (auto i = 0; i < 50000000 / num; ++i)
	{
		local_sum = local_sum + 2;
	}
	am.lock();
	sum += local_sum;
	am.unlock();
}


int main()
{
	{
		auto start_t = system_clock::now();

		for (auto i = 0; i < 50000000; ++i)
		{
			sum += 2;
		}

		auto end_t = system_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();

		cout << "Single Thread : Sum = " << sum;
		cout << ",Execute Time : " << exec_ms << " ms" << endl;
	}


	{

		for (int i = 1; i <= 16; i = i * 2) {
			sum = 0;
			vector<thread> threads;
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j) {
				threads.emplace_back(Optimistic_worker, i);
			}
			for (auto& th : threads) {
				th.join();
			}
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			cout << i << " Threads : Optimistic atomic Sum = " << sum;
			cout << " , Execute Time : " << exec_ms << " ms" << endl;
		}
	}

}