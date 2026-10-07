#include <iostream>
#include <string>
#include <sstream>
#include <unordered_set>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int N;
    if (!(std::cin >> N)) return 0;
    std::string line;
    std::getline(std::cin, line);

    for (int i = 0; i < N; ++i) {
        std::getline(std::cin, line);
        if (line.empty()) { --i; continue; }

        std::istringstream iss(line);
        std::string key;
        std::unordered_set<std::string> keys;
        while (iss >> key) {
            keys.insert(key);
        }

        int advance = 0;
        int strafe = 0;

        if (keys.count("W") || keys.count("Z")) advance += 1;
        if (keys.count("S")) advance -= 1;
        if (keys.count("A") || keys.count("Q")) strafe -= 1;
        if (keys.count("D")) strafe += 1;

        std::cout << advance << " " << strafe << "\n";
    }

    return 0;
}