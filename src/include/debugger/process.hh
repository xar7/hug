#pragma once

#include <cstdint>

#include <sys/ptrace.h>
#include <sys/types.h>

#include "regs.hh"

enum pstate {
    INIT,
    RUNNING,
    STOPPED,
    EXITED,
};

class Process {
public:
    Process() = default;
    Process(int pid) : status_(INIT), pid_(pid) {}

    void wait(int flag);

    long call_ptrace(enum __ptrace_request r, void *addr, void *data) const;
    void set_register_value(reg r, std::uintptr_t value) const;
    uint64_t get_register_value(reg r) const;
    void continue_exec(void);
    long peek_data(std::uintptr_t addr) const;
    void poke_data(std::uintptr_t addr, std::uintptr_t data) const;
    void single_step(void);

    pid_t get_pid(void) const;
    pstate get_status(void) const;
    int get_wstatus(void) const;

private:
    void update_status(void);

    pstate status_;
    pid_t pid_;
    int wstatus_;
};
