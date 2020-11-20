#pragma once

#include <cstdint>
#include <memory>

#include <sys/types.h>

#include "process.hh"

class Breakpoint {
public:
    Breakpoint(const Process &p, std::uintptr_t address, uint8_t data) : p_(p), addr_(address), data_(data) {};
    ~Breakpoint() = default;

    void set(void);
    void unset(void) const;

    bool operator==(const std::uintptr_t& addr) const;

    std::uintptr_t get_addr(void) const;

private:
    Process p_;
    std::uintptr_t addr_;
    uint8_t data_;
};
