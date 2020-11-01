#include "mapping.hh"

bool Mapping::operator==(const std::string& name) const {
    return name_ == name;
}
