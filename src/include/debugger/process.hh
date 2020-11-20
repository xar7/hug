#include <sys/ptrace.h>
#include <sys/types.h>

enum pstate {
    INIT,
    RUNNING,
    STOPPED
};

class Process {
public:
    Process(int pid) : s_(INIT), pid_(pid) {}

    void call_ptrace(enum __ptrace_request r, void *addr, void *data);
private:
    pstate s_;
    pid_t pid_;
};
