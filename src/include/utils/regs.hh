#pragma once

#include <cstdint>
#include <map>
#include <string>

typedef std::uintptr_t reg_t;

#define REG_FROM_REGTABLE(regtable, reg)        \
    reinterpret_cast<reg_t *>(&regtable)[reg]

enum reg {
    r15 = 0,
    r14,
    r13,
    r12,
    rbp,
    rbx,
    r11,
    r10,
    r9,
    r8,
    rax,
    rcx,
    rdx,
    rsi,
    rdi,
    orig_rax,
    rip,
    cs,
    eflags,
    rsp,
    ss,
    fs_base,
    gs_base,
    fd,
    es,
    fs,
    gs,
    reg_number
};

namespace utils {
    extern const std::map<reg, std::string> regs_str;
}
