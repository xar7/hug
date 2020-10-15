#include <unistd.h>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include "debugger.h"

void Debugger::start_inferior() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        std::cerr << "Unable to fork !" << std::endl;
    }
    else if (pid == 0) {
        /* Child */
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
            std::cerr << "ptrace TRACEME request failed!" << std::endl;

        char *const arg[2] = { binary_path_, NULL };
        if (execvp(arg[0], arg))
            std::cerr << "execvp failed!" << std::endl;
    }
    else {
        /* Parent */
        inferior_pid_ = pid;
    }
}

void Debugger::wait_inferior() {
    waitpid(inferior_pid_, &wstatus_, 0);

    if (WIFSTOPPED(wstatus_) && WSTOPSIG(wstatus_) == SIGTRAP) {
        std::cout << "inferior stopped on sigtrap" << std::endl;
    }
    else if (WIFEXITED(wstatus_)) {
        std::cout << "inferior exited!" << std::endl;
    }
}

void Debugger::continue_inferior() {
    ptrace(PTRACE_CONT, inferior_pid_, NULL, NULL);
}
