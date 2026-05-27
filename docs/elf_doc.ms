.TL
ELF \(em Executable and Linkable Format
.AU
Dokumentacija
.AB no
.AE
.NH 1
Pregled
.PP
ELF je standardni binarni format objektnih programa na UNIX-like sistemima.
Postoje tri tipa ELF datoteka:
.IP \(bu
\fBrelocatable\fR \(em masinski kod i podaci namenjeni povezivanju sa drugim objektnim programima (.o fajlovi)
.IP \(bu
\fBexecutable\fR \(em program u obliku pogodnom za izvrsavanje
.IP \(bu
\fBshared object\fR \(em kod i podaci za staticko i dinamicko povezivanje (.so fajlovi)

.NH 1
Struktura ELF datoteke
.PP
Svaka ELF datoteka pocinje obaveznim \fBELF headerom\fR, nakon cega mogu slediti:
.DS L
+-----------------------+
|      ELF Header       |  (obavezno)
+-----------------------+
| Program Header Table  |  (linking: opciono, execution: obavezno)
+-----------------------+
|      Sections ...     |
+-----------------------+
| Section Header Table  |  (linking: obavezno, execution: opciono)
+-----------------------+
.DE
.PP
\fBLinking view\fR \(em koristi se pri povezivanju; opisuje sekcije.
.br
\fBExecution view\fR \(em koristi se pri izvrsavanju; opisuje segmente.

.NH 1
Primitivni tipovi (ELF64)
.TS
allbox tab(|);
lb lb lb lb
l l l l.
Tip|Velicina|Poravnanje|Znacenje
Elf64_Addr|8|8|Unsigned program address
Elf64_Off|8|8|Unsigned file offset
Elf64_Half|2|2|Unsigned medium integer
Elf64_Word|4|4|Unsigned integer
Elf64_Sword|4|4|Signed integer
Elf64_Xword|8|8|Unsigned long integer
Elf64_Sxword|8|8|Signed long integer
unsigned char|1|1|Unsigned small integer
.TE

.NH 1
ELF Header \(em Elf64_Ehdr
.DS L
typedef struct {
    unsigned char  e_ident[16]; // identifikacija datoteke
    Elf64_Half     e_type;      // tip ELF datoteke
    Elf64_Half     e_machine;   // ciljna arhitektura
    Elf64_Word     e_version;   // verzija formata
    Elf64_Addr     e_entry;     // virtuelna adresa ulazne tacke
    Elf64_Off      e_phoff;     // offset program header table
    Elf64_Off      e_shoff;     // offset section header table
    Elf64_Word     e_flags;     // zastavice specificne za procesor
    Elf64_Half     e_ehsize;    // velicina ELF headera u bajtovima
    Elf64_Half     e_phentsize; // velicina jednog unosa u PHT
    Elf64_Half     e_phnum;     // broj unosa u PHT
    Elf64_Half     e_shentsize; // velicina jednog unosa u SHT
    Elf64_Half     e_shnum;     // broj unosa u SHT
    Elf64_Half     e_shstrndx;  // indeks unosa za .shstrtab
} Elf64_Ehdr;
.DE

.NH 2
e_ident[]
.PP
Niz od 16 bajtova na pocetku svake ELF datoteke:
.TS
allbox tab(|);
lb lb lb
l l l.
Indeks|Konstanta|Vrednost / opis
0\(en3|EI_MAG0\(enEI_MAG3|0x7f 'E' 'L' 'F' \(em magic number
4|EI_CLASS|ELFCLASS32=1, ELFCLASS64=2
5|EI_DATA|ELFDATA2LSB=1 (little-endian), ELFDATA2MSB=2
6|EI_VERSION|EV_CURRENT=1
7|EI_OSABI|ELFOSABI_NONE=0, ELFOSABI_GNU=3, ...
8|EI_ABIVERSION|Verzija ABI-ja
9\(en15|EI_PAD|Padding, mora biti 0
.TE

.NH 2
e_type
.TS
allbox tab(|);
lb lb lb
l l l.
Konstanta|Vrednost|Opis
ET_REL|1|Relocatable file
ET_EXEC|2|Executable file
ET_DYN|3|Shared object file
ET_CORE|4|Core file
.TE

.NH 2
e_machine (izbor)
.TS
allbox tab(|);
lb lb
l l.
Konstanta|Vrednost
EM_386|3
EM_ARM|40
EM_X86_64|62
EM_RISCV|243
.TE

.NH 2
Ostala polja
.IP \(bu
\fBe_entry\fR \(em virtuelna adresa ulazne tacke; 0 ako ne postoji
.IP \(bu
\fBe_phoff / e_shoff\fR \(em offset od pocetka datoteke do PHT/SHT; 0 ako tabela ne postoji
.IP \(bu
\fBe_shstrndx\fR \(em indeks unosa u SHT koji opisuje sekciju sa imenima sekcija (.shstrtab)

.NH 1
Section Header Table \(em Elf64_Shdr
.PP
Section header table je niz struktura Elf64_Shdr; svaki unos opisuje jednu sekciju.
Lokacija tabele: e_shoff. Broj unosa: e_shnum. Velicina unosa: e_shentsize.
Unos sa indeksom 0 je rezervisan (sve nule).
.DS L
typedef struct {
    Elf64_Word  sh_name;      // offset u .shstrtab (ime sekcije)
    Elf64_Word  sh_type;      // tip sekcije
    Elf64_Xword sh_flags;     // jednobiti atributi
    Elf64_Addr  sh_addr;      // virt. adresa u memoriji (0 ako se ne ucitava)
    Elf64_Off   sh_offset;    // offset od pocetka datoteke do sekcije
    Elf64_Xword sh_size;      // velicina sekcije u bajtovima
    Elf64_Word  sh_link;      // indeks povezane sekcije (zavisi od tipa)
    Elf64_Word  sh_info;      // dodatna informacija (zavisi od tipa)
    Elf64_Xword sh_addralign; // poravnanje (0 ili 1 = bez zahteva)
    Elf64_Xword sh_entsize;   // velicina unosa ako sekcija sadrzi tabelu
} Elf64_Shdr;
.DE

.NH 2
sh_type
.TS
allbox tab(|);
lb lb lb
l l l.
Konstanta|Vrednost|Opis
SHT_NULL|0|Neaktivan unos
SHT_PROGBITS|1|Podaci programa
SHT_SYMTAB|2|Tabela simbola
SHT_STRTAB|3|Tabela stringova
SHT_RELA|4|Relokacioni zapisi sa addend poljem
SHT_NOBITS|8|Sekcija bez podataka u datoteci (.bss)
SHT_REL|9|Relokacioni zapisi bez addend polja
.TE

.NH 2
sh_flags
.TS
allbox tab(|);
lb lb
l l.
Konstanta|Opis
SHF_WRITE|Sekcija je upisiva
SHF_ALLOC|Sekcija zauzima memoriju pri izvrsavanju
SHF_EXECINSTR|Sekcija sadrzi izvrsne instrukcije
.TE

.NH 2
sh_link i sh_info po tipu
.TS
allbox tab(|);
lb lb lb
l l l.
sh_type|sh_link|sh_info
SHT_RELA / SHT_REL|Indeks tabele simbola|Indeks sekcije na koju se primenjuju relokacije
SHT_SYMTAB / SHT_DYNSYM|Indeks tabele stringova|Indeks poslednjeg lokalnog simbola + 1
ostalo|SHN_UNDEF|0
.TE

.NH 2
Rezervisani indeksi sekcija
.TS
allbox tab(|);
lb lb lb
l l l.
Konstanta|Vrednost|Opis
SHN_UNDEF|0|Nedefinisana sekcija
SHN_ABS|0xfff1|Simbol ima apsolutnu vrednost
SHN_COMMON|0xfff2|Simbol je common data object
.TE

.NH 1
Specijalne sekcije (izbor)
.TS
allbox tab(|);
lb lb lb
l l l.
Naziv|Tip|Atributi
.text|SHT_PROGBITS|SHF_ALLOC+SHF_EXECINSTR
.data|SHT_PROGBITS|SHF_ALLOC+SHF_WRITE
.bss|SHT_NOBITS|SHF_ALLOC+SHF_WRITE
.rodata|SHT_PROGBITS|SHF_ALLOC
.symtab|SHT_SYMTAB|\(em
.strtab|SHT_STRTAB|\(em
.shstrtab|SHT_STRTAB|\(em
.rela<name>|SHT_RELA|\(em
.rel<name>|SHT_REL|\(em
.dynamic|SHT_DYNAMIC|\(em
.TE

.NH 1
Symbol Table \(em Elf64_Sym
.PP
Symbol table je sekcija tipa SHT_SYMTAB; svaki unos opisuje jedan simbol.
ELF datoteka moze imati samo jednu .symtab sekciju.
.DS L
typedef struct {
    Elf64_Word    st_name;  // offset u tabeli stringova (sh_link)
    unsigned char st_info;  // binding (bits 7:4) + tip (bits 3:0)
    unsigned char st_other; // vidljivost simbola
    Elf64_Half    st_shndx; // indeks sekcije kojoj simbol pripada
    Elf64_Addr    st_value; // vrednost simbola
    Elf64_Xword   st_size;  // velicina (0 ako nije poznata)
} Elf64_Sym;
.DE
.DS L
#define ELF64_ST_BIND(val)       (((unsigned char)(val)) >> 4)
#define ELF64_ST_TYPE(val)       ((val) & 0xf)
#define ELF64_ST_INFO(bind,type) (((bind) << 4) + ((type) & 0xf))
.DE

.NH 2
Binding (st_info, gornja 4 bita)
.TS
allbox tab(|);
lb lb lb
l l l.
Konstanta|Vrednost|Opis
STB_LOCAL|0|Lokalni simbol, nije vidljiv van datoteke
STB_GLOBAL|1|Globalni simbol, vidljiv linkeru
STB_WEAK|2|Slabiji prioritet od globalnog
.TE

.NH 2
Tip simbola (st_info, donja 4 bita)
.TS
allbox tab(|);
lb lb lb
l l l.
Konstanta|Vrednost|Opis
STT_NOTYPE|0|Nespecifikovan tip
STT_OBJECT|1|Podatak
STT_FUNC|2|Funkcija
STT_SECTION|3|Sekcija
.TE

.NH 2
Vidljivost (st_other)
.TS
allbox tab(|);
lb lb lb
l l l.
Konstanta|Vrednost|Opis
STV_DEFAULT|0|Podrazumevana vidljivost
STV_HIDDEN|2|Simbol nije vidljiv izvan modula
STV_PROTECTED|3|Nije preemptibilan, nije exportovan
.TE

.NH 2
st_value
.IP \(bu
U \fBrelocatable\fR datotekama: offset od pocetka sekcije do simbola
.IP \(bu
U \fBexecutable/shared object\fR datotekama: virtuelna adresa simbola

.NH 1
Relocation Table \(em Elf64_Rela / Elf64_Rel
.PP
Relocation table je sekcija tipa SHT_RELA ili SHT_REL; opisuje mesta u kodu koja treba ispraviti pri povezivanju.
ELF datoteka moze imati vise relokacionih sekcija.
.DS L
typedef struct {
    Elf64_Addr   r_offset;  // lokacija koja se ispravlja
    Elf64_Xword  r_info;    // simbol + tip relokacije
    Elf64_Sxword r_addend;  // eksplicitna addend vrednost
} Elf64_Rela;  // SHT_RELA

typedef struct {
    Elf64_Addr  r_offset;
    Elf64_Xword r_info;  // addend se uzima iz masinskg koda
} Elf64_Rel;   // SHT_REL
.DE
.DS L
#define ELF64_R_SYM(i)           ((i) >> 32)
#define ELF64_R_TYPE(i)          ((i) & 0xffffffff)
#define ELF64_R_INFO(sym, type)  ((((Elf64_Xword)(sym)) << 32) + (type))
.DE

.NH 2
r_offset
.IP \(bu
U \fBrelocatable\fR datotekama: offset od pocetka sekcije do mesta koje se ispravlja
.IP \(bu
U \fBexecutable/shared object\fR datotekama: virtuelna adresa mesta koje se ispravlja

.NH 2
r_info
.PP
Gornja 32 bita \(em indeks simbola u tabeli simbola (sh_link).
.br
Donja 32 bita \(em tip relokacije (specificno za arhitekturu).

.NH 2
Tipovi relokacija za x86-64 (izbor)
.TS
allbox tab(|);
lb lb lb
l l l.
Konstanta|Vrednost|Opis
R_X86_64_PC32|2|PC-relative 32-bit signed
R_X86_64_PLT32|4|32-bit PLT address
R_X86_64_32S|11|Direct 32-bit sign extended
.TE

.NH 1
Korisni alati
.DS L
readelf -h file.o       # ELF header
readelf -S file.o       # section header table
readelf -s file.o       # symbol table
readelf -r file.o       # relocation table
objdump -d file.o       # disassembly
hd -v file.o            # hex dump
.DE
