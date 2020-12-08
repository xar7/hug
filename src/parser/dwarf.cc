#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dwarf.hh"
#include "log.hh"

void DwarfParser::dies_traversal_rec(Dwarf_Debug dbg, Dwarf_Die die,
        dwarf_die_handler_ptr funcptr) {
    funcptr(die);

    Dwarf_Die child_die;
    int res = dwarf_child(die, &child_die, &error_);
    if (res == DW_DLV_OK) {
        dies_traversal_rec(dbg, child_die, funcptr);
        Dwarf_Die current_die = child_die;
        Dwarf_Die sib_die = current_die;
        while (res == DW_DLV_OK) {
            current_die = sib_die;
            res = dwarf_siblingof(dbg, current_die, &sib_die, &error_);
            dies_traversal_rec(dbg, sib_die, funcptr);
        }
    }
}

// XXX As I am lazy and having troubles at finding good docs concernings
//     libdwarf and the dwarf format, most of my parser logics is actually
//     contained in this function which browse all of the dwarf dies and
//     apply the handler function passed as arg to them.
//     Yeyeye this is ugly but it works
int DwarfParser::dies_traversal(dwarf_die_handler_ptr funcptr) {
    Dwarf_Unsigned cu_header_length = 0;
    Dwarf_Half  version;
    Dwarf_Half address_size;
    Dwarf_Off abbrev_offset;
    Dwarf_Unsigned next_cu_header;
    Dwarf_Die current_die;

    for (int cu_it = 0;; cu_it++) {
        int res = dwarf_next_cu_header(dbg_, &cu_header_length, &version,
                                       &abbrev_offset, &address_size, &next_cu_header,
                                       &error_);

        if (res == DW_DLV_ERROR) {
            ERR("DwarfParserError: error in dwarf_next_cu_header during loading of DIEs.");
            dwarf_errmsg(error_);
            return 1;
        }
        else if (res == DW_DLV_NO_ENTRY) {
            LOG("DwarfParser: end of CU header list, found %d.", cu_it);
            return 0;
        }

        // CU header have one single sibling the CU DIE.
        res = dwarf_siblingof(dbg_, 0, &current_die, &error_);
        if (res == DW_DLV_ERROR || res == DW_DLV_NO_ENTRY) {
            ERR("DwarfParserError: impossible to find CU DIE. This should be unreachable.");
        }

        dies_traversal_rec(dbg_, current_die, funcptr);
    }
}

int DwarfParser::init(void) {
    // XXX This is a wrong way to do things, but it works.
    //     instead of opening two times the file, I should do
    //     everything at the same time.

    // XXX Also some kind of lazy evaluation should be easy to
    //     implement

    int rc = ElfParser::init();
    if (rc != PARSER_INIT_OK) {
        return rc;
    }

    fd_ = open(path_.c_str(), O_RDONLY);
    if (fd_ < 0) {
        perror("DwarfParserError on open: ");
        return PARSER_INIT_FAIL;
    }

    if (dwarf_init(fd_, DW_DLC_READ, errhand_, errarg_, &dbg_, &error_) != DW_DLV_OK) {
        ERR("DwarfParserError on dwarf_init call.");
        dwarf_errmsg(error_);
        return PARSER_INIT_FAIL;
    }

    // Browse all CU header and DIE to fill our custom line table.
    // XXX This is extremely inefficient as we know where the CU DIEs are
    //     (sibling of CU header) and don't have to browse everything.
    dies_traversal(std::bind(&DwarfParser::load_cu_line_table, this,
                             std::placeholders::_1));


    return PARSER_INIT_OK;
}

int DwarfParser::load_cu_line_table(Dwarf_Die d) {
    Dwarf_Half tag;
    const char *tagname = NULL;
    if (dwarf_tag(d, &tag, &error_) != DW_DLV_OK) {
        ERR("DwarfParserError on dwarf_tag.");
        dwarf_errmsg(error_);
    }
    if (dwarf_get_TAG_name(tag, &tagname) != DW_DLV_OK) {
        ERR("DwarfParserError on dwarf_get_TAG_name.");
        dwarf_errmsg(error_);
    }

    if (strcmp(tagname, "DW_TAG_compile_unit") != 0) {
        return 0;
    }

    Dwarf_Line *lines;
    Dwarf_Signed nlines;

    if (dwarf_srclines(d, &lines, &nlines, &error_) != DW_DLV_OK) {
        ERR("DwarfParserError on dwarf_srclines.");
        dwarf_errmsg(error_);
    }

    for (auto i = 0; i < nlines; i++) {
        Dwarf_Unsigned lineno;
        Dwarf_Addr lineaddr;
        if (dwarf_lineno(lines[i], &lineno, &error_) != DW_DLV_OK) {
            ERR("DwarfParserError on dwarf_lineno.");
            dwarf_errmsg(error_);
        }
        if (dwarf_lineaddr(lines[i], &lineaddr, &error_) != DW_DLV_OK) {
            ERR("DwarfParserError on dwarf_lineaddr.");
            dwarf_errmsg(error_);
        }

        line_table_[lineaddr] = lineno;
    }

    return 0;
}

void DwarfParser::dump_line_table(std::ostream& o) {
    o << "Dumping line table for dwarf file : " << path_ << '\n';
    for (const auto& x : line_table_) {
        o << '(' << std::hex << x.first <<
            ',' << std::dec << x.second << ')' << '\n';
    }
}

line_number_t DwarfParser::get_associated_line(std::uintptr_t addr) {
    
}

void DwarfParser::destroy(void) {
    // XXX Let's hope it doesn't fail since we don't have a return value.
    if (dwarf_finish(dbg_, &error_) != DW_DLV_OK) {
        ERR("DwarfParserError on dwarf_finish call.");
        dwarf_errmsg(error_);
    }

    if (close(fd_) < 0) {
        perror("DwarfParserError on close: ");
    }
}

std::vector<Dwarf_Die> DwarfParser::get_dies_with_name(std::string name) {
    std::vector<Dwarf_Die> dies;

    dies_traversal([&](Dwarf_Die d) -> int {
        char *die_name = NULL;
        if (dwarf_diename(d, &die_name, &error_) == 0) {
            if (strcmp(die_name, name.c_str()) == 0)
                dies.push_back(d);
        }

        dwarf_dealloc(dbg_, die_name, DW_DLA_STRING);
        return 0;
    });

    return dies;
}
