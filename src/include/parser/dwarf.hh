#pragma once

#include <libdwarf/libdwarf.h>
#include <libdwarf/dwarf.h>
#include <string>

#include "elf.hh"

class DwarfParser : public ElfParser {
public:
    DwarfParser(std::string path) : ElfParser(path) {};

    int init(void) override;
    void destroy(void) override;
private:
};
