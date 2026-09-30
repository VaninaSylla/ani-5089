#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    long long budget;
    if (!(std::cin >> budget)) return 0;
    std::string line;
    std::getline(std::cin, line);

    int S;
    std::cin >> S;
    std::getline(std::cin, line);

    int trompe = 0;
    for (int i = 0; i < S; ++i) {
        std::getline(std::cin, line);
        if (line.empty()) { --i; continue; }
        std::istringstream iss(line);
        std::string name;
        long long debug, release;
        iss >> name >> debug >> release;

        long long factor = (debug + release / 2) / release;
        bool tient = (release <= budget);
        bool depasse = !tient;
        bool isTrompe = (debug > budget && tient);

        std::cout << name << " " << factor << " " << (tient ? "TIENT" : "DEPASSE") << "\n";

        if (isTrompe) trompe++;
    }

    std::cout << "TROMPE " << trompe << "\n";

    return 0;
}