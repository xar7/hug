#pragma once

#include <libdwarf/libdwarf.h>
#include <libdwarf/dwarf.h>
#include <string>
#include <vector>

#include "elf.hh"

class DwarfParser : public ElfParser {
public:
    DwarfParser(std::string path) : ElfParser(path) {};

    int init(void) override;
    void destroy(void) override;

    void dies_traversal(Dwarf_Debug dbg, Dwarf_Die die);
    int load_all_dies(void);
private:
    Dwarf_Debug dbg_;
    Dwarf_Error error_;
    Dwarf_Handler errhand_;
    Dwarf_Ptr errarg_;

    int fd_;

    std::vector<Dwarf_Die> dies_;
};
