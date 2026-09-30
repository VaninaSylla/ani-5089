#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string targetArch;
    std::getline(std::cin, targetArch);
    if (targetArch.empty()) return 0;

    int F;
    std::string line;
    std::getline(std::cin, line);
    F = std::stoi(line);

    long long totalSize = 0;
    bool signed_ = false;
    bool abiOk = false;
    int inutile = 0;

    std::string libPrefix = "lib/" + targetArch + "/";

    for (int i = 0; i < F; ++i) {
        std::getline(std::cin, line);
        if (line.empty()) { --i; continue; }
        std::istringstream iss(line);
        std::string path;
        long long size;
        iss >> path >> size;
        totalSize += size;

        if (path.rfind("META-INF/", 0) == 0) {
            if (path.size() >= 4) {
                std::string ext = path.substr(path.size() - 4);
                if (ext == ".RSA" || ext == ".DSA" || (path.size() >= 3 && path.substr(path.size() - 3) == ".EC")) {
                    signed_ = true;
                }
            }
        }

        if (path.rfind(libPrefix, 0) == 0) {
            abiOk = true;
        } else if (path.rfind("lib/", 0) == 0) {
            inutile++;
        }
    }

    std::cout << totalSize << "\n";
    std::cout << (signed_ ? "SIGNE" : "NON SIGNE") << "\n";
    std::cout << (abiOk ? "ABI OUI" : "ABI NON") << "\n";
    std::cout << "INUTILE " << inutile << "\n";

    return 0;
}