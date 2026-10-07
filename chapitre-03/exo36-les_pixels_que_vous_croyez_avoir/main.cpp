#include <iostream>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int N;
    if (!(std::cin >> N)) {
        return 0;
    }

    int lisible = 0;

    for (int i = 0; i < N; ++i) {
        long long width, height, scale, fov;
        std::cin >> width >> height >> scale >> fov;

        long long real_width = (width * scale) / 100;
        long long real_height = (height * scale) / 100;
        long long px_per_deg = (real_width + fov / 2) / fov;

        std::cout << real_width << " " << real_height << " " << px_per_deg << "\n";

        if (px_per_deg >= 15) {
            lisible++;
        }
    }

    std::cout << "LISIBLE " << lisible << "\n";
    return 0;
}