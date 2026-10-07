#include <iostream>
#include <string>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int P, F;
    if (!(std::cin >> P >> F)) return 0;

    int counter = 0;
    int premier = 0;

    for (int i = 1; i <= F; ++i) {
        std::string action;
        std::cin >> action;

        if (action == "releve") {
            counter = 0;
            std::cout << counter << " VIVANTE\n";
        } else if (action == "travaille") {
            counter++;
            if (counter >= P) {
                std::cout << counter << " MORTE\n";
                if (premier == 0) premier = i;
            } else {
                std::cout << counter << " VIVANTE\n";
            }
        }
    }

    std::cout << "PREMIER " << premier << "\n";
    return 0;
}