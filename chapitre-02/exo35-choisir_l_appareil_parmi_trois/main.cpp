#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <sstream>

struct Device {
    std::string serial;
    std::string state;
    std::string model;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int D;
    if (!(std::cin >> D)) {
        return 0;
    }
    std::string line;
    std::getline(std::cin, line);

    std::vector<Device> devices;
    for (int i = 0; i < D; ++i) {
        std::getline(std::cin, line);
        if (line.empty()) {
            --i;
            continue;
        }
        std::istringstream iss(line);
        Device d;
        iss >> d.serial >> d.state >> d.model;
        devices.push_back(d);
    }

    std::getline(std::cin, line);
    std::string target = line;

    if (target != "-") {
        bool found = false;
        for (const auto& d : devices) {
            if (d.serial == target) {
                found = true;
                if (d.state == "device") {
                    std::cout << d.serial << "\n";
                } else {
                    std::cout << "ERREUR " << d.serial << " est " << d.state << "\n";
                }
                break;
            }
        }
        if (!found) {
            std::cout << "ERREUR cible introuvable\n";
        }
    } else {
        std::vector<std::string> available;
        for (const auto& d : devices) {
            if (d.state == "device") {
                available.push_back(d.serial);
            }
        }
        if (available.empty()) {
            std::cout << "ERREUR aucun appareil\n";
        } else if (available.size() == 1) {
            std::cout << available[0] << "\n";
        } else {
            std::sort(available.begin(), available.end());
            std::cout << "ERREUR plusieurs appareils\n";
            for (const auto& s : available) {
                std::cout << s << "\n";
            }
        }
    }

    return 0;
}