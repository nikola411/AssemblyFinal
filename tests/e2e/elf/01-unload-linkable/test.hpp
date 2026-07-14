#pragma once
#include "Elf.hpp"
#include "ElfHelpers.hpp"
#include "DataTypes.hpp"

#include <sstream>
#include <vector>
#include <cstring>

/*
 * Binary layout:
 *
 *  Offset  Size  Content
 *  ------  ----  -------
 *     0      64  ELF header
 *    64       8  .text:      { 0x00..0x07 }
 *    72       8  .data:      { 0x10..0x17 }
 *    80       4  .rodata:    { 0xAA 0xBB 0xCC 0xDD }
 *    84     144  .symtab:    6 symbols * 24 bytes
 *   228      44  .strtab:    "\0helper\0local_var\0const_val\0main\0global_var\0"
 *   272      48  .rela.text: 2 RELA entries
 *   320      48  .rela.data: 2 RELA entries
 *   368      69  .shstrtab
 *   440     512  SHT: 8 headers * 64 bytes
 *
 * SHT:
 *   [0] .symtab      sh_link=1, sh_info=4(first global)
 *   [1] .strtab
 *   [2] .text        SHF_ALLOC|SHF_EXECINSTR
 *   [3] .data        SHF_ALLOC|SHF_WRITE
 *   [4] .rodata      SHF_ALLOC
 *   [5] .rela.text   sh_link=0, sh_info=2
 *   [6] .rela.data   sh_link=0, sh_info=3
 *   [7] .shstrtab    e_shstrndx=7
 *
 * Symbols (local first, then global):
 *   [0] null
 *   [1] helper     LOCAL FUNC    .text   offset=4  size=2
 *   [2] local_var  LOCAL OBJECT  .data   offset=4  size=4
 *   [3] const_val  LOCAL OBJECT  .rodata offset=0  size=1
 *   [4] main       GLOBAL FUNC   .text   offset=0  size=8
 *   [5] global_var GLOBAL OBJECT .data   offset=0  size=4
 *
 * Relocations:
 *   .rela.text[0]  r_offset=0  sym=main(4)       type=REL32_DIRECT  addend=0
 *   .rela.text[1]  r_offset=4  sym=helper(1)     type=REL12_DIRECT  addend=-4
 *   .rela.data[0]  r_offset=0  sym=global_var(5) type=REL32_DIRECT  addend=0
 *   .rela.data[1]  r_offset=4  sym=const_val(3)  type=REL12_DIRECT  addend=8
 */

namespace test_01 {

template<typename T>
static void put(std::vector<uint8_t>& buf, size_t offset, const T& obj)
{
    if (offset + sizeof(T) > buf.size())
        buf.resize(offset + sizeof(T), 0);
    std::memcpy(buf.data() + offset, &obj, sizeof(T));
}

static std::vector<uint8_t> build_elf()
{
    std::vector<uint8_t> buf;

    Elf64_Ehdr ehdr = {};
    ehdr.e_ident[EI_MAG0]    = 0x7F; ehdr.e_ident[EI_MAG1] = 'E';
    ehdr.e_ident[EI_MAG2]    = 'L';  ehdr.e_ident[EI_MAG3] = 'F';
    ehdr.e_ident[EI_CLASS]   = ELFCLASS64;
    ehdr.e_ident[EI_DATA]    = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = 1;
    ehdr.e_ident[EI_OSABI]   = ELFOSABI_LINUX;
    ehdr.e_type      = ET_REL;   ehdr.e_machine   = EM_X86_64;
    ehdr.e_version   = 1;        ehdr.e_ehsize    = sizeof(Elf64_Ehdr);
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    ehdr.e_shnum     = 8;        ehdr.e_shstrndx  = 7;
    ehdr.e_shoff     = 440;
    put(buf, 0, ehdr);

    // .text at 64
    uint8_t text[]   = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
    uint8_t data[]   = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
    uint8_t rodata[] = {0xAA,0xBB,0xCC,0xDD};
    if (buf.size() < 84) buf.resize(84, 0);
    std::memcpy(buf.data() + 64, text,   8);
    std::memcpy(buf.data() + 72, data,   8);
    std::memcpy(buf.data() + 80, rodata, 4);

    // .symtab at 84
    Elf64_Sym s = {};
    put(buf, 84 + 0*sizeof(Elf64_Sym), s);  // [0] null

    s = {}; s.st_name=1;  s.st_info=ELF64_ST_INFO(STB_LOCAL,STT_FUNC);   s.st_shndx=2; s.st_value=4; s.st_size=2;
    put(buf, 84 + 1*sizeof(Elf64_Sym), s);  // [1] helper

    s = {}; s.st_name=8;  s.st_info=ELF64_ST_INFO(STB_LOCAL,STT_OBJECT); s.st_shndx=3; s.st_value=4; s.st_size=4;
    put(buf, 84 + 2*sizeof(Elf64_Sym), s);  // [2] local_var

    s = {}; s.st_name=18; s.st_info=ELF64_ST_INFO(STB_LOCAL,STT_OBJECT); s.st_shndx=4; s.st_value=0; s.st_size=1;
    put(buf, 84 + 3*sizeof(Elf64_Sym), s);  // [3] const_val

    s = {}; s.st_name=28; s.st_info=ELF64_ST_INFO(STB_GLOBAL,STT_FUNC);   s.st_shndx=2; s.st_value=0; s.st_size=8;
    put(buf, 84 + 4*sizeof(Elf64_Sym), s);  // [4] main

    s = {}; s.st_name=33; s.st_info=ELF64_ST_INFO(STB_GLOBAL,STT_OBJECT); s.st_shndx=3; s.st_value=0; s.st_size=4;
    put(buf, 84 + 5*sizeof(Elf64_Sym), s);  // [5] global_var

    // .strtab at 228: "\0helper\0local_var\0const_val\0main\0global_var\0"
    const char strtab[] = "\0helper\0local_var\0const_val\0main\0global_var";
    if (228 + sizeof(strtab) > buf.size()) buf.resize(228 + sizeof(strtab), 0);
    std::memcpy(buf.data() + 228, strtab, sizeof(strtab));

    // .rela.text at 272
    Elf64_Rela r = {};
    r = {}; r.r_offset=0; r.r_info=ELF64_R_INFO(4,REL32_DIRECT); r.r_addend=0;
    put(buf, 272 + 0*sizeof(Elf64_Rela), r);
    r = {}; r.r_offset=4; r.r_info=ELF64_R_INFO(1,REL12_DIRECT); r.r_addend=-4;
    put(buf, 272 + 1*sizeof(Elf64_Rela), r);

    // .rela.data at 320
    r = {}; r.r_offset=0; r.r_info=ELF64_R_INFO(5,REL32_DIRECT); r.r_addend=0;
    put(buf, 320 + 0*sizeof(Elf64_Rela), r);
    r = {}; r.r_offset=4; r.r_info=ELF64_R_INFO(3,REL12_DIRECT); r.r_addend=8;
    put(buf, 320 + 1*sizeof(Elf64_Rela), r);

    // .shstrtab at 368
    // [0]\0 [1].symtab\0 [9].strtab\0 [17].text\0 [23].data\0
    // [29].rodata\0 [37].rela.text\0 [48].rela.data\0 [59].shstrtab\0
    const char shstrtab[] =
        "\0.symtab\0.strtab\0.text\0.data\0.rodata\0.rela.text\0.rela.data\0.shstrtab";
    if (368 + sizeof(shstrtab) > buf.size()) buf.resize(368 + sizeof(shstrtab), 0);
    std::memcpy(buf.data() + 368, shstrtab, sizeof(shstrtab));

    if (buf.size() < 440) buf.resize(440, 0);

    // SHT at 440
    auto sput = [&](int i, Elf64_Shdr h){ put(buf, 440 + i*sizeof(Elf64_Shdr), h); };

    Elf64_Shdr h = {};
    h={}; h.sh_name=1;  h.sh_type=SHT_SYMTAB; h.sh_offset=84;  h.sh_size=6*sizeof(Elf64_Sym);
    h.sh_link=1; h.sh_info=4; h.sh_addralign=8; h.sh_entsize=sizeof(Elf64_Sym); sput(0,h);

    h={}; h.sh_name=9;  h.sh_type=SHT_STRTAB; h.sh_offset=228; h.sh_size=44; h.sh_addralign=1; sput(1,h);

    h={}; h.sh_name=17; h.sh_type=SHT_PROGBITS; h.sh_flags=SHF_ALLOC|SHF_EXECINSTR;
    h.sh_offset=64; h.sh_size=8; h.sh_addralign=4; sput(2,h);

    h={}; h.sh_name=23; h.sh_type=SHT_PROGBITS; h.sh_flags=SHF_ALLOC|SHF_WRITE;
    h.sh_offset=72; h.sh_size=8; h.sh_addralign=4; sput(3,h);

    h={}; h.sh_name=29; h.sh_type=SHT_PROGBITS; h.sh_flags=SHF_ALLOC;
    h.sh_offset=80; h.sh_size=4; h.sh_addralign=1; sput(4,h);

    h={}; h.sh_name=37; h.sh_type=SHT_RELA; h.sh_offset=272; h.sh_size=2*sizeof(Elf64_Rela);
    h.sh_link=0; h.sh_info=2; h.sh_addralign=8; h.sh_entsize=sizeof(Elf64_Rela); sput(5,h);

    h={}; h.sh_name=48; h.sh_type=SHT_RELA; h.sh_offset=320; h.sh_size=2*sizeof(Elf64_Rela);
    h.sh_link=0; h.sh_info=3; h.sh_addralign=8; h.sh_entsize=sizeof(Elf64_Rela); sput(6,h);

    h={}; h.sh_name=59; h.sh_type=SHT_STRTAB; h.sh_offset=368;
    h.sh_size=sizeof(shstrtab); h.sh_addralign=1; sput(7,h);

    return buf;
}

static std::string run()
{
    Elf elf;
    elf.LoadBinary(build_elf());

    SymbolTable     symTable;
    SectionTable    sections;
    RelocationTable relocations;
    elf.UnloadLinkable(symTable, sections, relocations);

    std::ostringstream out;
    out << "=== Symbol Table ===\n";
    out << SymbolTableToString(symTable);
    out << "=== Section Table ===\n";
    out << SectionTableToString(sections);
    out << "=== Relocation Table ===\n";
    out << RelocationTableToString(relocations);
    return out.str();
}

} // namespace test_01
