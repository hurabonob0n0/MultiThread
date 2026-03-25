#include <iostream>
#include <thread>
#include <mutex>

bool g_ready = false;
int g_data = 0;
std::mutex gl;

void Receiver()
{
	int count{};
	gl.lock();
	while (false == g_ready) {
		gl.unlock();
		++count;
		gl.lock();
	} 
	std::cout << "I got : " << g_data << std::endl;
	gl.unlock();
	std::cout << "Count : " << count << std::endl;
}

void Sender()
{
	std::cout << "Enter Number : ";
	std::cin >> g_data;
	gl.lock();
	g_ready = true;
	gl.unlock();
}

int main()
{
	std::thread r{ Receiver };
	std::thread s{ Sender };
	r.join(); s.join();
}

