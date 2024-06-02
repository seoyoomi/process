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

// 명령어 실행
void execute(const vector<string>& tokens) {
    static map<string, function<void(const vector<string>&)>> commands = {
        {"echo", echo},
        {"dummy", dummy},
        {"gcd", gcd},
        {"sum", sum},
        {"prime", prime}
    };

    if (tokens.empty()) return;
    auto cmd = commands.find(tokens[0]);
    if (cmd != commands.end()) {
        cmd->second(tokens);
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
        cout << "prompt> " << line << endl; // 프롬프트 출력
        auto tokens = parse(line);
        lock_guard<mutex> lg(mtx);
        execute(tokens);
        this_thread::sleep_for(seconds(1)); // 시간 간격 조절
    }

    file.close();
    return 0;
}
