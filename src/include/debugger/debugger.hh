#pragma once

#include <link.h>
#include <sys/types.h>

#include <filesystem>
#include <string>
#include <vector>

#include "breakpoint.hh"
#include "elfparser.hh"
#include "mapping.hh"
#include "process.hh"
#include "utils.hh"
#include "regs.hh"

class Debugger {
public:
    Debugger(char *binary_name) : bin_path_(binary_name), bin_name_(binary_name), elf_(binary_name) {};
    ~Debugger() = default;

    void get_memory_mapping();
    void dump_mapping(std::ostream& o) const;

    void start_inferior(void);
    void wait_inferior(void);
    void continue_inferior(void);

    void routine(void);

    void add_breakpoint(std::uintptr_t address);
    void add_breakpoint(std::string symbol_name);


    uint64_t get_register_value(reg r) const;
    void set_register_value(reg r, std::uintptr_t value);

private:
    std::filesystem::path bin_path_;
    char *bin_name_;

    ElfParser elf_;
    Process inf_;

    std::vector<Mapping> mappings_;
    std::vector<Breakpoint> breakpoints_;
};
