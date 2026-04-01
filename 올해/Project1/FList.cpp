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




class FLIST {
private:
	NODE* head, * tail;
	
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
		auto prev = head;
		prev->lock();
		auto curr = prev->next;
		curr->lock();
		while (curr->data < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->data == x) {
			prev->unlock();	curr->unlock();
			return false;
		}
		else {
			auto newNode = new NODE(x);
			newNode->next = curr;
			prev->next = newNode;
			prev->unlock();	curr->unlock();
			return true;
		}
	}

	bool Remove(int x)
	{
		auto prev = head;
		prev->lock();
		auto curr = prev->next;
		curr->lock();
		while (curr->data < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->data != x) {
			prev->unlock();	curr->unlock();
			return false;
		}
		else {
			prev->next = curr->next;
			prev->unlock();	curr->unlock();
			delete curr;
			return true;
		}
	}

	bool Contains(int x)
	{
		auto prev = head;
		prev->lock();
		auto curr = prev->next;
		curr->lock();
		while (curr->data < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->data == x) {
			prev->unlock();	curr->unlock();
			return true;
		}
		else {
			prev->unlock();	curr->unlock();
			return false;
		}
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