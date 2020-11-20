#include <sys/ptrace.h>

#include "breakpoint.hh"

void Breakpoint::set(void) {
    auto data = p_.peek_data(addr_);
    data_ = static_cast<uint8_t>(data & 0xff);
    p_.poke_data(addr_, ((data & ~0xff) | 0xcc));
}

void Breakpoint::unset(void) const {
    auto data = p_.peek_data(addr_);
    p_.poke_data(addr_, ((data & ~0xff) | data_));
}

bool Breakpoint::operator==(const std::uintptr_t& addr) const {
    return addr_ == addr;
}

std::uintptr_t Breakpoint::get_addr(void) const {
    return addr_;
}
