#pragma once
#include <stdio.h>

// To be improved

#if DEBUG
#define LOG(...) fprintf(stderr, "[hug][log] "); \
    fprintf(stderr, __VA_ARGS__);                \
    fprintf(stderr, "\n");
#else
#define LOG(...)
#endif /* DEBUG */
