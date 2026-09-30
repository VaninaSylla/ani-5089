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

    int P;
    if (!(std::cin >> P)) {
        return 0;
    }
    std::string line;
    std::getline(std::cin, line);

    std::vector<std::pair<std::string, std::string>> prefixes;
    for (int i = 0; i < P; ++i) {
        std::getline(std::cin, line);
        if (line.empty()) {
            --i;
            continue;
        }
        std::istringstream iss(line);
        std::string prefix, module;
        iss >> prefix >> module;
        prefixes.emplace_back(prefix, module);
    }

    std::sort(prefixes.begin(), prefixes.end(),
        [](const auto& a, const auto& b) {
            return a.first.size() > b.first.size();
        });

    int L;
    std::cin >> L;
    std::getline(std::cin, line);

    std::unordered_set<std::string> modules;
    int unknown = 0;

    for (int i = 0; i < L; ++i) {
        std::getline(std::cin, line);
        size_t pos = line.find("undefined reference to");
        if (pos == std::string::npos) {
            continue;
        }
        pos += 22;  // length of "undefined reference to"
        while (pos < line.size() &&
               (line[pos] == ' ' || line[pos] == '\t' || line[pos] == '\'')) {
            pos++;
        }
        size_t end = line.find("'", pos);
        if (end == std::string::npos) {
            continue;
        }
        std::string symbol = line.substr(pos, end - pos);

        bool found = false;
        for (const auto& [prefix, module] : prefixes) {
            if (symbol.rfind(prefix, 0) == 0) {
                modules.insert(module);
                found = true;
                break;
            }
        }
        if (!found) {
            unknown++;
        }
    }

    std::vector<std::string> result(modules.begin(), modules.end());
    std::sort(result.begin(), result.end());
    for (const auto& m : result) {
        std::cout << m << "\n";
    }
    if (unknown > 0) {
        std::cout << "INCONNU " << unknown << "\n";
    }

    return 0;
}
