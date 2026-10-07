#include <iostream>
#include <string>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int N;
    if (!(std::cin >> N)) {
        return 0;
    }

    bool saisi = false;
    int sans_garde = 0;

    for (int i = 0; i < N; ++i) {
        std::string evt;
        std::cin >> evt;

        if (evt == "enfonce") {
            if (!saisi) {
                std::cout << "SAISIR\n";
                saisi = true;
            } else {
                std::cout << "RIEN\n";
            }
            sans_garde++;
        } else if (evt == "repete") {
            std::cout << "RIEN\n";
            sans_garde++;
        } else if (evt == "relache") {
            if (saisi) {
                std::cout << "LACHER\n";
                saisi = false;
            } else {
                std::cout << "RIEN\n";
            }
        }
    }

    std::cout << "SANS_GARDE " << sans_garde << "\n";
    return 0;
}