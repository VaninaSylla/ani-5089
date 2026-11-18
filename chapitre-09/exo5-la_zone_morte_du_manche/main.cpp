#include <iostream>

int main() {
    long long seuil = 0;
    long long maximum = 0;
    std::cin >> seuil >> maximum;

    int n = 0;
    std::cin >> n;

    const long long course = 1000 - seuil;
    const long long denom = course * course;
    long long morts = 0;

    for (int tour = 0; tour < n; ++tour) {
        long long brut = 0;
        std::cin >> brut;

        const long long absolu = brut < 0 ? -brut : brut;
        if (absolu < seuil) {
            ++morts;
            std::cout << 0 << '\n';
            continue;
        }

        const long long reste = absolu - seuil;
        long long vitesse = 0;
        if (denom != 0 && reste > 0) {
            vitesse = maximum * reste * reste / denom;
        }
        if (brut < 0) {
            vitesse = -vitesse;
        }
        std::cout << vitesse << '\n';
    }

    std::cout << "MORTS " << morts << '\n';
    return 0;
}
