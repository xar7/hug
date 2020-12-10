#include <iostream>
#include <elf.h>
#include <link.h>
#include <libdwarf/libdwarf.h>

#include "debugger.hh"
#include "dwarf.hh"
#include "elf.hh"
#include "mapping.hh"
#include "utils.hh"

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
    d.dump_mapping(std::cout);
    d.add_breakpoint("add");
    d.dump_registers(std::cout);
    d.continue_inferior();
    d.wait_inferior();
    std::cout << d.get_current_line() << std::endl;
    d.continue_inferior();
    d.wait_inferior();

    ElfParser p(argv[1]);
    if (p.init() == PARSER_INIT_FAIL) {
        std::cerr << "ElfParser initialization failed." << std::endl;
    }
    auto sym_main = p.get_symbol("main");
    std::cout << "main=" << std::hex << "0x" <<sym_main->st_value << std::endl;
    p.destroy();

    DwarfParser dp(argv[1]);
    if (dp.init() == PARSER_INIT_FAIL) {
        std::cerr << "DwarfParser initialization failed." << std::endl;
    }

    dp.dump_line_table(std::cout);
    dp.destroy();

    return 0;
}
