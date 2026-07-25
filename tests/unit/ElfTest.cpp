/*
 * Unit tests for the Elf class and ElfHelpers.
 *
 * Groups:
 *   [1]  ElfIO template utility  – ReadObject/WriteObject/ReadTable/WriteTable
 *   [2]  ELF ident bytes         – magic, class, encoding, version, OSABI, padding
 *   [3]  Header roundtrip        – SetElfHeader / GetElfHeader
 *   [4]  LoadLinkable safety     – no-crash across symbol/section/relocation combos
 *   [5]  FileWrite               – file creation, magic bytes, file size
 *   [6]  PHT roundtrip           – SetProgramHeaderTable / GetProgramHeaderTable
 *   [7]  SHT roundtrip           – SetSectionHeaderTable / GetSectionHeaderTable
 *   [8]  Section content         – GetSectionContent / UpdateSectionContent
 *   [9]  Helper functions        – GetSectionIndex, MakeSectionHeader
 *   [10] WriteSymtabSection
 *   [11] WriteStrtabSection
 *   [12] WriteShstrtabSection
 *   [13] WriteProgramSections
 *
 * Known open bugs exercised below (see docs/TODO.md) — these tests are
 * expected to FAIL until the corresponding bug is fixed; they exist so the
 * bug shows up as a red test instead of silently rotting:
 *   - [11] WriteStrtabSection: sh_name for `.strtab` is hardcoded to 0 instead
 *     of being computed (should be 8, right after ".symtab\0").
 */

#include "Elf.hpp"
#include "ElfHelpers.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ── tiny test harness ─────────────────────────────────────────────────────────

static int g_passed = 0;
static int g_failed = 0;

#define RUN(fn)                                                                 \
    do {                                                                        \
        std::cout << "  " << #fn << " ... ";                                   \
        try {                                                                   \
            fn();                                                               \
            std::cout << "\033[32mPASS\033[0m\n";                              \
            g_passed++;                                                         \
        } catch (const std::exception& _e) {                                   \
            std::cout << "\033[31mFAIL\033[0m: " << _e.what() << "\n";        \
            g_failed++;                                                         \
        }                                                                       \
    } while (0)

#define ASSERT(cond)                                                            \
    do {                                                                        \
        if (!(cond))                                                            \
            throw std::runtime_error("assert(" #cond ") at line "              \
                                     + std::to_string(__LINE__));               \
    } while (0)

#define ASSERT_EQ(a, b)                                                         \
    do {                                                                        \
        auto _a = (a);                                                          \
        auto _b = (b);                                                          \
        if (_a != _b)                                                           \
            throw std::runtime_error(                                           \
                std::string("assert_eq(") + #a + ", " + #b + ") -> ("         \
                + std::to_string((long long)_a) + " != "                       \
                + std::to_string((long long)_b) + ") at line "                 \
                + std::to_string(__LINE__));                                    \
    } while (0)

// ── test-data builders ────────────────────────────────────────────────────────

static Symbol::s_ptr make_sym(const std::string& name,
                               const std::string& section,
                               uint32_t offset   = 0,
                               bool     isGlobal = false,
                               uint16_t value    = 0)
{
    return std::make_shared<Symbol>(name, section, offset, value,
                                    /*defined=*/true, isGlobal);
}

static Section::s_ptr make_sec(const std::string& name,
                                std::vector<uint8_t> data = {})
{
    auto s             = std::make_shared<Section>();
    s->name            = name;
    s->data            = data;
    s->locationCounter = (uint32_t)data.size();
    return s;
}

static Relocation::s_ptr make_rel(const std::string& symbolName,
                                   uint32_t offset,
                                   eRelocationType type,
                                   int32_t addend = 0)
{
    auto r         = std::make_shared<Relocation>();
    r->symbolName  = symbolName;
    r->offset      = offset;
    r->type        = type;
    r->addend      = addend;
    return r;
}

// A well-formed ELF header with all mandatory fields set.
static Elf64_Ehdr blank_ehdr()
{
    Elf64_Ehdr h      = {};
    h.e_ident[EI_MAG0]       = 0x7F;
    h.e_ident[EI_MAG1]       = 'E';
    h.e_ident[EI_MAG2]       = 'L';
    h.e_ident[EI_MAG3]       = 'F';
    h.e_ident[EI_CLASS]      = ELFCLASS64;
    h.e_ident[EI_DATA]       = ELFDATA2LSB;
    h.e_ident[EI_VERSION]    = 1;
    h.e_ident[EI_OSABI]      = ELFOSABI_LINUX;
    h.e_ident[EI_ABIVERSION] = 0;
    h.e_type                  = ET_REL;
    h.e_machine               = EM_X86_64;
    h.e_version               = 1;
    h.e_ehsize                = sizeof(Elf64_Ehdr);
    h.e_phentsize             = sizeof(Elf64_Phdr);
    h.e_shentsize             = sizeof(Elf64_Shdr);
    return h;
}

// Elf with a raw content buffer of (sizeof(Ehdr) + extra_bytes) usable space,
// for tests that only need a big-enough backing buffer (PHT/SHT/section
// content roundtrips) without going through LoadLinkable.
static Elf make_elf_with_space(size_t extra_bytes)
{
    Elf elf;
    elf.content = std::vector<uint8_t>(sizeof(Elf64_Ehdr) + extra_bytes, 0);
    return elf;
}

// Elf with just enough space for, and populated with, a default ELF header.
static Elf make_elf_with_default_header()
{
    Elf elf;
    elf.content = std::vector<uint8_t>(sizeof(Elf64_Ehdr), 0);
    elf.SetElfHeader(InitEhdr());
    return elf;
}

// ── [1] ElfIO template utility ────────────────────────────────────────────────

void test_write_read_object_uint32()
{
    std::vector<uint8_t> buf(16, 0);
    uint32_t val = 0xDEADBEEF;
    ElfIO::WriteObject<uint32_t>(4, buf, val);
    uint32_t got = ElfIO::ReadObject<uint32_t>(4, buf);
    ASSERT_EQ(got, val);
}

void test_write_read_object_at_offset_zero()
{
    std::vector<uint8_t> buf(8, 0);
    uint64_t val = 0x0102030405060708ULL;
    ElfIO::WriteObject<uint64_t>(0, buf, val);
    uint64_t got = ElfIO::ReadObject<uint64_t>(0, buf);
    ASSERT_EQ(got, val);
}

void test_write_read_object_full_ehdr_struct()
{
    std::vector<uint8_t> buf(sizeof(Elf64_Ehdr) + 8, 0);
    Elf64_Ehdr h = blank_ehdr();
    h.e_shnum    = 7;
    h.e_phnum    = 3;
    ElfIO::WriteObject<Elf64_Ehdr>(0, buf, h);
    Elf64_Ehdr got = ElfIO::ReadObject<Elf64_Ehdr>(0, buf);
    ASSERT_EQ(got.e_shnum,          (Elf64_Half)7);
    ASSERT_EQ(got.e_phnum,          (Elf64_Half)3);
    ASSERT_EQ(got.e_ident[EI_MAG0], (uint8_t)0x7F);
}

void test_write_read_table_basic()
{
    std::vector<uint32_t> table = {10, 20, 30, 40};
    std::vector<uint8_t>  buf(table.size() * sizeof(uint32_t), 0);
    ElfIO::WriteTable<uint32_t>(0, buf, table);
    auto got = ElfIO::ReadTable<uint32_t>(0, (Elf64_Half)table.size(), buf);
    ASSERT_EQ(got.size(), table.size());
    ASSERT_EQ(got[0], 10u);
    ASSERT_EQ(got[1], 20u);
    ASSERT_EQ(got[2], 30u);
    ASSERT_EQ(got[3], 40u);
}

void test_write_read_table_at_nonzero_offset()
{
    std::vector<uint32_t> table = {0xAA, 0xBB};
    std::vector<uint8_t>  buf(32, 0xFF);
    Elf64_Off off = 8;
    ElfIO::WriteTable<uint32_t>(off, buf, table);

    // bytes before the offset must be untouched
    for (size_t i = 0; i < (size_t)off; ++i)
        ASSERT_EQ(buf[i], (uint8_t)0xFF);

    auto got = ElfIO::ReadTable<uint32_t>(off, (Elf64_Half)table.size(), buf);
    ASSERT_EQ(got[0], 0xAAu);
    ASSERT_EQ(got[1], 0xBBu);
}

void test_write_read_table_of_shdrs()
{
    Elf64_Shdr s0 = {}; s0.sh_type = SHT_PROGBITS; s0.sh_size = 16;
    Elf64_Shdr s1 = {}; s1.sh_type = SHT_SYMTAB;   s1.sh_size = 24;
    std::vector<Elf64_Shdr> table = {s0, s1};

    std::vector<uint8_t> buf(table.size() * sizeof(Elf64_Shdr), 0);
    ElfIO::WriteTable<Elf64_Shdr>(0, buf, table);
    auto got = ElfIO::ReadTable<Elf64_Shdr>(0, (Elf64_Half)table.size(), buf);

    ASSERT_EQ(got.size(), (size_t)2);
    ASSERT_EQ(got[0].sh_type, (Elf64_Word)SHT_PROGBITS);
    ASSERT_EQ(got[0].sh_size, (Elf64_Xword)16);
    ASSERT_EQ(got[1].sh_type, (Elf64_Word)SHT_SYMTAB);
    ASSERT_EQ(got[1].sh_size, (Elf64_Xword)24);
}

// ── [2] ELF ident bytes ───────────────────────────────────────────────────────

void test_elf_magic_ei_mag0_is_7f()
{
    Elf elf = make_elf_with_default_header();
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_MAG0], (uint8_t)0x7F);
}

void test_elf_magic_ei_mag1_is_E()
{
    Elf elf = make_elf_with_default_header();
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_MAG1], (uint8_t)'E');
}

void test_elf_magic_ei_mag2_is_L()
{
    Elf elf = make_elf_with_default_header();
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_MAG2], (uint8_t)'L');
}

void test_elf_magic_ei_mag3_is_F()
{
    Elf elf = make_elf_with_default_header();
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_MAG3], (uint8_t)'F');
}

void test_elf_class_is_64bit()
{
    Elf elf = make_elf_with_default_header();
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_CLASS], (uint8_t)ELFCLASS64);
}

void test_elf_encoding_is_little_endian()
{
    Elf elf = make_elf_with_default_header();
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_DATA], (uint8_t)ELFDATA2LSB);
}

void test_elf_ident_version_is_1()
{
    Elf elf = make_elf_with_default_header();
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_VERSION], (uint8_t)1);
}

void test_elf_osabi_is_linux()
{
    Elf elf = make_elf_with_default_header();
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_OSABI], (uint8_t)ELFOSABI_LINUX);
}

void test_elf_ident_abiversion_is_zero()
{
    Elf elf = make_elf_with_default_header();
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_ABIVERSION], (uint8_t)0);
}

void test_elf_ident_padding_bytes_are_zero()
{
    Elf elf = make_elf_with_default_header();
    Elf64_Ehdr h = elf.GetElfHeader();
    for (int i = EI_PAD; i < 16; ++i)
        ASSERT_EQ(h.e_ident[i], (uint8_t)0);
}

// ── [3] SetElfHeader / GetElfHeader roundtrip ─────────────────────────────────

void test_set_get_elf_header_type_field()
{
    Elf elf = make_elf_with_default_header();
    Elf64_Ehdr h = blank_ehdr();
    h.e_type     = ET_EXEC;
    elf.SetElfHeader(h);
    ASSERT_EQ(elf.GetElfHeader().e_type, (Elf64_Half)ET_EXEC);
}

void test_set_get_elf_header_machine_field()
{
    Elf elf = make_elf_with_default_header();
    Elf64_Ehdr h = blank_ehdr();
    h.e_machine  = EM_X86_64;
    elf.SetElfHeader(h);
    ASSERT_EQ(elf.GetElfHeader().e_machine, (Elf64_Half)EM_X86_64);
}

void test_set_get_elf_header_section_count()
{
    Elf elf = make_elf_with_default_header();
    Elf64_Ehdr h  = blank_ehdr();
    h.e_shnum     = 5;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);
    Elf64_Ehdr got = elf.GetElfHeader();
    ASSERT_EQ(got.e_shnum,     (Elf64_Half)5);
    ASSERT_EQ(got.e_shentsize, (Elf64_Half)sizeof(Elf64_Shdr));
}

void test_set_get_elf_header_section_offset()
{
    Elf elf = make_elf_with_default_header();
    Elf64_Ehdr h = blank_ehdr();
    h.e_shoff    = sizeof(Elf64_Ehdr);
    elf.SetElfHeader(h);
    ASSERT_EQ(elf.GetElfHeader().e_shoff, (Elf64_Off)sizeof(Elf64_Ehdr));
}

void test_set_get_elf_header_phdr_fields()
{
    Elf elf = make_elf_with_default_header();
    Elf64_Ehdr h  = blank_ehdr();
    h.e_phnum     = 2;
    h.e_phentsize = sizeof(Elf64_Phdr);
    h.e_phoff     = sizeof(Elf64_Ehdr);
    elf.SetElfHeader(h);
    Elf64_Ehdr got = elf.GetElfHeader();
    ASSERT_EQ(got.e_phnum,     (Elf64_Half)2);
    ASSERT_EQ(got.e_phentsize, (Elf64_Half)sizeof(Elf64_Phdr));
    ASSERT_EQ(got.e_phoff,     (Elf64_Off)sizeof(Elf64_Ehdr));
}

void test_set_get_elf_header_magic_preserved()
{
    Elf elf = make_elf_with_default_header();
    Elf64_Ehdr h = blank_ehdr();
    h.e_shnum    = 9;
    elf.SetElfHeader(h);
    Elf64_Ehdr got = elf.GetElfHeader();
    ASSERT_EQ(got.e_ident[EI_MAG0], (uint8_t)0x7F);
    ASSERT_EQ(got.e_ident[EI_MAG1], (uint8_t)'E');
    ASSERT_EQ(got.e_ident[EI_MAG2], (uint8_t)'L');
    ASSERT_EQ(got.e_ident[EI_MAG3], (uint8_t)'F');
    ASSERT_EQ(got.e_shnum,          (Elf64_Half)9);
}

void test_set_elf_header_returns_true()
{
    Elf elf = make_elf_with_default_header();
    ASSERT(elf.SetElfHeader(blank_ehdr()) == true);
}

// ── [4] LoadLinkable safety ───────────────────────────────────────────────────

void test_loadlinkable_empty_inputs_no_crash()
{
    Elf elf;
    elf.LoadLinkable({}, {});
    ASSERT(elf.content.size() >= sizeof(Elf64_Ehdr));
}

void test_loadlinkable_with_one_section_no_crash()
{
    Elf elf;
    SectionTable secs = { make_sec("text", {0x00, 0x00, 0x00, 0x00}) };
    elf.LoadLinkable({}, secs);
    ASSERT(true);
}

void test_loadlinkable_with_local_and_global_symbols_no_crash()
{
    Elf elf;
    SymbolTable syms = {
        make_sym("local_a",  "text", 0, false),
        make_sym("global_b", "text", 4, true),
        make_sym("local_c",  "data", 8, false),
    };
    SectionTable secs = {
        make_sec("text", {0x00, 0x00, 0x00, 0x00}),
        make_sec("data", {0xAA, 0xBB}),
    };
    elf.LoadLinkable(syms, secs);
    ASSERT(true);
}

void test_loadlinkable_with_relocations_no_crash()
{
    // NOTE: LoadLinkable never writes the ELF header / SHT into `content`
    // before calling WriteProgramSections, so ConvertRelocationToRela's symbol
    // lookup (GetSymbolByName -> GetSymbolTable -> GetSectionHeaderTable)
    // always misses at this point and silently yields a zeroed Elf64_Rela.
    // This is a no-crash test only; see [13] for a real correctness test of
    // relocation serialization with the symtab/SHT wired up by hand.
    Elf elf;
    auto sec = make_sec("text", {0x00, 0x00, 0x00, 0x00});
    sec->sectionRelocations.push_back(make_rel("external_func", 4, REL32_ABS));
    SectionTable secs = { sec };
    elf.LoadLinkable({}, secs);
    ASSERT(true);
}

// ── [5] FileWrite ──────────────────────────────────────────────────────────────

void test_filewrite_creates_file()
{
    Elf elf = make_elf_with_default_header();
    std::string path = "/tmp/elf_unit_creates.o";
    FileWrite(path, elf.content);
    std::ifstream f(path, std::ios::binary);
    ASSERT(f.is_open());
    f.close();
    std::remove(path.c_str());
}

void test_filewrite_magic_bytes_correct()
{
    Elf elf = make_elf_with_default_header();
    std::string path = "/tmp/elf_unit_magic.o";
    FileWrite(path, elf.content);
    std::ifstream f(path, std::ios::binary);
    ASSERT(f.is_open());
    uint8_t magic[4] = {};
    f.read(reinterpret_cast<char*>(magic), 4);
    ASSERT_EQ(magic[0], (uint8_t)0x7F);
    ASSERT_EQ(magic[1], (uint8_t)'E');
    ASSERT_EQ(magic[2], (uint8_t)'L');
    ASSERT_EQ(magic[3], (uint8_t)'F');
    f.close();
    std::remove(path.c_str());
}

void test_filewrite_size_at_least_ehdr()
{
    Elf elf = make_elf_with_default_header();
    std::string path = "/tmp/elf_unit_size.o";
    FileWrite(path, elf.content);
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    ASSERT(f.is_open());
    ASSERT(f.tellg() >= (std::streamsize)sizeof(Elf64_Ehdr));
    f.close();
    std::remove(path.c_str());
}

void test_filewrite_elfclass64_byte()
{
    Elf elf = make_elf_with_default_header();
    std::string path = "/tmp/elf_unit_class.o";
    FileWrite(path, elf.content);
    std::ifstream f(path, std::ios::binary);
    ASSERT(f.is_open());
    uint8_t buf[5] = {};
    f.read(reinterpret_cast<char*>(buf), 5);
    ASSERT_EQ(buf[EI_CLASS], (uint8_t)ELFCLASS64);
    f.close();
    std::remove(path.c_str());
}

void test_filewrite_little_endian_byte()
{
    Elf elf = make_elf_with_default_header();
    std::string path = "/tmp/elf_unit_encoding.o";
    FileWrite(path, elf.content);
    std::ifstream f(path, std::ios::binary);
    ASSERT(f.is_open());
    uint8_t buf[6] = {};
    f.read(reinterpret_cast<char*>(buf), 6);
    ASSERT_EQ(buf[EI_DATA], (uint8_t)ELFDATA2LSB);
    f.close();
    std::remove(path.c_str());
}

// ── [6] Program header table roundtrip ───────────────────────────────────────

void test_pht_empty_roundtrip()
{
    Elf elf = make_elf_with_space(256);
    Elf64_Ehdr h  = blank_ehdr();
    h.e_phoff     = sizeof(Elf64_Ehdr);
    h.e_phnum     = 0;
    h.e_phentsize = sizeof(Elf64_Phdr);
    elf.SetElfHeader(h);
    ASSERT_EQ(elf.GetProgramHeaderTable().size(), (size_t)0);
}

void test_pht_set_get_single_entry()
{
    Elf elf = make_elf_with_space(256);

    Elf64_Phdr entry = {};
    entry.p_type   = 1; // PT_LOAD
    entry.p_offset = sizeof(Elf64_Ehdr);
    entry.p_filesz = 0x1000;
    entry.p_memsz  = 0x2000;
    entry.p_align  = 4096;

    Elf64_Ehdr h  = blank_ehdr();
    h.e_phoff     = sizeof(Elf64_Ehdr);
    h.e_phnum     = 1;
    h.e_phentsize = sizeof(Elf64_Phdr);
    elf.SetElfHeader(h);
    elf.SetProgramHeaderTable({entry});

    auto phdr = elf.GetProgramHeaderTable();
    ASSERT_EQ(phdr.size(),       (size_t)1);
    ASSERT_EQ(phdr[0].p_type,   1u);
    ASSERT_EQ(phdr[0].p_filesz, (Elf64_Xword)0x1000);
    ASSERT_EQ(phdr[0].p_memsz,  (Elf64_Xword)0x2000);
    ASSERT_EQ(phdr[0].p_align,  (Elf64_Xword)4096);
}

void test_pht_set_get_two_entries()
{
    Elf elf = make_elf_with_space(256);

    Elf64_Phdr e0 = {};  e0.p_offset = 0;      e0.p_filesz = 100;
    Elf64_Phdr e1 = {};  e1.p_offset = 0x100;  e1.p_filesz = 200;

    Elf64_Ehdr h  = blank_ehdr();
    h.e_phoff     = sizeof(Elf64_Ehdr);
    h.e_phnum     = 2;
    h.e_phentsize = sizeof(Elf64_Phdr);
    elf.SetElfHeader(h);
    elf.SetProgramHeaderTable({e0, e1});

    auto phdr = elf.GetProgramHeaderTable();
    ASSERT_EQ(phdr.size(),       (size_t)2);
    ASSERT_EQ(phdr[0].p_filesz, (Elf64_Xword)100);
    ASSERT_EQ(phdr[1].p_filesz, (Elf64_Xword)200);
    ASSERT_EQ(phdr[1].p_offset, (Elf64_Off)0x100);
}

void test_pht_overwrite_replaces_entries()
{
    Elf elf = make_elf_with_space(256);

    Elf64_Ehdr h  = blank_ehdr();
    h.e_phoff     = sizeof(Elf64_Ehdr);
    h.e_phnum     = 1;
    h.e_phentsize = sizeof(Elf64_Phdr);
    elf.SetElfHeader(h);

    Elf64_Phdr first = {};  first.p_filesz  = 111;
    elf.SetProgramHeaderTable({first});

    Elf64_Phdr second = {}; second.p_filesz = 222;
    elf.SetProgramHeaderTable({second});

    auto phdr = elf.GetProgramHeaderTable();
    ASSERT_EQ(phdr[0].p_filesz, (Elf64_Xword)222);
}

// ── [7] Section header table roundtrip ───────────────────────────────────────

void test_sht_empty_roundtrip()
{
    Elf elf = make_elf_with_space(256);
    Elf64_Ehdr h  = blank_ehdr();
    h.e_shoff     = sizeof(Elf64_Ehdr);
    h.e_shnum     = 0;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);
    ASSERT_EQ(elf.GetSectionHeaderTable().size(), (size_t)0);
}

void test_sht_set_get_single_section()
{
    Elf elf = make_elf_with_space(256);

    Elf64_Shdr entry = {};
    entry.sh_name      = 0;
    entry.sh_type      = SHT_PROGBITS;
    entry.sh_flags     = SHF_ALLOC | SHF_EXECINSTR;
    entry.sh_offset    = sizeof(Elf64_Ehdr);
    entry.sh_size      = 4;
    entry.sh_addralign = 4;

    Elf64_Ehdr h  = blank_ehdr();
    h.e_shoff     = sizeof(Elf64_Ehdr);
    h.e_shnum     = 1;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);
    elf.SetSectionHeaderTable({entry});

    auto shdr = elf.GetSectionHeaderTable();
    ASSERT_EQ(shdr.size(),           (size_t)1);
    ASSERT_EQ(shdr[0].sh_type,      (Elf64_Word)SHT_PROGBITS);
    ASSERT_EQ(shdr[0].sh_size,      (Elf64_Xword)4);
    ASSERT_EQ(shdr[0].sh_addralign, (Elf64_Xword)4);
    ASSERT_EQ(shdr[0].sh_flags,     (Elf64_Xword)(SHF_ALLOC | SHF_EXECINSTR));
}

void test_sht_set_get_three_sections()
{
    Elf elf = make_elf_with_space(256);

    auto make_shdr = [](Elf64_Word name, Elf64_Word type, Elf64_Xword size) {
        Elf64_Shdr s = {};
        s.sh_name = name;
        s.sh_type = type;
        s.sh_size = size;
        return s;
    };

    std::vector<Elf64_Shdr> table = {
        make_shdr(0,  SHT_NULL,     0),
        make_shdr(1,  SHT_PROGBITS, 16),
        make_shdr(7,  SHT_SYMTAB,   24),
    };

    Elf64_Ehdr h  = blank_ehdr();
    h.e_shoff     = sizeof(Elf64_Ehdr);
    h.e_shnum     = 3;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);
    elf.SetSectionHeaderTable(table);

    auto got = elf.GetSectionHeaderTable();
    ASSERT_EQ(got.size(), (size_t)3);
    ASSERT_EQ(got[0].sh_type, (Elf64_Word)SHT_NULL);
    ASSERT_EQ(got[1].sh_type, (Elf64_Word)SHT_PROGBITS);
    ASSERT_EQ(got[1].sh_size, (Elf64_Xword)16);
    ASSERT_EQ(got[2].sh_type, (Elf64_Word)SHT_SYMTAB);
    ASSERT_EQ(got[2].sh_name, (Elf64_Word)7);
}

void test_sht_overwrite_replaces_entry()
{
    Elf elf = make_elf_with_space(256);

    Elf64_Ehdr h  = blank_ehdr();
    h.e_shoff     = sizeof(Elf64_Ehdr);
    h.e_shnum     = 1;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);

    Elf64_Shdr first = {};  first.sh_size  = 99;
    elf.SetSectionHeaderTable({first});

    Elf64_Shdr second = {}; second.sh_size = 42;
    elf.SetSectionHeaderTable({second});

    auto got = elf.GetSectionHeaderTable();
    ASSERT_EQ(got[0].sh_size, (Elf64_Xword)42);
}

void test_sht_section_returns_true()
{
    Elf elf = make_elf_with_space(256);
    Elf64_Ehdr h  = blank_ehdr();
    h.e_shoff     = sizeof(Elf64_Ehdr);
    h.e_shnum     = 1;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);
    ASSERT(elf.SetSectionHeaderTable({Elf64_Shdr{}}) == true);
}

// ── [8] Section content ───────────────────────────────────────────────────────

void test_get_section_content_returns_written_bytes()
{
    Elf elf = make_elf_with_space(256);

    std::vector<uint8_t> secData = {0x11, 0x22, 0x33, 0x44};
    Elf64_Shdr shdr = {};
    shdr.sh_offset = sizeof(Elf64_Ehdr);
    shdr.sh_size   = secData.size();

    Elf64_Ehdr h  = blank_ehdr();
    h.e_shoff     = sizeof(Elf64_Ehdr) + 64; // SHT lives after the section bytes
    h.e_shnum     = 1;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);
    elf.SetSectionHeaderTable({shdr});
    ElfIO::WriteTable<uint8_t>(shdr.sh_offset, elf.content, secData);

    auto got = elf.GetSectionContent(0);
    ASSERT_EQ(got.size(), secData.size());
    ASSERT_EQ(got[0], (uint8_t)0x11);
    ASSERT_EQ(got[3], (uint8_t)0x44);
}

void test_get_section_content_invalid_index_returns_empty()
{
    Elf elf = make_elf_with_space(256);
    Elf64_Ehdr h  = blank_ehdr();
    h.e_shoff     = sizeof(Elf64_Ehdr);
    h.e_shnum     = 0;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);
    auto got = elf.GetSectionContent(0);
    ASSERT_EQ(got.size(), (size_t)0);
}

void test_update_section_content_roundtrip()
{
    Elf elf = make_elf_with_space(256);

    Elf64_Shdr shdr = {};
    shdr.sh_offset = sizeof(Elf64_Ehdr);
    shdr.sh_size   = 3;

    Elf64_Ehdr h  = blank_ehdr();
    h.e_shoff     = sizeof(Elf64_Ehdr) + 64;
    h.e_shnum     = 1;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);
    elf.SetSectionHeaderTable({shdr});

    ASSERT(elf.UpdateSectionContent(0, {0xAA, 0xBB, 0xCC}) == true);
    auto got = elf.GetSectionContent(0);
    ASSERT_EQ(got.size(), (size_t)3);
    ASSERT_EQ(got[0], (uint8_t)0xAA);
    ASSERT_EQ(got[2], (uint8_t)0xCC);
}

void test_update_section_content_wrong_size_returns_false()
{
    Elf elf = make_elf_with_space(256);

    Elf64_Shdr shdr = {};
    shdr.sh_offset = sizeof(Elf64_Ehdr);
    shdr.sh_size   = 3;

    Elf64_Ehdr h  = blank_ehdr();
    h.e_shoff     = sizeof(Elf64_Ehdr) + 64;
    h.e_shnum     = 1;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);
    elf.SetSectionHeaderTable({shdr});

    ASSERT(elf.UpdateSectionContent(0, {0xAA, 0xBB}) == false);
}

// ── [9] Helper functions ─────────────────────────────────────────────────────

static void build_strtab_shdr(const std::vector<std::string>& names,
                               std::vector<uint8_t>& strtab,
                               std::vector<Elf64_Shdr>& shdr)
{
    strtab.clear();
    shdr.clear();
    for (const auto& name : names) {
        Elf64_Shdr s = {};
        s.sh_name = (Elf64_Word)strtab.size();
        strtab.insert(strtab.end(), name.begin(), name.end());
        strtab.push_back('\0');
        shdr.push_back(s);
    }
}

void test_get_section_index_first_section()
{
    std::vector<uint8_t>    strtab;
    std::vector<Elf64_Shdr> shdr;
    build_strtab_shdr({"text", "data", "bss"}, strtab, shdr);
    ASSERT_EQ(GetSectionIndex(shdr, strtab, "text"), 0);
}

void test_get_section_index_middle_section()
{
    std::vector<uint8_t>    strtab;
    std::vector<Elf64_Shdr> shdr;
    build_strtab_shdr({"text", "data", "bss"}, strtab, shdr);
    ASSERT_EQ(GetSectionIndex(shdr, strtab, "data"), 1);
}

void test_get_section_index_last_section()
{
    std::vector<uint8_t>    strtab;
    std::vector<Elf64_Shdr> shdr;
    build_strtab_shdr({"text", "data", "bss"}, strtab, shdr);
    ASSERT_EQ(GetSectionIndex(shdr, strtab, "bss"), 2);
}

void test_get_section_index_not_found_returns_minus1()
{
    std::vector<uint8_t>    strtab;
    std::vector<Elf64_Shdr> shdr;
    build_strtab_shdr({"text"}, strtab, shdr);
    ASSERT_EQ(GetSectionIndex(shdr, strtab, "missing"), -1);
}

void test_get_section_index_empty_table_returns_minus1()
{
    std::vector<uint8_t>    strtab;
    std::vector<Elf64_Shdr> shdr;
    ASSERT_EQ(GetSectionIndex(shdr, strtab, "text"), -1);
}

void test_get_section_index_with_dotted_names()
{
    std::vector<uint8_t>    strtab;
    std::vector<Elf64_Shdr> shdr;
    build_strtab_shdr({".text", ".data", ".rodata", ".bss"}, strtab, shdr);
    ASSERT_EQ(GetSectionIndex(shdr, strtab, ".rodata"), 2);
    ASSERT_EQ(GetSectionIndex(shdr, strtab, ".bss"),    3);
}

void test_make_section_header_name_offset()
{
    auto sec     = make_sec("text", {0x00, 0x01, 0x02, 0x03});
    Elf64_Shdr s = MakeSectionHeader(7, sec, 0);
    ASSERT_EQ(s.sh_name, (Elf64_Word)7);
}

void test_make_section_header_type_is_progbits()
{
    auto sec     = make_sec("text", {0xAA});
    Elf64_Shdr s = MakeSectionHeader(0, sec, 0);
    ASSERT_EQ(s.sh_type, (Elf64_Word)SHT_PROGBITS);
}

void test_make_section_header_size_matches_data()
{
    std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7, 8};
    auto sec     = make_sec("data", data);
    Elf64_Shdr s = MakeSectionHeader(0, sec, 0);
    ASSERT_EQ(s.sh_size, (Elf64_Xword)data.size());
}

void test_make_section_header_file_offset_includes_ehdr()
{
    auto sec       = make_sec("text", {0x00});
    Elf64_Off off  = 32;
    Elf64_Shdr s   = MakeSectionHeader(0, sec, off);
    ASSERT_EQ(s.sh_offset, (Elf64_Off)(sizeof(Elf64_Ehdr) + off));
}

void test_make_section_header_zero_data_offset()
{
    auto sec     = make_sec("text", {0x00});
    Elf64_Shdr s = MakeSectionHeader(0, sec, 0);
    ASSERT_EQ(s.sh_offset, (Elf64_Off)sizeof(Elf64_Ehdr));
}

void test_make_section_header_alignment_is_4()
{
    auto sec     = make_sec("text", {0x00});
    Elf64_Shdr s = MakeSectionHeader(0, sec, 0);
    ASSERT_EQ(s.sh_addralign, (Elf64_Xword)4);
}

void test_make_section_header_flags_alloc_and_write()
{
    auto sec     = make_sec("data", {0x00});
    Elf64_Shdr s = MakeSectionHeader(0, sec, 0);
    ASSERT_EQ(s.sh_flags, (Elf64_Xword)(SHF_ALLOC | SHF_WRITE));
}

void test_make_section_header_empty_section_size_zero()
{
    auto sec     = make_sec("bss", {});
    Elf64_Shdr s = MakeSectionHeader(0, sec, 0);
    ASSERT_EQ(s.sh_size, (Elf64_Xword)0);
}

void test_make_section_header_large_section()
{
    std::vector<uint8_t> data(1024, 0xFF);
    auto sec     = make_sec("rodata", data);
    Elf64_Shdr s = MakeSectionHeader(3, sec, 128);
    ASSERT_EQ(s.sh_name,   (Elf64_Word)3);
    ASSERT_EQ(s.sh_size,   (Elf64_Xword)1024);
    ASSERT_EQ(s.sh_offset, (Elf64_Off)(sizeof(Elf64_Ehdr) + 128));
}

// ── [10] WriteSymtabSection ──────────────────────────────────────────────────────

void test_load_symbol_table_pushes_one_shdr()
{
    Elf elf;
    elf.symTable = { make_sym("foo", "text") };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);

    ASSERT_EQ(shdrt.size(), (size_t)1);
    ASSERT_EQ(shdrt[0].sh_type,    (Elf64_Word)SHT_SYMTAB);
    ASSERT_EQ(shdrt[0].sh_entsize, (Elf64_Xword)sizeof(Elf64_Sym));
    ASSERT_EQ(shdrt[0].sh_size,    (Elf64_Xword)sizeof(Elf64_Sym));
    ASSERT_EQ(shdrt[0].sh_link,    (Elf64_Word)1); // .strtab immediately follows
}

void test_load_symbol_table_size_scales_with_symbol_count()
{
    Elf elf;
    elf.symTable = {
        make_sym("a", "text", 0, false),
        make_sym("b", "text", 0, true),
        make_sym("c", "text", 0, true),
    };
    elf.content = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    size_t written = elf.WriteSymtabSection(shdrt);

    ASSERT_EQ(shdrt[0].sh_size, (Elf64_Xword)(sizeof(Elf64_Sym) * 3));
    ASSERT_EQ(written,          sizeof(Elf64_Sym) * 3);
}

void test_load_symbol_table_locals_before_globals()
{
    // deliberately interleaved: global, local, global
    Elf elf;
    elf.symTable = {
        make_sym("g1", "text", 0, true),
        make_sym("l1", "text", 0, false),
        make_sym("g2", "text", 0, true),
    };
    elf.content = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);

    auto symt = ElfIO::ReadTable<Elf64_Sym>(shdrt[0].sh_offset,
                                             (Elf64_Half)(shdrt[0].sh_size / sizeof(Elf64_Sym)),
                                             elf.content);
    ASSERT_EQ(symt.size(), (size_t)3);
    ASSERT_EQ(ELF64_ST_BIND(symt[0].st_info), (unsigned char)STB_LOCAL);
    ASSERT_EQ(ELF64_ST_BIND(symt[1].st_info), (unsigned char)STB_GLOBAL);
    ASSERT_EQ(ELF64_ST_BIND(symt[2].st_info), (unsigned char)STB_GLOBAL);
}

// ── [11] WriteStrtabSection ──────────────────────────────────────────────────────

void test_load_string_table_pushes_shdr_after_symtab()
{
    Elf elf;
    elf.symTable = { make_sym("foo", "text") };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);

    ASSERT_EQ(shdrt.size(),      (size_t)2);
    ASSERT_EQ(shdrt[1].sh_type,  (Elf64_Word)SHT_STRTAB);
    ASSERT_EQ(shdrt[1].sh_offset, shdrt[0].sh_offset + shdrt[0].sh_size);
}

void test_load_string_table_contains_symbol_name()
{
    Elf elf;
    elf.symTable = { make_sym("hello", "text") };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);

    ASSERT(ReadString(elf.content, shdrt[1].sh_offset) == std::string("hello"));
}

void test_load_string_table_sh_link_and_info_unused()
{
    // per ELF spec, sh_link/sh_info are unused (0) for SHT_STRTAB sections
    Elf elf;
    elf.symTable = { make_sym("x", "text") };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);

    ASSERT_EQ(shdrt[1].sh_link, (Elf64_Word)0);
    ASSERT_EQ(shdrt[1].sh_info, (Elf64_Word)0);
}

// KNOWN OPEN BUG (docs/TODO.md): sh_entsize for .strtab is hardcoded to
// sizeof(uint8_t) (1); string tables have no fixed-size entries, so per the
// ELF spec and this project's own docs/CLAUDE.md table it must be 0.
void test_load_string_table_entsize_is_zero()
{
    Elf elf;
    elf.symTable = { make_sym("x", "text") };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);

    ASSERT_EQ(shdrt[1].sh_entsize, (Elf64_Xword)0);
}

// KNOWN OPEN BUG (docs/TODO.md): sh_name for .strtab is hardcoded to 1
// instead of being computed; it should point right after ".symtab\0" (8
// bytes: 7 chars + terminator), matching where WriteShstrtabSection
// actually writes the ".strtab" name into the shstrtab buffer.
void test_load_string_table_sh_name_points_past_symtab_name()
{
    Elf elf;
    elf.symTable = { make_sym("x", "text") };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);

    ASSERT_EQ(shdrt[1].sh_name, (Elf64_Word)(std::string(".symtab").size() + 1));
}

// ── [12] WriteShstrtabSection ─────────────────────────────────────────

void test_load_shstrtab_pushes_third_shdr()
{
    Elf elf;
    elf.symTable = { make_sym("foo", "text") };
    elf.sections = { make_sec("text", {0, 0, 0, 0}) };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);
    elf.WriteShstrtabSection(shdrt);

    ASSERT_EQ(shdrt.size(),        (size_t)3);
    ASSERT_EQ(shdrt[2].sh_type,    (Elf64_Word)SHT_STRTAB);
    ASSERT_EQ(shdrt[2].sh_entsize, (Elf64_Xword)0);
    ASSERT_EQ(shdrt[2].sh_link,    (Elf64_Word)0);
}

void test_load_shstrtab_offset_immediately_after_strtab()
{
    Elf elf;
    elf.symTable = { make_sym("foo", "text") };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);
    elf.WriteShstrtabSection(shdrt);

    ASSERT_EQ(shdrt[2].sh_offset, shdrt[1].sh_offset + shdrt[1].sh_size);
}

void test_load_shstrtab_name_resolves_to_dot_shstrtab()
{
    Elf elf;
    elf.symTable = { make_sym("foo", "text") };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);
    elf.WriteShstrtabSection(shdrt);

    ASSERT(ReadString(elf.content, shdrt[2].sh_offset + shdrt[2].sh_name) ==
           std::string(".shstrtab"));
}

void test_load_shstrtab_contains_all_section_names()
{
    Elf elf;
    elf.sections = {
        make_sec(".text", {0, 0, 0, 0}),
        make_sec(".data", {1, 2}),
    };
    elf.content = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);
    elf.WriteShstrtabSection(shdrt);

    auto bytes = ElfIO::ReadTable<uint8_t>(shdrt[2].sh_offset, (Elf64_Half)shdrt[2].sh_size, elf.content);
    std::string blob(bytes.begin(), bytes.end());
    ASSERT(blob.find(".text") != std::string::npos);
    ASSERT(blob.find(".data") != std::string::npos);
}

// ── [13] WriteProgramSections ──────────────────────────────────────────────────────

void test_load_section_data_progbits_shdr_per_section()
{
    Elf elf;
    elf.sections = {
        make_sec("text", {0x11, 0x22, 0x33, 0x44}),
        make_sec("data", {0xAA, 0xBB}),
    };
    elf.content = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    size_t written = elf.WriteProgramSections(shdrt, /*startOffset=*/100, /*shstrtOffset=*/16);

    ASSERT_EQ(shdrt.size(),     (size_t)2);
    ASSERT_EQ(shdrt[0].sh_type, (Elf64_Word)SHT_PROGBITS);
    ASSERT_EQ(shdrt[0].sh_size, (Elf64_Xword)4);
    ASSERT_EQ(shdrt[1].sh_size, (Elf64_Xword)2);
    ASSERT(written > 0);
}

// KNOWN OPEN BUG (docs/TODO.md): the literal-pool loop in WriteProgramSections
// builds and fills in a full Elf64_Shdr for the pool, but never actually
// does `shdrt.push_back(shdr)` -- only the plain-section (PROGBITS) loop
// does that. The pool's bytes get written to `poolsData`, but no shdr is
// ever registered for it, so `shdrt` never grows past the PROGBITS entries.
void test_load_section_data_literal_pool_shdr()
{
    Elf elf;
    auto sec = make_sec("text", {0x00, 0x00, 0x00, 0x00});
    sec->literalPool = {0xAA, 0xBB, 0xCC, 0xDD};
    elf.sections = { sec };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteProgramSections(shdrt, 100, 16);

    ASSERT_EQ(shdrt.size(),     (size_t)2); // .text + .text.pool
    ASSERT_EQ(shdrt[1].sh_type, (Elf64_Word)SHT_LITPOOL);
    ASSERT_EQ(shdrt[1].sh_size, (Elf64_Xword)4);
}

// KNOWN OPEN BUG (docs/TODO.md): the final byte-writing block in
// WriteProgramSections seeds its running `size` counter with sectionData.size()
// instead of 0, so every section after the first is written that many bytes
// past the sh_offset recorded for it. This reads through the declared
// sh_offset -- the contract any real consumer relies on -- so it is expected
// to FAIL until that block is fixed.
void test_load_section_data_bytes_readable_at_declared_offset()
{
    Elf elf;
    elf.sections = { make_sec("text", {0xDE, 0xAD, 0xBE, 0xEF}) };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteProgramSections(shdrt, 100, 16);

    auto bytes = ElfIO::ReadTable<uint8_t>(shdrt[0].sh_offset, (Elf64_Half)shdrt[0].sh_size, elf.content);
    ASSERT_EQ(bytes[0], (uint8_t)0xDE);
    ASSERT_EQ(bytes[3], (uint8_t)0xEF);
}

// Same known bug as above, demonstrated with two sections: the second
// section's declared sh_offset should not overlap the first's actual bytes.
void test_load_section_data_two_sections_readable_independently()
{
    Elf elf;
    elf.sections = {
        make_sec("a", {0x01, 0x02}),
        make_sec("b", {0x03, 0x04, 0x05}),
    };
    elf.content = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteProgramSections(shdrt, 100, 16);

    auto bytesA = ElfIO::ReadTable<uint8_t>(shdrt[0].sh_offset, (Elf64_Half)shdrt[0].sh_size, elf.content);
    auto bytesB = ElfIO::ReadTable<uint8_t>(shdrt[1].sh_offset, (Elf64_Half)shdrt[1].sh_size, elf.content);
    ASSERT_EQ(bytesA[0], (uint8_t)0x01);
    ASSERT_EQ(bytesA[1], (uint8_t)0x02);
    ASSERT_EQ(bytesB[0], (uint8_t)0x03);
    ASSERT_EQ(bytesB[2], (uint8_t)0x05);
}

void test_load_section_data_relocation_serialization()
{
    // ConvertRelocationToRela resolves the symbol via GetSymbolByName, which
    // needs a real ehdr + written SHT in `content` -- wire those up by hand.
    Elf elf;
    elf.symTable = { make_sym("target", "text", 0, /*isGlobal=*/true) };
    auto sec = make_sec("text", {0x00, 0x00, 0x00, 0x00});
    sec->sectionRelocations.push_back(make_rel("target", 4, REL32_ABS, 8));
    elf.sections = { sec };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt); // shdrt[0] = .symtab, symbol bytes written
    elf.WriteStrtabSection(shdrt); // shdrt[1] = .strtab, needed by GetSymbolByName

    Elf64_Ehdr ehdr  = InitEhdr();
    ehdr.e_shoff     = 3000; // parked well past everything else written below
    ehdr.e_shnum     = 2;
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(ehdr);
    elf.SetSectionHeaderTable({shdrt[0], shdrt[1]}); // lets GetSymbolByName find "target"

    elf.WriteProgramSections(shdrt, /*startOffset=*/200, /*shstrtOffset=*/16);

    // shdrt is now: [0] .symtab, [1] .strtab, [2] .text, [3] .text.rela
    ASSERT_EQ(shdrt.size(),        (size_t)4);
    ASSERT_EQ(shdrt[3].sh_type,    (Elf64_Word)SHT_RELA);
    ASSERT_EQ(shdrt[3].sh_size,    (Elf64_Xword)sizeof(Elf64_Rela));
    ASSERT_EQ(shdrt[3].sh_entsize, (Elf64_Xword)sizeof(Elf64_Rela));

    Elf64_Rela expected = Elf::ConvertRelocationToRela(*sec->sectionRelocations[0], elf);
    ASSERT_EQ(expected.r_offset, (Elf64_Addr)4);
    ASSERT_EQ(expected.r_addend, (Elf64_Sxword)8);
    ASSERT_EQ(ELF64_R_SYM(expected.r_info),  (Elf64_Xword)0); // "target" is symt[0]
    ASSERT_EQ(ELF64_R_TYPE(expected.r_info), (Elf64_Xword)REL32_ABS);
}

void test_load_section_data_rela_sh_link_and_info()
{
    Elf elf;
    elf.symTable = { make_sym("target", "text", 0, true) };
    auto sec = make_sec("text", {0x00, 0x00, 0x00, 0x00});
    sec->sectionRelocations.push_back(make_rel("target", 0, REL32_ABS));
    elf.sections = { sec };
    elf.content  = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);
    elf.WriteShstrtabSection(shdrt); // real pipeline always runs this before WriteProgramSections

    Elf64_Ehdr ehdr  = InitEhdr();
    ehdr.e_shoff     = 3000;
    ehdr.e_shnum     = 2;
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(ehdr);
    elf.SetSectionHeaderTable({shdrt[0], shdrt[1]});

    elf.WriteProgramSections(shdrt, 200, 16);

    // shdrt: [0] .symtab, [1] .strtab, [2] .shstrtab, [3] .text, [4] .text.rela
    ASSERT_EQ(shdrt.size(), (size_t)5);
    ASSERT_EQ(shdrt[4].sh_info, (Elf64_Word)3); // .text's shdr index
    ASSERT_EQ(shdrt[4].sh_link, (Elf64_Word)0); // .symtab's shdr index
}

// ── [14] UnloadLinkable ───────────────────────────────────────────────────────

// Drives the write pipeline by hand and finalizes e_shoff/e_shnum/e_shstrndx
// plus SetSectionHeaderTable. LoadLinkable itself doesn't do this yet (see
// docs/TODO.md), so tests that need a fully self-consistent buffer for
// UnloadLinkable to read back must do it themselves. Returns the final shdrt.
static std::vector<Elf64_Shdr> write_full_linkable(Elf& elf, size_t contentSize = 8192)
{
    elf.content = std::vector<uint8_t>(contentSize, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);
    elf.WriteShstrtabSection(shdrt);

    // WriteProgramSections resolves relocation symbols via GetSymbolByName,
    // which needs a real ehdr + SHT already written into `content`. Park a
    // provisional SHT (symtab/strtab/shstrtab only) far in the tail of the
    // buffer, well past anything WriteProgramSections is about to write.
    Elf64_Ehdr provisional  = InitEhdr();
    provisional.e_shoff     = contentSize - 1024;
    provisional.e_shnum     = (Elf64_Half)shdrt.size();
    provisional.e_shentsize = sizeof(Elf64_Shdr);
    provisional.e_shstrndx  = 2;
    elf.SetElfHeader(provisional);
    elf.SetSectionHeaderTable(shdrt);

    // Use the actual last shdr's end, not a size-sum: WriteSymtabSection
    // places .symtab at sizeof(Ehdr)+1 (one byte of slack), so re-deriving
    // the offset from sizeof(Ehdr) + individual sizes drifts by that byte.
    size_t shstrtOffset = sizeof(".symtab") + sizeof(".strtab") + sizeof(".shstrtab");
    size_t startOffset  = shdrt.back().sh_offset + shdrt.back().sh_size;
    size_t sectiondSize = elf.WriteProgramSections(shdrt, (int)startOffset, (int)shstrtOffset);

    Elf64_Ehdr ehdr  = InitEhdr();
    ehdr.e_shoff     = startOffset + sectiondSize;
    ehdr.e_shnum     = (Elf64_Half)shdrt.size();
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    ehdr.e_shstrndx  = 2; // shdrt[0]=.symtab, [1]=.strtab, [2]=.shstrtab
    elf.SetElfHeader(ehdr);
    elf.SetSectionHeaderTable(shdrt);

    return shdrt;
}

// KNOWN OPEN BUG (docs/TODO.md): WriteShstrtabSection hardcodes
// e_shstrndx = 3. At the point it runs, shdrt only has 3 entries (symtab,
// strtab, shstrtab itself), so shstrtab's real index is 2. UnloadLinkable
// reads shdrt[e_shstrndx] to find the section-name string table -- with the
// wrong index it would read some other shdr instead of shstrtab.
void test_write_shstrtab_section_sets_correct_e_shstrndx()
{
    Elf elf;
    elf.content = std::vector<uint8_t>(4096, 0);

    std::vector<Elf64_Shdr> shdrt;
    elf.WriteSymtabSection(shdrt);
    elf.WriteStrtabSection(shdrt);
    elf.WriteShstrtabSection(shdrt);

    ASSERT_EQ(elf.GetElfHeader().e_shstrndx, (Elf64_Half)2);
}

void test_unload_linkable_symbols_and_sections_roundtrip()
{
    Elf elf;
    elf.symTable = { make_sym("main", "text", 0, true) };
    elf.sections = { make_sec("text", {0x11, 0x22, 0x33, 0x44}) };

    write_full_linkable(elf);

    SymbolTable outSyms;
    SectionTable outSecs;
    elf.UnloadLinkable(outSyms, outSecs);

    ASSERT_EQ(outSyms.size(), (size_t)1);
    ASSERT(outSyms[0]->name == "main");

    ASSERT_EQ(outSecs.size(), (size_t)1);
    ASSERT(outSecs[0]->name == "text");
    ASSERT_EQ(outSecs[0]->data.size(), (size_t)4);
    ASSERT_EQ(outSecs[0]->data[0], (uint8_t)0x11);
    ASSERT_EQ(outSecs[0]->data[3], (uint8_t)0x44);
}

void test_unload_linkable_literal_pool_roundtrip()
{
    Elf elf;
    auto sec = make_sec("text", {0x00, 0x00, 0x00, 0x00});
    sec->literalPool = {0xAA, 0xBB, 0xCC, 0xDD};
    elf.sections = { sec };

    write_full_linkable(elf);

    SymbolTable outSyms;
    SectionTable outSecs;
    elf.UnloadLinkable(outSyms, outSecs);

    ASSERT_EQ(outSecs.size(), (size_t)1);
    ASSERT_EQ(outSecs[0]->literalPool.size(), (size_t)4);
    ASSERT_EQ(outSecs[0]->literalPool[0], (uint8_t)0xAA);
    ASSERT_EQ(outSecs[0]->literalPool[3], (uint8_t)0xDD);
}

void test_unload_linkable_section_relocation_values_correct()
{
    Elf elf;
    elf.symTable = { make_sym("target", "a", 0, true) };
    auto sec = make_sec("a", {0x00, 0x00, 0x00, 0x00});
    sec->sectionRelocations.push_back(make_rel("target", 4, REL32_ABS, 8));
    elf.sections = { sec };

    write_full_linkable(elf);

    SymbolTable outSyms;
    SectionTable outSecs;
    elf.UnloadLinkable(outSyms, outSecs);

    ASSERT_EQ(outSecs[0]->sectionRelocations.size(), (size_t)1);
    ASSERT_EQ(outSecs[0]->sectionRelocations[0]->offset, (uint32_t)4);
    ASSERT_EQ(outSecs[0]->sectionRelocations[0]->addend, (int32_t)8);
    ASSERT(outSecs[0]->sectionRelocations[0]->symbolName == "target");
}

// KNOWN OPEN BUG (docs/TODO.md): the `relocations` member used inside
// UnloadLinkable's relocation-reading loops is never cleared per shdr, so
// each section after the first inherits every earlier section's
// relocations too, instead of only its own.
void test_unload_linkable_section_relocations_do_not_leak_across_sections()
{
    Elf elf;
    elf.symTable = {
        make_sym("sym_a", "a", 0, true),
        make_sym("sym_b", "b", 0, true),
    };
    auto secA = make_sec("a", {0x00, 0x00, 0x00, 0x00});
    auto secB = make_sec("b", {0x00, 0x00, 0x00, 0x00});
    secA->sectionRelocations.push_back(make_rel("sym_a", 4, REL32_ABS, 10));
    secB->sectionRelocations.push_back(make_rel("sym_b", 8, REL32_ABS, 20));
    elf.sections = { secA, secB };

    write_full_linkable(elf);

    SymbolTable outSyms;
    SectionTable outSecs;
    elf.UnloadLinkable(outSyms, outSecs);

    ASSERT_EQ(outSecs.size(), (size_t)2);
    ASSERT_EQ(outSecs[0]->sectionRelocations.size(), (size_t)1);
    ASSERT_EQ(outSecs[1]->sectionRelocations.size(), (size_t)1);
    ASSERT_EQ(outSecs[0]->sectionRelocations[0]->addend, (int32_t)10);
    ASSERT_EQ(outSecs[1]->sectionRelocations[0]->addend, (int32_t)20);
}

// Same known bug as above, for pool relocations.
void test_unload_linkable_pool_relocations_do_not_leak_across_sections()
{
    Elf elf;
    elf.symTable = {
        make_sym("sym_a", "a", 0, true),
        make_sym("sym_b", "b", 0, true),
    };
    auto secA = make_sec("a", {0x00, 0x00, 0x00, 0x00});
    auto secB = make_sec("b", {0x00, 0x00, 0x00, 0x00});
    secA->poolRelocations.push_back(make_rel("sym_a", 0, REL32_ABS, 1));
    secB->poolRelocations.push_back(make_rel("sym_b", 0, REL32_ABS, 2));
    elf.sections = { secA, secB };

    write_full_linkable(elf);

    SymbolTable outSyms;
    SectionTable outSecs;
    elf.UnloadLinkable(outSyms, outSecs);

    ASSERT_EQ(outSecs.size(), (size_t)2);
    ASSERT_EQ(outSecs[0]->poolRelocations.size(), (size_t)1);
    ASSERT_EQ(outSecs[1]->poolRelocations.size(), (size_t)1);
    ASSERT_EQ(outSecs[0]->poolRelocations[0]->addend, (int32_t)1);
    ASSERT_EQ(outSecs[1]->poolRelocations[0]->addend, (int32_t)2);
}

// ── [15] End-to-end roundtrip ─────────────────────────────────────────────────

static Symbol::s_ptr find_sym(const SymbolTable& syms, const std::string& name)
{
    for (const auto& s : syms)
        if (s->name == name)
            return s;
    return nullptr;
}

static Section::s_ptr find_sec(const SectionTable& secs, const std::string& name)
{
    for (const auto& s : secs)
        if (s->name == name)
            return s;
    return nullptr;
}

// Builds two full SymbolTable/SectionTable structures (symbols, section
// data, a literal pool, and both kinds of relocations), runs them through
// the real public pipeline (LoadLinkable -> raw bytes -> LoadBinary ->
// UnloadLinkable), and checks the output matches the input field by field.
// This is the only test that drives LoadLinkable's own offset/index
// bookkeeping end to end instead of wiring the buffer up by hand, so it can
// catch integration bugs the per-function unit tests above cannot see.
void test_e2e_loadlinkable_unloadlinkable_roundtrip()
{
    SymbolTable inSyms = {
        make_sym("local_var",   "data", 0, false),
        make_sym("global_func", "text", 0, true),
        make_sym("target",      "data", 0, true, /*value=*/4),
    };

    auto text = make_sec("text", {0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11});
    text->literalPool = {0xAA, 0xBB, 0xCC, 0xDD};
    text->sectionRelocations.push_back(make_rel("target", 0, REL32_ABS, 5));
    text->poolRelocations.push_back(make_rel("global_func", 0, REL32_ABS, 6));

    auto data = make_sec("data", {0x01, 0x02, 0x03, 0x04});

    SectionTable inSecs = { text, data };

    Elf writer;
    writer.LoadLinkable(inSyms, inSecs);
    std::vector<uint8_t> raw = writer.content; // "raw elf" bytes

    Elf reader;
    reader.LoadBinary(raw);

    SymbolTable outSyms;
    SectionTable outSecs;
    reader.UnloadLinkable(outSyms, outSecs);

    // -- symbols --
    ASSERT_EQ(outSyms.size(), inSyms.size());

    auto outLocal = find_sym(outSyms, "local_var");
    ASSERT(outLocal != nullptr);
    ASSERT(outLocal->section == "data");
    ASSERT(outLocal->isGlobal == false);

    auto outGlobal = find_sym(outSyms, "global_func");
    ASSERT(outGlobal != nullptr);
    ASSERT(outGlobal->section == "text");
    ASSERT(outGlobal->isGlobal == true);

    auto outTarget = find_sym(outSyms, "target");
    ASSERT(outTarget != nullptr);
    ASSERT(outTarget->section == "data");
    ASSERT_EQ(outTarget->value, (uint16_t)4);

    // -- sections --
    ASSERT_EQ(outSecs.size(), inSecs.size());

    auto outText = find_sec(outSecs, "text");
    ASSERT(outText != nullptr);
    ASSERT_EQ(outText->data.size(), text->data.size());
    ASSERT_EQ(outText->data[4], (uint8_t)0x11);
    ASSERT_EQ(outText->literalPool.size(), (size_t)4);
    ASSERT_EQ(outText->literalPool[0], (uint8_t)0xAA);
    ASSERT_EQ(outText->sectionRelocations.size(), (size_t)1);
    ASSERT(outText->sectionRelocations[0]->symbolName == "target");
    ASSERT_EQ(outText->sectionRelocations[0]->addend, (int32_t)5);
    ASSERT_EQ(outText->poolRelocations.size(), (size_t)1);
    ASSERT(outText->poolRelocations[0]->symbolName == "global_func");
    ASSERT_EQ(outText->poolRelocations[0]->addend, (int32_t)6);

    auto outData = find_sec(outSecs, "data");
    ASSERT(outData != nullptr);
    ASSERT_EQ(outData->data.size(), (size_t)4);
    ASSERT_EQ(outData->data[3], (uint8_t)0x04);
}

// ── main ─────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "\n\033[1m[1] ElfIO template utility\033[0m\n";
    RUN(test_write_read_object_uint32);
    RUN(test_write_read_object_at_offset_zero);
    RUN(test_write_read_object_full_ehdr_struct);
    RUN(test_write_read_table_basic);
    RUN(test_write_read_table_at_nonzero_offset);
    RUN(test_write_read_table_of_shdrs);

    std::cout << "\n\033[1m[2] ELF ident bytes\033[0m\n";
    RUN(test_elf_magic_ei_mag0_is_7f);
    RUN(test_elf_magic_ei_mag1_is_E);
    RUN(test_elf_magic_ei_mag2_is_L);
    RUN(test_elf_magic_ei_mag3_is_F);
    RUN(test_elf_class_is_64bit);
    RUN(test_elf_encoding_is_little_endian);
    RUN(test_elf_ident_version_is_1);
    RUN(test_elf_osabi_is_linux);
    RUN(test_elf_ident_abiversion_is_zero);
    RUN(test_elf_ident_padding_bytes_are_zero);

    std::cout << "\n\033[1m[3] SetElfHeader / GetElfHeader roundtrip\033[0m\n";
    RUN(test_set_get_elf_header_type_field);
    RUN(test_set_get_elf_header_machine_field);
    RUN(test_set_get_elf_header_section_count);
    RUN(test_set_get_elf_header_section_offset);
    RUN(test_set_get_elf_header_phdr_fields);
    RUN(test_set_get_elf_header_magic_preserved);
    RUN(test_set_elf_header_returns_true);

    std::cout << "\n\033[1m[4] LoadLinkable safety\033[0m\n";
    RUN(test_loadlinkable_empty_inputs_no_crash);
    RUN(test_loadlinkable_with_one_section_no_crash);
    RUN(test_loadlinkable_with_local_and_global_symbols_no_crash);
    RUN(test_loadlinkable_with_relocations_no_crash);

    std::cout << "\n\033[1m[5] FileWrite\033[0m\n";
    RUN(test_filewrite_creates_file);
    RUN(test_filewrite_magic_bytes_correct);
    RUN(test_filewrite_size_at_least_ehdr);
    RUN(test_filewrite_elfclass64_byte);
    RUN(test_filewrite_little_endian_byte);

    std::cout << "\n\033[1m[6] Program header table roundtrip\033[0m\n";
    RUN(test_pht_empty_roundtrip);
    RUN(test_pht_set_get_single_entry);
    RUN(test_pht_set_get_two_entries);
    RUN(test_pht_overwrite_replaces_entries);

    std::cout << "\n\033[1m[7] Section header table roundtrip\033[0m\n";
    RUN(test_sht_empty_roundtrip);
    RUN(test_sht_set_get_single_section);
    RUN(test_sht_set_get_three_sections);
    RUN(test_sht_overwrite_replaces_entry);
    RUN(test_sht_section_returns_true);

    std::cout << "\n\033[1m[8] Section content\033[0m\n";
    RUN(test_get_section_content_returns_written_bytes);
    RUN(test_get_section_content_invalid_index_returns_empty);
    RUN(test_update_section_content_roundtrip);
    RUN(test_update_section_content_wrong_size_returns_false);

    std::cout << "\n\033[1m[9] Helper functions\033[0m\n";
    RUN(test_get_section_index_first_section);
    RUN(test_get_section_index_middle_section);
    RUN(test_get_section_index_last_section);
    RUN(test_get_section_index_not_found_returns_minus1);
    RUN(test_get_section_index_empty_table_returns_minus1);
    RUN(test_get_section_index_with_dotted_names);
    RUN(test_make_section_header_name_offset);
    RUN(test_make_section_header_type_is_progbits);
    RUN(test_make_section_header_size_matches_data);
    RUN(test_make_section_header_file_offset_includes_ehdr);
    RUN(test_make_section_header_zero_data_offset);
    RUN(test_make_section_header_alignment_is_4);
    RUN(test_make_section_header_flags_alloc_and_write);
    RUN(test_make_section_header_empty_section_size_zero);
    RUN(test_make_section_header_large_section);

    std::cout << "\n\033[1m[10] WriteSymtabSection\033[0m\n";
    RUN(test_load_symbol_table_pushes_one_shdr);
    RUN(test_load_symbol_table_size_scales_with_symbol_count);
    RUN(test_load_symbol_table_locals_before_globals);

    std::cout << "\n\033[1m[11] WriteStrtabSection\033[0m\n";
    RUN(test_load_string_table_pushes_shdr_after_symtab);
    RUN(test_load_string_table_contains_symbol_name);
    RUN(test_load_string_table_sh_link_and_info_unused);
    RUN(test_load_string_table_entsize_is_zero);
    RUN(test_load_string_table_sh_name_points_past_symtab_name);

    std::cout << "\n\033[1m[12] WriteShstrtabSection\033[0m\n";
    RUN(test_load_shstrtab_pushes_third_shdr);
    RUN(test_load_shstrtab_offset_immediately_after_strtab);
    RUN(test_load_shstrtab_name_resolves_to_dot_shstrtab);
    RUN(test_load_shstrtab_contains_all_section_names);

    std::cout << "\n\033[1m[13] WriteProgramSections\033[0m\n";
    RUN(test_load_section_data_progbits_shdr_per_section);
    RUN(test_load_section_data_literal_pool_shdr);
    RUN(test_load_section_data_bytes_readable_at_declared_offset);
    RUN(test_load_section_data_two_sections_readable_independently);
    RUN(test_load_section_data_relocation_serialization);
    RUN(test_load_section_data_rela_sh_link_and_info);

    std::cout << "\n\033[1m[14] UnloadLinkable\033[0m\n";
    RUN(test_write_shstrtab_section_sets_correct_e_shstrndx);
    RUN(test_unload_linkable_symbols_and_sections_roundtrip);
    RUN(test_unload_linkable_literal_pool_roundtrip);
    RUN(test_unload_linkable_section_relocation_values_correct);
    RUN(test_unload_linkable_section_relocations_do_not_leak_across_sections);
    RUN(test_unload_linkable_pool_relocations_do_not_leak_across_sections);

    std::cout << "\n\033[1m[15] End-to-end roundtrip\033[0m\n";
    RUN(test_e2e_loadlinkable_unloadlinkable_roundtrip);

    int total = g_passed + g_failed;
    std::cout << "\n" << std::string(50, '=') << "\n";
    if (g_failed == 0)
        std::cout << "\033[32m\033[1m";
    else
        std::cout << "\033[31m\033[1m";
    std::cout << "  " << g_passed << " passed  |  "
              << g_failed << " failed  /  " << total << " total"
              << "\033[0m\n\n";

    return g_failed == 0 ? 0 : 1;
}
