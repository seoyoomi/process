#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

using namespace std;
mutex cout_mutex;

void shell(int id, int sleep_time) {
    while (true) {
        // Command simulation (Replace this section with actual command parsing and execution)
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "Running: [" << id << "F]" << endl;
        }
        this_thread::sleep_for(chrono::seconds(sleep_time));
    }
}

void monitor(int id, int interval) {
    while (true) {
        {
            lock_guard<mutex> lock(cout_mutex);
            cout << "---------------------------" << endl;
            cout << "DQ and WQ status from Monitor Process [" << id << "B]" << endl;
            // Here you would actually fetch and display the status of DQ and WQ
            cout << "---------------------------" << endl;
        }
        this_thread::sleep_for(chrono::seconds(interval));
    }
}

int main() {
    thread shell_process(shell, 0, 5);  // Assuming shell process has ID 0 and sleeps for 5 seconds
    thread monitor_process(monitor, 1, 10);  // Assuming monitor process has ID 1 and prints every 10 seconds

    shell_process.join();
    monitor_process.join();

    return 0;
}
