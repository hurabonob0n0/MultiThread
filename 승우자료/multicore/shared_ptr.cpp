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
	NODE* volatile next;
	std::mutex mtx;
	volatile bool removed;
	NODE(int x) : next(nullptr), value(x), removed(false) {}
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

class L_SET {
private:
	NODE* head, * tail;
public:
	L_SET() {
		head = new NODE(std::numeric_limits<int>::min());
		tail = new NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~L_SET()
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
		return (p->removed == false)
			&& (c->removed == false)
			&& (p->next == c);
	}

	bool add(int x)
	{

	}

	bool remove(int x)
	{

	}

	bool contains(int x)
	{

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

#include <queue>

class L_SET_FL {
private:
	NODE* head, * tail;
	std::queue <NODE*> free_list;
	std::mutex fl_mtx;
public:
	void my_delete(NODE* node) {
		std::lock_guard<std::mutex> lg(fl_mtx);
		free_list.push(node);
	}

	void recycle() {
		while (false == free_list.empty()) {
			auto node = free_list.front();
			free_list.pop();
			delete node;
		}
	}

	L_SET_FL() {
		head = new NODE(std::numeric_limits<int>::min());
		tail = new NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~L_SET_FL()
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
		return (p->removed == false)
			&& (c->removed == false)
			&& (p->next == c);
	}

	bool add(int x)
	{

	}

	bool remove(int x)
	{

	}

	bool contains(int x)
	{

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

class NODE_SP {
public:
	int value;
	std::shared_ptr<NODE_SP> next;
	std::mutex mtx;
	volatile bool removed;
	NODE_SP(int x) : next(nullptr), value(x), removed(false) {}
	void lock() { mtx.lock(); }
	void unlock() { mtx.unlock(); }
};

class L_SET_SP {
private:
	std::shared_ptr<NODE_SP> head, tail;
public:
	L_SET_SP() {
		head = std::make_shared<NODE_SP>(std::numeric_limits<int>::min());
		tail = std::make_shared<NODE_SP>(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~L_SET_SP()
	{
	}

	void recycle()
	{
	}

	void clear()
	{
		head->next = tail;
	}

	bool validate(const std::shared_ptr<NODE_SP>& p,
		const std::shared_ptr<NODE_SP>& c)
	{
		return (p->removed == false)
			&& (c->removed == false)
			&& (std::atomic_load(&p->next) == c);
	}

	bool add(int x)
	{
		while (true) {
			auto prev = head;
			auto curr = std::atomic_load(&prev->next);
			while (curr->value < x) {
				prev = curr;
				curr = std::atomic_load(&curr->next);
			}

			prev->lock(); curr->lock();
			if (false == validate(prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}
			if (curr->value == x) {
				prev->unlock();	curr->unlock();
				return false;
			}
			else {
				auto newNode = std::make_shared<NODE_SP>(x);
				newNode->next = curr;
				std::atomic_exchange(&prev->next , newNode);
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
				curr = std::atomic_load(&curr->next);
			}

			prev->lock(); curr->lock();
			if (false == validate(prev, curr)) {
				prev->unlock(); curr->unlock();
				continue;
			}
			if (curr->value != x) {
				prev->unlock();	curr->unlock();
				return false;
			}
			else {
				curr->removed = true;
				std::atomic_exchange(&prev->next , std::atomic_load(&curr->next));
				prev->unlock();	curr->unlock();
				//delete curr;
				return true;
			}
		}
	}

	bool contains(int x)
	{
		auto curr = head;
		while (curr->value < x) {
			curr = std::atomic_load(&curr->next);
		}
		return (curr->removed == false) && (curr->value == x);
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




L_SET_SP set;

const int LOOP = 400'0000;
const int RANGE = 1000;

#include <array>

class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

std::array<std::vector<HISTORY>, MAX_THREADS> history;

void check_history(int num_threads)
{
	std::array <int, RANGE> survive = {};
	std::cout << "Checking Consistency : ";
	if (history[0].size() == 0) {
		std::cout << "No history.\n";
		return;
	}
	for (int i = 0; i < num_threads; ++i) {
		for (auto& op : history[i]) {
			if (false == op.o_value) continue;
			if (op.op == 3) continue;
			if (op.op == 0) survive[op.i_value]++;
			if (op.op == 1) survive[op.i_value]--;
		}
	}
	for (int i = 0; i < RANGE; ++i) {
		int val = survive[i];
		if (val < 0) {
			std::cout << "ERROR. The value " << i << " removed while it is not in the set.\n";
			exit(-1);
		}
		else if (val > 1) {
			std::cout << "ERROR. The value " << i << " is added while the set already have it.\n";
			exit(-1);
		}
		else if (val == 0) {
			if (set.contains(i)) {
				std::cout << "ERROR. The value " << i << " should not exists.\n";
				exit(-1);
			}
		}
		else if (val == 1) {
			if (false == set.contains(i)) {
				std::cout << "ERROR. The value " << i << " shoud exists.\n";
				exit(-1);
			}
		}
	}
	std::cout << " OK\n";
}

void benchmark_check(int num_threads, int th_id)
{
	for (int i = 0; i < LOOP / num_threads; ++i) {
		int op = rand() % 3;
		switch (op) {
		case 0: {
			int v = rand() % RANGE;
			history[th_id].emplace_back(0, v, set.add(v));
			break;
		}
		case 1: {
			int v = rand() % RANGE;
			history[th_id].emplace_back(1, v, set.remove(v));
			break;
		}
		case 2: {
			int v = rand() % RANGE;
			history[th_id].emplace_back(2, v, set.contains(v));
			break;
		}
		}
	}
}
void benchmark(const int num_threads)
{
	for (int i = 0; i < LOOP / num_threads; ++i) {
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
	// 싱글 스래드에서 돌리는 경우
	{
		{
			set.clear();
			std::vector<std::thread> threads;
			auto start = high_resolution_clock::now();
			benchmark(1);
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<milliseconds>(stop - start);
			std::cout << "Threads: " << 1
				<< ", Duration: " << duration.count() << " ms.\n";
			std::cout << "Set: "; set.print20();
		}
		{
			// Consistency check
			set.clear();
			std::vector<std::thread> threads;
			for (int i = 0; i < MAX_THREADS; ++i)
				history[i].clear();
			auto start = high_resolution_clock::now();
			benchmark_check(1, 0);
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<milliseconds>(stop - start);
			std::cout << "Threads: " << 1
				<< ", Duration: " << duration.count() << " ms.\n";
			std::cout << "Set: "; set.print20();
			check_history(1);
		}
	}


	for (int num_thread = MAX_THREADS; num_thread >= 1; num_thread /= 2) {
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

	// Consistency check
	std::cout << "\n\nConsistency Check\n";

	for (int num_thread = MAX_THREADS; num_thread >= 1; num_thread /= 2) {
		set.clear();
		std::vector<std::thread> threads;
		for (int i = 0; i < MAX_THREADS; ++i)
			history[i].clear();
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_thread; ++i)
			threads.emplace_back(benchmark_check, num_thread, i);
		for (auto& th : threads)
			th.join();
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - start);
		std::cout << "Threads: " << num_thread
			<< ", Duration: " << duration.count() << " ms.\n";
		std::cout << "Set: "; set.print20();
		check_history(num_thread);
	}
}

/*
* 실행결과
Threads: 1, Duration: 27456 ms.
Set: 0, 7, 9, 10, 13, 19, 20, 21, 25, 29, 32, 33, 36, 38, 40, 42, 43, 45, 46, 47,
Threads: 1, Duration: 27690 ms.
Set: 0, 3, 4, 5, 6, 8, 9, 11, 12, 13, 15, 16, 20, 23, 26, 27, 28, 29, 30, 33,
Checking Consistency :  OK

C:\2025-2\multicore\x64\Release\multicore.exe(프로세스 13168)이(가) -1073741819 코드(0xc0000005)와 함께 종료되었습니다.
이 창을 닫으려면 아무 키나 누르세요...
*/