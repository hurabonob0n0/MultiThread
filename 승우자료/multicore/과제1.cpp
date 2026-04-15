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

void worker(int thread_id, int loop_count) {
	for (int i = 0; i < 50000000 / loop_count; i++) {
		sum += 2;
	}
}

void mtx_worker(int thread_id, int loop_count) {
	for (int i = 0; i < 50000000 / loop_count; i++) {
		mtx.lock();
		sum += 2;
		mtx.unlock();
	}
}


class Bakery {
private:
	vector<bool> flag;
	vector<int> label;
	int num_threads;

public:
	Bakery(int n) : num_threads(n), flag(n), label(n) {
		for (int i = 0; i < num_threads; ++i) {
			flag[i] = false;
			label[i] = 0;
		}
	}

	void lock(int i) {
		flag[i] = true;

		int max_label = 0;
		for (int j = 0; j < num_threads; j++) {
			max_label = max(max_label, label[j]);
		}
		label[i] = max_label + 1;

		for (int j = 0; j < num_threads; j++) {
			if (j != i) {
				while (flag[j] && (label[j] < label[i] ||
					(label[j] == label[i] && j < i))) {
				}
			}
		}
	}

	void unlock(int i) {
		flag[i] = false;
	}
};

void bakery_worker(Bakery* bakery ,int thread_id, int loop_count) {
	bakery->lock(thread_id);
	for (int i = 0; i < 50'000'000 / loop_count; i++) {
		sum += 2;
	}
	bakery->unlock(thread_id);
}

class Bakery_Atomic {
private:
	vector<atomic<bool>> flag;
	vector<atomic<int>> label;
	int num_threads;

public:
	Bakery_Atomic(int n) : num_threads(n), flag(n), label(n) {
		for (int i = 0; i < num_threads; ++i) {
			flag[i] = false;
			label[i] = 0;
		}
	}

	void lock(int i) {
		flag[i] = true;

		int max_label = 0;
		for (int j = 0; j < num_threads; j++) {
			max_label = max(max_label, label[j].load());
		}
		label[i] = max_label + 1;

		for (int j = 0; j < num_threads; j++) {
			if (j != i) {
				while (flag[j] && (label[j] < label[i] ||
					(label[j] == label[i] && j < i))) {
				}
			}
		}
	}

	void unlock(int i) {
		flag[i] = false;
	}
};

void Atomic_bakery_worker(Bakery_Atomic* Abakery, int thread_id, int loop_count) {
	Abakery->lock(thread_id);
	for (int i = 0; i < 50000000 / loop_count; i++) {
		sum += 2;
	}
	Abakery->unlock(thread_id);
}

int main()
{
		// 아무것도 안했을때
	{
		cout << "아무것도 사용안함" << endl;
		for (int num_thread = 1; num_thread <= MAX_THREADS; num_thread = num_thread * 2) {
			sum = 0;
			vector<thread> threads;
			auto start_t = system_clock::now();
			for (int i = 0; i < num_thread; ++i) {
				threads.emplace_back(worker, i, num_thread);
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
		// 뮤텍스 사용
	{
		cout << "뮤텍스 사용" << endl;
		for (int num_thread = 1; num_thread <= MAX_THREADS; num_thread = num_thread * 2) {
			sum = 0;
			vector<thread> threads;
			auto start_t = system_clock::now();
			for (int i = 0; i < num_thread; ++i) {
				threads.emplace_back(mtx_worker, i, num_thread);
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
	{
		cout << "빵집 알고리즘 사용" << endl;
		// 빵집 알고리즘 사용
		for (int num_thread = 1; num_thread <= MAX_THREADS; num_thread = num_thread * 2) {
			sum = 0;
			Bakery bakery(num_thread);
			vector<thread> threads;
			auto start_t = system_clock::now();
			for (int i = 0; i < num_thread; ++i) {
				threads.emplace_back(bakery_worker, &bakery, i, num_thread);
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
	{
		cout << "Atomic 빵집 알고리즘 사용" << endl;
		// Atomic 빵집 알고리즘 사용
		for (int num_thread = 1; num_thread <= MAX_THREADS; num_thread = num_thread * 2) {
			sum = 0;
			Bakery_Atomic Abakery(num_thread);
			vector<thread> threads;
			auto start_t = system_clock::now();
			for (int i = 0; i < num_thread; ++i) {
				threads.emplace_back(Atomic_bakery_worker, &Abakery, i, num_thread);
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

/*
* 실행결과
아무것도 사용안함
1 Threads  , Execute Time : 78 ms sum = 100000000
2 Threads  , Execute Time : 75 ms sum = 54822316
4 Threads  , Execute Time : 46 ms sum = 42131478
8 Threads  , Execute Time : 48 ms sum = 23300800
뮤텍스 사용
1 Threads  , Execute Time : 1540 ms sum = 100000000
2 Threads  , Execute Time : 1882 ms sum = 100000000
4 Threads  , Execute Time : 2312 ms sum = 100000000
8 Threads  , Execute Time : 2728 ms sum = 100000000
빵집 알고리즘 사용
1 Threads  , Execute Time : 113 ms sum = 100000000
2 Threads  , Execute Time : 80 ms sum = 51563612
4 Threads  , Execute Time : 74 ms sum = 40805972
8 Threads  , Execute Time : 81 ms sum = 19993288
Atomic 빵집 알고리즘 사용
1 Threads  , Execute Time : 109 ms sum = 100000000
2 Threads  , Execute Time : 106 ms sum = 100000000
4 Threads  , Execute Time : 106 ms sum = 100000000
8 Threads  , Execute Time : 106 ms sum = 100000000
*/