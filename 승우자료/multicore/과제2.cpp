#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <mutex>

using namespace std;
using namespace chrono;

const int MAX_THREADS = 8;
volatile int sum = 0;
mutex mtx;

atomic<bool> lock_flag(false);

bool CAS(atomic_bool *mem, bool old_value, bool new_value_) {
	return atomic_compare_exchange_strong(mem, &old_value, new_value_);
}

void cas_lock() {
	while (!CAS(&lock_flag, false, true)) {
		this_thread::yield();
	}
	
}

void cas_unlock() {
	lock_flag.store(false);
}



void worker3(const int thread_id, const int loop_count) {
	for (int i = 0; i < 50000000 / loop_count; i++) {
		cas_lock();
		sum += 2;
		cas_unlock();
	}
}



int main()
{
	{
		for (int num_thread = 1; num_thread <= MAX_THREADS; num_thread = num_thread * 2) {
			sum = 0;
			vector<thread> threads;
			auto start_t = system_clock::now();
			for (int i = 0; i < num_thread; ++i) {
				threads.emplace_back(worker3, i, num_thread);
			}
			for (int i = 0; i < num_thread; i++) {
				threads[i].join();
			}
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			cout << num_thread << " Threads " << " , Execute Time : " << exec_ms << " ms" << " sum = " << sum << endl;
		}
	}
		
}