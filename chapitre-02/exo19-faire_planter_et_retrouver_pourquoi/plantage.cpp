#include <iostream>

int main() {
    // Plantage volontaire : déréférencement de pointeur nul
    int* ptr = nullptr;
    *ptr = 42;  // Crash ici
    return 0;
}