#pragma once

#include <link.h>
#include <sys/types.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "breakpoint.hh"
#include "dwarf.hh"
#include "elf.hh"
#include "mapping.hh"
#include "process.hh"
#include "regs.hh"
#include "utils.hh"

class Debugger {
public:
    Debugger(char *binary_name) : bin_path_(binary_name), bin_name_(binary_name) {};
    ~Debugger() = default;

    void get_memory_mapping();
    void dump_mapping(std::ostream& o) const;

    void start_inferior(void);
    void wait_inferior(void);
    void continue_inferior(void);

    void routine(void);

    void add_breakpoint(std::uintptr_t address);
    void add_breakpoint(std::string symbol_name);


    reg_t get_register_value(reg r) const;
    void set_register_value(reg r, std::uintptr_t value);
    void dump_registers(std::ostream& o);

    Mapping get_mapping(const std::string& bin) const;

    void next_instruction(void) const;
    line_number_t get_current_line(void) const;
private:
    std::filesystem::path bin_path_;
    char *bin_name_;

    std::unique_ptr<DwarfParser> elf_;
    Process inf_;

    std::vector<Mapping> mappings_;
    std::vector<Breakpoint> breakpoints_;
};
