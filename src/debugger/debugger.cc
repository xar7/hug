#include <algorithm>
#include <fstream>
#include <iostream>
#include <link.h>
#include <sstream>
#include <sys/personality.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "debugger.hh"
#include "log.hh"
#include "mapping.hh"
#include "utils.hh"
#include "regs.hh"

#define GET_CURRENT_PERSONALITY (0xffffffff)

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

        /* Disable ASLR in the child */
        int current_personality = personality(GET_CURRENT_PERSONALITY);
        personality(current_personality | ADDR_NO_RANDOMIZE);

        char *const arg[2] = { binary_path_, nullptr };
        if (execvp(arg[0], arg))
            std::cerr << "execvp failed!" << std::endl;
    }
    else {
        /* Parent */
        inferior_pid_ = pid;
    }
}

void Debugger::get_memory_mapping() {
    /* Reading /proc/PID/task/PID/maps is faster than /proc/PID/maps
       check comment on gdb/linux-tdep.c:linux_vsyscall_range_raw */
    std::stringstream procfilename;
    procfilename << "/proc/" << inferior_pid_ << "/task/" << inferior_pid_ << "/maps";

    std::ifstream procfile(procfilename.str());
    if (!procfile.is_open()) {
        std::cerr << "Unable to open " << procfilename.str() << std::endl;
        return;
    }

    std::string line;
    while (std::getline(procfile, line)) {
        Mapping m;
        m.pid_ = inferior_pid_;

        std::istringstream iss(line);
        std::string token;

        iss >> token;
        /* token contains BEGIN_ADDR-END_ADDR we have to split until - */
        auto split_index = token.find('-', 0);
        m.begin_ = std::stoull(token.substr(0, split_index), 0, 16);
        m.end_ = std::stoull(token.substr(split_index + 1, token.size()), 0, 16);

        iss >> token;
        /* token contains permission string */
        m.perm = 0;
        for (size_t i = 0; i < 4; i++) {
            if (token[i] != '-') {
                m.perm |= (1 << i);
            }
        }

        iss >> m.offset_;
        iss >> token;
        /* XXX Use device */
        iss >> token;
        /* XXX Use inode */
        iss >> m.name_;
    }

    return;
}

uint64_t Debugger::get_register_value(reg r) const {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, inferior_pid_, nullptr, &regs);
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
            auto bp = std::find(breakpoints_.begin(), breakpoints_.end(),
                                get_register_value(reg::rip) - 1);
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
