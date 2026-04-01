#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

class NODE {
public:
	int data;
	NODE* next;
	std::mutex mtx;
	NODE(int value) : data(value), next(nullptr) {}

public:
	void lock() {
		mtx.lock();
	}
	void unlock() {
		mtx.unlock();
	}
};

class DUMMY_MUTEX {
public:
	void lock() {};
	void unlock() {};
};


class FLIST {
private:
	NODE* head, * tail;
	//DUMMY_MUTEX mtx; // Dummy mutex for testing without locking

public:
	FLIST()
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
		head->lock(); // Lock the mutex to ensure thread safety
		NODE* pred = head;
		NODE* curr = pred->next;
		curr->lock(); // Lock the current node's mutex before accessing it
		while (curr->data < x) {
			pred->unlock(); // Unlock the previous node's mutex before moving forward
			pred = curr;
			curr = curr->next;
			curr->lock(); // Lock the next node's mutex before accessing it
		}

		if (curr->data == x) {
			pred->unlock();
			curr->unlock(); // Unlock the mutex before returning
			return false; // Element already exists
		}
		else {
			NODE* new_node = new NODE{ x };
			pred->next = new_node;
			new_node->next = curr;
			pred->unlock();
			curr->unlock(); // Unlock the mutex before returning
			return true; // Element added successfully
		}
	}

	bool Remove(int x)
	{
		head->lock(); // Lock the mutex to ensure thread safety
		NODE* pred = head;
		NODE* curr = pred->next;
		curr->lock();
		while (curr->data < x) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}
		if (curr->data == x) {
			pred->next = curr->next;
			pred->unlock();
			curr->unlock();
			delete curr;
			return true; // Element removed successfully
		}
		else {
			pred->unlock();
			curr->unlock();
			return false;
		}
	}

	bool Contains(int x)
	{
		
		NODE* curr = head->next;
		curr->lock();
		while (curr->data < x) {
			curr = curr->next;
		}
		if (curr->data == x) {
			curr->unlock();
			return true; // Element found
		}
		curr->unlock();
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

FLIST my_set;

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