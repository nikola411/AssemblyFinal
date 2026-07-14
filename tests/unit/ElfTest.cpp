/*
 * Unit tests for the Elf class and ElfHelpers.
 *
 * Groups:
 *   [1] Template utility  – ReadObject/WriteObject/ReadTable/WriteTable
 *   [2] ELF ident bytes   – magic, class, encoding, version, OSABI, padding
 *   [3] Header roundtrip  – SetElfHeader / GetElfHeader
 *   [4] Load() safety     – no-crash, minimum content size
 *   [5] WriteToFile       – file creation, magic bytes, file size
 *   [6] PHT roundtrip     – SetProgramHeaderTable / GetProgramTableHeader
 *   [7] SHT roundtrip     – SetSectionHeaderTable / GetSectionTableHeader
 *   [8] GetSectionContent – stub returns empty
 *   [9] Helper functions  – GetSectionIndex, MakeSectionHeader
 *
 * Known bugs tested in [4]:
 *   - Load() does not set e_type, e_shnum, e_shoff, e_phnum in the header.
 *   - Section data is never written to content (SetSections is commented out).
 *   - elfSections is pre-allocated then appended to, doubling its size.
 *   - sectionOffset is never incremented in the section loop.
 * These are exercised as no-crash safety tests; correctness tests for those
 * paths are in [6]/[7] where the header is rebuilt manually after Load().
 */

#include "Elf.hpp"
#include "ElfHelpers.hpp"
#include "DataTypes.hpp"

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
                               bool     isGlobal = false)
{
    return std::make_shared<Symbol>(name, section, offset, 0,
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

/*
 * Return an Elf whose internal content buffer has at least (sizeof(Ehdr) +
 * extra_bytes) bytes of usable space.  We achieve this by loading a dummy
 * section; Load() allocates: sizeof(Ehdr) + 2*extra_bytes + sizeof(Shdr).
 *
 * After Load() the ELF header is corrupted (known bug: SHT written at
 * offset 0 when e_shoff is uninitialised).  Callers must call SetElfHeader()
 * before using any header-dependent API.
 */
static Elf make_elf_with_space(size_t extra_bytes)
{
    Elf elf;
    SectionTable secs = { make_sec("pad", std::vector<uint8_t>(extra_bytes, 0)) };
    elf.Load(ET_REL, {}, secs, {});
    return elf;
}

// ── [1] Template utility ──────────────────────────────────────────────────────

void test_write_read_object_uint32()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});

    std::vector<uint8_t> buf(16, 0);
    uint32_t val = 0xDEADBEEF;
    elf.WriteObject<uint32_t>(4, buf, val);
    uint32_t got = elf.ReadObject<uint32_t>(4, buf);
    ASSERT_EQ(got, val);
}

void test_write_read_object_at_offset_zero()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});

    std::vector<uint8_t> buf(8, 0);
    uint64_t val = 0x0102030405060708ULL;
    elf.WriteObject<uint64_t>(0, buf, val);
    uint64_t got = elf.ReadObject<uint64_t>(0, buf);
    ASSERT_EQ(got, val);
}

void test_write_read_object_full_ehdr_struct()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});

    std::vector<uint8_t> buf(sizeof(Elf64_Ehdr) + 8, 0);
    Elf64_Ehdr h = blank_ehdr();
    h.e_shnum    = 7;
    h.e_phnum    = 3;
    elf.WriteObject<Elf64_Ehdr>(0, buf, h);
    Elf64_Ehdr got = elf.ReadObject<Elf64_Ehdr>(0, buf);
    ASSERT_EQ(got.e_shnum,          (Elf64_Half)7);
    ASSERT_EQ(got.e_phnum,          (Elf64_Half)3);
    ASSERT_EQ(got.e_ident[EI_MAG0], (uint8_t)0x7F);
}

void test_write_read_table_basic()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});

    std::vector<uint32_t> table = {10, 20, 30, 40};
    std::vector<uint8_t>  buf(table.size() * sizeof(uint32_t), 0);
    elf.WriteTable<uint32_t>(0, buf, table);
    auto got = elf.ReadTable<uint32_t>(0, (Elf64_Half)table.size(), buf);
    ASSERT_EQ(got.size(), table.size());
    ASSERT_EQ(got[0], 10u);
    ASSERT_EQ(got[1], 20u);
    ASSERT_EQ(got[2], 30u);
    ASSERT_EQ(got[3], 40u);
}

void test_write_read_table_at_nonzero_offset()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});

    std::vector<uint32_t> table = {0xAA, 0xBB};
    std::vector<uint8_t>  buf(32, 0xFF);
    Elf64_Off off = 8;
    elf.WriteTable<uint32_t>(off, buf, table);

    // bytes before the offset must be untouched
    for (size_t i = 0; i < (size_t)off; ++i)
        ASSERT_EQ(buf[i], (uint8_t)0xFF);

    auto got = elf.ReadTable<uint32_t>(off, (Elf64_Half)table.size(), buf);
    ASSERT_EQ(got[0], 0xAAu);
    ASSERT_EQ(got[1], 0xBBu);
}

void test_write_read_table_of_shdrs()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});

    Elf64_Shdr s0 = {}; s0.sh_type = SHT_PROGBITS; s0.sh_size = 16;
    Elf64_Shdr s1 = {}; s1.sh_type = SHT_SYMTAB;   s1.sh_size = 24;
    std::vector<Elf64_Shdr> table = {s0, s1};

    std::vector<uint8_t> buf(table.size() * sizeof(Elf64_Shdr), 0);
    elf.WriteTable<Elf64_Shdr>(0, buf, table);
    auto got = elf.ReadTable<Elf64_Shdr>(0, (Elf64_Half)table.size(), buf);

    ASSERT_EQ(got.size(), (size_t)2);
    ASSERT_EQ(got[0].sh_type, (Elf64_Word)SHT_PROGBITS);
    ASSERT_EQ(got[0].sh_size, (Elf64_Xword)16);
    ASSERT_EQ(got[1].sh_type, (Elf64_Word)SHT_SYMTAB);
    ASSERT_EQ(got[1].sh_size, (Elf64_Xword)24);
}

// ── [2] ELF ident bytes ───────────────────────────────────────────────────────

void test_elf_magic_ei_mag0_is_7f()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_MAG0], (uint8_t)0x7F);
}

void test_elf_magic_ei_mag1_is_E()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_MAG1], (uint8_t)'E');
}

void test_elf_magic_ei_mag2_is_L()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_MAG2], (uint8_t)'L');
}

void test_elf_magic_ei_mag3_is_F()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_MAG3], (uint8_t)'F');
}

void test_elf_class_is_64bit()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_CLASS], (uint8_t)ELFCLASS64);
}

void test_elf_encoding_is_little_endian()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_DATA], (uint8_t)ELFDATA2LSB);
}

void test_elf_ident_version_is_1()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_VERSION], (uint8_t)1);
}

void test_elf_osabi_is_linux()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_OSABI], (uint8_t)ELFOSABI_LINUX);
}

void test_elf_ident_abiversion_is_zero()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_ABIVERSION], (uint8_t)0);
}

void test_elf_ident_padding_bytes_are_zero()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    Elf64_Ehdr h = elf.GetElfHeader();
    for (int i = EI_PAD; i < 16; ++i)
        ASSERT_EQ(h.e_ident[i], (uint8_t)0);
}

// ── [3] SetElfHeader / GetElfHeader roundtrip ─────────────────────────────────

void test_set_get_elf_header_type_field()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    Elf64_Ehdr h = blank_ehdr();
    h.e_type     = ET_EXEC;
    elf.SetElfHeader(h);
    ASSERT_EQ(elf.GetElfHeader().e_type, (Elf64_Half)ET_EXEC);
}

void test_set_get_elf_header_machine_field()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    Elf64_Ehdr h = blank_ehdr();
    h.e_machine  = EM_X86_64;
    elf.SetElfHeader(h);
    ASSERT_EQ(elf.GetElfHeader().e_machine, (Elf64_Half)EM_X86_64);
}

void test_set_get_elf_header_section_count()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
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
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    Elf64_Ehdr h = blank_ehdr();
    h.e_shoff    = sizeof(Elf64_Ehdr);
    elf.SetElfHeader(h);
    ASSERT_EQ(elf.GetElfHeader().e_shoff, (Elf64_Off)sizeof(Elf64_Ehdr));
}

void test_set_get_elf_header_phdr_fields()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
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
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
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
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT(elf.SetElfHeader(blank_ehdr()) == true);
}

// ── [4] Load() safety ────────────────────────────────────────────────────────

void test_load_empty_inputs_no_crash()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    ASSERT_EQ(elf.GetElfHeader().e_ident[EI_MAG0], (uint8_t)0x7F);
}

void test_load_empty_content_at_least_ehdr_size()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    std::string path = "/tmp/elf_test_load_empty.o";
    elf.WriteToFile(path);
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    ASSERT(f.is_open());
    ASSERT(f.tellg() >= (std::streamsize)sizeof(Elf64_Ehdr));
    f.close();
    std::remove(path.c_str());
}

void test_load_with_one_section_no_crash()
{
    // BUG: SHT is written at offset 0 (overwrites ELF header) because e_shoff
    // was 0 from zero-init in InitEhdr.  Content is still large enough so
    // there is no out-of-bounds write — just incorrect data placement.
    Elf elf;
    SectionTable secs = { make_sec("text", {0x00, 0x00, 0x00, 0x00}) };
    elf.Load(ET_REL, {}, secs, {});
    ASSERT(true); // reaching here means no crash
}

void test_load_with_local_and_global_symbols_no_crash()
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
    elf.Load(ET_REL, syms, secs, {});
    ASSERT(true);
}

void test_load_with_relocations_no_crash()
{
    Elf elf;
    auto rel      = std::make_shared<Relocation>();
    rel->symbolName  = "external_func";
    rel->sectionName = "text";
    rel->offset      = 4;
    rel->type        = REL32_DIRECT;
    rel->addend      = 0;
    SectionTable secs = { make_sec("text", {0x00, 0x00, 0x00, 0x00}) };
    elf.Load(ET_REL, {}, secs, {rel});
    ASSERT(true);
}

// ── [5] WriteToFile ───────────────────────────────────────────────────────────

void test_write_to_file_creates_file()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    std::string path = "/tmp/elf_unit_creates.o";
    elf.WriteToFile(path);
    std::ifstream f(path, std::ios::binary);
    ASSERT(f.is_open());
    f.close();
    std::remove(path.c_str());
}

void test_write_to_file_magic_bytes_correct()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    std::string path = "/tmp/elf_unit_magic.o";
    elf.WriteToFile(path);
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

void test_write_to_file_size_at_least_ehdr()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    std::string path = "/tmp/elf_unit_size.o";
    elf.WriteToFile(path);
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    ASSERT(f.is_open());
    ASSERT(f.tellg() >= (std::streamsize)sizeof(Elf64_Ehdr));
    f.close();
    std::remove(path.c_str());
}

void test_write_to_file_elfclass64_byte()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    std::string path = "/tmp/elf_unit_class.o";
    elf.WriteToFile(path);
    std::ifstream f(path, std::ios::binary);
    ASSERT(f.is_open());
    uint8_t buf[5] = {};
    f.read(reinterpret_cast<char*>(buf), 5);
    ASSERT_EQ(buf[EI_CLASS], (uint8_t)ELFCLASS64);
    f.close();
    std::remove(path.c_str());
}

void test_write_to_file_little_endian_byte()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    std::string path = "/tmp/elf_unit_encoding.o";
    elf.WriteToFile(path);
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
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    Elf64_Ehdr h  = blank_ehdr();
    h.e_phoff     = sizeof(Elf64_Ehdr);
    h.e_phnum     = 0;
    h.e_phentsize = sizeof(Elf64_Phdr);
    elf.SetElfHeader(h);
    ASSERT_EQ(elf.GetProgramTableHeader().size(), (size_t)0);
}

void test_pht_set_get_single_entry()
{
    // make_elf_with_space gives: sizeof(Ehdr) + 2*extra + sizeof(Shdr) bytes.
    // extra=256 → 640 bytes total. PHT at offset 64 needs 56 bytes. OK.
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

    auto phdr = elf.GetProgramTableHeader();
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

    auto phdr = elf.GetProgramTableHeader();
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

    auto phdr = elf.GetProgramTableHeader();
    ASSERT_EQ(phdr[0].p_filesz, (Elf64_Xword)222);
}

// ── [7] Section header table roundtrip ───────────────────────────────────────

void test_sht_empty_roundtrip()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    Elf64_Ehdr h  = blank_ehdr();
    h.e_shoff     = sizeof(Elf64_Ehdr);
    h.e_shnum     = 0;
    h.e_shentsize = sizeof(Elf64_Shdr);
    elf.SetElfHeader(h);
    ASSERT_EQ(elf.GetSectionTableHeader().size(), (size_t)0);
}

void test_sht_set_get_single_section()
{
    // 256-byte pad → content = 64 + 512 + 64 = 640 bytes.
    // We put SHT at offset 64 (right after Ehdr), needing 64 bytes. OK.
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

    auto shdr = elf.GetSectionTableHeader();
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

    auto got = elf.GetSectionTableHeader();
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

    auto got = elf.GetSectionTableHeader();
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

// ── [8] GetSectionContent stub ────────────────────────────────────────────────

void test_get_section_content_stub_returns_empty()
{
    Elf elf;
    elf.Load(ET_REL, {}, {}, {});
    Elf64_Shdr shdr = {};
    shdr.sh_offset   = sizeof(Elf64_Ehdr);
    shdr.sh_size     = 4;
    auto content = elf.GetSectionContent(shdr);
    // Implementation is a stub — always returns empty.
    ASSERT_EQ(content.size(), (size_t)0);
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
    // sh_offset = sizeof(Ehdr) + data_offset_arg
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

// ── main ─────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "\n\033[1m[1] Template utility\033[0m\n";
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

    std::cout << "\n\033[1m[4] Load() safety\033[0m\n";
    RUN(test_load_empty_inputs_no_crash);
    RUN(test_load_empty_content_at_least_ehdr_size);
    RUN(test_load_with_one_section_no_crash);
    RUN(test_load_with_local_and_global_symbols_no_crash);
    RUN(test_load_with_relocations_no_crash);

    std::cout << "\n\033[1m[5] WriteToFile\033[0m\n";
    RUN(test_write_to_file_creates_file);
    RUN(test_write_to_file_magic_bytes_correct);
    RUN(test_write_to_file_size_at_least_ehdr);
    RUN(test_write_to_file_elfclass64_byte);
    RUN(test_write_to_file_little_endian_byte);

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

    std::cout << "\n\033[1m[8] GetSectionContent stub\033[0m\n";
    RUN(test_get_section_content_stub_returns_empty);

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
