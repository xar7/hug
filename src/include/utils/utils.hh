#pragma once

#include <sys/ptrace.h>

#include <map>
#include <string>


namespace utils {
    extern std::string signalstr[31];
    extern std::map<enum __ptrace_request, std::string> ptrace_request_str;
}
