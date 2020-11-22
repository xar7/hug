#pragma once

#include <sys/ptrace.h>

#include <map>
#include <string>


namespace utils {
    extern const std::array<std::string, 31> signalstr;
    extern const std::map<enum __ptrace_request, std::string> ptrace_request_str;
}
