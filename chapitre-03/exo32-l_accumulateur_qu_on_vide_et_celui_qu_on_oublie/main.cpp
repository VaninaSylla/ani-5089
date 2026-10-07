#include <iostream>
#include <string>
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

    long long total_dx = 0, total_dy = 0;
    long long bug_dx = 0, bug_dy = 0;

    for (int i = 0; i < N; ++i) {
        std::getline(std::cin, line);
        if (line.empty()) {
            --i;
            continue;
        }

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "bouge") {
            long long dx, dy;
            iss >> dx >> dy;
            total_dx += dx;
            total_dy += dy;
            bug_dx = dx;
            bug_dy = dy;
        } else if (cmd == "image") {
            std::cout << total_dx << " " << total_dy << " " << bug_dx << " " << bug_dy << "\n";
            total_dx = 0;
            total_dy = 0;
        }
    }

    return 0;
}