#include "logger.hpp"

#include <stdint.h> // NOLINT(*-deprecated-headers)
#include <malloc.h>

void *operator new(size_t sz) {
    void *ptr = malloc(sz);
    fatal_assert(ptr != nullptr);
    return ptr;
}

void operator delete(void *ptr) noexcept {
    free(ptr);
}
