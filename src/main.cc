#include <iostream>

#include "debugger.h"

int main(int argc, char **argv) {
    std::cout << "hug" << std::endl;
    std::cout << "Launch with cmdline :";
    for (auto i = 0 ; i < argc; i++)
    {
        std::cout << ' ' << argv[i];
    }
    std::cout << std::endl;

    Debugger d(argv[1]);
    d.start_inferior();
    d.wait_inferior();
    d.continue_inferior();
    d.wait_inferior();

    return 0;
}
