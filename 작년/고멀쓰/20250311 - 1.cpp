#include <iostream>
#include <thread>

using namespace std;

int sum = 0;
int sum2 = 0;

void worker() {
	for (auto i = 0; i < 50000000 / 2; ++i)
	{
		//sum += 2;
		_asm add sum, 2;
		/*
		sum += 2;
		sum += 2;
		sum += 2;
		sum += 2;
		sum += 2;
		sum += 2;
		sum += 2;
		sum += 2;
		sum += 2;
		*/
	}
}

//void worker2() {
//	for (auto i = 0; i < 50000000 / 2; ++i)
//		sum2 += 2;
//}
//
//struct worker2 {
//	void operator() (
//		for (auto i = 0; i < 50000000 / 2; ++i)
//			sum += 2;
//	std::cout << "sum = " << sum << std::endl;
//	);
//};

int main()
{
	char ch;
	cin >> ch;

	std::thread t1{ worker };
	std::thread t2{ worker };
	// 모든 쓰레드가 종료될 때까지 기다려야 함.
	t1.join();
	t2.join();

	std::cout << "sum = " << sum << std::endl;
}