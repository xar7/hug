#include <algorithm>
#include <fstream>
#include <iostream>
#include <link.h>
#include <sstream>
#include <sys/personality.h>
#include <sys/wait.h>
#include <unistd.h>

#include "debugger.hh"
#include "log.hh"
#include "mapping.hh"
#include "process.hh"
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
        inf_ = Process(pid);
    }
}

void Debugger::get_memory_mapping() {
    /* Reading /proc/PID/task/PID/maps is faster than /proc/PID/maps
       check comment on gdb/linux-tdep.c:linux_vsyscall_range_raw */
    std::stringstream procfilename;
    procfilename << "/proc/" << inf_.get_pid() << "/task/" << inf_.get_pid() << "/maps";

    std::ifstream procfile(procfilename.str());
    if (!procfile.is_open()) {
        std::cerr << "Unable to open " << procfilename.str() << std::endl;
        return;
    }

    std::string line;
    while (std::getline(procfile, line)) {
        Mapping m;
        m.pid_ = inf_.get_pid();

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
    o << "Memory mapping for process " <<  inf_.get_pid() << '\n';
    for (auto const& x : mappings_) {
        o << x << '\n';
    }
}

uint64_t Debugger::get_register_value(reg r) const {
    return inf_.get_register_value(r);
}

void Debugger::set_register_value(reg r, std::uintptr_t value) {
    inf_.set_register_value(r, value);
}

void Debugger::wait_inferior(void) {
    inf_.wait(0);

    if (inf_.get_status() == STOPPED) {
        LOG("inferior got stopped by signal `%s`.", utils::signalstr[WSTOPSIG(inf_.get_wstatus()) - 1].c_str());
        LOG("rip is at %p", (void *) get_register_value(reg::rip));
        if (WSTOPSIG(inf_.get_wstatus()) == SIGTRAP) {
            auto bp = std::find(breakpoints_.begin(), breakpoints_.end(),
                                get_register_value(reg::rip) - 1);
            if (bp != breakpoints_.end()) {
                bp->unset();
                LOG("inferior hit a breakpoint");
                set_register_value(reg::rip, bp->get_addr());
                inf_.single_step();
                inf_.wait(0);
                bp->set();
            }
        }
    }
    else if (inf_.get_status() == EXITED) {
        LOG("inferior exited");
    }
}

void Debugger::continue_inferior(void) {
    return inf_.continue_exec();
}

void Debugger::add_breakpoint(std::uintptr_t address) {
    auto data = inf_.peek_data(address);
    auto bp = Breakpoint(inf_, address, static_cast<uint8_t>(data & 0xff));
    bp.set();
    breakpoints_.push_back(std::move(bp));
}

void Debugger::add_breakpoint(std::string symbol_name) {
    auto sym = elf_.get_symbol(symbol_name);
    if (!sym) {
        ERR("Unable to locate symbol %s in %s.", symbol_name.c_str(), bin_path_.c_str());
        return;
    }
    else if (ELF64_ST_TYPE(sym->st_info) != STT_FUNC) {
        ERR("%s is not a function name.", symbol_name.c_str());
        return;
    }

    std::uintptr_t address = sym->st_value;
    if (elf_.is_pie()) {
        auto mapping = std::find(mappings_.begin(), mappings_.end(),
                                 std::filesystem::absolute(bin_path_).string());
        if (mapping == mappings_.end()) {
            ERR("Unable to find memory mapping for %s",
                std::filesystem::absolute(bin_path_).string().c_str());
            return;

            /* XXX we should probably report the error the caller instead of
               just returning */
        }
        address += mapping->begin_;
    }

    return add_breakpoint(address);
}
