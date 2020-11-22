#include <cassert>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include "log.hh"
#include "process.hh"
#include "utils.hh"

void Process::wait(int flag) {
    waitpid(pid_, &wstatus_, flag);
    update_status();
}

void Process::update_status(void) {
    if (WIFSTOPPED(wstatus_)) {
        status_ = STOPPED;
    }
    else if (WIFEXITED(wstatus_)) {
        status_ = EXITED;
    }
}

pstate Process::get_status(void) const {
    return status_;
}

int Process::get_wstatus(void) const {
    return wstatus_;
}

long Process::call_ptrace(enum __ptrace_request r, void *addr, void *data) const {
    /* every ptrace functions exposed by this class need our process to be stopped */
    assert(status_ == STOPPED);
    long rc = ptrace(r, pid_, addr, data);
    if (rc == -1) {
        ERR("ptrace error: ptrace(%s, %d, %p, %p)", utils::ptrace_request_str[r].c_str()
            , pid_, addr, data);
        /* XXX Do something clever here */
    }

    return rc;
}

void Process::set_register_value(reg r, std::uintptr_t value) const {
    user_regs_struct regs;
    call_ptrace(PTRACE_GETREGS, 0, &regs);
    REG_FROM_REGTABLE(regs, r) = value;
    call_ptrace(PTRACE_SETREGS, 0, &regs);
}

uint64_t Process::get_register_value(reg r) const {
    user_regs_struct regs;
    call_ptrace(PTRACE_GETREGS, 0, &regs);
    return REG_FROM_REGTABLE(regs, r);
}

void Process::continue_exec(void) {
    call_ptrace(PTRACE_CONT, nullptr, nullptr);
    status_ = RUNNING;
}

long Process::peek_data(std::uintptr_t addr) const {
    return call_ptrace(PTRACE_PEEKDATA, reinterpret_cast<void *>(addr), nullptr);
}

void Process::poke_data(std::uintptr_t addr, std::uintptr_t data) const {
    call_ptrace(PTRACE_POKEDATA, reinterpret_cast<void *>(addr),
                reinterpret_cast<void *>(data));
}

void Process::single_step(void) {
    call_ptrace(PTRACE_SINGLESTEP, nullptr, nullptr);
    status_ = RUNNING;
}

pid_t Process::get_pid(void) const {
    return pid_;
}
