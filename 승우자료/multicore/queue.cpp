#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <Windows.h>

const int MAX_THREADS = 16;

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

class C_QUEUE {
	NODE* head, * tail;
	std::mutex set_lock;
public:
	C_QUEUE() {
		head = tail = new NODE(-1);
	}

	~C_QUEUE() {
		clear();
		delete head;
	}

	void clear() {
		NODE* curr = head->next;
		while (nullptr != curr) {
			NODE* next = curr->next;
			delete curr;
			curr = next;
		}
		tail = head;
		head->next = nullptr;
	}

	void enqueue(int x)
	{
		NODE* new_node = new NODE(x);
		set_lock.lock();
		tail->next = new_node;
		tail = new_node;
		set_lock.unlock();
	}

	int dequeue()
	{
		NODE* temp;
		set_lock.lock();
		if (nullptr == head->next) {
			set_lock.unlock();
			return -1;
		}
		int res = head->next->value;
		temp = head;
		head = head->next;
		set_lock.unlock();
		delete temp;
		return res;
	}

	void print20()
	{
		NODE* curr = head->next;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class LF_QUEUE {
	NODE* volatile head, * volatile tail;
public:
	LF_QUEUE() {
		head = tail = new NODE(-1);
	}

	~LF_QUEUE() {
		clear();
		delete head;
	}

	void clear() {
		NODE* curr = head->next;
		while (nullptr != curr) {
			NODE* next = curr->next;
			delete curr;
			curr = next;
		}
		tail = head;
		head->next = nullptr;
	}

	bool CAS(NODE* volatile* addr, NODE* expected, NODE* new_value)
	{
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic<NODE*>*>(addr),
			&expected,
			new_value);
	}

	void enqueue(int x)
	{
		NODE* new_node = new NODE(x);
		while (true) {
			NODE* old_tail = tail;
			NODE* old_next = old_tail->next;
			if (old_tail != tail)
				continue;
			if (old_next == nullptr) {
				if (true == CAS(&old_tail->next, nullptr, new_node)) {
					CAS(&tail, old_tail, new_node);
					return;
				}
			}
			else
				CAS(&tail, old_tail, old_next);

		}
	}

	int dequeue()
	{
		while (true) {
			NODE* old_head = head;
			NODE* old_next = old_head->next;
			NODE* old_tail = tail;
			if (old_head != head)
				continue;
			if (old_next == nullptr)
				return -1;
			if (old_tail == old_head) {
				CAS(&tail, old_tail, old_next);
				continue;
			}
			int res = old_next->value;
			if (true == CAS(&head, old_head, old_next)) {
				delete old_head;
				return res;
			}
		}
	}

	void print20()
	{
		NODE* curr = head->next;
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next)
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

class STNODE;
class STPTR {
public:
	std::atomic<long long> raw;
	void set_ptr(STNODE* p) {
		raw = reinterpret_cast<long long>(p) << 32;
	}
	STNODE* get_ptr() {
		return reinterpret_cast<STNODE*>(raw >> 32);
	}
	STNODE* get_ptr(int* stamp) {
		long long cur_raw = raw;
		*stamp = static_cast<int>(cur_raw & 0xFFFFFFFF);
		return reinterpret_cast<STNODE*>(cur_raw >> 32);
	}
	bool CAS(STNODE* old_value, STNODE* new_value,
		int old_stamp, int new_stamp)
	{
		long long old_raw = (reinterpret_cast<long long>(old_value) << 32) | old_stamp;
		long long new_raw = (reinterpret_cast<long long>(new_value) << 32) | new_stamp;
		return std::atomic_compare_exchange_strong(&raw, &old_raw, new_raw);
	}
};

class STNODE {
public:
	int value;
	STPTR next;
	STNODE(int v) : value(v) {}
};

class LFST_QUEUE32 {
	STPTR head, tail;
public:
	LFST_QUEUE32() {
		head.set_ptr(new STNODE(-1));
		tail.set_ptr(head.get_ptr());
	}

	~LFST_QUEUE32() {
		clear();
		delete head.get_ptr();
	}

	void clear() {
		STNODE* curr = head.get_ptr()->next.get_ptr();
		while (nullptr != curr) {
			STNODE* next = curr->next.get_ptr();
			delete curr;
			curr = next;
		}
		tail.set_ptr(head.get_ptr());
		head.get_ptr()->next.set_ptr(nullptr);
	}

	void enqueue(int x)
	{
		STNODE* new_node = new STNODE(x);
		while (true) {
			int tail_stamp = 0;
			STNODE* old_tail = tail.get_ptr(&tail_stamp);
			int next_stamp = 0;
			STNODE* old_next = old_tail->next.get_ptr(&next_stamp);
			if (old_tail != tail.get_ptr())
				continue;
			if (old_next == nullptr) {
				if (true == old_tail->next.CAS(nullptr, new_node, next_stamp, next_stamp + 1)) {
					tail.CAS(old_tail, new_node, tail_stamp, tail_stamp + 1);
					return;
				}
			}
			else
				tail.CAS(old_tail, old_next, tail_stamp, tail_stamp + 1);
		}
	}

	int dequeue()
	{
		while (true) {
			int head_stamp = 0;
			STNODE* old_head = head.get_ptr(&head_stamp);
			int next_stamp = 0;
			STNODE* old_next = old_head->next.get_ptr(&next_stamp);
			int tail_stamp = 0;
			STNODE* old_tail = tail.get_ptr(&tail_stamp);
			if (old_head != head.get_ptr())	continue;
			if (old_next == nullptr) return -1;
			if (old_tail == old_head) {
				tail.CAS(old_tail, old_next, tail_stamp, tail_stamp + 1);
				continue;
			}
			int res = old_next->value;
			if (true == head.CAS(old_head, old_next, head_stamp, head_stamp + 1)) {
				delete old_head;
				return res;
			}
		}
	}

	void print20()
	{
		STNODE* curr = head.get_ptr()->next.get_ptr();
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next.get_ptr())
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};
#include <queue>

class STNODE64;

thread_local std::queue<STNODE64> free_nodes;

class STPTR64 {
public:
	alignas(16) STNODE64* volatile ptr;
	long long stamp;

	void set_ptr(STNODE64* p) {
		ptr = p;
		stamp++;
	}
	STNODE64* get_ptr() {
		return ptr;
	}
	STNODE64* get_ptr(long long* stamp) {
		STPTR64 temp{ 0, 0 };
		STPTR64 old{ 0,0 };
		if (false == CAS128(&temp, &old, this)) {
			std::cout << "CAS128 failed in get_ptr!\n";
			exit(-1);
		}
		*stamp = temp.stamp;
		return temp.ptr;
	}

	bool CAS128(STPTR64* addr, STPTR64* expected, STPTR64* new_value)
	{

		return InterlockedCompareExchange128(
			reinterpret_cast<LONG64*>(addr),
			new_value->stamp,
			reinterpret_cast<LONG64>(new_value->ptr),
			reinterpret_cast<LONG64*>(expected));
	}

	bool CAS(STNODE64* old_value, STNODE64* new_value,
		long long old_stamp, long long new_stamp)
	{
		STPTR64 old_p{ old_value, old_stamp };
		STPTR64 new_p{ new_value, new_stamp };
		return CAS128(this, &old_p, &new_p);
	}
};

class STNODE64 {
public:
	long long value;
	STPTR64 next;
	STNODE64(long long v) : value(v) {}
};
class LFST_QUEUE64 {
	STPTR64 head, tail;
public:
	LFST_QUEUE64() {
		head.set_ptr(new STNODE64(-1));
		tail.set_ptr(head.get_ptr());
	}

	~LFST_QUEUE64() {
		clear();
		delete head.get_ptr();
	}

	void clear() {
		STNODE64* curr = head.get_ptr()->next.get_ptr();
		while (nullptr != curr) {
			STNODE64* next = curr->next.get_ptr();
			delete curr;
			curr = next;
		}
		tail.set_ptr(head.get_ptr());
		head.get_ptr()->next.set_ptr(nullptr);
	}

	void enqueue(int x)
	{
		STNODE64* new_node = new STNODE64(x);
		while (true) {
			long long tail_stamp = 0;
			STNODE64* old_tail = tail.get_ptr(&tail_stamp);
			long long next_stamp = 0;
			STNODE64* old_next = old_tail->next.get_ptr(&next_stamp);
			if (old_tail != tail.get_ptr())
				continue;
			if (old_next == nullptr) {
				if (true == old_tail->next.CAS(nullptr, new_node, next_stamp, next_stamp + 1)) {
					tail.CAS(old_tail, new_node, tail_stamp, tail_stamp + 1);
					return;
				}
			}
			else
				tail.CAS(old_tail, old_next, tail_stamp, tail_stamp + 1);
		}
	}

	long long dequeue()
	{
		while (true) {
			long long head_stamp = 0;
			STNODE64* old_head = head.get_ptr(&head_stamp);
			long long next_stamp = 0;
			STNODE64* old_next = old_head->next.get_ptr(&next_stamp);
			long long tail_stamp = 0;
			STNODE64* old_tail = tail.get_ptr(&tail_stamp);
			if (old_head != head.get_ptr())	continue;
			if (old_next == nullptr) return -1;
			if (old_tail == old_head) {
				tail.CAS(old_tail, old_next, tail_stamp, tail_stamp + 1);
				continue;
			}
			long long res = old_next->value;
			if (true == head.CAS(old_head, old_next, head_stamp, head_stamp + 1)) {
				delete old_head;
				return res;
			}
		}
	}

	void print20()
	{
		STNODE64* curr = head.get_ptr()->next.get_ptr();
		for (int i = 0; i < 20 && curr != nullptr; i++, curr = curr->next.get_ptr())
			std::cout << curr->value << ", ";
		std::cout << "\n";
	}
};

LFST_QUEUE64 my_queue;

const int NUM_TEST = 10000000;

void benchmark(const int num_thread, int th_id)
{
	const int loop_count = NUM_TEST / num_thread;

	int key = 0;
	for (int i = 0; i < loop_count; i++) {
		if ((i < 32) || (rand() % 2 == 0))
			my_queue.enqueue(key++);
		else
			my_queue.dequeue();
	}

}

int main()
{
	using namespace std::chrono;

	for (int num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		my_queue.clear();
		auto st = high_resolution_clock::now();
		std::vector<std::thread> threads;
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(benchmark, num_threads, i);
		for (int i = 0; i < num_threads; ++i)
			threads[i].join();
		auto ed = high_resolution_clock::now();
		auto time_span = duration_cast<milliseconds>(ed - st).count();
		std::cout << "Thread Count = " << num_threads << ", Exec Time = " << time_span << "ms.\n";
		std::cout << "Result : ";  my_queue.print20();
	}
}


/*

C_QUEUE 
Thread Count = 1, Exec Time = 1050ms.
Result : 4999625, 4999626, 4999627, 4999628, 4999629, 4999630, 4999631, 4999632, 4999633, 4999634, 4999635, 4999636, 4999637, 4999638, 4999639, 4999640, 4999641, 4999642, 4999643, 4999644,
Thread Count = 2, Exec Time = 1399ms.
Result : 2499181, 2499182, 2499183, 2499184, 2499185, 2499186, 2499187, 2499188, 2499189, 2499190, 2499191, 2499192, 2499193, 2499194, 2499195, 2499196, 2499197, 2499198, 2499199, 2499200,
Thread Count = 4, Exec Time = 1997ms.
Result : 1248487, 1248488, 1248489, 1248490, 1248491, 1248492, 1248493, 1248494, 1248495, 1248496, 1248497, 1248498, 1248499, 1248500, 1248501, 1248502, 1248503, 1248504, 1248505, 1248506,
Thread Count = 8, Exec Time = 2310ms.
Result : 624712, 624713, 624714, 624715, 624716, 624717, 624718, 624719, 624720, 624721, 624722, 624723, 624724, 624725, 624726, 624727, 624728, 624729, 624730, 624731,
Thread Count = 16, Exec Time = 2322ms.
Result : 310605, 310606, 310607, 310608, 310609, 310610, 310611, 310612, 310613, 310614, 310615, 310616, 310617, 310618, 310619, 310620, 310621, 310622, 310623, 310624,

LF_QUEUE
Thread Count = 1, Exec Time = 819ms.
Result : 4999625, 4999626, 4999627, 4999628, 4999629, 4999630, 4999631, 4999632, 4999633, 4999634, 4999635, 4999636, 4999637, 4999638, 4999639, 4999640, 4999641, 4999642, 4999643, 4999644,
Thread Count = 2, Exec Time = 923ms.
Result : 2499221, 2499222, 2499223, 2499224, 2499225, 2499226, 2499227, 2499228, 2499229, 2499230, 2499231, 2499232, 2499233, 2499234, 2499235, 2499236, 2499237, 2499238, 2499239, 2499240,
Thread Count = 4, Exec Time = 989ms.
Result : 1248154, 1248155, 1248156, 1248157, 1248158, 1248159, 1248160, 1248161, 1248162, 1248163, 1248164, 1248165, 1248166, 1248167, 1248168, 1248169, 1248170, 1248171, 1248172, 1248173,
Thread Count = 8, Exec Time = 1202ms.
Result : 624712, 624713, 624714, 624715, 624716, 624717, 624718, 624719, 624720, 624721, 624722, 624723, 624724, 624725, 624726, 624727, 624728, 624729, 624730, 624731,

LFST_QUEUE64
* 디버그모드
Thread Count = 1, Exec Time = 4465ms.
Result : 4999625, 4999626, 4999627, 4999628, 4999629, 4999630, 4999631, 4999632, 4999633, 4999634, 4999635, 4999636, 4999637, 4999638, 4999639, 4999640, 4999641, 4999642, 4999643, 4999644,
Thread Count = 2, Exec Time = 3724ms.
Result : 2499252, 2499253, 2499254, 2499255, 2499256, 2499257, 2499258, 2499259, 2499260, 2499261, 2499262, 2499263, 2499264, 2499265, 2499266, 2499267, 2499268, 2499269, 2499270, 2499271,
Thread Count = 4, Exec Time = 3941ms.
Result : 1248188, 1248189, 1248190, 1248191, 1248192, 1248193, 1248194, 1248195, 1248196, 1248197, 1248198, 1248199, 1248200, 1248201, 1248202, 1248203, 1248204, 1248205, 1248206, 1248207,
Thread Count = 8, Exec Time = 4988ms.
Result : 624556, 624557, 624558, 624559, 624560, 624561, 624562, 624563, 624564, 624565, 624566, 624567, 624568, 624569, 624570, 624571, 624572, 624573, 624574, 624575,
Thread Count = 16, Exec Time = 7571ms.
Result : 309436, 309437, 309438, 309439, 309440, 309441, 309442, 309443, 309444, 309445, 309446, 309447, 309448, 309449, 309450, 309451, 309452, 309453, 309454, 309455,

* 릴리즈모드
Thread Count = 1, Exec Time = 1425ms.
Result : 4999625, 4999626, 4999627, 4999628, 4999629, 4999630, 4999631, 4999632, 4999633, 4999634, 4999635, 4999636, 4999637, 4999638, 4999639, 4999640, 4999641, 4999642, 4999643, 4999644,
Thread Count = 2, Exec Time = 1388ms.
Result : 2499193, 2499194, 2499195, 2499196, 2499197, 2499198, 2499199, 2499200, 2499201, 2499202, 2499203, 2499204, 2499205, 2499206, 2499207, 2499208, 2499209, 2499210, 2499211, 2499212,
Thread Count = 4, Exec Time = 1437ms.
Result : 1248325, 1248326, 1248327, 1248328, 1248329, 1248330, 1248331, 1248332, 1248333, 1248334, 1248335, 1248336, 1248337, 1248338, 1248339, 1248340, 1248341, 1248342, 1248343, 1248344,
Thread Count = 8, Exec Time = 1460ms.
Result : 624712, 624713, 624714, 624715, 624716, 624717, 624718, 624719, 624720, 624721, 624722, 624723, 624724, 624725, 624726, 624727, 624728, 624729, 624730, 624731,
*/