#include <iostream>
#include <vector>
#include <string>

struct Command {
    std::string name;
    long long scale;
    long long threshold;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int C;
    if (!(std::cin >> C)) return 0;

    std::vector<Command> commands(C);
    for (int i = 0; i < C; ++i) {
        std::cin >> commands[i].name >> commands[i].scale >> commands[i].threshold;
    }

    int T;
    std::cin >> T;

    for (int t = 0; t < T; ++t) {
        long long result = 0;
        for (int i = 0; i < C; ++i) {
            long long raw;
            std::cin >> raw;
            long long contrib = (raw * commands[i].scale) / 1000;
            if (contrib < 0) contrib = -contrib;
            if (contrib >= commands[i].threshold) {
                long long signed_contrib = (raw * commands[i].scale) / 1000;
                result += signed_contrib;
            }
        }
        std::cout << result << "\n";
    }

    return 0;
}