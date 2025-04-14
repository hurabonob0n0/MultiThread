#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <array>


class HISTORY {
public:
	int op;
	int i_value;
	bool o_value;
	HISTORY(int o, int i, bool re) : op(o), i_value(i), o_value(re) {}
};

std::array<std::vector<HISTORY>, 16> history;


struct LNODE {
	int key;
	LNODE* next;
	bool marked;
	std::mutex sm;
	LNODE() { next = nullptr; marked = false; }
	LNODE(int x) : key(x), next(nullptr), marked(false) {}
	void lock()
	{
		sm.lock();
	}
	void unlock()
	{
		sm.unlock();
	}
};

class LLIST {
	LNODE* head, * tail;
public:
	LLIST()
	{
		head = new LNODE{ std::numeric_limits<int>::min() };
		tail = new LNODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~LLIST()
	{
		delete head;
		delete tail;
	}

	void clear()
	{
		while (head->next != tail) {
			auto ptr = head->next;
			head->next = head->next->next;
			delete ptr;
		}
	}

	bool validate(LNODE* pred, LNODE* curr)
	{
		return !pred->marked && !curr->marked && pred->next == curr;
	}

	bool Add(int key)
	{
		LNODE* pred = head;
		LNODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		pred->lock(); curr->lock();
		if (true == validate(pred, curr)) {
			if (curr->key == key) {
				pred->unlock(); curr->unlock();
				return false;
			}
			else {
				auto n = new LNODE{ key };
				n->next = curr;
				pred->next = n;
				pred->unlock(); curr->unlock();
				return true;
			}
		}
		else {
			pred->unlock(); curr->unlock();
			return false;
		}

	}
	bool Remove(int key)
	{
		LNODE* pred = head;
		LNODE* curr = pred->next;
		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		pred->lock(); curr->lock();
		if (true == validate(pred, curr)) {
			if (curr->key == key) {
				auto n = curr;
				curr->marked = true;
				pred->next = n->next;
				pred->unlock(); curr->unlock();
				return true;
			}
			else {
				pred->unlock(); curr->unlock();
				return false;
			}
		}
		else {
			pred->unlock(); curr->unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		LNODE* curr = head;
		while (curr->key < key)
			curr = curr->next;
		return curr->key == key && !curr->marked;
	}
	void print20()
	{
		auto p = head->next;

		for (int i = 0; i < 20; ++i) {
			if (tail == p) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};

struct SPNODE {
	int key;
	std::shared_ptr<SPNODE> next;
	bool marked;
	std::mutex sm;

	SPNODE() : key(0), next(nullptr), marked(false) {}
	SPNODE(int x) : key(x), next(nullptr), marked(false) {}

	void lock() { sm.lock(); }
	void unlock() { sm.unlock(); }
};




class SPLLIST {
	std::shared_ptr<SPNODE> head, tail;

public:
	SPLLIST() {
		head = std::make_shared<SPNODE>(std::numeric_limits<int>::min());
		tail = std::make_shared<SPNODE>(std::numeric_limits<int>::max());
		head->next = tail;
	}

	void clear() {
		head->lock();
		auto curr = head->next;
		head->next = tail;
		head->unlock();
		while (curr != tail) {
			curr = curr->next;
		}
	}

	bool validate(const std::shared_ptr<SPNODE>& pred, const std::shared_ptr<SPNODE>& curr) {
		return !pred->marked && !curr->marked && pred->next == curr;
	}

	bool Add(int key) {
		while (true) {
			auto pred = head;
			auto curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock();
			curr->lock();

			if (validate(pred, curr)) {
				if (curr->key == key) {
					pred->unlock();
					curr->unlock();
					return false;
				}
				else {
					auto newNode = std::make_shared<SPNODE>(key);
					newNode->next = curr;
					pred->next = newNode;
					pred->unlock();
					curr->unlock();
					return true;
				}
			}

			pred->unlock();
			curr->unlock();
		}
	}

	bool Remove(int key) {
		while (true) {
			auto pred = head;
			auto curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock();
			curr->lock();

			if (validate(pred, curr)) {
				if (curr->key == key) {
					curr->marked = true;
					pred->next = curr->next;
					pred->unlock();
					curr->unlock();
					return true;
				}
				else {
					pred->unlock();
					curr->unlock();
					return false;
				}
			}

			pred->unlock();
			curr->unlock();
		}
	}

	bool Contains(int key) {
		auto curr = head;
		while (curr->key < key)
			curr = curr->next;
		return curr->key == key && !curr->marked;
	}

	void print20() {
		auto p = head->next;
		for (int i = 0; i < 20; ++i) {
			if (p == tail) break;
			std::cout << p->key << ", ";
			p = p->next;
		}
		std::cout << std::endl;
	}
};


constexpr int NUM_TEST = 4000000;
constexpr int KEY_RANGE = 1000;

LLIST l_list;
SPLLIST sp_list;

void check_history(int num_threads)
{
	std::array <int, KEY_RANGE> survive = {};
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
	for (int i = 0; i < KEY_RANGE; ++i) {
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
			if (l_list.Contains(i)) {
				std::cout << "ERROR. The value " << i << " should not exists.\n";
				exit(-1);
			}
		}
		else if (val == 1) {
			if (false == l_list.Contains(i)) {
				std::cout << "ERROR. The value " << i << " shoud exists.\n";
				exit(-1);
			}
		}
	}
	std::cout << " OK\n";
}


void benchmark_check(int num_threads, int th_id)
{
	for (int i = 0; i < NUM_TEST / num_threads; ++i) {
		int op = rand() % 3;
		switch (op) {
		case 0: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(0, v, sp_list.Add(v));
			break;
		}
		case 1: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(1, v, sp_list.Remove(v));
			break;
		}
		case 2: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(2, v, sp_list.Contains(v));
			break;
		}
		}
	}
}

void benchmark_l(int num_thread)
{
	int key;
	const int num_loop = NUM_TEST / num_thread;

	for (int i = 0; i < num_loop; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			l_list.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			l_list.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			l_list.Contains(key);
			break;
		default: std::cout << "Error\n";
			exit(-1);
		}
	}
}
void benchmark_sp(int num_thread)
{
	int key;
	const int num_loop = NUM_TEST / num_thread;

	for (int i = 0; i < num_loop; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			sp_list.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			sp_list.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			sp_list.Contains(key);
			break;
		default: std::cout << "Error\n";
			exit(-1);
		}
	}
}

int main()
{
	using namespace std::chrono;

	{
		std::cout << "---------------------LLIST--------------------" << std::endl;
		for (int i = 1; i <= 32; i = i * 2) {
			std::vector <std::thread> threads;
			l_list.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark_l, i);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			std::cout << i << " Threads : SET = ";
			l_list.print20();
			std::cout << ", Exec time = " << exec_ms << "ms.\n;";
		}
	}

	{
		std::cout << "---------------------SPLLIST--------------------" << std::endl;
		for (int i = 1; i <= 32; i = i * 2) {
			std::vector <std::thread> threads;
			sp_list.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark_sp, i);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			std::cout << i << " Threads : SET = ";
			sp_list.print20();
			std::cout << ", Exec time = " << exec_ms << "ms.\n;";
		}
	}
}

