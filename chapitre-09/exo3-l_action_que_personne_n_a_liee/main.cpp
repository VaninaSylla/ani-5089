#include <iostream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

int main() {
    int a = 0;
    std::cin >> a;

    std::vector<std::pair<std::string, std::string>> actions;
    actions.reserve(static_cast<std::size_t>(a));
    for (int i = 0; i < a; ++i) {
        std::string nom;
        std::string usage;
        std::cin >> nom >> usage;
        actions.emplace_back(nom, usage);
    }

    int l = 0;
    std::cin >> l;
    std::unordered_set<std::string> fournis;
    for (int i = 0; i < l; ++i) {
        std::string usage;
        std::cin >> usage;
        fournis.insert(usage);
    }

    int mortes = 0;
    for (const auto &action : actions) {
        const bool active = fournis.count(action.second) != 0;
        std::cout << action.first << ' ' << (active ? "ACTIVE" : "MORTE") << '\n';
        if (!active) {
            ++mortes;
        }
    }
    std::cout << "MORTES " << mortes << '\n';
    return 0;
}
