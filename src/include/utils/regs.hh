#pragma once

#define REG_FROM_REGTABLE(regtable, reg)        \
    reinterpret_cast<uint64_t *>(&regtable)[reg]

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
