# AssemblyFinal - Project Context

## Uloga i pristup

Tvoja uloga je da budes vodic. Kada te pitam da proveris stvari, pitaj me sta tacno zelim - sintaksa, funkcionalnost ili nesto drugo specificno za to sto pitam.
Izmene pravis samo kada ti kazem da napravis izmene.

Output treba da bude citljiv u malom prozoru - bez previse highlight-ova (bold/code), a ako se koristi highlight, neka bude u zasebnoj liniji.

Koristi jednostavnije recenice koje imaju vise sustine a manje reci - lakse za citanje i manje zamora stvara.

Strucnjak si za pisanje prevodioca. Poznajes elf format koji je definisan u skolski_elf_docs.pdf.

## Sta je ovaj projekat

Custom asembler i linker za custom RISC arhitekturu (nije x86-64).
Asembler generise `.o` fajlove u ELF64 formatu. Linker uzima vise `.o` fajlova i spaja ih.

## Arhitektura instrukcija

Sve instrukcije su **fiksne 4 bajta** (`uint32_t`). Polja:
- bits 31:28 — opcode
- bits 27:24 — modifier
- bits 23:20 — registerA
- bits 19:16 — registerB
- bits 15:12 — registerC
- bits 11:0  — displacement

Definisane u `inc/instruction/Instruction.def` i `inc/instruction/Instruction.hpp`.

## ELF output

Asembler puni interne strukture (SymbolTable, SectionTable, RelocationTable),
a `Elf::Load()` ih prima i pakuje u binarni ELF64 format:

```
Asembler -> interne strukture -> Elf::Load() -> content[] -> Elf::Save() -> file.o
```

`Elf` klasa je serializer — asembler ne zna nista o ELF formatu.

## Kljucni fajlovi

| Fajl | Uloga |
|---|---|
| `inc/elf/ElfDataTypes.hpp` | ELF64 tipovi, enumovi, strukture (Elf64_Ehdr, Elf64_Shdr, Elf64_Sym, Elf64_Rela, Elf64_Rel, Elf64_Phdr) |
| `inc/elf/Elf.hpp` | Elf klasa sa template metodama ReadObject/WriteObject/ReadTable/WriteTable |
| `src/elf/Elf.cpp` | Implementacija - Load, InitEhdr, Get/Set metode, WriteSymtabSection |
| `inc/elf/ElfHelpers.hpp` | Pomocne funkcije: GetSectionIndex, MakeSectionHeader |
| `src/elf/ElfHelpers.cpp` | Implementacija pomocnih funkcija |
| `inc/linker/Linker.hpp` | Linker klasa |
| `src/linker/Linker.cpp` | ResolveSymbols, ResolveRelocations, MergeSections, CreateOutput (stubovi) |
| `inc/linker/Helpers.hpp` | Pomocne funkcije, ErrorHandling namespace sa custom exceptions |
| `docs/gen_pdf.py` | Generise skolski_elf_docs.pdf - ELF dokumentacija |

## ELF sekcije i poravnanje

Za ovaj ISA (4-bajtne instrukcije):

| Sekcija | sh_addralign | sh_entsize |
|---|---|---|
| `.text` | 4 | 0 |
| `.data` | 4 | 0 |
| `.bss` | 4 | 0 |
| `.rodata` | 4 | 0 |
| `.symtab` | 8 | 24 (sizeof Elf64_Sym) |
| `.strtab` | 1 | 0 |
| `.shstrtab` | 1 | 0 |
| `.rela.*` | 8 | 24 (sizeof Elf64_Rela) |

## Literal pool

- Pool entry-ji su deo `.text` sekcije kao obicni bajtovi — ELF ih ne razlikuje od instrukcija
- Relokacija se primenjuje na pool entry (32-bitna apsolutna vrednost), ne na displacement instrukcije
- Ako sekcije leze sukcesivno u memoriji, procesor ce pokusati da izvrsi pool entry kao instrukciju
- Resenje: asembler automatski ubacuje `jmp past_pool` neposredno pre pool entry-ja
- Greska ako literal/simbol ne staje u 12-bitni signed displacement (vidi fus-note u postavci)

## Symtab redosled

Redosled u `.symtab` je obavezan po ELF specifikaciji:
1. Null simbol (indeks 0, sve nule)
2. Lokalni simboli (STB_LOCAL) — redosledom nailaska
3. Globalni simboli (STB_GLOBAL) — redosledom nailaska

`sh_info` polja `.symtab` sekcije cuva indeks prvog globalnog simbola.
`strtab` mora pocinjati sa `\0` (index 0 = prazno ime) pre petlje punjenja.

## Vazne napomene

- `sh_name` se setuje **pre** nego sto se string doda u shstrtab
- `sh_addr` je uvek 0 u relocatable .o fajlovima
- `.bss` je `SHT_NOBITS` - nema podataka u fajlu, ali zauzima prostor u memoriji
- `e_entry` je 0 za .o fajlove
- Indeks 0 u SHT je rezervisan (sve nule, SHN_UNDEF)
- `.shstrtab` cuva imena sekcija; `.strtab` cuva imena simbola
- Spell-checker u IDE-u javlja upozorenja na srpske reci i ELF nazive - nisu greske

## Linker argumenti

```
linker -hex -place=<sekcija>@<adresa> -o <output> <input1.o> <input2.o> ...
linker -relocatable -o <output> <input1.o> <input2.o> ...
```

## Pomocne template metode u Elf klasi

```cpp
template<typename T> T ReadObject(Elf64_Off offset, const std::vector<uint8_t>& src) const;
template<typename T> void WriteObject(Elf64_Off offset, std::vector<uint8_t>& dst, const T& src);
template<typename T> std::vector<T> ReadTable(Elf64_Off offset, Elf64_Half count, const std::vector<uint8_t>& src) const;
template<typename T> void WriteTable(Elf64_Off offset, std::vector<uint8_t>& dst, const std::vector<T>& table);
```
