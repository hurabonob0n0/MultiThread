#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>
#include <mutex>
#include <chrono>

using namespace std;
using namespace std::chrono;

// --- 전역 변수 설정 ---
volatile int g_sum = 0;
std::atomic<int> g_asum(0);
std::mutex g_mutex;

// 베이커리 알고리즘용 변수
volatile bool* choosing;
volatile int* label;
int g_num_threads;

// CAS 락용 변수
std::atomic<int> g_cas_lock(0);

// --- 타이머 ---
steady_clock::time_point start_time;
void timer_start() { start_time = high_resolution_clock::now(); }
void print_elapsed(const string& label, int threads) {
    auto d = high_resolution_clock::now() - start_time;
    cout << label << " (" << threads << " threads): "
        << duration_cast<milliseconds>(d).count() << " ms | Sum: " << g_sum << endl;
}

// --- 1. No Lock (Data Race 발생) ---
void work_nolock(int count) {
    for (int i = 0; i < count; ++i) g_sum += 2;
}

// --- 2. Mutex Lock ---
void work_mutex(int count) {
    for (int i = 0; i < count; ++i) {
        //lock_guard<mutex> lock(g_mutex);
        g_mutex.lock();
        g_sum += 2;
        g_mutex.unlock();
    }
}

// --- 3. Bakery Algorithm ---
void bakery_lock(int i) {
    choosing[i] = true;
    int max_label = 0;
    for (int k = 0; k < g_num_threads; ++k) {
        if (label[k] > max_label) max_label = label[k];
    }
    label[i] = max_label + 1; // 티켓 번호 할당 
    choosing[i] = false;

    for (int j = 0; j < g_num_threads; ++j) {
        if (i == j) continue;
        while (choosing[j]); // 다른 쓰레드가 번호 고르는 중이면 대기
        // 번호가 작거나, 번호가 같으면 ID가 작은 쓰레드 우선 
        while (label[j] != 0 && (label[j] < label[i] || (label[j] == label[i] && j < i)));
    }
}

void bakery_unlock(int i) {
    label[i] = 0;
}

void work_bakery(int count, int tid) {
    for (int i = 0; i < count; ++i) {
        bakery_lock(tid);
        g_sum += 2;
        bakery_unlock(tid);
    }
}

void cas_lock() {
    int expected = 0;
    // 0(Unlock 상태)이면 1(Lock 상태)로 바꾸고 탈출, 실패하면 계속 시도(Spin)
    while (!g_cas_lock.compare_exchange_strong(expected, 1)) {
        expected = 0;
    }
}

void cas_unlock() {
    g_cas_lock.store(0);
}

void work_cas(int count) {
    for (int i = 0; i < count; ++i) {
        cas_lock();
        g_sum += 2;
        cas_unlock();
    }
}

// --- 테스트 실행기 ---
void run_test(int num_threads, const string& type) {
    g_sum = 0;
    g_asum = 0;
    g_cas_lock = 0;
    int count_per_thread = 50000000 / num_threads;

    vector<thread> workers;
    timer_start();

    for (int i = 0; i < num_threads; ++i) {
        if (type == "NoLock") workers.emplace_back(work_nolock, count_per_thread);
        else if (type == "Mutex") workers.emplace_back(work_mutex, count_per_thread);
        else if (type == "Bakery") workers.emplace_back(work_bakery, count_per_thread, i);
        else if (type == "CAS") workers.emplace_back(work_cas, count_per_thread);
    }

    for (auto& w : workers) w.join();
    print_elapsed(type, num_threads);
}

int main() {
    int thread_counts[] = { 1, 2, 4, 8 ,16};

    for (int tc : thread_counts) {
        cout << "\n--- Testing with " << tc << " Threads ---" << endl;
        g_num_threads = tc;
        choosing = new bool[tc] {false};
        label = new int[tc] {0};

        run_test(tc, "NoLock");
        run_test(tc, "Mutex");
        run_test(tc, "Bakery");
        run_test(tc, "CAS");

        delete[] choosing;
        delete[] label;
    }
    return 0;
}