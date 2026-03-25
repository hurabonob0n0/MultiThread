#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;
using namespace std::chrono;

std::mutex am;
volatile int sum;

volatile int victim = 0;
volatile bool flag[2] = { false, false };
//atomic<int> victim = 0;
//atomic<bool> flag[2] = { false, false };

void p_lock(int myID)
{
	int other = 1 - myID;
	//am.lock();
	flag[myID] = true;
	//_asm mfence;
	atomic_thread_fence(memory_order_seq_cst);
	victim = myID;
	//am.unlock();
	//am.lock();
	//_asm mfence;
	atomic_thread_fence(memory_order_seq_cst);
	while (flag[other] && victim == myID) {}// am.unlock(); am.lock(); }
	//am.unlock();
}
void p_unlock(int myID)
{
	am.lock();
	flag[myID] = false;
	am.unlock();
}


void worker(/*const int num_threads,*/ const int thread_id)
{
	for (int i = 0; i < 25000000/*50000000 / num_threads*/; ++i)
	{
		//am.lock();
		p_lock(thread_id);
		sum += 2;
		p_unlock(thread_id);
		//am.unlock();
	}
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
		sum = 0;

		auto start_t = system_clock::now();
		std::thread t1{ worker,0 };
		std::thread t2{ worker,1 };
		t1.join();
		t2.join();
		auto end_t = system_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();

		cout << "2 Threads , Sum = " << sum;
		cout << " , Execute Time : " << exec_ms << " ms" << endl;
	}
}
