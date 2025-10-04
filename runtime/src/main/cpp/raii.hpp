#pragma once

#include "logger.hpp"

#include <sys/mman.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h> // NOLINT(*-deprecated-headers)
#include <dirent.h>

template<bool allow_invalid = false>
struct RAIIFD {
    RAIIFD(int fd) { // NOLINT(*-explicit-constructor)
        if (!allow_invalid) fatal_assert(fd != -1);
        value = fd;
    }

    ~RAIIFD() {
        if (isValid()) fatal_assert(close(value) >= 0);
    }

    bool isValid() const { // NOLINT(*-use-nodiscard)
        return value != -1;
    }

    operator int() const { // NOLINT(*-explicit-constructor)
        return value;
    }

    int value;
};

struct RAIIFile {
    RAIIFile(int dirfd, const char *name) {
        RAIIFD res(openat(dirfd, name, O_RDONLY));

        struct stat s{};
        fatal_assert(fstat(res.value, &s) >= 0);

        void *base = mmap(nullptr, s.st_size, PROT_READ, MAP_SHARED, res, 0);
        fatal_assert(base != MAP_FAILED);

        data = base;
        length = s.st_size;
    }

    ~RAIIFile() {
        fatal_assert(munmap(data, length) >= 0);
    }

    void *data;
    uint32_t length;
};

struct RAIIDir {
    explicit RAIIDir(int dirfd) {
        auto tmp = fdopendir(dup(dirfd));
        fatal_assert(tmp != nullptr);
        value = tmp;
    }

    ~RAIIDir() {
        closedir(value);
    }

    operator DIR *() const { // NOLINT(*-explicit-constructor)
        return value;
    }

    DIR *value;
};

template<typename T>
struct RAIILink {
    explicit RAIILink() {
        value = nullptr;
        next = nullptr;
    }

    ~RAIILink() {
        // Note: deleting null pointer has no effect
        delete value;
        delete next;
    }

    T *value;
    RAIILink<T> *next;
};

struct RAIIStr {
    RAIIStr(char *value) : value(value) { // NOLINT(*-explicit-constructor)
    }

    ~RAIIStr() {
        free(value);
    }

    operator char *() const { // NOLINT(*-explicit-constructor)
        return value;
    }

    char *value;
};
