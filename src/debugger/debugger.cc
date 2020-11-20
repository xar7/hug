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
#include "regs.hh"

#define GET_CURRENT_PERSONALITY (0xffffffff)

void Debugger::start_inferior(void) {
    LOG("Initializing elf parser for : %s.", bin_name_);
    elf_.init();
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

        char *const arg[2] = { bin_name_, nullptr };
        if (execv(arg[0], arg))
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
        m.begin_ = std::stoul(token.substr(0, split_index), 0, 16);
        m.end_ = std::stoul(token.substr(split_index + 1, token.size()), 0, 16);


        iss >> m.perm_;
        iss >> m.offset_;
        iss >> token;
        /* XXX Use device */
        iss >> token;
        /* XXX Use inode */
        iss >> m.name_;

        mappings_.push_back(m);
    }

    return;
}

void Debugger::dump_mapping(std::ostream& o) const {
    o << "Memory mapping for process " <<  inferior_pid_ << '\n';
    for (auto const& x : mappings_) {
        std::cout << x << '\n';
    }
}

uint64_t Debugger::get_register_value(reg r) const {
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS, inferior_pid_, nullptr, &regs);
    return REG_FROM_REGTABLE(regs, r);
}

void Debugger::set_register_value(reg r, std::uintptr_t value) const {
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
                LOG("After setting rip to bp addr rip is now at %p.", (void *) get_register_value(reg::rip));
                if (ptrace(PTRACE_SINGLESTEP, inferior_pid_, 0, 0) < 0)
                    std::cerr << "ptrace failure !" << std::endl;
                waitpid(inferior_pid_, &wstatus_, 0);
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

void Debugger::add_breakpoint(std::uintptr_t address) {
    auto data = ptrace(PTRACE_PEEKDATA, inferior_pid_, address, NULL);
    auto bp = Breakpoint(inferior_pid_, address, static_cast<uint8_t>(data & 0xff));
    bp.set();
    breakpoints_.push_back(std::move(bp));
}

void Debugger::add_breakpoint(std::string symbol_name) {
    auto sym = elf_.get_symbol(symbol_name);
    if (!sym) {
        std::cerr << "Unable to locate symbol " << symbol_name << "in" << bin_path_ << std::endl;
        return;
    }

    std::uintptr_t address = sym->st_value;
    if (elf_.is_pie()) {
        auto mapping = std::find(mappings_.begin(), mappings_.end(),
                                 std::filesystem::absolute(bin_path_).string());
        if (mapping == mappings_.end()) {
            ERR("Unable to find memory mapping for %s", std::filesystem::absolute(bin_path_).string().c_str());
        }
        address += mapping->begin_;
    }

    return add_breakpoint(address);
}
