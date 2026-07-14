# TODO

## Elf::LoadSectionData (src/elf/Elf.cpp) — bagovi

- [ ] **Linija 272** `SHT_LITPOOL` ne postoji u `Elf64_Shdr_Type` (`ElfDataTypes.hpp:89-97`) — ne kompajlira se, treba dodati custom vrednost za tip sekcije literal pool-a
- [ ] **Linije 301, 322** `sectionRelocationData`/`poolRelocationData` su `std::vector<uint8_t>`, ali `sectionRelocations`/`poolRelocations` su `RelocationTable` (`std::vector<Relocation::s_ptr>`) — `insert` ne kompajlira se (cannot convert shared_ptr to uint8_t). Nedostaje korak serijalizacije `Relocation` → `Elf64_Rela` pre upisa u bafer
- [ ] **Linije 295-296, 316-317** `sh_size`/`sh_entsize` za RELA sekcije računati kao `sizeof(Elf64_Rela) * broj_relokacija` / `sizeof(Elf64_Rela)`, ne `sizeof(uint8_t) * ...` (ovo je posledica prethodnog bug-a, ali treba popraviti i posle serijalizacije)
- [ ] **Linije 298, 319** `sh_link = i + 2` postavlja indeks ciljne sekcije, ali `Elf::UnloadLinkable` (linija 149) čita ciljnu sekciju iz `sh_info`, ne `sh_link`. Standardna ELF konvencija: `sh_link` = indeks symtab-a, `sh_info` = indeks ciljne sekcije za RELA. Treba zameniti mesta i postaviti `sh_link` na pravi symtab indeks
- [ ] `shstrtOffset` se dosledno uvećava (linije 257, 278, 303, 324) za imena sekcija/pool/rela, ali nigde u funkciji (ni u pozivaocu `LoadLinkable`, koji je i sam nedovršen stub) se stringovi imena stvarno ne upisuju u shstrtab bafer — `sh_name` će pokazivati na offsete gde ništa nije upisano
- [ ] `i + 2` u `sh_link` računu pretpostavlja da `shdrt` već ima 2 unosa (`.symtab` + `.strtab`) pre sekcija, ali `LoadSymbolTable` ubacuje samo JEDAN shdr (za `.symtab`, linija 181) iako upisuje i symtab i strtab podatke — nema posebnog shdr za `.strtab`. Treba ili dodati taj shdr, ili uskladiti offset račun
