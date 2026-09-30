#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <sstream>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int V;
    if (!(std::cin >> V)) {
        return 0;
    }
    std::string line;
    std::getline(std::cin, line);

    std::unordered_map<std::string, std::string> machine;
    for (int i = 0; i < V; ++i) {
        std::getline(std::cin, line);
        if (line.empty()) {
            --i;
            continue;
        }
        size_t eq = line.find("=");
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            machine[key] = val;
        }
    }

    int F;
    std::cin >> F;
    std::getline(std::cin, line);

    for (int i = 0; i < F; ++i) {
        std::getline(std::cin, line);
        if (line.empty()) {
            --i;
            continue;
        }

        std::vector<std::string> terms;
        std::istringstream iss(line);
        std::string term;
        while (iss >> term) {
            if (term == "&&") {
                continue;
            }
            terms.push_back(term);
        }

        bool allTrue = true;
        for (const auto& t : terms) {
            bool neg = false;
            std::string termStr = t;
            if (!termStr.empty() && termStr[0] == '!') {
                neg = true;
                termStr = termStr.substr(1);
            }
            size_t eq = termStr.find("=");
            if (eq == std::string::npos) {
                allTrue = false;
                break;
            }
            std::string key = termStr.substr(0, eq);
            std::string val = termStr.substr(eq + 1);

            bool termVal = false;
            auto it = machine.find(key);
            if (it != machine.end() && it->second == val) {
                termVal = true;
            }
            if (neg) {
                termVal = !termVal;
            }
            if (!termVal) {
                allTrue = false;
                break;
            }
        }

        std::cout << (allTrue ? "OUI" : "NON") << "\n";
    }

    return 0;
}
