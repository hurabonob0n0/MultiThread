#define ex13
#define chapter2

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>

using namespace std;
using namespace std::chrono;

#ifdef chapter1
const int ci50000000 = 50000000;

int sum;
volatile int vsum;
volatile std::atomic_int asum;
std::mutex am;
volatile int sumarr[16];
struct CACHE_INT {
	alignas(64) volatile int sum;
};
volatile CACHE_INT alignedSumarr[16];

std::chrono::steady_clock::time_point g_StartTime;

inline void Timer_Start() { g_StartTime = high_resolution_clock::now(); }
inline void elapsed_time() {
	auto d = high_resolution_clock::now() - g_StartTime;
	std::cout << duration_cast<milliseconds>(d).count() << " msecs\n";
}


#pragma region 1장 관련 함수
void thread_plus2(const int numThreads)
{
	for (auto i = 0; i < ci50000000 / numThreads; ++i) {
		sum += 2;
	}
}

void thread_plus2_with_mutex(const int numThreads)
{
	for (auto i = 0; i < ci50000000 / numThreads; ++i) {
		am.lock();
		vsum += 2;
		am.unlock();
	}
}

void thread_plus2_with_atomic_int(const int numThreads)
{
	for (auto i = 0; i < ci50000000 / numThreads; ++i) {
		asum += 2;
	}
}

void thread_plus2_Optimistic(const int numThreads)
{
	volatile int local_sum = 0;
	for (auto i = 0; i < ci50000000 / numThreads; ++i) {
		local_sum += 2;
	}
	am.lock();
	vsum += local_sum;
	am.unlock();
}

void thread_plus2_Optimistic_2(const int numThreads)
{
	volatile int local_sum = 0;
	for (auto i = 0; i < ci50000000 / numThreads; ++i) {
		local_sum += 2;
	}
	asum += local_sum;
}

void thread_plus2_Optimistic_3(const int numThreads, const int thread_id)
{
	sumarr[thread_id] = 0;
	for (auto i = 0; i < ci50000000 / numThreads; ++i) {
		sumarr[thread_id] += 2;
	}
	am.lock();
	vsum += sumarr[thread_id];
	am.unlock();
}

void thread_plus2_Optimistic_4(const int numThreads, const int thread_id)
{
	alignedSumarr[thread_id].sum = 0;
	for (auto i = 0; i < ci50000000 / numThreads; ++i) {
		alignedSumarr[thread_id].sum += 2;
	}
	am.lock();
	vsum += alignedSumarr[thread_id].sum;
	am.unlock();
}


inline void printvsumonconsole() { cout << vsum << endl; }
#pragma endregion



#ifdef ex1
int main()
{
	sum = 0;
	Timer_Start();
	for (auto i = 0; i < ci50000000; ++i)
		vsum = vsum + 2;
	std::cout << "vsum : " << vsum << std::endl;
	elapsed_time();		//73ms
}
#endif // ex1

#ifdef ex3
int main()
{
	Timer_Start();
	thread t1{ thread_plus2,2 };
	thread t2{ thread_plus2,2 };
	t1.join(); t2.join();
	cout << vsum << endl;
	elapsed_time();		//40ms
}
#endif //Data Race를 발생시킨다.

#ifdef ex4
int main()
{
	Timer_Start();
	thread t1{ thread_plus2_with_mutex,2 };
	thread t2{ thread_plus2_with_mutex,2 };
	t1.join(); t2.join();
	cout << vsum << endl;
	elapsed_time();		//721ms
}
#endif

#ifdef ex5 // 쓰레드 개수 2배씩 늘리면서 해보기
int main()
{
	for (int j = 1; j <= 16; j *= 2)
	{
		sum = 0;
		vector<thread> vecthreads;
		Timer_Start();
		for (int i = 0; i < j; ++i) {
			vecthreads.emplace_back(thread_plus2, j);
		}
		for (auto& threads : vecthreads)
			threads.join();
		cout << "--------" << "num threads : " << j << " --------" << endl;
		cout << "sum : " << sum << endl;
		elapsed_time();
		cout << endl;
	}

	for (int j = 1; j <= 16; j *= 2)
	{
		vsum = 0;
		vector<thread> vecthreads;
		Timer_Start();
		for (int i = 0; i < j; ++i) {
			vecthreads.emplace_back(thread_plus2_with_mutex, j);
		}
		for (auto& threads : vecthreads)
			threads.join();
		cout << "--------" << "num threads : " << j << " --------" << endl;
		printvsumonconsole();
		elapsed_time();
	}

	//	--------num threads : 1 --------
	//	100000000
	//	554 msecs
	//	--------num threads : 2 --------
	//	100000000
	//	557 msecs
	//	--------num threads : 4 --------
	//	100000000
	//	566 msecs
	//	--------num threads : 8 --------
	//	100000000
	//	564 msecs
	//	--------num threads : 16 --------
	//	100000000
	//	572 msecs
}
#endif

#ifdef ex6
int main()
{
	for (int j = 1; j <= 16; j *= 2)
	{
		asum = 0;
		vector<thread> vecthreads;
		Timer_Start();
		for (int i = 0; i < j; ++i) {
			vecthreads.emplace_back(thread_plus2_with_atomic_int, j);
		}
		for (auto& threads : vecthreads)
			threads.join();
		cout << "--------" << "num threads : " << j << " --------" << endl;
		cout << "atomic sum : " << asum << endl;
		elapsed_time();
	}
	/*--------num threads : 1 --------
		atomic sum : 100000000
		175 msecs
		--------num threads : 2 --------
		atomic sum : 100000000
		174 msecs
		--------num threads : 4 --------
		atomic sum : 100000000
		173 msecs
		--------num threads : 8 --------
		atomic sum : 100000000
		175 msecs
		--------num threads : 16 --------
		atomic sum : 100000000
		181 msecs
		*/
}
#endif

#ifdef ex8
int main()
{
	for (int j = 1; j <= 16; j *= 2)
	{
		vsum = 0;
		vector<thread> vecthreads;
		Timer_Start();
		for (int i = 0; i < j; ++i) {
			vecthreads.emplace_back(thread_plus2_Optimistic, j);
		}
		for (auto& threads : vecthreads)
			threads.join();
		cout << "--------" << "num threads : " << j << " --------" << endl;
		cout << "volatile sum : " << vsum << endl;
		elapsed_time();
	}

	for (int j = 1; j <= 16; j *= 2)
	{
		asum = 0;
		vector<thread> vecthreads;
		Timer_Start();
		for (int i = 0; i < j; ++i) {
			vecthreads.emplace_back(thread_plus2_Optimistic_2, j);
		}
		for (auto& threads : vecthreads)
			threads.join();
		cout << "--------" << "num threads : " << j << " --------" << endl;
		cout << "atomic sum : " << asum << endl;
		elapsed_time();
	}

	//--------num threads : 1 --------
	//	volatile sum : 100000000
	//	12 msecs
	//	--------num threads : 2 --------
	//	volatile sum : 100000000
	//	10 msecs
	//	--------num threads : 4 --------
	//	volatile sum : 100000000
	//	10 msecs
	//	--------num threads : 8 --------
	//	volatile sum : 100000000
	//	12 msecs
	//	--------num threads : 16 --------
	//	volatile sum : 100000000
	//	16 msecs
	//	--------num threads : 1 --------

}
#endif

#ifdef ex9
int main()
{
	for (int j = 1; j <= 16; j *= 2)
	{
		vsum = 0;
		vector<thread> vecthreads;
		Timer_Start();
		for (int i = 0; i < j; ++i) {
			vecthreads.emplace_back(thread_plus2_Optimistic_3, j, i);
		}
		for (auto& threads : vecthreads)
			threads.join();
		cout << "--------" << "num threads : " << j << " --------" << endl;
		cout << "volatile sum : " << vsum << endl;
		elapsed_time();
	}
	//--------num threads : 1 --------
	//	volatile sum : 100000000
	//	73 msecs
	//	--------num threads : 2 --------
	//	volatile sum : 100000000
	//	72 msecs
	//	--------num threads : 4 --------
	//	volatile sum : 100000000
	//	72 msecs
	//	--------num threads : 8 --------
	//	volatile sum : 100000000
	//	74 msecs
	//	--------num threads : 16 --------
	//	volatile sum : 100000000
	//	76 msecs

}
#endif // 굳이 로컬변수로 해야하나? 그냥 전역변수로 여러 개 두면 안돼??? 그래서 전역변수 sumarr를 써봤다. 시간이 많이 늦어졌다. 
// 왜? 같은 캐시 라인에 있는 값을 계속 갱신해서! 캐시 라인은 64바이트다. 그래서 같은 캐시 라인 안에 있는 값을 다른 캐시에서 쓰려면 다른 캐시 라인을 무효화(invalidate)시키기 때문에 cache miss가 많이 나서 느려진다.

#ifdef ex10
int main()
{
	for (int j = 1; j <= 16; j *= 2)
	{
		vsum = 0;
		vector<thread> vecthreads;
		Timer_Start();
		for (int i = 0; i < j; ++i) {
			vecthreads.emplace_back(thread_plus2_Optimistic_4, j, i);
		}
		for(auto& threads : vecthreads)
			threads.join();
		cout << "--------" << "num threads : " << j << " --------" << endl;
		cout << "volatile sum : " << vsum << endl;
		elapsed_time();
	}
	/*--------num threads : 1 --------
		volatile sum : 100000000
		75 msecs
		--------num threads : 2 --------
		volatile sum : 100000000
		71 msecs
		--------num threads : 4 --------
		volatile sum : 100000000
		74 msecs
		--------num threads : 8 --------
		volatile sum : 100000000
		73 msecs
		--------num threads : 16 --------
		volatile sum : 100000000
		66 msecs*/
}
#endif // 따라서 다른 전역변수에다가 좀 모아두고 싶으면? 64바이트의 공간을 할당해놓은 구조체 변수를 사용해서 한다. 이렇게 되면 

#endif // chapter1

#ifdef chapter2
volatile bool g_ready = false;
int g_data = 0;

void Receiver()
{
	while (false == g_ready);
	cout << "I got " << g_data << endl;
}

void Sender()
{
	cin >> g_data;
	g_ready = true;
}

volatile int sum = 0;
volatile bool flag[2]{ false,false };
volatile int victim = 0;

void pLock(int myID)
{
	int other = 1 - myID;
	flag[myID] = true;
	victim = myID;
	while (flag[other] && victim == myID) {}
	//교수님 코드는 while(iswaiting[other] && victim == myID){}
}

void pUnlock(int myID)
{
	flag[myID] = false;
}

void ThreadFunc(int thid)
{
	for (auto i = 0; i < 25000000; ++i)
	{
		pLock(thid);
		sum = sum + 2;
		pUnlock(thid);
	}
}



#endif // chapter2



#ifdef ex11
int main()
{
	// 귀찮은데 다음에 니가 해봐. 대충 내용은 컴파일러가 멀티쓰레드 지원 안한다는 내용
}
#endif

#ifdef ex12
int main()
{
	thread t1{ Sender };
	thread t2{ Receiver };
	t1.join(); t2.join();
}
#endif // 이렇게 하면 해결이 되긴 된다. // 여기 뒤로 volatile 포인터에 대해 나오는데 여기는 나도 이해가 안된다.

#ifdef ex13
int main()
{
	thread t1{ ThreadFunc,0 };
	thread t2{ ThreadFunc,1 };
	t1.join(); t2.join();

	cout << "Sum = " << sum << '\n';
}
#endif

#ifdef ex14
int main()
{

}
#endif

#ifdef ex15
int main()
{

}
#endif

#ifdef ex16
int main()
{

}
#endif

#ifdef ex17
int main()
{

}
#endif

#ifdef ex18
int main()
{

}
#endif

#ifdef ex19
int main()
{

}
#endif

#ifdef ex20
int main()
{

}
#endif

#ifdef ex21
int main()
{

}
#endif

#ifdef ex22
int main()
{

}
#endif
