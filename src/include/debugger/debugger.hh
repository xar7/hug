#pragma once

#include <sys/types.h>
#include <vector>

#include "breakpoint.hh"
#include "regs.hh"

class Debugger {
public:
    Debugger(char *binary_path) : binary_path_(binary_path) {};
    ~Debugger() = default;

    void start_inferior(void);
    void wait_inferior(void);
    void continue_inferior(void);
    void add_breakpoint(std::intptr_t address);
    uint64_t get_register_value(reg r);

private:
    int wstatus_ = 0;
    pid_t inferior_pid_;
    char *binary_path_;

    std::vector<Breakpoint> breakpoints_;
};
