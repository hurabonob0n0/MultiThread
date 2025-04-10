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

struct NODE {
	int key;
	NODE* next;
	std::mutex sm;
	NODE() { next = nullptr; }
	NODE(int x) : key(x), next(nullptr) {}
	void lock()
	{
		sm.lock();
	}
	void unlock()
	{
		sm.unlock();
	}
};

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

class CLIST {
	NODE* head, * tail;
	std::mutex	sm;
public:
	CLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~CLIST() {
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
	bool Add(int key)
	{
		NODE* pred = head;
		sm.lock();
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == key) {
			sm.unlock();
			return false;
		}
		else {
			auto n = new NODE{ key };
			n->next = curr;
			pred->next = n;
			sm.unlock();
			return true;
		}
	}
	bool Remove(int key)
	{
		NODE* pred = head;
		sm.lock();
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == key) {
			auto n = curr;
			pred->next = n->next;
			sm.unlock();
			delete n;
			return true;
		}
		else {
			sm.unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		NODE* pred = head;
		sm.lock();
		NODE* curr = pred->next;

		while (curr->key < key) {
			pred = curr;
			curr = curr->next;
		}

		if (curr->key == key) {
			sm.unlock();
			return true;
		}
		else {
			sm.unlock();
			return false;
		}
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

class FLIST {
	NODE* head, * tail;
public:
	FLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~FLIST()
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
	bool Add(int key)
	{
		NODE* pred = head;
		pred->lock();
		NODE* curr = pred->next;
		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->key == key) {
			pred->unlock(); curr->unlock();
			return false;
		}
		else {
			auto n = new NODE{ key };
			n->next = curr;
			pred->next = n;
			pred->unlock(); curr->unlock();
			return true;
		}
	}
	bool Remove(int key)
	{
		NODE* pred = head;
		pred->lock();
		NODE* curr = pred->next;
		curr->lock();
		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->key == key) {
			auto n = curr;
			pred->next = n->next;
			pred->unlock(); curr->unlock();
			delete n;
			return true;
		}
		else {
			pred->unlock(); curr->unlock();
			return false;
		}
	}
	bool Contains(int key)
	{
		NODE* pred = head;
		pred->lock();
		NODE* curr = pred->next;
		curr->lock();

		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr = curr->next;
			curr->lock();
		}

		if (curr->key == key) {
			pred->unlock(); curr->unlock();
			return true;
		}
		else {
			pred->unlock(); curr->unlock();
			return false;
		}
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

class OLIST {
	NODE* head, * tail;
public:
	OLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~OLIST()
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

	bool validate(NODE* pred, NODE* curr)
	{
		NODE* n = head;
		while (n->key <= pred->key) {
			if (pred == n)
				return pred->next == curr;
			n = n->next;
		}
		return false;
	}

	bool Add(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;
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
					auto n = new NODE{ key };
					n->next = curr;
					pred->next = n;
					pred->unlock(); curr->unlock();
					return true;
				}
			}
			else {
				pred->unlock(); curr->unlock();
			}
		}
	}
	bool Remove(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			pred->lock(); curr->lock();
			if (true == validate(pred, curr)) {
				if (curr->key == key) {
					auto n = curr;
					pred->next = n->next;
					pred->unlock(); curr->unlock();
					// delete n;
					return true;
				}
				else {
					pred->unlock(); curr->unlock();
					return false;
				}
			}
			else {
				pred->unlock(); curr->unlock();
				continue;
			}
		}
	}
	bool Contains(int key)
	{
		while (true) {
			NODE* pred = head;
			NODE* curr = pred->next;

			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			std::lock_guard <std::mutex> pl{ pred->sm };
			std::lock_guard <std::mutex> cl{ curr->sm };

			if (curr->key == key) {
				return true;
			}
			else {
				return false;
			}
		}
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

constexpr int NUM_TEST = 4000000;
constexpr int KEY_RANGE = 1000;

CLIST c_list;
FLIST f_list;
OLIST o_list;
LLIST l_list;

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
			history[th_id].emplace_back(0, v, l_list.Add(v));
			break;
		}
		case 1: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(1, v, l_list.Remove(v));
			break;
		}
		case 2: {
			int v = rand() % KEY_RANGE;
			history[th_id].emplace_back(2, v, l_list.Contains(v));
			break;
		}
		}
	}
}
void benchmark_c(int num_thread)
{
	int key;
	const int num_loop = NUM_TEST / num_thread;

	for (int i = 0; i < num_loop; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			c_list.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			c_list.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			c_list.Contains(key);
			break;
		default: std::cout << "Error\n";
			exit(-1);
		}
	}
}
void benchmark_f(int num_thread)
{
	int key;
	const int num_loop = NUM_TEST / num_thread;

	for (int i = 0; i < num_loop; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			f_list.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			f_list.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			f_list.Contains(key);
			break;
		default: std::cout << "Error\n";
			exit(-1);
		}
	}
}
void benchmark_o(int num_thread)
{
	int key;
	const int num_loop = NUM_TEST / num_thread;

	for (int i = 0; i < num_loop; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			o_list.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			o_list.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			o_list.Contains(key);
			break;
		default: std::cout << "Error\n";
			exit(-1);
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

int main()
{
	using namespace std::chrono;

	{
		auto start_t = system_clock::now();
		benchmark_l(1);
		auto end_t = system_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();

		std::cout << "Single Thread SET = ";
		l_list.print20();
		std::cout << ", Exec time = " << exec_ms << "ms.\n;";
	}
	//-------------CLIST------------------------
	{
		std::cout << "-------------CLIST-------------" << std::endl;
		for (int i = 1; i <= 32; i = i * 2) {
			std::vector <std::thread> threads;
			c_list.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark_c, i);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			std::cout << i << " Threads : SET = ";
			c_list.print20();
			std::cout << ", Exec time = " << exec_ms << "ms.\n;";
		}
	}
	//-------------FLIST------------------------
	{
		std::cout << "-------------FLIST-------------" << std::endl;
		for (int i = 1; i <= 32; i = i * 2) {
			std::vector <std::thread> threads;
			f_list.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark_f, i);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			std::cout << i << " Threads : SET = ";
			f_list.print20();
			std::cout << ", Exec time = " << exec_ms << "ms.\n;";
		}
	}
	//-------------OLIST------------------------
	{
		std::cout << "-------------OLIST-------------" << std::endl;
		for (int i = 1; i <= 32; i = i * 2) {
			std::vector <std::thread> threads;
			o_list.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark_o, i);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			std::cout << i << " Threads : SET = ";
			o_list.print20();
			std::cout << ", Exec time = " << exec_ms << "ms.\n;";
		}
	}



	// 알고리즘 정확성 검사 --------LList--------------------
	{
		std::cout << "---------------------LLIST TEST--------------------" << std::endl;
		for (int i = 1; i <= 16; i = i * 2) {
			std::vector <std::thread> threads;
			l_list.clear();
			for (auto& h : history) h.clear();
			auto start_t = system_clock::now();
			for (int j = 0; j < i; ++j)
				threads.emplace_back(benchmark_check, i, j);
			for (auto& th : threads)
				th.join();
			auto end_t = system_clock::now();
			auto exec_t = end_t - start_t;
			auto exec_ms = duration_cast<milliseconds>(exec_t).count();

			std::cout << i << " Threads : SET = ";
			l_list.print20();
			std::cout << ", Exec time = " << exec_ms << "ms.\n;";
			check_history(i);
		}
	}
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
}

