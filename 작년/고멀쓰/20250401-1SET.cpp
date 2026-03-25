#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;
using namespace chrono;

std::chrono::steady_clock::time_point g_StartTime;

inline void Timer_Start() { g_StartTime = high_resolution_clock::now(); }
inline void elapsed_time() {
	auto d = high_resolution_clock::now() - g_StartTime;
	std::cout << duration_cast<milliseconds>(d).count() << " msecs\n";
}


class NODE {
public:
	int key;
	NODE* next;
	NODE() { next = nullptr; }
	NODE(int key_value) {
		next = nullptr;
		key = key_value;
	}
	~NODE() {}
};


class CLIST {
	NODE* head, * tail;
	mutex glock;
public:
	CLIST()
	{
		head = new NODE{ std::numeric_limits<int>::min() };
		tail = new NODE{ std::numeric_limits<int>::max() };
		head->next = tail;
	}
	~CLIST() {}
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
		NODE* curr = pred->next;

		while (curr->key < key)
		{
			pred = curr;
			curr = curr->next;
		}
		if (curr->key == key)
			return false;
		else {
			auto n = new NODE{ key };
			n->next = curr;
			pred->next = n;
			return true;
		}
	}
	bool Remove(int key)
	{
		NODE* pred = head;
		NODE* curr = pred->next;

		while (curr->key < key)
		{
			pred = curr;
			curr = curr->next;
		}
		if (curr->key == key)
			return false;
		else {
			auto n = curr;
			pred->next = n->next;
			delete n;
			return true;
		}
	}
	bool Contains(int key)
	{
		NODE* pred = head;
		NODE* curr = pred->next;

		while (curr->key < key)
		{
			pred = curr;
			curr = curr->next;
		}
		if (curr->key == key)
			return true;
		else 
			return false;
	}
	void print20()
	{
		auto p = head->next;
		for (int i = 0; i < 20; ++i)
		{
			if (tail == p) break;
			cout << p->key << ', ';
			p = p->next;
		}

	}
};

const int NUM_TEST = 4000000;
const int KEY_RANGE = 1000;
CLIST g_set;

void ThreadFunc_Benchmark(int num_thread)
{
	int key;
	const int num_loop = NUM_TEST / num_thread;
	for (int i = 0; i < num_loop; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;

			g_set.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			g_set.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			g_set.Contains(key);
			break;
		default: cout << "Error\n";
			exit(-1);
		}
	}
}

int main()
{
	for (int j = 1; j <= 1; j *= 2)
	{
		g_set.clear();
		vector<thread> vecthreads;
		Timer_Start();
		for (int i = 0; i < j; ++i) {
			vecthreads.emplace_back(ThreadFunc_Benchmark, j);
		}
		for (auto& threads : vecthreads)
			threads.join();
		cout << "--------" << "num threads : " << j << " --------" << endl;
		g_set.print20();
		elapsed_time();
	}
}
