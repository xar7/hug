#pragma once

#include <sys/types.h>

class Mapping {
public:
    pid_t pid_;
    std::string name_;
    std::uintptr_t begin_;
    std::uintptr_t end_;
    std::uintptr_t offset_;
    uint8_t perm;
};
