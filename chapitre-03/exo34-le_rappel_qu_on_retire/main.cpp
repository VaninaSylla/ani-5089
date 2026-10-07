#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

struct Callback {
    int id;
    std::string type;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int N;
    if (!(std::cin >> N)) return 0;

    std::vector<Callback> registry;

    for (int i = 0; i < N; ++i) {
        std::string cmd;
        std::cin >> cmd;

        if (cmd == "poser") {
            int id;
            std::string type;
            std::cin >> id >> type;

            // Remove if already exists
            for (auto it = registry.begin(); it != registry.end(); ++it) {
                if (it->id == id) {
                    registry.erase(it);
                    break;
                }
            }
            // Add at end
            registry.push_back({id, type});
        } else if (cmd == "retirer") {
            int id;
            std::cin >> id;
            for (auto it = registry.begin(); it != registry.end(); ++it) {
                if (it->id == id) {
                    registry.erase(it);
                    break;
                }
            }
        } else if (cmd == "envoyer") {
            std::string type;
            std::cin >> type;
            bool first = true;
            for (const auto& cb : registry) {
                if (cb.type == type) {
                    if (!first) std::cout << " ";
                    std::cout << cb.id;
                    first = false;
                }
            }
            if (first) {
                std::cout << "AUCUN";
            }
            std::cout << "\n";
        }
    }

    return 0;
}