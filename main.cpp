//
// Created by Ludw on 4/25/2024.
//
#include "app.h"
#include <iomanip>

int main() {
    App app;

    std::cout << std::setw(30);

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}