#include <algorithm>
#include <link.h>
#include <unistd.h>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include "debugger.hh"
#include "log.hh"
#include "utils.hh"
#include "regs.hh"

#define REG_FROM_REGTABLE(regtable, reg)        \
    reinterpret_cast<uint64_t *>(&regtable)[reg]

void Debugger::start_inferior(void) {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        std::cerr << "Unable to fork !" << std::endl;
    }
    else if (pid == 0) {
        /* Child */
        if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0)
            std::cerr << "ptrace TRACEME request failed!" << std::endl;

        char *const arg[2] = { binary_path_, nullptr };
        if (execvp(arg[0], arg))
            std::cerr << "execvp failed!" << std::endl;
    }
    else {
        /* Parent */
        inferior_pid_ = pid;
    }
}

uint64_t Debugger::get_register_value(reg r) const {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, inferior_pid_, NULL, &regs);
    return REG_FROM_REGTABLE(regs, r);
}

// XXX yeyeye check error code
void Debugger::set_register_value(reg r, std::intptr_t value) const {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, inferior_pid_, nullptr, &regs);
    REG_FROM_REGTABLE(regs, r) = value;
    ptrace(PTRACE_SETREGS, inferior_pid_, nullptr, &regs);
}

void Debugger::wait_inferior(void) {
    waitpid(inferior_pid_, &wstatus_, 0);

    if (WIFSTOPPED(wstatus_)) {
        LOG("inferior got stopped by signal `%s`.", utils::signalstr[WSTOPSIG(wstatus_) - 1].c_str());
        LOG("rip is at %p", (void *) get_register_value(reg::rip));
        if (WSTOPSIG(wstatus_) == SIGTRAP) {
            auto bp = std::find(breakpoints_.begin(), breakpoints_.end(), get_register_value(reg::rip));
            if (bp != breakpoints_.end()) {
                bp->unset();
                LOG("inferior hit a breakpoint");
                set_register_value(reg::rip, bp->get_addr());
                ptrace(PTRACE_SINGLESTEP, inferior_pid_, nullptr, nullptr);
                bp->set();
            }
        }
    }
    else if (WIFEXITED(wstatus_)) {
        LOG("inferior exited");
    }
}

void Debugger::continue_inferior(void) {
    ptrace(PTRACE_CONT, inferior_pid_, nullptr, nullptr);
}

void Debugger::add_breakpoint(std::intptr_t address) {
    auto data = ptrace(PTRACE_PEEKDATA, inferior_pid_, address, NULL);
    auto bp = Breakpoint(inferior_pid_, address, static_cast<uint8_t>(data & 0xff));
    bp.set();
    breakpoints_.push_back(std::move(bp));
}
