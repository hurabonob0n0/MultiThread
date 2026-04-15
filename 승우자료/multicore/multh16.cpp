#include <iostream>
#include <thread>

using namespace std;

volatile int* ptr = nullptr;
volatile bool done = false;
int error;


void update_ptr()
{
	for (int i = 0; i <= 25000000; ++i) {
		*ptr = -(1 + *ptr);
	}
	done = true;
}
void watch_ptr()
{
	error = 0;
	while (!done) {
		int v = *ptr;
		if ((v != 0) && (v != -1)) {
			printf("%X ", v);
			error++;
		}
	}

	cout << "Error = " << error << endl;
}

int main()
{
	int value[32];
	long long addr = reinterpret_cast<long long>(&value[31]);
	addr = addr - (addr % 64);
	addr = addr - 1;
	ptr = reinterpret_cast<int*>(addr);
	*ptr = 0;
	thread t1(watch_ptr);
	thread t2(update_ptr);
	t1.join();
	t2.join();
}