#include <sys/ptrace.h>

#include "breakpoint.hh"

void Breakpoint::set(void) {
    auto data = ptrace(PTRACE_PEEKDATA, pid_, addr_, nullptr);
    data_ = static_cast<uint8_t>(data & 0xff);

    ptrace(PTRACE_POKEDATA, pid_, addr_, ((data & ~0xff) | 0xcc));
}

void Breakpoint::unset(void) const {
    auto data = ptrace(PTRACE_PEEKDATA, pid_, addr_, nullptr);

    ptrace(PTRACE_POKEDATA, pid_, addr_, ((data & ~0xff) | data_));
}

bool Breakpoint::operator==(const std::uintptr_t& addr) const {
    return addr_ == addr;
}

std::intptr_t Breakpoint::get_addr(void) const {
    return addr_;
}
