#ifndef ELF_DATA_TYPES_HPP
#define ELF_DATA_TYPES_HPP

#include <inttypes.h>

// Primitivni tipovi iz ELF64 specifikacije.
// Poravnanje je uvek jednako velicini tipa.
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

// Indeksi u e_ident nizu na pocetku ELF headera.
// e_ident sadrzi informacije nezavisne od arhitekture (magic, class, endianness, ABI).
enum Elf64_IdentIndex : uint8_t {
    EI_MAG0       = 0,
    EI_MAG1       = 1,
    EI_MAG2       = 2,
    EI_MAG3       = 3,
    EI_CLASS      = 4,
    EI_DATA       = 5,
    EI_VERSION    = 6,
    EI_OSABI      = 7,
    EI_ABIVERSION = 8,
    EI_PAD        = 9,
};

// Tip ELF datoteke. Cuva se u e_type polju ELF headera.
// Odredjuje da li je datoteka objektni fajl, izvrsni program ili deljena biblioteka.
enum Elf64_EType : Elf64_Half {
    ET_REL  = 1,
    ET_EXEC = 2,
    ET_DYN  = 3,
    ET_CORE = 4,
};

// Ciljna arhitektura procesora. Cuva se u e_machine polju ELF headera.
enum Elf64_EMachine : Elf64_Half {
    EM_386    =   3,
    EM_ARM    =  40,
    EM_X86_64 =  62,
    EM_AARCH64 = 183,
    EM_RISCV  = 243,
};

// Vrednosti za e_ident[EI_CLASS]: oznacava da li je ELF 32-bit ili 64-bit.
enum Elf64_Class : uint8_t {
    ELFCLASS32 = 1,
    ELFCLASS64 = 2,
};

// Vrednosti za e_ident[EI_DATA]: poredak bajtova u datoteci.
enum Elf64_DataEncoding : uint8_t {
    ELFDATA2LSB = 1,
    ELFDATA2MSB = 2,
};

// Vrednosti za e_ident[EI_OSABI]: ciljni operativni sistem / ABI konvencija.
enum Elf64_OSABI : uint8_t {
    ELFOSABI_NONE  = 0,
    ELFOSABI_LINUX = 3,
};

// ELF header - uvek se nalazi na offsetu 0, velicine 64 bajta.
// Opisuje celu datoteku i sadrzi offsete do PHT i SHT.
typedef struct {
    unsigned char e_ident[16];
    Elf64_Half    e_type;
    Elf64_Half    e_machine;
    Elf64_Word    e_version;
    Elf64_Addr    e_entry;
    Elf64_Off     e_phoff;
    Elf64_Off     e_shoff;
    Elf64_Word    e_flags;
    Elf64_Half    e_ehsize;
    Elf64_Half    e_phentsize;
    Elf64_Half    e_phnum;
    Elf64_Half    e_shentsize;
    Elf64_Half    e_shnum;
    Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

// Tip sekcije. Cuva se u sh_type polju Section Header Table unosa.
// Govori linkeru kako da interpretira sadrzaj sekcije.
enum Elf64_Shdr_Type : Elf64_Word {
    SHT_NULL     = 0,
    SHT_PROGBITS = 1,
    SHT_SYMTAB   = 2,
    SHT_STRTAB   = 3,
    SHT_RELA     = 4,
    SHT_DYNAMIC  = 6,
    SHT_NOBITS   = 8,
    SHT_REL      = 9,
    SHT_DYNSYM   = 11,
};

// Atributi sekcije. Cuva se u sh_flags polju; vrednosti se kombinuju bitski (OR).
// Odredjuje da li se sekcija ucitava u memoriju, da li je upisiva i da li je izvrsna.
enum Elf64_Shdr_Flags : Elf64_Xword {
    SHF_WRITE     = 0x1,
    SHF_ALLOC     = 0x2,
    SHF_EXECINSTR = 0x4,
};

// Specijalni indeksi sekcija koji se koriste u st_shndx polju tabele simbola.
// Ne odgovaraju stvarnim unosima u SHT vec oznacavaju specijalne situacije.
enum Elf64_SHN : Elf64_Half {
    SHN_UNDEF  = 0,
    SHN_ABS    = 0xfff1,
    SHN_COMMON = 0xfff2,
};

// Unos u Section Header Table - opisuje jednu sekciju unutar ELF datoteke.
// Lokacija tabele: e_shoff, broj unosa: e_shnum, velicina unosa: e_shentsize.
typedef struct {
    Elf64_Word  sh_name;      // indeks imena sekcije u string tabeli sekcija
    Elf64_Word  sh_type;      // tip sekcije (SHT_*)
    Elf64_Xword sh_flags;     // atributi sekcije (SHF_*)
    Elf64_Addr  sh_addr;      // adresa sekcije u memoriji, ako je učitana
    Elf64_Off   sh_offset;    // offset sekcije u fajlu
    Elf64_Xword sh_size;      // veličina sekcije u bajtovima
    Elf64_Word  sh_link;      // dodatni indeks u zavisnosti od tipa sekcije
    Elf64_Word  sh_info;      // dodatne informacije u zavisnosti od tipa sekcije
    Elf64_Xword sh_addralign; // potreban alignment sekcije
    Elf64_Xword sh_entsize;   // veličina jednog elementa ako je tabela
} Elf64_Shdr;

// Binding simbola - gornja 4 bita st_info polja.
// Odredjuje vidljivost simbola i prioritet pri resoluciji visestrukih definicija.
enum Elf64_Sym_Binding : uint8_t {
    STB_LOCAL  = 0,
    STB_GLOBAL = 1,
    STB_WEAK   = 2,
};

// Tip simbola - donja 4 bita st_info polja.
// Govori linkeru sta simbol predstavlja (funkcija, promenljiva, sekcija...).
enum Elf64_Sym_Type : uint8_t {
    STT_NOTYPE  = 0,
    STT_OBJECT  = 1,
    STT_FUNC    = 2,
    STT_SECTION = 3,
    STT_FILE    = 4,
};

// Vidljivost simbola - donja 2 bita st_other polja.
// Kontrolise da li je simbol dostupan dinamickom linkeru iz drugih biblioteka.
enum Elf64_Sym_Visibility : uint8_t {
    STV_DEFAULT   = 0,
    STV_HIDDEN    = 2,
    STV_PROTECTED = 3,
};

// Unos u tabeli simbola (.symtab / .dynsym).
// Mapira ime simbola na adresu ili vrednost. Linker koristi ovu tabelu
// za razresavanje medjufajlovskih referenci.
typedef struct {
    Elf64_Word    st_name;   // offset imena simbola u .strtab string tabeli (0 = nema ime)
    unsigned char st_info;   // gornja 4 bita = binding (STB_LOCAL/GLOBAL/WEAK), donja 4 = tip (STT_NOTYPE/OBJECT/FUNC/SECTION)
    unsigned char st_other;  // vidljivost simbola u donja 2 bita (STV_DEFAULT/HIDDEN/PROTECTED), ostalo rezervisano
    Elf64_Half    st_shndx;  // indeks sekcije u kojoj je simbol definisan; SHN_UNDEF = nedefinisan, SHN_ABS = apsolutna vrednost
    Elf64_Addr    st_value;  // vrednost simbola: adresa u izvrsnom fajlu, offset unutar sekcije u objektnom fajlu
    Elf64_Xword   st_size;   // velicina simbola u bajtovima (npr. velicina funkcije ili promenljive), 0 ako nije poznata
} Elf64_Sym;

#define ELF64_ST_BIND(val)       (((unsigned char)(val)) >> 4)
#define ELF64_ST_TYPE(val)       ((val) & 0xf)
#define ELF64_ST_INFO(bind,type) (((bind) << 4) + ((type) & 0xf))

// Tipovi relokacija za x86-64 arhitekturu.
// Svaki tip odredjuje formulu kojom linker izracunava patch vrednost
// (S = vrednost simbola, A = addend, P = adresa koja se zakrpljuje).
enum Elf64_Rela_Type_x86_64 : uint32_t {
    R_X86_64_64    =  1,  // S + A          ; apsolutna 64-bitna adresa simbola, upisuje se u 8 bajta
    R_X86_64_PC32  =  2,  // S + A - P      ; PC-relativni 32-bitni offset, koristi se za direktne pozive/skokove
    R_X86_64_PLT32 =  4,  // L + A - P      ; PC-relativni offset do PLT unosa simbola, za pozive dinamickih funkcija
    R_X86_64_32    = 10,  // S + A (zero-extend) ; apsolutna 32-bitna adresa, zero-extend na 64 bita, upisuje se u 4 bajta
    R_X86_64_32S   = 11,  // S + A (sign-extend) ; apsolutna 32-bitna adresa, sign-extend na 64 bita, upisuje se u 4 bajta
};

// Relokacioni zapis sa eksplicitnim addend poljem (sekcije tipa SHT_RELA).
// Linker cita ove zapise i zakrpljuje masinski kod na poziciji r_offset.
typedef struct {
    Elf64_Addr   r_offset;  // offset u sekciji gde linker treba da upisuje patch (byte offset od pocetka sekcije)
    Elf64_Xword  r_info;    // upakovan: gornja 32 bita = indeks simbola u .symtab, donja 32 bita = tip relokacije (R_*)
    Elf64_Sxword r_addend;  // vrednost koja se dodaje na adresu simbola pri izracunavanju patch vrednosti (moze biti negativna)
} Elf64_Rela;

// Tipovi relokacija za custom 32-bit arhitekturu ovog projekta.
// Vrednosti se direktno upisuju u donja 32 bita r_info polja Elf64_Rela zapisa.
enum Elf64_Rela_Type_custom : uint32_t {
    R_CUSTOM_ABS12 = 1,  // S + A (sign-extend); mala vrednost simbola upisuje se direktno u 12-bitni displacement polje instrukcije
    R_CUSTOM_PC12  = 2,  // pool(S) + A - P; PC-relativni 12-bitni offset do unosa u bazenu literala (.pool sekcija)
    R_CUSTOM_ABS32 = 3,  // S + A; velika vrednost simbola koja se upisuje u 32 bita u memoriji (pool)
};

// Relokacioni zapis bez eksplicitnog addend polja (sekcije tipa SHT_REL).
// Addend se uzima direktno iz masinskog koda na poziciji r_offset.
typedef struct {
    Elf64_Addr  r_offset;
    Elf64_Xword r_info;
} Elf64_Rel;

#define ELF64_R_SYM(i)          ((i) >> 32)
#define ELF64_R_TYPE(i)         ((i) & 0xffffffff)
#define ELF64_R_INFO(sym, type) ((((Elf64_Xword)(sym)) << 32) + (type))

typedef struct {
    Elf64_Word  p_type;
    Elf64_Word  p_flags;
    Elf64_Off   p_offset;
    Elf64_Addr  p_vaddr;
    Elf64_Addr  p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

#endif
