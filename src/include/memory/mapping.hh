#pragma once
#include <string>
#include <sys/types.h>

class Mapping {
public:
    bool operator==(const std::string& name) const;
    pid_t pid_;
    std::string name_;
    std::uintptr_t begin_;
    std::uintptr_t end_;
    std::uintptr_t offset_;
    uint8_t perm;
};

std::ostream& operator<<(std::ostream& o, const Mapping& m);
