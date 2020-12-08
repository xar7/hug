#pragma once

#include <functional>
#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>
#include <map>
#include <string>
#include <vector>

#include "elf.hh"
#include "utils.hh"
#include "regs.hh"

typedef std::function<int(Dwarf_Die)> dwarf_die_handler_ptr;

class DwarfParser : public ElfParser {
public:
    DwarfParser(std::string path) : ElfParser(path) {};

    int init(void) override;
    void destroy(void) override;

    std::vector<Dwarf_Die> get_dies_with_name(std::string name);

    int dies_traversal(dwarf_die_handler_ptr funcptr);
    void *get_variable_current_location(std::string varname, reg_t rip);

    int load_cu_line_table(Dwarf_Die d);
    void dump_line_table(std::ostream& o);

    Dwarf_Error error_;
private:
    void dies_traversal_rec(Dwarf_Debug dbg, Dwarf_Die die,
                            dwarf_die_handler_ptr funcptr);

    Dwarf_Debug dbg_;
    Dwarf_Handler errhand_;
    Dwarf_Ptr errarg_;

    int fd_;

    std::vector<Dwarf_Die> dies_;
    std::map<std::uintptr_t, line_number_t> line_table_;
};
