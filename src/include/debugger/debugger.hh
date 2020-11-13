#pragma once

#include <link.h>
#include <sys/types.h>

#include <filesystem>
#include <string>
#include <vector>

#include "breakpoint.hh"
#include "elfparser.hh"
#include "mapping.hh"
#include "utils.hh"
#include "regs.hh"

class Debugger {
public:
    Debugger(char *binary_name) : bin_path_(binary_name), bin_name_(binary_name), elf_(binary_name) {};
    ~Debugger() = default;

    void get_memory_mapping();

    void start_inferior(void);
    void wait_inferior(void);
    void continue_inferior(void);

    void add_breakpoint(std::uintptr_t address);
    void add_breakpoint(std::string symbol_name);


    uint64_t get_register_value(reg r) const;
    void set_register_value(reg r, std::uintptr_t value) const;

private:
    int wstatus_ = 0;
    pid_t inferior_pid_;
    std::filesystem::path bin_path_;
    char *bin_name_;

    ElfParser elf_;

    std::vector<Mapping> mappings_;
    std::vector<Breakpoint> breakpoints_;
};
