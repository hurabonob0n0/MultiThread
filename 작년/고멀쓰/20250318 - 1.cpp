#include <iostream>
#include <thread>
#include <mutex>

volatile bool g_ready = false;
volatile int g_data = 0;
std::mutex gl;

void Receiver()
{
	int count{};
	while (false == g_ready) {
		++count;
	} 
	std::cout << "I got : " << g_data << std::endl;
	std::cout << "Count : " << count << std::endl;
}

void Sender()
{
	int tt;
	std::cout << "Enter Number : ";
	std::cin >> tt;
	g_data = tt;
	g_ready = true;
}

int main()
{
	std::thread r{ Receiver };
	std::thread s{ Sender };
	r.join(); s.join();
}

