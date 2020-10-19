#include <sys/ptrace.h>

#include "breakpoint.hh"

void Breakpoint::set(void) {
    auto data = ptrace(PTRACE_PEEKDATA, pid_, addr_, NULL);
    data_ = static_cast<uint8_t>(data & 0xff);

    ptrace(PTRACE_POKEDATA, pid_, addr_, ((data & ~0xff) | 0xcc));
};

void Breakpoint::unset(void) {
    auto data = ptrace(PTRACE_PEEKDATA, pid_, addr_, NULL);

    ptrace(PTRACE_POKEDATA, pid_, addr_, ((data & ~0xff) | data_));
}
