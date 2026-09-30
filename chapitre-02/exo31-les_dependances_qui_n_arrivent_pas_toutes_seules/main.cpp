#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <sstream>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int N;
    if (!(std::cin >> N)) {
        return 0;
    }
    std::string line;
    std::getline(std::cin, line);

    std::unordered_map<std::string, std::vector<std::string>> deps;
    for (int i = 0; i < N; ++i) {
        std::getline(std::cin, line);
        if (line.empty()) {
            --i;
            continue;
        }
        std::istringstream iss(line);
        std::string module;
        iss >> module;
        std::string dep;
        while (iss >> dep) {
            deps[module].push_back(dep);
        }
        if (deps.find(module) == deps.end()) {
            deps[module] = {};
        }
    }

    int M;
    std::cin >> M;
    std::getline(std::cin, line);
    std::getline(std::cin, line);
    std::istringstream iss(line);
    std::vector<std::string> direct;
    std::string mod;
    while (iss >> mod) {
        direct.push_back(mod);
    }

    std::unordered_set<std::string> all;
    std::queue<std::string> q;
    for (const auto& m : direct) {
        if (all.insert(m).second) {
            q.push(m);
        }
    }

    while (!q.empty()) {
        std::string cur = q.front();
        q.pop();
        for (const auto& d : deps[cur]) {
            if (all.insert(d).second) {
                q.push(d);
            }
        }
    }

    std::vector<std::string> result(all.begin(), all.end());
    std::sort(result.begin(), result.end());

    for (const auto& m : result) {
        std::cout << m << "\n";
    }

    return 0;
}