#include <iostream>
#include <elf.h>
#include <link.h>

#include "mapping.hh"
#include "debugger.hh"
#include "elfparser.hh"

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
    d.get_memory_mapping();
    d.dump_mapping();
    d.add_breakpoint("main");
    d.continue_inferior();
    d.wait_inferior();
    d.continue_inferior();
    d.wait_inferior();

    ElfParser p(argv[1]);
    p.init();
    auto sym_main = p.get_symbol("main");
    std::cout << "main=" << std::hex << "0x" <<sym_main->st_value << std::endl;
    p.destroy();

    return 0;
}
