#include <sys/ptrace.h>

#include "log.hh"
#include "process.hh"
#include "utils.hh"

void Process::call_ptrace(enum __ptrace_request r, void *addr, void *data) {
    if (ptrace(r, pid_, addr, data) < 0) {
        ERR("ptrace error: pid %d request %s", pid_, utils::ptrace_request_str[r].c_str());
        /* XXX Do something clever here */
    }
}
