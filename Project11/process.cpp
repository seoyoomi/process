#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <functional>
#include <map>
#include <mutex>
#include <chrono>

using namespace std;
using namespace std::chrono;

mutex mtx;

// 명령어 처리 함수들
void echo(const vector<string>& args) {
    if (args.size() > 1) {
        // 두 번째 인수만 출력합니다. 추가 인수는 무시합니다.
        cout << args[1] << endl;
    }
}

void dummy(const vector<string>& args) {
    // 아무것도 하지 않음
}

void gcd(const vector<string>& args) {
    if (args.size() < 3) {
        cout << "Usage: gcd x y" << endl;
        return;
    }
    int x = stoi(args[1]);
    int y = stoi(args[2]);
    while (y != 0) {
        int t = y;
        y = x % y;
        x = t;
    }
    cout << "GCD: " << x << endl;
}

void prime(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: prime x" << endl;
        return;
    }
    int n = stoi(args[1]);
    vector<bool> is_prime(n + 1, true);
    int count = 0;
    for (int p = 2; p * p <= n; ++p) {
        if (is_prime[p]) {
            for (int i = p * p; i <= n; i += p)
                is_prime[i] = false;
        }
    }
    for (int p = 2; p <= n; ++p) {
        if (is_prime[p])
            cout << p << " ";
    }
    cout << endl;
}

void sum(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: sum x" << endl;
        return;
    }
    int n = stoi(args[1]);
    long long total = 0;
    for (int i = 1; i <= n; ++i) {
        total += i;
    }
    cout << "Sum: " << total % 1000000 << endl;
}

// 명령어 파싱
vector<string> parse(const string& input) {
    stringstream ss(input);
    string token;
    vector<string> tokens;
    while (getline(ss, token, ' ')) {
        tokens.push_back(token);
    }
    return tokens;
}

// 옵션 파싱 함수
map<string, string> parseOptions(vector<string>& tokens) {
    map<string, string> options;
    auto it = tokens.begin();
    while (it != tokens.end()) {
        if (it->at(0) == '-') {
            if (next(it) != tokens.end() && next(it)->at(0) != '-') {
                options[*it] = *next(it);
                it = tokens.erase(it);
                it = tokens.erase(it);
            }
            else {
                cerr << "Option " << *it << " needs a value" << endl;
                exit(1);
            }
        }
        else {
            ++it;
        }
    }
    return options;
}

// 명령어 실행
void execute(const vector<string>& tokens, const map<string, string>& options) {
    static map<string, function<void(const vector<string>&)>> commands = {
        {"echo", echo},
        {"dummy", dummy},
        {"gcd", gcd},
        {"prime", prime},
        {"sum", sum}
    };
    if (tokens.empty()) return;
    auto cmd = commands.find(tokens[0]);
    if (cmd == commands.end()) {
        cout << "Unknown command: " << tokens[0] << endl;
        return;
    }

    int n_instances = options.count("-n") ? stoi(options.at("-n")) : 1;
    int duration = options.count("-d") ? stoi(options.at("-d")) : 0;
    int period = options.count("-p") ? stoi(options.at("-p")) : 0;
    int threads = options.count("-m") ? stoi(options.at("-m")) : 1;

    vector<thread> thread_pool;
    for (int i = 0; i < n_instances; ++i) {
        thread_pool.emplace_back([&, i]() {
            auto start_time = steady_clock::now();
            while (true) {
                auto current_time = steady_clock::now();
                if (duration > 0 && duration_cast<seconds>(current_time - start_time).count() >= duration) {
                    break;
                }

                if (tokens[0] == "sum" && threads > 1) {
                    vector<thread> sum_threads;
                    int part = stoi(tokens[1]) / threads;
                    for (int j = 0; j < threads; ++j) {
                        sum_threads.emplace_back([&, j]() {
                            long long local_sum = 0;
                            int start = j * part + 1;
                            int end = (j + 1) * part;
                            if (j == threads - 1) end = stoi(tokens[1]);
                            for (int k = start; k <= end; ++k) {
                                local_sum += k;
                            }
                            lock_guard<mutex> lg(mtx);
                            cout << local_sum % 1000000 << endl;
                            });
                    }
                    for (auto& t : sum_threads) {
                        t.join();
                    }
                }
                else {
                    lock_guard<mutex> lg(mtx);
                    cmd->second(tokens);
                }

                if (period > 0) {
                    this_thread::sleep_for(seconds(period));
                }
                else {
                    break;
                }
            }
            });
    }

    for (auto& t : thread_pool) {
        t.join();
    }
}

int main() {
    ifstream file;
    file.open("commands.txt");
    string line;
    while (getline(file, line)) {
        cout << "prompt> " << line << endl; // 프롬프트 출력
        auto tokens = parse(line);
        auto options = parseOptions(tokens);
        execute(tokens, options);
        this_thread::sleep_for(seconds(2)); // 시간 간격 조절
    }
    file.close();
    return 0;
}
