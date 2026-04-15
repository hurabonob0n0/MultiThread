#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>

const int MAX_THREADS = 32;

class NODE {
public:
	int value;
	NODE* next;
	NODE(int x) : next(nullptr), value(x) {}
};

class C_SET {
private:
	NODE* head, * tail;
	std::mutex mtx;
public:
	C_SET() {
		head = new NODE(std::numeric_limits<int>::min());
		tail = new NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~C_SET()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		mtx.lock();
		NODE* curr = head->next;
		while (curr != tail) {
			NODE* temp = curr;
			curr = curr->next;
			delete temp;
		}
		head->next = tail;
		mtx.unlock();
	}

	bool add(int x)
	{
		mtx.lock();	
		auto prev = head;
		auto curr = prev->next;

		while (curr->value < x) {
			prev = curr;
			curr = curr->next;
		}

		if (curr->value == x) {
			mtx.unlock();
			return false;
		}
		else {
			auto newNode = new NODE(x);
			newNode->next = curr;
			prev->next = newNode;
			mtx.unlock();	
			return true;
		}
	}

	bool remove(int x)
	{
		mtx.lock();
		NODE* pred = head;
		NODE* curr = pred->next;

		while (curr->value < x) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->value == x) {
			auto n = curr;
			pred->next = n->next;
			delete n;
			mtx.unlock();
			return true;
		}
		else {
			mtx.unlock();
			return false;
		}
	}

	bool contains(int x)
	{
		mtx.lock();
		NODE* pred = head;
		NODE* curr = pred->next;

		while (curr->value < x) {
			pred = curr;
			curr = curr->next;
		}
		mtx.unlock();
		if (curr->value == x) {
			
			return true;
		}
		else {

			return false;
		}
	}

	void print20()
	{
		auto curr = head->next;
		for (int i = 0; i < 20 && curr != tail; ++i) {
			std::cout << curr->value << ", ";
			curr = curr->next;
		}
		std::cout << std::endl;
	}
};

C_SET set;

void benchmark(const int num_threads)
{
	const int LOOP = 400'0000 / num_threads;
	const int RANGE = 1000;

	for (int i = 0; i < LOOP; ++i) {
		int value = rand() % RANGE;
		int op = rand() % 3;
		if (op == 0) set.add(value);
		else if (op == 1) set.remove(value);
		else set.contains(value);
	}
}

int main()
{
	using namespace std::chrono;
	for (int num_thread = 1; num_thread <= MAX_THREADS; num_thread *= 2) {
		set.clear();
		std::vector<std::thread> threads;
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_thread; ++i)
			threads.emplace_back(benchmark, num_thread);
		for (auto& th : threads)
			th.join();
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - start);
		std::cout << "Threads: " << num_thread
			<< ", Duration: " << duration.count() << " ms.\n";
		std::cout << "Set: "; set.print20();
	}
}

/*
Threads: 1, Duration: 992 ms.
Set: 0, 7, 9, 10, 13, 19, 20, 21, 25, 29, 32, 33, 36, 38, 40, 42, 43, 45, 46, 47,
Threads: 2, Duration: 1046 ms.
Set: 1, 2, 3, 8, 10, 12, 13, 23, 24, 30, 34, 35, 36, 38, 40, 42, 43, 44, 47, 48,
Threads: 4, Duration: 1604 ms.
Set: 1, 2, 3, 6, 9, 11, 13, 14, 16, 17, 18, 19, 21, 23, 24, 26, 33, 34, 35, 36,
Threads: 8, Duration: 14640 ms.
Set: 3, 8, 10, 11, 12, 13, 17, 18, 19, 20, 21, 22, 24, 29, 30, 32, 33, 34, 35, 36,
Threads: 16, Duration: 19410 ms.
Set: 1, 2, 4, 7, 8, 9, 10, 11, 12, 13, 15, 18, 19, 20, 21, 24, 25, 28, 29, 30,
Threads: 32, Duration: 19735 ms.
Set: 1, 2, 5, 8, 9, 10, 12, 14, 15, 16, 17, 18, 19, 22, 23, 25, 28, 29, 30, 32,
*/
