#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <set>
#include <unordered_set>

const int MAX_THREADS = 16;
int num_threads = 1;

class NODE {
public:
	int value;
	NODE* volatile next;
	NODE(int v) : value(v), next(nullptr) {}
};

class DUMMY_MUTEX {
public:
	void lock() {}
	void unlock() {}
};

class C_STACK {
	NODE* top;
	std::mutex set_lock;
public:
	C_STACK() {
		top = nullptr;
	}

	~C_STACK() {
		clear();
	}

	void clear() {
		while (nullptr != top) pop();
	}

	void push(int x)
	{
		NODE* new_node = new NODE(x);
		set_lock.lock();
		new_node->next = top;
		top = new_node;
		set_lock.unlock();
	}

	int pop()
	{
		set_lock.lock();
		if (nullptr == top) {
			set_lock.unlock();
			return -2;
		}
		int res = top->value;
		auto temp = top;
		top = top->next;
		set_lock.unlock();
		delete temp;
		return res;
	}

	void print20()
	{
		NODE* curr = top;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class LF_STACK {
	NODE* top;
public:
	LF_STACK() {
		top = nullptr;
	}

	~LF_STACK() {
		clear();
	}

	void clear() {
		while (nullptr != top) pop();
	}

	bool CAS(NODE** volatile addr, NODE* expected, NODE* new_value)
	{
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<std::atomic<NODE*>*>(addr),
			&expected,
			new_value);
	}

	void push(int x)
	{
		NODE* new_node = new NODE(x);
		while(true){
			NODE* curr_top = top;
			new_node->next = curr_top;
			if (true == CAS(&top, curr_top, new_node))
				return;
		}
	}

	int pop()
	{
		while(true){
			NODE* curr_top = top;
			if (nullptr == curr_top)
				return -2;
			NODE* next_node = curr_top->next;
			int res = curr_top->value;
			if (curr_top != top)
				continue;
			if (true == CAS(&top, curr_top, next_node)) {
				return res;
			}
		}
	}

	void print20()
	{
		NODE* curr = top;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class BACKOFF {
	int minDelay, maxDelay;
	int limit;
public:
	BACKOFF(int min_d, int max_d)
		:minDelay(min_d), maxDelay(max_d), limit(min_d){
		if (0 == limit) {
			std::cout << "ERROR: Min Delay must be greater than 0.\n";
			exit(-1);
		}
	}
	void backoff()
	{
		int delay = (rand() % limit);
		limit = limit * 2;
		if (limit > maxDelay)
			limit = maxDelay;
		std::this_thread::sleep_for(std::chrono::microseconds(delay));
	}
};

class LFBO_STACK {
	NODE* top;
public:
	LFBO_STACK() {
		top = nullptr;
	}

	~LFBO_STACK() {
		clear();
	}

	void clear() {
		while (nullptr != top) pop();
	}

	bool CAS(NODE** volatile addr, NODE* expected, NODE* new_value)
	{
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<std::atomic<NODE*>*>(addr),
			&expected,
			new_value);
	}

	void push(int x)
	{
		BACKOFF backoff(1, num_threads);
		NODE* new_node = new NODE(x);
		while (true) {
			NODE* curr_top = top;
			new_node->next = curr_top;
			if (true == CAS(&top, curr_top, new_node))
				return;
			else {
				backoff.backoff();
			}
		}
	}

	int pop()
	{
		BACKOFF backoff(1, num_threads);
		while (true) {
			NODE* curr_top = top;
			if (nullptr == curr_top)
				return -2;
			NODE* next_node = curr_top->next;
			int res = curr_top->value;
			if (curr_top != top)
				continue;
			if (true == CAS(&top, curr_top, next_node)) {
				return res;
			}
			else {
				backoff.backoff();
			}
		}
	}

	void print20()
	{
		NODE* curr = top;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};


constexpr int ST_EMPTY = 0;
constexpr int ST_WAITING = 1;
constexpr int ST_BUSY = 2;
constexpr int TIME_OUT = 100;

class LockFreeExchanger {
	std::atomic<long long> slot;
public:
	LockFreeExchanger() : slot(0) {}
	int exchange(int my_item, bool* busy) {
		*busy = false;
		for (int j = 0; j < TIME_OUT; ++j) {
			long long s = slot;
			int item = (int)(s & 0xFFFFFFFF);
			int status = (int)((s >> 32) & 0x3);
			long long new_s;
			int spins = 0;
			switch (status) {
			case ST_EMPTY: {
				new_s = ((long long)my_item & 0xFFFFFFFF) | ((long long)ST_WAITING << 32);
				if (std::atomic_compare_exchange_strong(&slot, &s, new_s)) {
					// wait for a partner
					spins = 0;
					for (int i = 0; i < TIME_OUT; ++i) {
						s = slot;
						status = (int)((s >> 32) & 0x3);
						if (status == ST_BUSY) {
							int their_item = (int)(s & 0xFFFFFFFF);
							slot = 0; // set to EMPTY
							return their_item;
						}
					}
					if (std::atomic_compare_exchange_strong(&slot, &s, 0)) { // set to EMPTY
						slot = 0; // set to EMPTY
						return -2; // TIME OUT
					}
					else {
						// someone is BUSY
						s = slot;
						int their_item = (int)(s & 0xFFFFFFFF);
						slot = 0; // set to EMPTY
						return their_item;
					}
					break;
			case ST_WAITING: {
				new_s = ((long long)my_item & 0xFFFFFFFF) | ((long long)ST_BUSY << 32);
				if (std::atomic_compare_exchange_strong(&slot, &s, new_s)) {
					int their_item = item;
					return their_item;
				}
				break;
			case ST_BUSY: {
				*busy = true;
				break;
			}
			}
				}
				return -2; // TIME OUT
			}
			}
		}
	}
};


class EliminationArray {
	int range;
	LockFreeExchanger exchanger[MAX_THREADS / 2 - 1];
public:
	EliminationArray() { range = 1; }
	~EliminationArray() {}
	int Visit(int value) {
		int slot = rand() % range;
		bool busy;
		int ret = exchanger[slot].exchange(value, &busy);
		int old_range = range;
		if ((ret == -2) && (old_range > 1))  // TIME OUT
			range = old_range - 1;
		if ((true == busy) && (old_range <= num_threads / 2 - 1))
			range = old_range + 1; // MAX RANGE is # of thread / 2
		return ret;
	}
};


class LFEL_STACK {
	NODE* top;
	EliminationArray elim_array;

public:
	LFEL_STACK() {
		top = nullptr;
	}

	~LFEL_STACK() {
		clear();
	}

	void clear() {
		while (nullptr != top) pop();
	}

	bool CAS(NODE* volatile* addr, NODE* expected, NODE* desired)
	{
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic<NODE*>*>(addr),
			&expected,
			desired);
	}

	void push(int x)
	{
		NODE* new_node = new NODE(x);
		while (true) {
			new_node->next = top;
			if (CAS(&top, new_node->next, new_node))
				return;
			int elim_res = elim_array.Visit(x);
			if (elim_res == -2) // TIME OUT
				continue;
			else
				return; // eliminated
		}

	}

	int pop()
	{
		while (true) {
			NODE* curr_top = top;
			if (nullptr == curr_top) {
				return -2;
			}
			NODE* next_node = curr_top->next;
			if (CAS(&top, curr_top, next_node)) {
				int res = curr_top->value;
				//delete curr_top;
				return res;
			}
			else {}

		}

	}

	void print20()
	{
		NODE* curr = top;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};




LFEL_STACK my_stack;

struct HISTORY {
	std::vector <int> push_values, pop_values;
};
std::atomic_int stack_size;
thread_local int thread_id;
const int NUM_TEST = 10000000;

void benchmark(const int num_thread)
{
	int key = 0;
	const int loop_count = NUM_TEST / num_thread;
	for (auto i = 0; i < loop_count; ++i) {
		if ((rand() % 2 == 0) || (i < 1000))
			my_stack.push(key++);
		else
			my_stack.pop();
	}
}

void benchmark_test(const int th_id, const int num_threads, HISTORY& h)
{
	thread_id = th_id;
	int loop_count = NUM_TEST / num_threads;
	for (int i = 0; i < loop_count; i++) {
		if ((rand() % 2) || i < 128 / num_threads) {
			h.push_values.push_back(i);
			stack_size++;
			my_stack.push(i);
		}
		else {
			volatile int curr_size = stack_size--;
			int res = my_stack.pop();
			if (res == -2) {
				stack_size++;
				if ((curr_size > num_threads * 2) && (stack_size > num_threads)) {
					std::cout << "ERROR Non_Empty Stack Returned NULL\n";
					exit(-1);
				}
			}
			else h.pop_values.push_back(res);
		}
	}
}

void check_history(std::vector <HISTORY>& h)
{
	std::unordered_multiset <int> pushed, poped, in_stack;

	for (auto& v : h)
	{
		for (auto num : v.push_values) pushed.insert(num);
		for (auto num : v.pop_values) poped.insert(num);
		while (true) {
			int num = my_stack.pop();
			if (num == -2) break;
			poped.insert(num);
		}
	}
	for (auto num : pushed) {
		if (poped.count(num) < pushed.count(num)) {
			std::cout << "Pushed Number " << num << " does not exists in the STACK.\n";
			exit(-1);
		}
		if (poped.count(num) > pushed.count(num)) {
			std::cout << "Pushed Number " << num << " is poped more than " << poped.count(num) - pushed.count(num) << " times.\n";
			exit(-1);
		}
	}
	for (auto num : poped)
		if (pushed.count(num) == 0) {
			std::multiset <int> sorted;
			for (auto num : poped)
				sorted.insert(num);
			std::cout << "There were elements in the STACK no one pushed : ";
			int count = 20;
			for (auto num : sorted)
				std::cout << num << ", ";
			std::cout << std::endl;
			exit(-1);

		}
	std::cout << "NO ERROR detectd.\n";
}

int main()
{
	using namespace std::chrono;

	for (int n = 1; n <= MAX_THREADS; n = n * 2) {
		num_threads = n;
		my_stack.clear();
		std::vector<std::thread> tv;
		std::vector<HISTORY> history;
		history.resize(n);
		stack_size = 0;
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < n; ++i) {
			tv.emplace_back(benchmark_test, i, n, std::ref(history[i]));
		}
		for (auto& th : tv)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		size_t ms = duration_cast<milliseconds>(exec_t).count();
		std::cout << n << " Threads,  " << ms << "ms. ----";
		my_stack.print20();
		check_history(history);
	}

	for (int n = 1; n <= MAX_THREADS; n *= 2) {
		num_threads = n;
		my_stack.clear();
		std::vector<std::thread> tv;
		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < n; ++i) {
			tv.emplace_back(benchmark, n);
		}
		for (auto& th : tv)
			th.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		size_t ms = duration_cast<milliseconds>(exec_t).count();
		std::cout << n << " Threads,  " << ms << "ms. ----";
		my_stack.print20();
	}
}

/*
* C_STACK 실행결과
1 Threads,  694ms. ----4966605, 4966604, 4966593, 4966581, 4966580, 4966579, 4966578, 4965123, 4965103, 4965102, 4965101, 4965100, 4965094, 4965027, 4965026, 4965025, 4965024, 4965023, 4965022, 4965010,
2 Threads,  1348ms. ----2500421, 2500420, 2500419, 2500416, 2500413, 2500412, 2500409, 2500408, 2500375, 2500371, 2500353, 2484202, 2484201, 2484151, 2484148, 2484147, 2484146, 2484144, 2484140, 2484139,
4 Threads,  1717ms. ----1250473, 1250458, 1250442, 1250441, 1250440, 1250439, 1250421, 1250419, 1250418, 1250382, 1250381, 1250380, 1250379, 1242999, 1244544, 1239076, 1239074, 1239073, 1244496, 1244494,
8 Threads,  2178ms. ----625269, 625268, 623906, 623904, 623903, 623890, 623886, 623885, 623884, 623848, 623847, 623846, 623844, 623843, 623832, 623831, 623830, 623827, 623824, 623820,
16 Threads,  2116ms. ----312876, 312870, 312869, 312863, 312861, 312858, 312857, 312856, 312855, 312853, 312852, 312846, 312835, 312834, 312831, 312830, 312829, 312828, 312825, 312818

LF_STACK 실행결과
1 Threads,  687ms. ----4966605, 4966604, 4966593, 4966581, 4966580, 4966579, 4966578, 4965123, 4965103, 4965102, 4965101, 4965100, 4965094, 4965027, 4965026, 4965025, 4965024, 4965023, 4965022, 4965010,
2 Threads,  654ms. ----2500421, 2500420, 2500419, 2500416, 2500413, 2500412, 2500409, 2500408, 2500375, 2500371, 2500353, 2486974, 2476670, 2486972, 2476669, 2476667, 2476665, 2476664, 2476660, 2476659,
4 Threads,  897ms. ----1250473, 1250458, 1250442, 1250441, 1250440, 1250439, 1250421, 1250419, 1250418, 1250382, 1250381, 1250380, 1250379, 1243141, 1243140, 1243139, 1243137, 1243135, 1243134, 1243133,
8 Threads,  1178ms. ----625269, 625268, 623906, 623904, 623903, 623890, 623886, 623885, 623884, 623848, 623847, 623846, 623844, 623843, 623832, 623831, 623830, 623827, 623824, 623820,
16 Threads,  1186ms. ----312876, 312870, 312869, 312863, 312861, 312858, 312857, 312856, 312855, 312853, 312852, 312846, 312835, 312834, 312831, 312830, 312829, 312828, 312825, 312818,


*/