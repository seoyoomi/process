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
#include <condition_variable>
#include <future>

using namespace std;
using namespace std::chrono;

mutex mtx;

// 명령어 옵션 파싱
struct CommandOptions {
    int repeat = 1;      // 명령을 반복 실행할 횟수
    int duration = 200;  // 명령 실행 최대 시간 (초)
    int period = 0;      // 주기적 실행 간격 (초)
    int multithread = 1; // 병렬 처리를 위한 스레드 수 (기본값은 1, 단일 스레드)

    void parseOptions(const vector<string>& args) {
        for (size_t i = 2; i < args.size(); i += 2) {
            if (args[i] == "-n" && i + 1 < args.size()) {
                repeat = stoi(args[i + 1]);
            }
            else if (args[i] == "-d" && i + 1 < args.size()) {
                duration = stoi(args[i + 1]);
            }
            else if (args[i] == "-p" && i + 1 < args.size()) {
                period = stoi(args[i + 1]);
            }
            else if (args[i] == "-m" && i + 1 < args.size()) {
                multithread = stoi(args[i + 1]);
            }
        }
    }
};

// 명령어 처리 함수들
void echo(const vector<string>& args, const CommandOptions& opts) {
    for (int i = 0; i < opts.repeat; ++i) {
        if (args.size() > 1) {
            cout << args[1] << endl;
        }
        this_thread::sleep_for(seconds(opts.period));
    }
}

void dummy(const vector<string>& args, const CommandOptions& opts) {
    for (int i = 0; i < opts.repeat; ++i) {
        // 실제로 아무 일도 하지 않지만, 옵션에 따라 반복 및 일시 중지
        this_thread::sleep_for(seconds(opts.period));
    }
}

void gcd(const vector<string>& args, const CommandOptions& opts) {
    if (args.size() < 3) {
        cout << "Usage: gcd x y" << endl;
        return;
    }
    int x = stoi(args[1]);
    int y = stoi(args[2]);
    for (int i = 0; i < opts.repeat; ++i) {
        int t;
        while (y != 0) {
            t = y;
            y = x % y;
            x = t;
        }
        cout << "GCD: " << x << endl;
        this_thread::sleep_for(seconds(opts.period));
    }
}

void prime(const vector<string>& args, const CommandOptions& opts) {
    if (args.size() < 2) {
        cout << "Usage: prime x" << endl;
        return;
    }
    int n = stoi(args[1]);
    for (int r = 0; r < opts.repeat; ++r) {
        vector<bool> is_prime(n + 1, true);
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
        this_thread::sleep_for(seconds(opts.period));
    }
}

void sum(const vector<string>& args, const CommandOptions& opts) {
    if (args.size() < 2) {
        cout << "Usage: sum x" << endl;
        return;
    }
    int n = stoi(args[1]);

    auto partial_sum = [](int start, int end) -> long long {
        long long total = 0;
        for (int i = start; i <= end; ++i) {
            total += i;
        }
        return total;
    };

    for (int r = 0; r < opts.repeat; ++r) {
        if (opts.multithread > 1) {
            vector<future<long long>> futures;
            int step = n / opts.multithread;
            long long total = 0;

            for (int i = 0; i < opts.multithread; ++i) {
                int start = i * step + 1;
                int end = (i == opts.multithread - 1) ? n : (i + 1) * step;
                futures.push_back(async(launch::async, partial_sum, start, end));
            }

            for (auto& f : futures) {
                total += f.get();
            }
            cout << "Sum: " << total % 1000000 << endl;
        }
        else {
            long long total = 0;
            for (int i = 1; i <= n; ++i) {
                total += i;
            }
            cout << "Sum: " << total % 1000000 << endl;
        }
        this_thread::sleep_for(seconds(opts.period));
    }
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

// 명령어 실행
void execute(const vector<string>& tokens) {
    static map<string, function<void(const vector<string>&, const CommandOptions&)>> commands = {
        {"echo", echo},
        {"dummy", dummy},
        {"gcd", gcd},
        {"prime", prime},
        {"sum", sum}
        // 추가적인 명령어 함수 매핑 필요
    };

    if (tokens.empty()) return;
    auto cmd = commands.find(tokens[0]);
    if (cmd != commands.end()) {
        CommandOptions opts;
        opts.parseOptions(tokens);
        cmd->second(tokens, opts);
    }
    else {
        cout << "Unknown command: " << tokens[0] << endl;
    }
}

int main() {
    ifstream file;
    file.open("commands.txt");
    string line;

    while (getline(file, line)) {
        cout << "prompt> " << line << endl;
        auto tokens = parse(line);
        lock_guard<mutex> lg(mtx);
        execute(tokens);
        this_thread::sleep_for(seconds(1)); // 시간 간격 조절
    }

    file.close();
    return 0;
}
