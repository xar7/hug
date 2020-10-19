#include <unistd.h>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include "debugger.hh"
#include "log.hh"
#include "regs.hh"

void Debugger::start_inferior(void) {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        std::cerr << "Unable to fork !" << std::endl;
    }
    else if (pid == 0) {
        /* Child */
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
            std::cerr << "ptrace TRACEME request failed!" << std::endl;

        char *const arg[2] = { binary_path_, NULL };
        if (execvp(arg[0], arg))
            std::cerr << "execvp failed!" << std::endl;
    }
    else {
        /* Parent */
        inferior_pid_ = pid;
    }
}

uint64_t Debugger::get_register_value(reg r) {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, inferior_pid_, NULL, &regs);
    return (reinterpret_cast<uint64_t *>(&regs))[r];
}

void Debugger::wait_inferior(void) {
    waitpid(inferior_pid_, &wstatus_, 0);

    if (WIFSTOPPED(wstatus_) && WSTOPSIG(wstatus_) == SIGTRAP) {
        LOG("inferior got trapped!");
        LOG("rip is at %p", (void *) get_register_value(reg::rip));
    }
    else if (WIFEXITED(wstatus_)) {
        LOG("inferior exited");
    }
}

void Debugger::continue_inferior(void) {
    ptrace(PTRACE_CONT, inferior_pid_, NULL, NULL);
}

void Debugger::add_breakpoint(std::intptr_t address) {
    auto data = ptrace(PTRACE_PEEKDATA, inferior_pid_, address, NULL);
    breakpoints_.push_back(Breakpoint(address, static_cast<uint8_t>(data & 0xff)));

    ptrace(PTRACE_POKEDATA, inferior_pid_, address, ((data & ~0xff) | 0xcc));
}
