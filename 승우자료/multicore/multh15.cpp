#include <iostream>
#include <thread>

const int LOOP_COUNT = 100000000;

std::atomic<int> x, y;
int trace_x[LOOP_COUNT], trace_y[LOOP_COUNT];


void update_x() {
	for(int i = 0; i < LOOP_COUNT; ++i) {
		x = i;
		trace_y[i] = y;
	}
}

void update_y() {
	for(int i = 0; i < LOOP_COUNT; ++i) {
		y = i;
		trace_x[i] = x;
	}
}

int main()
{
	std::thread t1(update_x);
	std::thread t2(update_y);
	
	t1.join();
	t2.join();

	int count = 0;

	for(int i = 0; i < LOOP_COUNT -1; ++i) {
		if (trace_x[i] == trace_x[i + 1]) {
			if(trace_y[trace_x[i]] == trace_y[trace_x[i] + 1]) {
				if (trace_y[trace_x[i]] == i)
					count++;	
			}
		}
	}
	std::cout << "Total Memory Inconsistency = " << count << std::endl;

}