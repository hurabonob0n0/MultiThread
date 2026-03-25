#include <thread>
#include <iostream>
#include <mutex>

using namespace std;

volatile bool g_done = false;
volatile int* g_bound;
mutex am;
//atomic<int>* g_bound;
int g_error;

void bouncer()
{
	for (int j = 0; j <= 50000000; ++j) {
		am.lock();
		*g_bound = -(1 + *g_bound);
		am.unlock();
	}
	g_done = true;

}
void checker()
{
	while (!g_done) {
		am.lock();
		int v = *g_bound;
		am.unlock();
		if ((v != 0) && (v != -1)) {
			g_error++;
			printf("%X", v);
		}
	}
}

int main()
{
	//g_bound = new int{ 0 };
	int arr[32];
	//atomic<int>arr[32];
	g_bound = arr + 20;
	
	long long temp = reinterpret_cast<long long> (g_bound);
	temp = (temp / 64) * 64;
	//temp = temp - 4;
	//temp = temp - 1; //-- 이렇게 하면 에러가 발생함.
	temp = temp - 2; //-- 이렇게 하면 에러가 발생함.
	g_bound = reinterpret_cast<int*>(temp);

	*g_bound = 0;

	thread r{ checker };
	thread s{ bouncer };
	r.join();
	s.join();

	cout << "err count : " << g_error << endl;
}