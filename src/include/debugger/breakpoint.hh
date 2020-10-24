#pragma once

#include <cstdint>
#include <memory>

#include <sys/types.h>

class Breakpoint {
public:
    Breakpoint(pid_t pid, std::intptr_t address, uint8_t data) : pid_(pid), addr_(address), data_(data) {};
    ~Breakpoint() = default;

    void set(void);
    void unset(void) const;

    bool operator==(std::intptr_t addr);

    std::intptr_t get_addr(void) const;

private:
    pid_t pid_;
    std::intptr_t addr_;
    uint8_t data_;
};
