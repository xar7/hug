#pragma once

#include <elf.h>
#include <link.h>

class ElfParser {
public:
    ElfParser(std::string elf_path) : path_(elf_path) {};

    /* Initialize parser state and check the binary :
       - check the elf magic number
       - get addresses of differents sections */
    int init(void);

    void destroy(void);

private:
    std::string path_;

    size_t size_;

    ElfW(Ehdr) *ehdr_;
    ElfW(Phdr) *phdr_;
    ElfW(Shdr) *shdr_;
    ElfW(Sym) *symtab_;

};
