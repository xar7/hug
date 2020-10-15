#include <sys/types.h>

class Debugger {
public:
    Debugger(char *binary_path) : binary_path_(binary_path) {};
    ~Debugger() = default;

    void start_inferior();
    void wait_inferior();
    void continue_inferior();

private:
    int wstatus_ = 0;
    pid_t inferior_pid_;
    char *binary_path_;
};
