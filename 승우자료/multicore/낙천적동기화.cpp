#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>

const int MAX_THREADS = 16;

class NODE {
public:
	int value;
	NODE* next;
	std::mutex mtx;
	NODE(int x) : next(nullptr), value(x) {}
	void lock() { mtx.lock(); }
	void unlock() { mtx.unlock(); }
};

class DUMMY_MTX {
public:
	void lock() {}
	void unlock() {}
};

class C_SET {
private:
	NODE* head, * tail;
	DUMMY_MTX mtx;
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
		NODE* curr = head->next;
		while (curr != tail) {
			NODE* temp = curr;
			curr = curr->next;
			delete temp;
		}
		head->next = tail;
	}

	bool add(int x)
	{
		auto prev = head;
		mtx.lock();
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
		auto prev = head;
		mtx.lock();
		auto curr = prev->next;

		while (curr->value < x) {
			prev = curr;
			curr = curr->next;
		}

		if (curr->value != x) {
			mtx.unlock();
			return false;
		}
		else {
			prev->next = curr->next;
			mtx.unlock();
			delete curr;
			return true;
		}
	}

	bool contains(int x)
	{
		auto prev = head;
		mtx.lock();
		auto curr = prev->next;

		while (curr->value < x) {
			prev = curr;
			curr = curr->next;
		}

		if (curr->value == x) {
			mtx.unlock();
			return true;
		}
		else {
			mtx.unlock();
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

class F_SET {
private:
	NODE* head, * tail;
public:
	F_SET() {
		head = new NODE(std::numeric_limits<int>::min());
		tail = new NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~F_SET()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		NODE* curr = head->next;
		while (curr != tail) {
			NODE* temp = curr;
			curr = curr->next;
			delete temp;
		}
		head->next = tail;
	}

	bool add(int x)
	{
		auto prev = head;
		prev->lock();
		auto curr = prev->next;
		curr->lock();
		while (curr->value < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->value == x) {
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

	bool remove(int x)
	{
		auto prev = head;
		prev->lock();
		auto curr = prev->next;
		curr->lock();
		while (curr->value < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->value != x) {
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

	bool contains(int x)
	{
		auto prev = head;
		prev->lock();
		auto curr = prev->next;
		curr->lock();
		while (curr->value < x) {
			prev->unlock();
			prev = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->value == x) {
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
		auto curr = head->next;
		for (int i = 0; i < 20 && curr != tail; ++i) {
			std::cout << curr->value << ", ";
			curr = curr->next;
		}
		std::cout << std::endl;
	}
};

class O_SET {
private:
	NODE* head, * tail;
public:
	O_SET() {
		head = new NODE(std::numeric_limits<int>::min());
		tail = new NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~O_SET()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		NODE* curr = head->next;
		while (curr != tail) {
			NODE* temp = curr;
			curr = curr->next;
			delete temp;
		}
		head->next = tail;
	}

	bool validate(int x, NODE* p, NODE* c)
	{
		auto prev = head;
		auto curr = prev->next;
		while (curr->value < x) {
			prev = curr;
			curr = curr->next;
		}
		return ((prev == p) && (curr == c));
	}

	bool add(int x)
	{
		while (true) {
			auto prev = head;
			auto curr = prev->next;
			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (false == validate(x, prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}
			if (curr->value == x) {
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
	}

	bool remove(int x)
	{
		while (true) {
			auto prev = head;
			auto curr = prev->next;
			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (false == validate(x, prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}
			if (curr->value != x) {
				prev->unlock();	curr->unlock();
				return false;
			}
			else {
				prev->next = curr->next;
				prev->unlock();	curr->unlock();
				//delete curr;
				return true;
			}
			
		}
	}

	bool contains(int x)
	{
		while (true) {
			auto prev = head;
			auto curr = prev->next;
			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
			}
			prev->lock(); curr->lock();
			if (false == validate(x, prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}

			if (curr->value == x) {
				prev->unlock();	curr->unlock();
				return true;
			}
			else {
				prev->unlock();	curr->unlock();
				return false;
			}
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
	for (int num_thread = 16; num_thread >= 1; num_thread /= 2) {
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
* 낙천적 동기화
Threads: 16, Duration: 1480 ms.
Set: 1, 2, 4, 7, 8, 9, 10, 11, 12, 13, 15, 18, 19, 20, 21, 24, 25, 28, 29, 30,
Threads: 8, Duration: 1403 ms.
Set: 3, 8, 10, 11, 12, 13, 17, 18, 19, 20, 21, 22, 24, 29, 30, 32, 33, 34, 35, 36,
Threads: 4, Duration: 2919 ms.
Set: 1, 2, 3, 6, 9, 11, 13, 14, 16, 17, 18, 19, 21, 23, 24, 26, 33, 34, 35, 36,
Threads: 2, Duration: 4053 ms.
Set: 1, 2, 3, 8, 10, 12, 13, 23, 24, 30, 34, 35, 36, 38, 40, 42, 43, 44, 47, 48,
Threads: 1, Duration: 5053 ms.
Set: 0, 7, 9, 10, 13, 19, 20, 21, 25, 29, 32, 33, 36, 38, 40, 42, 43, 45, 46, 47,


* 세밀한 동기화
Threads: 16, Duration: 152223 ms.
Set: 1, 2, 4, 7, 8, 9, 10, 11, 12, 13, 15, 18, 19, 20, 21, 24, 25, 28, 29, 30,
Threads: 8, Duration: 135632 ms.
Set: 3, 8, 10, 11, 12, 13, 17, 18, 19, 20, 21, 22, 24, 29, 30, 32, 33, 34, 35, 36,
Threads: 4, Duration: 48187 ms.
Set: 1, 2, 3, 6, 9, 11, 13, 14, 16, 17, 18, 19, 21, 23, 24, 26, 33, 34, 35, 36,
Threads: 2, Duration: 23376 ms.
Set: 1, 2, 3, 8, 10, 12, 13, 23, 24, 30, 34, 35, 36, 38, 40, 42, 43, 44, 47, 48,
Threads: 1, Duration: 25692 ms.
Set: 0, 7, 9, 10, 13, 19, 20, 21, 25, 29, 32, 33, 36, 38, 40, 42, 43, 45, 46, 47,

*/