#include <iostream>
#include <thread>

int sum = 0;

int main()
{
	for (auto i = 0; i < 50000000; ++i)
		sum += 2;
	std::cout << "sum = " << sum << std::endl;
}