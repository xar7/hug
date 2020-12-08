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
        return PARSER_INIT_FAIL;
    }


    return PARSER_INIT_OK;
}

void DwarfParser::destroy(void) {
    // XXX Let's hope it doesn't fail since we don't have a return value.
    if (dwarf_finish(dbg_, &error_) != DW_DLV_OK) {
        ERR("DwarfParserError on dwarf_finish call.");
    }

    if (close(fd_) < 0) {
        perror("DwarfParserError on close: ");
    }
}
