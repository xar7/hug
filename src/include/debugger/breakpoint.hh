#pragma once

#include <cstdint>
#include <sys/types.h>

class Breakpoint {
public:
    Breakpoint(std::intptr_t address, uint8_t data) : addr_(address), data_(data) {};
    ~Breakpoint() = default;

private:
    std::intptr_t addr_;
    uint8_t data_;
};
