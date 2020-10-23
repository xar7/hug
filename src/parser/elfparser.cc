#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "elfparser.hh"
#include "log.hh"

int ElfParser::init(void) {
    int fd = open(path_.c_str(), O_RDONLY);
    if (fd < 0) {
        ERR("ElfParser: Unable to open %s !", path_.c_str());
        perror("ElfParserError: ");
        return fd;
    }

    struct stat stat_buf;
    if (fstat(fd, &stat_buf) < 0) {
        ERR("ElfParser: stat failed!");
    }
    size_ = stat_buf.st_size;

    ehdr_ = static_cast<ElfW(Ehdr) *>(mmap(NULL, stat_buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (ehdr_ == MAP_FAILED) {
        ERR("ElfParser: mmap failed at mapping elf.");
        return 1;
    }

    phdr_ = reinterpret_cast<ElfW(Phdr) *>(reinterpret_cast<char *>(ehdr_) + ehdr_->e_shoff);
    shdr_ = reinterpret_cast<ElfW(Shdr) *>(reinterpret_cast<char *>(ehdr_) + ehdr_->e_shoff);

    /* Find the symtable */
    for (auto i = 0; i < ehdr_->e_shnum; i++) {
        if (shdr_[i].sh_type == SHT_SYMTAB) {
            symtab_ = reinterpret_cast<ElfW(Sym) *>(reinterpret_cast<char *>(ehdr_) + shdr_[i].sh_offset);
            break;
        }
    }

    if (close(fd) < 0) {
        ERR("ElfParser: close failed.");
        perror("ElfParserError: ");
        return 1;
    }

    return 0;
}

void ElfParser::destroy(void) {
    // XXX yeye check error codes
    munmap(ehdr_, sizeof(ElfW(Ehdr)));
}
