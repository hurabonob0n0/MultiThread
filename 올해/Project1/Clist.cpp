#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

class NODE {
public:
	int data;
	NODE* next;
	NODE(int value) : data(value), next(nullptr) {}
};

class DUMMY_MUTEX {
public:
	void lock() {};
	void unlock() {};
};

class CLIST {
private:
	NODE* head, * tail;
	std::mutex mtx; // Mutex for thread safety
	//DUMMY_MUTEX mtx; // Dummy mutex for testing without locking

public:
	CLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}

	void clear()
	{
		NODE* current = head->next;
		while (head->next != tail) {
			NODE* temp = head->next;
			head->next = temp->next;
			delete temp;
		}
	}

	bool Add(int x)
	{
		mtx.lock(); // Lock the mutex to ensure thread safety
		NODE* pred = head;
		NODE* curr = pred->next;
		while (curr->data < x) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->data == x) {
			mtx.unlock(); // Unlock the mutex before returning
			return false; // Element already exists
		}
		else {
			NODE* new_node = new NODE{ x };
			pred->next = new_node;
			new_node->next = curr;
			mtx.unlock(); // Unlock the mutex after modifying the list
			return true; // Element added successfully
		}
	}

	bool Remove(int x)
	{
		mtx.lock();
		NODE* pred = head;
		NODE* curr = pred->next;
		while (curr->data < x) {
			pred = curr;
			curr = curr->next;
		}
		if (curr->data == x) {
			pred->next = curr->next;
			delete curr;
			mtx.unlock();
			return true; // Element removed successfully
		}
		else {
			mtx.unlock();
			return false;
		}
	}

	bool Contains(int x)
	{
		mtx.lock();
		NODE* curr = head->next;
		while (curr->data < x) {
			curr = curr->next;
		}
		if (curr->data == x) {
			mtx.unlock();
			return true; // Element found
		}
		mtx.unlock();
		return false;
	}

	void print20()
	{
		NODE* curr = head->next;
		int count = 0;
		while (curr != tail && count < 20) {
			std::cout << curr->data << ", ";
			curr = curr->next;
			count++;
		}
		std::cout << "\n";
	}
};

constexpr int MAX_THREADS = 16;
constexpr int NUM_TEST = 400'0000;

CLIST my_set;

void benchmark(int num_threads)
{
	const int LOOP = NUM_TEST / num_threads;
	for (int i = 0; i < LOOP; ++i) {
		int value = rand() % 1000;
		int op = rand() % 3;
		switch (op) {
		case 0:
			my_set.Add(value);
			break;
		case 1:
			my_set.Remove(value);
			break;
		case 2:
			my_set.Contains(value);
			break;
		}
	}
}

int main()
{
	using namespace std::chrono;
	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		std::vector<std::thread> threads;
		auto start_time = high_resolution_clock::now();
		for (int j = 0; j < num_threads; ++j) {
			threads.emplace_back(benchmark, num_threads);
		}
		for (auto& thread : threads) {
			thread.join();
		}
		auto end_time = high_resolution_clock::now();
		auto elapsed = end_time - start_time;
		auto exec_ms = duration_cast<milliseconds>(elapsed).count();
		my_set.print20();
		std::cout << "Threads: " << num_threads << ", Time: " << exec_ms << " miliseconds\n";
		my_set.clear();
	}
}


//0, 7, 9, 10, 13, 19, 20, 21, 25, 29, 32, 33, 36, 38, 40, 42, 43, 45, 46, 47,
//Threads: 1, Time : 1458 miliseconds
//1, 2, 3, 8, 10, 12, 13, 23, 24, 30, 34, 35, 36, 38, 40, 42, 43, 44, 47, 48,
//Threads : 2, Time : 1567 miliseconds
//1, 2, 3, 6, 9, 11, 13, 14, 16, 17, 18, 19, 21, 23, 24, 26, 33, 34, 35, 36,
//Threads : 4, Time : 2394 miliseconds
//3, 8, 10, 11, 12, 13, 17, 18, 19, 20, 21, 22, 24, 29, 30, 32, 33, 34, 35, 36,
//Threads : 8, Time : 8241 miliseconds
//1, 2, 4, 7, 8, 9, 10, 11, 12, 13, 15, 18, 19, 20, 21, 24, 25, 28, 29, 30,
//Threads : 16, Time : 9723 miliseconds