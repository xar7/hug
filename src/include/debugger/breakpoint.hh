#pragma once

#include <cstdint>
#include <memory>

#include <sys/types.h>

class Breakpoint {
public:
    Breakpoint(pid_t pid, std::uintptr_t address, uint8_t data) : pid_(pid), addr_(address), data_(data) {};
    ~Breakpoint() = default;

    void set(void);
    void unset(void) const;

    bool operator==(const std::uintptr_t& addr) const;

    std::uintptr_t get_addr(void) const;

private:
    pid_t pid_;
    std::uintptr_t addr_;
    uint8_t data_;
};
