#pragma once

#include <functional>
#include <libdwarf/libdwarf.h>
#include <libdwarf/dwarf.h>
#include <string>
#include <vector>

#include "elf.hh"

typedef std::function<int(Dwarf_Die)> dwarf_die_handler_ptr;

class DwarfParser : public ElfParser {
public:
    DwarfParser(std::string path) : ElfParser(path) {};

    int init(void) override;
    void destroy(void) override;

    int dies_traversal(dwarf_die_handler_ptr funcptr);

    Dwarf_Error error_;
private:
    void dies_traversal_rec(Dwarf_Debug dbg, Dwarf_Die die,
                            dwarf_die_handler_ptr funcptr);

    Dwarf_Debug dbg_;
    Dwarf_Handler errhand_;
    Dwarf_Ptr errarg_;

    int fd_;

    std::vector<Dwarf_Die> dies_;
};
