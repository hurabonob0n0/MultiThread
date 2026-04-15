#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>
#include <set>

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
				curr->removed = true;
				std::atomic_thread_fence(std::memory_order_seq_cst);
				prev->next = curr->next;
				prev->unlock();	curr->unlock();
				//delete curr;
				return true;
			}
		}
	}

	bool contains(int x)
	{
		auto curr = head->next;
		while (curr->value < x)
			curr = curr->next;
		return (curr->value == x) && (curr->removed == false);
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
				curr->removed = true;
				std::atomic_thread_fence(std::memory_order_seq_cst);
				prev->next = curr->next;
				prev->unlock();	curr->unlock();
				my_delete(curr);
				return true;
			}
		}
	}

	bool contains(int x)
	{
		auto curr = head->next;
		while (curr->value < x)
			curr = curr->next;
		return (curr->value == x) && (curr->removed == false);
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

//class NODE_SP {
//public:
//	int value;
//	std::shared_ptr<NODE_SP> next;
//	std::mutex mtx;
//	volatile bool removed;
//	NODE_SP(int x) : next(nullptr), value(x), removed(false) {}
//	void lock() { mtx.lock(); }
//	void unlock() { mtx.unlock(); }
//};
//
//class L_SET_SP{
//private:
//	std::shared_ptr<NODE_SP> head, tail;
//public:
//	L_SET_SP() {
//		head = std::make_shared<NODE_SP>(std::numeric_limits<int>::min());
//		tail = std::make_shared<NODE_SP>(std::numeric_limits<int>::max());
//		head->next = tail;
//	}
//
//	~L_SET_SP()
//	{
//	}
//
//	void clear()
//	{
//		head->next = tail;
//	}
//
//	bool validate(const std::shared_ptr<NODE_SP> &p, 
//		const std::shared_ptr<NODE_SP> &c)
//	{
//		return (p->removed == false)
//			&& (c->removed == false)
//			&& (std::atomic_load(&p->next) == c);
//	}
//
//	bool add(int x)
//	{
//		while (true) {
//			auto prev = head;
//			auto curr = std::atomic_load(&prev->next);
//			while (curr->value < x) {
//				prev = curr;
//				curr = std::atomic_load(&curr->next);
//			}
//
//			prev->lock(); curr->lock();
//			if (false == validate(prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//			if (curr->value == x) {
//				prev->unlock();	curr->unlock();
//				return false;
//			}
//			else {
//				auto newNode = std::make_shared<NODE_SP>(x);;
//				newNode->next = curr;
//				std::atomic_exchange(&prev->next, newNode);
//				prev->unlock();	curr->unlock();
//				return true;
//			}
//		}
//	}
//
//	bool remove(int x)
//	{
//		while (true) {
//			auto prev = head;
//			auto curr = std::atomic_load(&prev->next);
//			while (curr->value < x) {
//				prev = curr;
//				curr = std::atomic_load(&curr->next);
//			}
//
//			prev->lock(); curr->lock();
//			if (false == validate(prev, curr)) {
//				prev->unlock(); curr->unlock();
//				continue;
//			}
//			if (curr->value != x) {
//				prev->unlock();	curr->unlock();
//				return false;
//			}
//			else {
//				curr->removed = true;
//				std::atomic_thread_fence(std::memory_order_seq_cst);
//				std::atomic_exchange(&prev->next, 
//					std::atomic_load(&curr->next));
//				prev->unlock();	curr->unlock();
//				//my_delete(curr);
//				return true;
//			}
//		}
//	}
//
//	bool contains(int x)
//	{
//		auto curr = std::atomic_load(&head->next);
//		while (curr->value < x)
//			curr = std::atomic_load(&curr->next);
//		return (curr->value == x) && (curr->removed == false);
//	}
//
//	void print20()
//	{
//		auto curr = head->next;
//		for (int i = 0; i < 20 && curr != tail; ++i) {
//			std::cout << curr->value << ", ";
//			curr = curr->next;
//		}
//		std::cout << std::endl;
//	}
//};

class NODE_ASP {
public:
	int value;
	std::atomic<std::shared_ptr<NODE_ASP>> next;
	std::mutex mtx;
	volatile bool removed;
	NODE_ASP(int x) : next(nullptr), value(x), removed(false) {}
	void lock() { mtx.lock(); }
	void unlock() { mtx.unlock(); }
};

class L_SET_ASP {
private:
	std::shared_ptr<NODE_ASP> head, tail;
public:
	L_SET_ASP() {
		head = std::make_shared<NODE_ASP>(std::numeric_limits<int>::min());
		tail = std::make_shared<NODE_ASP>(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~L_SET_ASP()
	{
	}

	void clear()
	{
		head->next = tail;
	}

	bool validate(const std::shared_ptr<NODE_ASP>& p,
		const std::shared_ptr<NODE_ASP>& c)
	{
		return (p->removed == false)
			&& (c->removed == false)
			&& (p->next.load() == c);
	}

	bool add(int x)
	{
		while (true) {
			auto prev = head;
			std::shared_ptr<NODE_ASP> curr = prev->next;
			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
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
				auto newNode = std::make_shared<NODE_ASP>(x);;
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
			std::shared_ptr<NODE_ASP> curr = prev->next;
			while (curr->value < x) {
				prev = curr;
				curr = curr->next;
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
				std::atomic_thread_fence(std::memory_order_seq_cst);
				prev->next = curr->next.load();
				prev->unlock();	curr->unlock();
				//my_delete(curr);
				return true;
			}
		}
	}

	bool contains(int x)
	{
		std::shared_ptr<NODE_ASP> curr = head->next;
		while (curr->value < x)
			curr = curr->next;
		return (curr->value == x) && (curr->removed == false);
	}

	void print20()
	{
		std::shared_ptr<NODE_ASP> curr = head->next;
		for (int i = 0; i < 20 && curr != tail; ++i) {
			std::cout << curr->value << ", ";
			curr = curr->next;
		}
		std::cout << std::endl;
	}
};

class LF_NODE;

class AMR { // Atomic Markable Reference
	volatile long long ptr_and_mark;
public:
	AMR(LF_NODE* ptr = nullptr, bool mark = false) {
		long long val = reinterpret_cast<long long>(ptr);
		if (0 != (val & 1)) {  // 수정!!!!!
			std::cout << "ERROR"; exit(-1);
		}
		if (true == mark) val |= 1;
		ptr_and_mark = val;
	}

	LF_NODE* get_ptr() {
		long long val = ptr_and_mark;
		return reinterpret_cast<LF_NODE*>(val & 0xFFFFFFFFFFFFFFFE);
	}
	bool get_mark() {
		return (1 == (ptr_and_mark & 1));
	}
	LF_NODE* get_ptr_and_mark(bool* mark) {
		long long val = ptr_and_mark;
		*mark = (1 == (val & 1));
		return reinterpret_cast<LF_NODE*>(val & 0xFFFFFFFFFFFFFFFE);
	}

	bool attempt_mark(LF_NODE* expected_ptr, bool new_mark)
	{
		return CAS(expected_ptr, expected_ptr,
			false, new_mark);
	}

	bool CAS(LF_NODE* expected_ptr, LF_NODE* new_ptr,
		bool expected_mark, bool new_mark)
	{
		long long expected_val
			= reinterpret_cast<long long>(expected_ptr);
		if (true == expected_mark) expected_val |= 1;
		long long new_val
			= reinterpret_cast<long long>(new_ptr);
		if (true == new_mark) new_val |= 1;
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic<long long> *>(&ptr_and_mark),
			&expected_val, new_val);
	}

};

class LF_NODE {
public:
	int value;
	AMR next;
	int epoch; // for EBR
	LF_NODE(int x) : value(x) {}
};

class LF_SET {
private:
	LF_NODE* head, * tail;
public:
	LF_SET() {
		head = new LF_NODE(std::numeric_limits<int>::min());
		tail = new LF_NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~LF_SET()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		LF_NODE* curr = head->next.get_ptr();
		while (curr != tail) {
			LF_NODE* temp = curr;
			curr = curr->next.get_ptr();
			delete temp;
		}
		head->next = tail;
	}

	void find(LF_NODE*& prev, LF_NODE*& curr, int x)
	{
		while (true) {
		retry:
			prev = head;
			curr = prev->next.get_ptr();
			while (true) {
				bool curr_mark;
				auto succ = curr->next.get_ptr_and_mark(&curr_mark);
				while (true == curr_mark) {
					if (false == prev->next.CAS(curr, succ, false, false))
						goto retry;
					curr = succ;
					succ = curr->next.get_ptr_and_mark(&curr_mark);
				}
				if (curr->value >= x)
					return;
				prev = curr;
				curr = succ;
			}
		}
	}

	bool add(int x)
	{
		while (true) {
			LF_NODE* prev, * curr;
			find(prev, curr, x);

			if (curr->value == x) {
				return false;
			}
			else {
				auto newNode = new LF_NODE(x);
				newNode->next = curr;
				if (true == prev->next.CAS(curr, newNode, false, false))
					return true;
				else delete newNode;
			}
		}
	}

	bool remove(int x)
	{
		while (true) {
			LF_NODE* prev, * curr;
			find(prev, curr, x);

			if (curr->value != x) {
				return false;
			}
			else {
				auto succ = curr->next.get_ptr();
				if (false == curr->next.attempt_mark(succ, true))
					continue;
				prev->next.CAS(curr, succ, false, false);
				return true;
			}
		}
	}

	bool contains(int x)
	{
		auto curr = head->next.get_ptr();
		while (curr->value < x)
			curr = curr->next.get_ptr();
		return (curr->value == x) && (curr->next.get_mark() == false);
	}

	void print20()
	{
		auto curr = head->next.get_ptr();
		for (int i = 0; i < 20 && curr != tail; ++i) {
			std::cout << curr->value << ", ";
			curr = curr->next.get_ptr();
		}
		std::cout << std::endl;
	}
};

int num_threads = 0;
thread_local int thread_id = 0;

class EBR {
private:
	std::queue<LF_NODE*> free_list[MAX_THREADS];
	std::atomic<int> epoch_counter;
	struct THREAD_COUNTER {
		alignas(64) std::atomic<int> local_epoch;
	};
	THREAD_COUNTER thread_counter[MAX_THREADS];
public:
	EBR() {
		epoch_counter = 0;
		for (int i = 0; i < MAX_THREADS; ++i) {
			thread_counter[i].local_epoch = std::numeric_limits<int>::max();
		}
	}
	~EBR() {
		recycle();
	}
	void recycle() {
		for (int i = 0; i < MAX_THREADS; ++i) {
			while (false == free_list[i].empty()) {
				auto node = free_list[i].front();
				free_list[i].pop();
				delete node;
			}
		}
	}
	void delete_node(LF_NODE* node) {
		node->epoch = epoch_counter.load();
		free_list[thread_id].push(node);
	}
	LF_NODE* new_node(int x) {
		if (false == free_list[thread_id].empty()) {
			auto node = free_list[thread_id].front();
			bool can_reuse = true;
			for (int i = 0; i < num_threads; ++i) {
				if (i == thread_id) continue;
				if (thread_counter[i].local_epoch <= node->epoch) {
					can_reuse = false;
					break;
				}
			}
			if (true == can_reuse) {
				free_list[thread_id].pop();
				node->value = x;
				node->next = nullptr;
				return node;
			}
		}
		return new LF_NODE(x);
	}
	void start_op() {
		int e = ++epoch_counter;
		thread_counter[thread_id].local_epoch = e;
	}
	void end_op() {
		thread_counter[thread_id].local_epoch = std::numeric_limits<int>::max();
	}
};

class LF_SET_EBR {
private:
	EBR ebr;
	LF_NODE* head, * tail;
public:
	LF_SET_EBR() {
		head = new LF_NODE(std::numeric_limits<int>::min());
		tail = new LF_NODE(std::numeric_limits<int>::max());
		head->next = tail;
	}

	~LF_SET_EBR()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		LF_NODE* curr = head->next.get_ptr();
		while (curr != tail) {
			LF_NODE* temp = curr;
			curr = curr->next.get_ptr();
			delete temp;
		}
		head->next = tail;
	}

	void find(LF_NODE*& prev, LF_NODE*& curr, int x)
	{
		while (true) {
		retry:
			prev = head;
			curr = prev->next.get_ptr();
			while (true) {
				bool curr_mark;
				auto succ = curr->next.get_ptr_and_mark(&curr_mark);
				while (true == curr_mark) {
					if (false == prev->next.CAS(curr, succ, false, false))
						goto retry;
					ebr.delete_node(curr);
					curr = succ;
					succ = curr->next.get_ptr_and_mark(&curr_mark);
				}
				if (curr->value >= x)
					return;
				prev = curr;
				curr = succ;
			}
		}
	}

	bool add(int x)
	{
		ebr.start_op();
		while (true) {
			LF_NODE* prev, * curr;
			find(prev, curr, x);

			if (curr->value == x) {
				ebr.end_op();
				return false;
			}
			else {
				auto newNode = ebr.new_node(x);
				newNode->next = curr;
				if (true == prev->next.CAS(curr, newNode, false, false)) {
					ebr.end_op();
					return true;
				}
				else ebr.delete_node(newNode);
			}
		}
	}

	bool remove(int x)
	{
		ebr.start_op();
		while (true) {
			LF_NODE* prev, * curr;
			find(prev, curr, x);

			if (curr->value != x) {
				ebr.end_op();
				return false;
			}
			else {
				auto succ = curr->next.get_ptr();
				if (false == curr->next.attempt_mark(succ, true))
					continue;
				if (true == prev->next.CAS(curr, succ, false, false))
					ebr.delete_node(curr);
				ebr.end_op();
				return true;
			}
		}
	}

	bool contains(int x)
	{
		ebr.start_op();
		auto curr = head->next.get_ptr();
		while (curr->value < x)
			curr = curr->next.get_ptr();
		auto res = (curr->value == x) && (curr->next.get_mark() == false);
		ebr.end_op();
		return res;
	}

	void print20()
	{
		auto curr = head->next.get_ptr();
		for (int i = 0; i < 20 && curr != tail; ++i) {
			std::cout << curr->value << ", ";
			curr = curr->next.get_ptr();
		}
		std::cout << std::endl;
	}
};

// 싱글 쓰레드 통합 API
enum INVO_OP { ADD = 0, REMOVE = 1, CONTAINS = 2 };
class INVOCATION {
public:
	INVO_OP op;
	int value;
	INVOCATION(INVO_OP o, int v) : op(o), value(v) {}
};

typedef bool RESPONSE;

class SEQ_SET {
	std::set<int> m_set;
public:
	RESPONSE apply(INVOCATION inv) {
		switch (inv.op) {
		case ADD:
			return m_set.insert(inv.value).second;
		case REMOVE:
			return (m_set.erase(inv.value) > 0);
		case CONTAINS:
			return (m_set.find(inv.value) != m_set.end());
		default:
			return false;
		}
	}
	void clear() {
		m_set.clear();
	}
	void print20() {
		int count = 0;
		for (auto& v : m_set) {
			std::cout << v << ", ";
			if (++count >= 20) break;
		}
		std::cout << std::endl;
	}
};

class LNODE;

class CONSENSUS {
	LNODE* value{ nullptr };
public:
	LNODE* decide(LNODE* v)
	{
		CAS(&value, nullptr, v);
		return value;
	}
	void CAS(LNODE** addr, LNODE* expected, LNODE* update)
	{
		std::atomic_compare_exchange_strong(
			reinterpret_cast<std::atomic<LNODE*>*>(addr),
			&expected, update);
	}
	void clear()
	{
		value = nullptr;
	}
};

class LNODE {
public:
	INVOCATION m_inv;
	int	m_seq;
	LNODE* m_next;
	CONSENSUS decide_next;
	LNODE(INVOCATION inv) : m_inv(inv), m_seq(0), m_next(nullptr) {}
};

class LFU_SET {
	LNODE* head[MAX_THREADS];
	LNODE* tail;
public:
	LFU_SET() {
		tail = new LNODE(INVOCATION(CONTAINS, 0)); // dummy
		for (int i = 0; i < MAX_THREADS; ++i) {
			head[i] = tail;
		}
	}

	~LFU_SET()
	{
		while (nullptr != tail) {
			LNODE* temp = tail;
			tail = tail->m_next;
			delete temp;
		}
	}

	LNODE* max_head()
	{
		LNODE* max_node = head[0];
		for (int i = 1; i < num_threads; ++i) {
			if (max_node->m_seq < head[i]->m_seq)
				max_node = head[i];
		}
		return max_node;
	}

	RESPONSE apply(INVOCATION inv)
	{
		int i = thread_id;
		auto prefer = new LNODE(inv);
		while (prefer->m_seq == 0) {
			LNODE* before = max_head();
			LNODE* after = before->decide_next.decide(prefer);
			before->m_next = after;
			after->m_seq = before->m_seq + 1;
			head[i] = after;
		}

		SEQ_SET seq_set;
		LNODE* curr = tail->m_next;
		while (curr != prefer) {
			seq_set.apply(curr->m_inv);
			curr = curr->m_next;
		}
		return seq_set.apply(inv);
	};

	void clear()
	{
		for (int i = 0; i < num_threads; ++i) {
			head[i] = tail;
		}
		LNODE* curr = tail->m_next;
		while (nullptr != curr) {
			LNODE* temp = curr;
			curr = curr->m_next;
			delete temp;
		}
		tail->m_next = nullptr;
		tail->decide_next.clear();
	}

	void print20()
	{
		SEQ_SET seq_set;
		LNODE* curr = tail->m_next;
		while (nullptr != curr) {
			seq_set.apply(curr->m_inv);
			curr = curr->m_next;
		}
		seq_set.print20();
	}
};


// 벤치 마킹
class STD_SET {
private:
	//SEQ_SET m_set;
	LFU_SET m_set;
	DUMMY_MTX mtx;
public:
	STD_SET() {}

	~STD_SET() {}

	void clear()
	{
		m_set.clear();
	}

	bool add(int x)
	{
		mtx.lock();
		auto res = m_set.apply(INVOCATION(ADD, x));
		mtx.unlock();
		return res;
	}


	bool remove(int x)
	{
		mtx.lock();
		auto res = m_set.apply(INVOCATION(REMOVE, x));
		mtx.unlock();
		return res;
	}

	bool contains(int x)
	{
		mtx.lock();
		auto res = m_set.apply(INVOCATION(CONTAINS, x));
		mtx.unlock();
		return res;
	}

	void print20()
	{
		m_set.print20();
	}
};



constexpr int MAX_LEVEL = 9;

class SK_NODE {
public:
	int value;
	SK_NODE* volatile next[MAX_LEVEL + 1];
	int top_level;	

	SK_NODE(int x, int top) : value(x), top_level(top)
	{
		for (auto& p : next) p = nullptr;
	}
	SK_NODE() : value(-1), top_level(0) {
		for (auto& p : next) p = nullptr;
	}
};

class C_SKLIST {
private:
	SK_NODE* head, * tail;
	std::mutex mtx;
public:
	C_SKLIST() {
		head = new SK_NODE(std::numeric_limits<int>::min(), MAX_LEVEL);
		tail = new SK_NODE(std::numeric_limits<int>::max(), MAX_LEVEL);
		for (auto& p : head->next) p = tail;
	}

	~C_SKLIST()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		SK_NODE* curr = head->next[0];
		while (curr != tail) {
			SK_NODE* temp = curr;
			curr = curr->next[0];
			delete temp;
		}
		for(auto& n : head->next)
			n = tail;
	}

	void find(SK_NODE* prevs[], SK_NODE* currs[], int x)
	{
		auto prev = head;
		for (int level = MAX_LEVEL; level >= 0; --level) {
			auto curr = prev->next[level];
			while (curr->value < x) {
				prev = curr;
				curr = curr->next[level];
			}
			prevs[level] = prev;
			currs[level] = curr;
		}
	}

	bool add(int x)
	{
		SK_NODE* prevs[MAX_LEVEL + 1];
		SK_NODE* currs[MAX_LEVEL + 1];
		mtx.lock();
		find(prevs, currs, x);

		if (currs[0]->value == x) {
			mtx.unlock();
			return false;
		}
		else {
			int node_level = 0;
			for(node_level = 0; node_level < MAX_LEVEL; ++node_level) {
				if (rand() % 2 == 0) break;
			}
			auto newNode = new SK_NODE(x, node_level);

			for (int level = 0; level <= node_level; ++level) {
				newNode->next[level] = currs[level];
				prevs[level]->next[level] = newNode;
			}
			mtx.unlock();
			return true;
		}
	}

	bool remove(int x)
	{

		SK_NODE* prevs[MAX_LEVEL + 1];
		SK_NODE* currs[MAX_LEVEL + 1];
		mtx.lock();

		find(prevs, currs, x);

		if (currs[0]->value != x) {
			mtx.unlock();
			return false;
		}
		else {
			int node_level = currs[0]->top_level;
			for (int level = 0; level <= node_level; ++level) {
				prevs[level]->next[level] = currs[0]->next[level];
			}
			mtx.unlock();
			delete currs[0];
			return true;
		}
	}

	bool contains(int x)
	{
		SK_NODE* prevs[MAX_LEVEL + 1];
		SK_NODE* currs[MAX_LEVEL + 1];
		mtx.lock();
		find(prevs, currs, x);
		bool res = (currs[0]->value == x);
		mtx.unlock();
		return res;
	}

	void print20()
	{
		auto curr = head->next[0];
		for (int i = 0; i < 20 && curr != tail; ++i) {
			std::cout << curr->value << ", ";
			curr = curr->next[0];
		}
		std::cout << std::endl;
	}
};


class SKNODE {
public:
	int value;
	SKNODE* volatile next[MAX_LEVEL + 1];
	int top_level;
	volatile bool marked;
	volatile bool fully_linked;
	std::recursive_mutex mtx;
	SKNODE(int x, int top) : value(x), top_level(top),
		marked(false), fully_linked(false)
	{
		for (auto& p : next) p = nullptr;
	}
	SKNODE() : value(-1), top_level(0),
		marked(false), fully_linked(false)
	{
		for (auto& p : next) p = nullptr;
	}
};

class Z_SKLIST {
private:
	SKNODE* head, * tail;
public:
	Z_SKLIST() {
		head = new SKNODE(std::numeric_limits<int>::min(), MAX_LEVEL);
		tail = new SKNODE(std::numeric_limits<int>::max(), MAX_LEVEL);
		for (auto& p : head->next) p = tail;
		head->fully_linked = tail->fully_linked = true;
	}

	~Z_SKLIST()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		SKNODE* curr = head->next[0];
		while (curr != tail) {
			SKNODE* temp = curr;
			curr = curr->next[0];
			delete temp;
		}
		for (auto& p : head->next) p = tail;
	}

	int find(SKNODE* prevs[], SKNODE* currs[], int x)
	{
		int max_level_found = -1;
		auto prev = head;
		for (int level = MAX_LEVEL; level >= 0; --level) {
			auto curr = prev->next[level];
			while (curr->value < x) {
				prev = curr;
				curr = curr->next[level];
			}
			if (max_level_found == -1 && curr->value == x)
				max_level_found = level;
			prevs[level] = prev;
			currs[level] = curr;
		}
		return max_level_found;
	}

	bool add(int x)
	{
		SKNODE* prevs[MAX_LEVEL + 1];
		SKNODE* currs[MAX_LEVEL + 1];

		while (true) {
			int f_level = find(prevs, currs, x);
			if (f_level != -1) {
				SKNODE* nodeFound = currs[f_level];
				if (!nodeFound->marked) {
					while (!nodeFound->fully_linked) { /* spin */ }
					return false;
				}
				continue;
			}
			int node_level = 0;
			for (node_level = 0; node_level < MAX_LEVEL; ++node_level) {
				if (rand() % 2 == 0) break;
			}
			SKNODE* newNode = new SKNODE(x, node_level);
			for (int level = 0; level <= node_level; ++level)
				newNode->next[level] = currs[level];
			int highestLocked = -1;
			bool valid = true;
			for (int level = 0; level <= node_level; ++level) {
				prevs[level]->mtx.lock();
				highestLocked = level;
				if (prevs[level]->marked || prevs[level]->next[level] != currs[level]) {
					valid = false;
					break;
				}
			}
			if (!valid) {
				for (int i = 0; i <= highestLocked; ++i)
					prevs[i]->mtx.unlock();
				delete newNode;
				continue;
			}
			for (int level = 0; level <= node_level; ++level)
				prevs[level]->next[level] = newNode;

			newNode->fully_linked = true;

			for (int i = 0; i <= highestLocked; ++i)
				prevs[i]->mtx.unlock();

			return true;
		}
	}
	bool remove(int x)
	{
		SKNODE* prevs[MAX_LEVEL + 1];
		SKNODE* currs[MAX_LEVEL + 1];

		while (true) {
			int f_level = find(prevs, currs, x);

			if (f_level == -1) return false;
			//Marking
			SKNODE* victim = currs[f_level];
			if (victim->marked) return false;
			if (!victim->fully_linked) return false;
			if (victim->top_level != f_level) return false;
			victim->mtx.lock();
			if (victim->marked) {
				victim->mtx.unlock();
				return false;
			}
			victim->marked = true;

			int top_level = victim->top_level;
			int highestLocked = -1;
			bool valid = true;

			for (int i = 0; i <= top_level; ++i) {
				prevs[i]->mtx.lock();
				highestLocked = i;
				valid = (prevs[i]->marked == false)
					&& (prevs[i]->next[i] == victim);
				if (false == valid) break;
			}
			if (false == valid) {
				for (int i = 0; i <= highestLocked; ++i)
					prevs[i]->mtx.unlock();
				victim->marked = false;
				victim->mtx.unlock();
				continue;
			}
			for (int i = top_level; i >= 0; --i) {
				prevs[i]->next[i] = victim->next[i];
			}
			for (int i = highestLocked; i >= 0; --i)
				prevs[i]->mtx.unlock();
			victim->mtx.unlock();
			// delete victim;
			return true;
		}
	}
	bool contains(int x)
	{
		SKNODE* prevs[MAX_LEVEL + 1];
		SKNODE* currs[MAX_LEVEL + 1];
		int f_level = find(prevs, currs, x);
		return (f_level != -1)
			&& (currs[f_level]->fully_linked == true)
			&& (currs[f_level]->marked == false);
	}

	void print20()
	{
		auto curr = head->next[0];
		for (int i = 0; i < 20 && curr != tail; ++i) {
			std::cout << curr->value << ", ";
			curr = curr->next[0];
		}
		std::cout << std::endl;
	}
};

class LFSKNODE;
class AMRSK { // Atomic Markable Reference
	volatile long long ptr_and_mark;
public:
	AMRSK(LFSKNODE* ptr = nullptr, bool mark = false) {
		long long val = reinterpret_cast<long long>(ptr);
		if (0 != (val & 1)) {  // 수정!!!!!
			std::cout << "ERROR"; exit(-1);
		}
		if (true == mark) val |= 1;
		ptr_and_mark = val;
	}

	LFSKNODE* get_ptr() {
		long long val = ptr_and_mark;
		return reinterpret_cast<LFSKNODE*>(val & 0xFFFFFFFFFFFFFFFE);
	}
	bool get_mark() {
		return (1 == (ptr_and_mark & 1));
	}
	LFSKNODE* get_ptr_and_mark(bool* mark) {
		long long val = ptr_and_mark;
		*mark = (1 == (val & 1));
		return reinterpret_cast<LFSKNODE*>(val & 0xFFFFFFFFFFFFFFFE);
	}

	bool attempt_mark(LFSKNODE* expected_ptr, bool new_mark)
	{
		return CAS(expected_ptr, expected_ptr,
			false, new_mark);
	}

	bool CAS(LFSKNODE* expected_ptr, LFSKNODE* new_ptr,
		bool expected_mark, bool new_mark)
	{
		long long expected_val
			= reinterpret_cast<long long>(expected_ptr);
		if (true == expected_mark) expected_val |= 1;
		long long new_val
			= reinterpret_cast<long long>(new_ptr);
		if (true == new_mark) new_val |= 1;
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<volatile std::atomic<long long> *>(&ptr_and_mark),
			&expected_val, new_val);
	}

};

class LFSKNODE {
public:
	int value;
	AMRSK next[MAX_LEVEL + 1];
	int top_level;
	LFSKNODE(int x, int top) : value(x), top_level(top)
	{
		for (auto& p : next) p = nullptr;
	}
	LFSKNODE() : value(-1), top_level(0)
	{
		for (auto& p : next) p = nullptr;
	}
};

class LF_SKLIST {
private:
	LFSKNODE* head, * tail;
public:
	LF_SKLIST() {
		head = new LFSKNODE(std::numeric_limits<int>::min(), MAX_LEVEL);
		tail = new LFSKNODE(std::numeric_limits<int>::max(), MAX_LEVEL);
		for (auto& p : head->next) p = tail;
	}

	~LF_SKLIST()
	{
		clear();
		delete head;
		delete tail;
	}

	void clear()
	{
		LFSKNODE* curr = head->next[0].get_ptr();
		while (curr != tail) {
			LFSKNODE* temp = curr;
			curr = curr->next[0].get_ptr();
			delete temp;
		}
		for (auto& p : head->next) p = tail;
	}

	bool find(LFSKNODE* prevs[], LFSKNODE* currs[], int x)
	{
	retry:
		auto prev = head;
		for (int level = MAX_LEVEL; level >= 0; --level) {
			auto curr = prev->next[level].get_ptr();
			while (true) {
				bool removed;
				auto succ = curr->next[level].get_ptr_and_mark(&removed);
				while (true == removed) {
					if (false == prev->next[level].CAS(curr, succ, false, false))
						goto retry;
					curr = succ;
					succ = curr->next[level].get_ptr_and_mark(&removed);
				}
				if (curr->value < x) {
					prev = curr;
					curr = succ;
				}
				else break;
			}
			prevs[level] = prev;
			currs[level] = curr;
		}
		return currs[0]->value == x;
	}

	bool add(int x)
	{
		LFSKNODE* prevs[MAX_LEVEL + 1];
		LFSKNODE* currs[MAX_LEVEL + 1];
		while (true) {
			bool found = find(prevs, currs, x);
			if (found) {
				return false;
			}
			int node_level = 0;
			for (node_level = 0; node_level < MAX_LEVEL; ++node_level) {
				if (rand() % 2 == 0) break;
			}
			auto newNode = new LFSKNODE(x, node_level);
			for (int level = 0; level <= node_level; ++level)
				newNode->next[level] = AMRSK(currs[level], false);
			if (true == prevs[0]->next[0].CAS(currs[0], newNode, false, false)) {
				for (int level = 1; level <= node_level; ++level) {
					while (true) {
						if (true == prevs[level]->next[level].CAS(currs[level], newNode, false, false))
							break;
						find(prevs, currs, x);
					}
				}
				return true;
			}
			else delete newNode;
		}
		
	}

	bool remove(int x)
	{
		LFSKNODE* prevs[MAX_LEVEL + 1];
		LFSKNODE* currs[MAX_LEVEL + 1];

		if (false == find(prevs, currs, x))
			return false;
		auto victim = currs[0];
		int top_level = victim->top_level;

		for (int level = top_level; level >= 1; --level) {
			bool removed = false;
			auto succ = victim->next[level].get_ptr_and_mark(&removed);
			while (false == removed) {
				victim->next[level].CAS(succ, succ, false, true);
				succ = victim->next[level].get_ptr_and_mark(&removed);
			}
		}
		bool removed = false;
		auto succ = victim->next[0].get_ptr_and_mark(&removed);
		while (true) {
			bool i_marked_it = victim->next[0].CAS(succ, succ, false, true);
			succ = victim->next[0].get_ptr_and_mark(&removed);
			if (i_marked_it) {
				find(prevs, currs, x);
				return true;
			}
			else if (removed) {
				return false;
			}
		}
	}

	bool contains(int x)
	{
		LFSKNODE* prev = head;
		LFSKNODE* curr = prev->next[0].get_ptr();
		for (int i = MAX_LEVEL; i >= 0; --i) {
			while (true) {
				curr = prev->next[i].get_ptr();	// 수정
				bool removed;
				auto succ = curr->next[i].get_ptr_and_mark(&removed);
				while (true == removed) {
					curr = succ;
					succ = curr->next[i].get_ptr_and_mark(&removed);
				}
				if (curr->value < x) {
					prev = curr;
					curr = succ;
				}
				else break;
			}
		}
		return (curr->value == x);
	}

	void print20()
	{
		auto curr = head->next[0].get_ptr();
		for (int i = 0; i < 20 && curr != tail; ++i) {
			std::cout << curr->value << ", ";
			curr = curr->next[0].get_ptr();
		}
		std::cout << std::endl;
	}
};

LF_SKLIST set;

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
	thread_id = th_id;
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
void benchmark(const int num_threads, int th_id)
{
	thread_id = th_id;
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
	// Consistency check
	std::cout << "Consistency Check\n";
	for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		set.clear();
		std::vector<std::thread> threads;
		for (int i = 0; i < MAX_THREADS; ++i)
			history[i].clear();
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(benchmark_check, num_threads, i);
		for (auto& th : threads)
			th.join();
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - start);
		std::cout << "Threads: " << num_threads
			<< ", Duration: " << duration.count() << " ms.\n";
		std::cout << "Set: "; set.print20();
		check_history(num_threads);
		//set.recycle();
	}
	std::cout << "\nBenchmarking\n";
	for (num_threads = 1; num_threads <= MAX_THREADS; num_threads *= 2) {
		set.clear();
		std::vector<std::thread> threads;
		auto start = high_resolution_clock::now();
		for (int i = 0; i < num_threads; ++i)
			threads.emplace_back(benchmark, num_threads, i);
		for (auto& th : threads)
			th.join();
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(stop - start);
		std::cout << "Threads: " << num_threads
			<< ", Duration: " << duration.count() << " ms.\n";
		std::cout << "Set: "; set.print20();
		//set.recycle();
	}
}