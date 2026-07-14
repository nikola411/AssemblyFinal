/*
 * Diagnostic test for Elf::UnloadLinkable.
 *
 * Builds a minimal ELF binary by hand, calls UnloadLinkable, and prints
 * SymbolTable / SectionTable / RelocationTable for manual inspection.
 *
 * ── Binary layout ────────────────────────────────────────────────────────────
 *
 *  Offset  Size  Content
 *  ------  ----  -------
 *     0      64  ELF header
 *    64       8  .text:      { 0x00..0x07 }    (8 bytes - two instructions)
 *    72       8  .data:      { 0x10..0x17 }    (8 bytes - two 4-byte values)
 *    80       4  .rodata:    { 0xAA 0xBB 0xCC 0xDD }
 *    84     144  .symtab:    6 symbols * 24 bytes
 *   228      44  .strtab:    "\0helper\0local_var\0const_val\0main\0global_var\0"
 *   272      48  .rela.text: 2 RELA entries * 24 bytes
 *   320      48  .rela.data: 2 RELA entries * 24 bytes
 *   368      69  .shstrtab
 *   440     512  SHT: 8 section headers * 64 bytes
 *
 * ── Section header table (SHT) ───────────────────────────────────────────────
 *
 *  [0] .symtab      SHT_SYMTAB    sh_link=1(strtab), sh_info=4(first global)
 *  [1] .strtab      SHT_STRTAB
 *  [2] .text        SHT_PROGBITS  SHF_ALLOC|SHF_EXECINSTR
 *  [3] .data        SHT_PROGBITS  SHF_ALLOC|SHF_WRITE
 *  [4] .rodata      SHT_PROGBITS  SHF_ALLOC
 *  [5] .rela.text   SHT_RELA      sh_link=0(symtab), sh_info=2(.text)
 *  [6] .rela.data   SHT_RELA      sh_link=0(symtab), sh_info=3(.data)
 *  [7] .shstrtab    SHT_STRTAB    e_shstrndx=7
 *
 * ── Symbols ───────────────────────────────────────────────────────────────────
 *
 *  [0] null                      all zeros
 *  [1] helper    LOCAL  FUNC    .text   offset=4  size=2  isBig=false
 *  [2] local_var LOCAL  OBJECT  .data   offset=4  size=4  isBig=true
 *  [3] const_val LOCAL  OBJECT  .rodata offset=0  size=1  isBig=false
 *  [4] main      GLOBAL FUNC    .text   offset=0  size=8  isBig=true
 *  [5] global_var GLOBAL OBJECT .data   offset=0  size=4  isBig=true
 *  sh_info=4: symbols [0..3] are local, [4..5] are global
 *
 * ── Relocations ───────────────────────────────────────────────────────────────
 *
 *  .rela.text:
 *    r_offset=0  sym=main(4)      type=REL32_DIRECT  addend=0
 *    r_offset=4  sym=helper(1)    type=REL12_DIRECT  addend=-4
 *  .rela.data:
 *    r_offset=0  sym=global_var(5) type=REL32_DIRECT addend=0
 *    r_offset=4  sym=const_val(3)  type=REL12_DIRECT addend=8
 *
 * ── .strtab (offset 228, 44 bytes) ───────────────────────────────────────────
 *
 *  [0]  '\0'
 *  [1]  "helper\0"      → helper.st_name = 1
 *  [8]  "local_var\0"   → local_var.st_name = 8
 *  [18] "const_val\0"   → const_val.st_name = 18
 *  [28] "main\0"        → main.st_name = 28
 *  [33] "global_var\0"  → global_var.st_name = 33
 *
 * ── .shstrtab (offset 368, 69 bytes) ─────────────────────────────────────────
 *
 *  [0]  '\0'
 *  [1]  ".symtab\0"      sh_name=1
 *  [9]  ".strtab\0"      sh_name=9
 *  [17] ".text\0"        sh_name=17
 *  [23] ".data\0"        sh_name=23
 *  [29] ".rodata\0"      sh_name=29
 *  [37] ".rela.text\0"   sh_name=37
 *  [48] ".rela.data\0"   sh_name=48
 *  [59] ".shstrtab\0"    sh_name=59
 */

#include "Elf.hpp"
#include "ElfHelpers.hpp"
#include "DataTypes.hpp"

#include <iostream>
#include <vector>
#include <cstring>

// Write T at 'offset' in buf, growing buf if needed.
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

    // ── ELF header ───────────────────────────────────────────────────────────
    Elf64_Ehdr ehdr = {};
    ehdr.e_ident[EI_MAG0]    = 0x7F;
    ehdr.e_ident[EI_MAG1]    = 'E';
    ehdr.e_ident[EI_MAG2]    = 'L';
    ehdr.e_ident[EI_MAG3]    = 'F';
    ehdr.e_ident[EI_CLASS]   = ELFCLASS64;
    ehdr.e_ident[EI_DATA]    = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = 1;
    ehdr.e_ident[EI_OSABI]   = ELFOSABI_LINUX;
    ehdr.e_type      = ET_REL;
    ehdr.e_machine   = EM_X86_64;
    ehdr.e_version   = 1;
    ehdr.e_ehsize    = sizeof(Elf64_Ehdr);
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    ehdr.e_shnum     = 8;
    ehdr.e_shstrndx  = 7;
    ehdr.e_shoff     = 440;
    put(buf, 0, ehdr);

    // ── Section data ─────────────────────────────────────────────────────────

    // .text at 64, 8 bytes
    uint8_t text_data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    for (int i = 0; i < 8; i++) { if (64+i >= buf.size()) buf.resize(64+i+1, 0); buf[64+i] = text_data[i]; }

    // .data at 72, 8 bytes
    uint8_t data_data[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    for (int i = 0; i < 8; i++) { if (72+i >= buf.size()) buf.resize(72+i+1, 0); buf[72+i] = data_data[i]; }

    // .rodata at 80, 4 bytes
    uint8_t rodata_data[] = {0xAA, 0xBB, 0xCC, 0xDD};
    for (int i = 0; i < 4; i++) { if (80+i >= buf.size()) buf.resize(80+i+1, 0); buf[80+i] = rodata_data[i]; }

    // ── .symtab at 84, 144 bytes (6 symbols * 24) ────────────────────────────

    // [0] null
    Elf64_Sym null_sym = {};
    put(buf, 84 + 0 * sizeof(Elf64_Sym), null_sym);

    // [1] helper   LOCAL FUNC  .text offset=4 size=2
    Elf64_Sym helper_sym = {};
    helper_sym.st_name  = 1;
    helper_sym.st_info  = ELF64_ST_INFO(STB_LOCAL, STT_FUNC);
    helper_sym.st_shndx = 2;   // .text
    helper_sym.st_value = 4;
    helper_sym.st_size  = 2;
    put(buf, 84 + 1 * sizeof(Elf64_Sym), helper_sym);

    // [2] local_var  LOCAL OBJECT  .data offset=4 size=4
    Elf64_Sym localvar_sym = {};
    localvar_sym.st_name  = 8;
    localvar_sym.st_info  = ELF64_ST_INFO(STB_LOCAL, STT_OBJECT);
    localvar_sym.st_shndx = 3;  // .data
    localvar_sym.st_value = 4;
    localvar_sym.st_size  = 4;
    put(buf, 84 + 2 * sizeof(Elf64_Sym), localvar_sym);

    // [3] const_val  LOCAL OBJECT  .rodata offset=0 size=1
    Elf64_Sym constval_sym = {};
    constval_sym.st_name  = 18;
    constval_sym.st_info  = ELF64_ST_INFO(STB_LOCAL, STT_OBJECT);
    constval_sym.st_shndx = 4;  // .rodata
    constval_sym.st_value = 0;
    constval_sym.st_size  = 1;
    put(buf, 84 + 3 * sizeof(Elf64_Sym), constval_sym);

    // [4] main  GLOBAL FUNC  .text offset=0 size=8
    Elf64_Sym main_sym = {};
    main_sym.st_name  = 28;
    main_sym.st_info  = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
    main_sym.st_shndx = 2;  // .text
    main_sym.st_value = 0;
    main_sym.st_size  = 8;
    put(buf, 84 + 4 * sizeof(Elf64_Sym), main_sym);

    // [5] global_var  GLOBAL OBJECT  .data offset=0 size=4
    Elf64_Sym globalvar_sym = {};
    globalvar_sym.st_name  = 33;
    globalvar_sym.st_info  = ELF64_ST_INFO(STB_GLOBAL, STT_OBJECT);
    globalvar_sym.st_shndx = 3;  // .data
    globalvar_sym.st_value = 0;
    globalvar_sym.st_size  = 4;
    put(buf, 84 + 5 * sizeof(Elf64_Sym), globalvar_sym);

    // ── .strtab at 228, 44 bytes ──────────────────────────────────────────────
    // "\0helper\0local_var\0const_val\0main\0global_var\0"
    const char strtab[] = "\0helper\0local_var\0const_val\0main\0global_var";
    if (228 + sizeof(strtab) > buf.size()) buf.resize(228 + sizeof(strtab), 0);
    std::memcpy(buf.data() + 228, strtab, sizeof(strtab));

    // ── .rela.text at 272, 48 bytes (2 entries * 24) ─────────────────────────

    // [0] .text+0: sym=main(4)  type=REL32_DIRECT  addend=0
    Elf64_Rela rela0 = {};
    rela0.r_offset = 0;
    rela0.r_info   = ELF64_R_INFO(4, REL32_DIRECT);
    rela0.r_addend = 0;
    put(buf, 272 + 0 * sizeof(Elf64_Rela), rela0);

    // [1] .text+4: sym=helper(1)  type=REL12_DIRECT  addend=-4
    Elf64_Rela rela1 = {};
    rela1.r_offset = 4;
    rela1.r_info   = ELF64_R_INFO(1, REL12_DIRECT);
    rela1.r_addend = -4;
    put(buf, 272 + 1 * sizeof(Elf64_Rela), rela1);

    // ── .rela.data at 320, 48 bytes (2 entries * 24) ─────────────────────────

    // [2] .data+0: sym=global_var(5)  type=REL32_DIRECT  addend=0
    Elf64_Rela rela2 = {};
    rela2.r_offset = 0;
    rela2.r_info   = ELF64_R_INFO(5, REL32_DIRECT);
    rela2.r_addend = 0;
    put(buf, 320 + 0 * sizeof(Elf64_Rela), rela2);

    // [3] .data+4: sym=const_val(3)  type=REL12_DIRECT  addend=8
    Elf64_Rela rela3 = {};
    rela3.r_offset = 4;
    rela3.r_info   = ELF64_R_INFO(3, REL12_DIRECT);
    rela3.r_addend = 8;
    put(buf, 320 + 1 * sizeof(Elf64_Rela), rela3);

    // ── .shstrtab at 368, 69 bytes ────────────────────────────────────────────
    // [0]\0 [1].symtab\0 [9].strtab\0 [17].text\0 [23].data\0
    // [29].rodata\0 [37].rela.text\0 [48].rela.data\0 [59].shstrtab\0
    const char shstrtab[] =
        "\0.symtab\0.strtab\0.text\0.data\0.rodata\0.rela.text\0.rela.data\0.shstrtab";
    if (368 + sizeof(shstrtab) > buf.size()) buf.resize(368 + sizeof(shstrtab), 0);
    std::memcpy(buf.data() + 368, shstrtab, sizeof(shstrtab));

    // ── padding to 440 ───────────────────────────────────────────────────────
    if (buf.size() < 440) buf.resize(440, 0);

    // ── SHT at 440, 8 headers * 64 bytes ─────────────────────────────────────

    // [0] .symtab
    Elf64_Shdr symtab_shdr = {};
    symtab_shdr.sh_name      = 1;
    symtab_shdr.sh_type      = SHT_SYMTAB;
    symtab_shdr.sh_offset    = 84;
    symtab_shdr.sh_size      = 6 * sizeof(Elf64_Sym);
    symtab_shdr.sh_link      = 1;                     // .strtab
    symtab_shdr.sh_info      = 4;                     // first global sym index
    symtab_shdr.sh_addralign = 8;
    symtab_shdr.sh_entsize   = sizeof(Elf64_Sym);
    put(buf, 440 + 0 * sizeof(Elf64_Shdr), symtab_shdr);

    // [1] .strtab
    Elf64_Shdr strtab_shdr = {};
    strtab_shdr.sh_name      = 9;
    strtab_shdr.sh_type      = SHT_STRTAB;
    strtab_shdr.sh_offset    = 228;
    strtab_shdr.sh_size      = 44;
    strtab_shdr.sh_addralign = 1;
    put(buf, 440 + 1 * sizeof(Elf64_Shdr), strtab_shdr);

    // [2] .text
    Elf64_Shdr text_shdr = {};
    text_shdr.sh_name      = 17;
    text_shdr.sh_type      = SHT_PROGBITS;
    text_shdr.sh_flags     = SHF_ALLOC | SHF_EXECINSTR;
    text_shdr.sh_offset    = 64;
    text_shdr.sh_size      = 8;
    text_shdr.sh_addralign = 4;
    put(buf, 440 + 2 * sizeof(Elf64_Shdr), text_shdr);

    // [3] .data
    Elf64_Shdr data_shdr = {};
    data_shdr.sh_name      = 23;
    data_shdr.sh_type      = SHT_PROGBITS;
    data_shdr.sh_flags     = SHF_ALLOC | SHF_WRITE;
    data_shdr.sh_offset    = 72;
    data_shdr.sh_size      = 8;
    data_shdr.sh_addralign = 4;
    put(buf, 440 + 3 * sizeof(Elf64_Shdr), data_shdr);

    // [4] .rodata
    Elf64_Shdr rodata_shdr = {};
    rodata_shdr.sh_name      = 29;
    rodata_shdr.sh_type      = SHT_PROGBITS;
    rodata_shdr.sh_flags     = SHF_ALLOC;
    rodata_shdr.sh_offset    = 80;
    rodata_shdr.sh_size      = 4;
    rodata_shdr.sh_addralign = 1;
    put(buf, 440 + 4 * sizeof(Elf64_Shdr), rodata_shdr);

    // [5] .rela.text
    Elf64_Shdr rela_text_shdr = {};
    rela_text_shdr.sh_name      = 37;
    rela_text_shdr.sh_type      = SHT_RELA;
    rela_text_shdr.sh_offset    = 272;
    rela_text_shdr.sh_size      = 2 * sizeof(Elf64_Rela);
    rela_text_shdr.sh_link      = 0;                  // .symtab
    rela_text_shdr.sh_info      = 2;                  // applies to .text (shdrt[2])
    rela_text_shdr.sh_addralign = 8;
    rela_text_shdr.sh_entsize   = sizeof(Elf64_Rela);
    put(buf, 440 + 5 * sizeof(Elf64_Shdr), rela_text_shdr);

    // [6] .rela.data
    Elf64_Shdr rela_data_shdr = {};
    rela_data_shdr.sh_name      = 48;
    rela_data_shdr.sh_type      = SHT_RELA;
    rela_data_shdr.sh_offset    = 320;
    rela_data_shdr.sh_size      = 2 * sizeof(Elf64_Rela);
    rela_data_shdr.sh_link      = 0;                  // .symtab
    rela_data_shdr.sh_info      = 3;                  // applies to .data (shdrt[3])
    rela_data_shdr.sh_addralign = 8;
    rela_data_shdr.sh_entsize   = sizeof(Elf64_Rela);
    put(buf, 440 + 6 * sizeof(Elf64_Shdr), rela_data_shdr);

    // [7] .shstrtab
    Elf64_Shdr shstrtab_shdr = {};
    shstrtab_shdr.sh_name      = 59;
    shstrtab_shdr.sh_type      = SHT_STRTAB;
    shstrtab_shdr.sh_offset    = 368;
    shstrtab_shdr.sh_size      = sizeof(shstrtab);
    shstrtab_shdr.sh_addralign = 1;
    put(buf, 440 + 7 * sizeof(Elf64_Shdr), shstrtab_shdr);

    return buf;
}

int main()
{
    std::vector<uint8_t> elfBinary = build_elf();

    Elf elf;
    elf.LoadBinary(elfBinary);

    SymbolTable     symTable;
    SectionTable    sections;
    RelocationTable relocations;

    elf.UnloadLinkable(symTable, sections, relocations);

    std::cout << "=== Symbol Table ===\n";
    std::cout << SymbolTableToString(symTable);

    std::cout << "=== Section Table ===\n";
    std::cout << SectionTableToString(sections);

    std::cout << "=== Relocation Table ===\n";
    std::cout << RelocationTableToString(relocations);

    return 0;
}
