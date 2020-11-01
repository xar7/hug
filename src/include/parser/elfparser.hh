#pragma once

#include <elf.h>
#include <link.h>

#include <memory>

class ElfParser {
public:
    ElfParser(std::string elf_path) : path_(elf_path) {};

    /* Initialize parser state and check the binary :
       - check the elf magic number
       - get addresses of differents sections */
    int init(void);
    void destroy(void);

    std::shared_ptr<ElfW(Sym)> get_symbol(std::string symbol_name);

    bool is_pie(void);

private:
    std::string path_;

    size_t size_;

    ElfW(Ehdr) *ehdr_;
    ElfW(Phdr) *phdr_;
    ElfW(Shdr) *shdr_;
    ElfW(Sym) *symtab_;
    ElfW(Shdr) *symtabhdr_;
    char *strtab_;
    char *shstrtab_;
    ElfW(Shdr) *strtabhdr_;

};
