//⚫ 실습 #7 :
//─ 다음 프로그램들의 속도 비교를 하라
//• Lock이 없는 처음 프로그램
//• Atomic 연산을 적용한 프로그램
//• Lock을 사용한 프로그램
// + 베이커리 알고리즘

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>
#include <mutex>
#include <chrono>

using namespace std::chrono;

volatile int sum;
volatile int* label;		// 값을 바꿀 때 volatile 적용
// int* volatile label2;	// 주소값을 바꿀 때 volatile 적용
volatile bool* flag;		// *flag = something 할 때 volatile 적용
// bool* volatile flag2;	// flag2 = something 할 때 volatile 적용
std::mutex am;
std::atomic_int asum;

steady_clock::time_point g_StartTime;

inline void Timer_Start() { g_StartTime = high_resolution_clock::now(); }
inline void elapsed_time() {
	auto d = high_resolution_clock::now() - g_StartTime;
	std::cout << duration_cast<milliseconds>(d).count() << " msecs\n";
}

void Bakery(int n)
{
	if (flag) delete[] flag;
	if (label) delete[] label;

	flag = nullptr;
	label = nullptr;

	flag = new bool[n];
	label = new int[n];

	for (int i = 0; i < n; ++i) {
		flag[i] = false;
		label[i] = 0;
	}
}

void Bakery_Free(int n)
{
	delete[] flag;
	delete[] label;
	flag = nullptr;
	label = nullptr;
}

void B_Lock(int threadID,int threadNum) 
{
	int i = threadID;
	flag[i] = true;
	label[i] = *std::max_element(label, label + threadNum) + 1;

	for (int j = 0; j < threadNum; ++j)
	{
		if (j == i)
			continue;
		//while (flag[j] && label[i] > *std::min_element(label, label+threadNum)) {}
		while (flag[j] && (label[j] != 0) && (label[i] > label[j])) {}
	}
}

void B_Unlock(int threadID)
{
	flag[threadID] = false;
}

void NoLock(int threadNum)
{
	for (int i = 0; i < 5000000 / threadNum; ++i)
	{
		sum += 2;
	}
}

void LockMutex(int threadNum)
{
	for (int i = 0; i < 5000000 / threadNum; ++i)
	{
		am.lock();
		sum += 2;
		am.unlock();
	}
}

void BakeryVolatile(int threadID, int threadNum)
{
	for (int i = 0; i < 5000000 / threadNum; ++i)
	{
		B_Lock(threadID, threadNum);
		sum += 2;
		B_Unlock(threadID);
	}
}

void BakeryAtomic(int threadID,int threadNum)
{
	for (int i = 0; i < 5000000 / threadNum; ++i) 
	{
		B_Lock(threadID,threadNum);
		asum += 2;
		B_Unlock(threadID);
	}
}

int main()
{
	{
		std::cout << "==================NoLock====================\n";
		for (int i = 1; i <= 8; i *= 2)
		{
			asum = 0;
			sum = 0;
			std::cout << "-------- " << "threads num : " << i<< " --------" << '\n';
			std::vector<std::thread> NoLockThread;
			Timer_Start();
			for (int j = 1; j <= i; ++j)
			{
				NoLockThread.emplace_back(NoLock,i);
			}
			for (auto& threads : NoLockThread)
			{
				threads.join();
			}
			elapsed_time();
			std::cout << "Sum : " << sum << '\n';

		}

	}

	{
		
		std::cout << "==================LockwithMutex====================\n";
		for (int i = 1; i <= 8; i *= 2)
		{
			asum = 0;
			sum = 0;
			std::cout << "-------- " << "threads num : " << i << " --------" << '\n';
			std::vector<std::thread> MutexThread;
			Timer_Start();
			for (int j = 1; j <= i; ++j)
			{
				MutexThread.emplace_back(LockMutex, i);
			}
			for (auto& threads : MutexThread)
			{
				threads.join();
			}
			elapsed_time();
			std::cout << "Sum : " << sum << '\n';

		}

	}

	{
		
		std::cout << "==================LockwithBakery====================\n";
		for (int i = 1; i <= 8; i *= 2)
		{
			asum = 0;
			sum = 0;
			std::cout << "-------- " << "threads num : " << i << " --------" << '\n';
			std::vector<std::thread> BakeryThread;
			Bakery(i);
			Timer_Start();
			for (int j = 1; j <= i; ++j)
			{
				BakeryThread.emplace_back( BakeryVolatile,j ,i);
			}
			for (auto& threads : BakeryThread)
			{
				threads.join();
			}
			elapsed_time();
			std::cout << "Sum : " << sum << '\n';

		}

	}

	{
		
		std::cout << "==================LockwithAtomicBakery====================\n";
		for (int i = 1; i <= 8; i *= 2)
		{
			asum = 0;
			sum = 0;
			std::cout << "-------- " << "threads num : " << i << " --------" << '\n';
			std::vector<std::thread> BakeryThread;
			Bakery(i);
			Timer_Start();
			for (int j = 1; j <= i; ++j)
			{
				BakeryThread.emplace_back( BakeryAtomic,j ,i );
			}
			for (auto& threads : BakeryThread)
			{
				threads.join();
			}
			elapsed_time();
			std::cout << "Sum : " << asum << '\n';

		}

	}
}
