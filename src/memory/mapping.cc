#include <iostream>

#include "mapping.hh"

bool Mapping::operator==(const std::string& name) const {
    return name_ == name;
}

std::ostream& operator<<(std::ostream& o, const Mapping& m) {
    return o << std::hex << m.begin_ << '-' << std::hex << m.end_ << ' ' << m.perm_ << ' ' << m.offset_ << ' ' << m.name_;
}
