#include <iostream>
#include "../headers/Debug.h"

int Debug::info() {
    std::cout << std::string("DEBUG::INFO:") + std::string("message") + std::string(" \n") << std::endl;
    return 0;
}