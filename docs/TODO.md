# TODO

Status below is cross-checked against `tests/unit/ElfTest.cpp` (rewritten to
match the current API — see groups [10]-[14]). Items marked with a test name
have a red test pinned on them; run `cd tests/unit && make run` to see them.

## `Elf::WriteStrtabSection`, `Elf::WriteShstrtabSection`, `Elf::WriteProgramSections`

All previously tracked bugs here are fixed and covered by passing tests
(groups [10]-[13]): `sh_name` offsets, `sh_link`/`sh_info` for `SHT_STRTAB`
and `SHT_RELA`, `sh_entsize`, the final byte-writing block's running offset,
missing `shdrt.push_back` for literal-pool/RELA shdrs, and relocation
serialization via `ConvertRelocationToRela`.

## `Elf::WriteShstrtabSection` — `e_shstrndx` off by one

- [ ] **`ehdr.e_shstrndx = 3;` (hardcoded).** At the point this runs, `shdrt`
  only has 3 entries (symtab, strtab, shstrtab itself — pushed just above),
  so shstrtab's real index is `2`, not `3`. `UnloadLinkable` reads
  `shdrt[ehdr.e_shstrndx]` to find the section-name string table; with the
  wrong index it reads whatever shdr comes right after shstrtab instead.
  Fix: use `(Elf64_Half)(shdrt.size() - 1)` instead of the literal `3`.
  Test: `test_write_shstrtab_section_sets_correct_e_shstrndx`.

## `Elf::LoadLinkable` (`src/elf/Elf.cpp`, ~line 622) — compiles, but wiring is incomplete

- [ ] **Never persists a complete, final ELF header into `content`.**
  `WriteShstrtabSection` does call `SetElfHeader` internally, but only to
  patch `e_shstrndx` onto whatever header already happens to be in
  `content` at that point — which is all zeros (no magic, no class, no
  `e_shoff`/`e_shnum`), since nothing has written a real header yet.
  `LoadLinkable` itself never calls `SetElfHeader`/`SetSectionHeaderTable`
  with the final `shdrt` (sections + relocations included) and the real
  `e_shoff`/`e_shnum`. A buffer built via `LoadLinkable` is not yet a valid,
  readable ELF file. See `test_loadlinkable_with_relocations_no_crash` for
  the no-crash-only coverage this currently gets, and
  `write_full_linkable()` in `ElfTest.cpp` for what a correct version needs
  to do by hand (including a provisional early SHT so relocation symbol
  lookups succeed before the final one is written).
- [ ] **`shstrtOffset` local variable is reused for two different
  quantities.** It's computed as
  `sizeof(".symtab") + sizeof(".strtab") + sizeof(".shstrt")` and passed as
  `WriteProgramSections`'s 3rd argument (a small running counter into the
  shstrtab name buffer). The literal `".shstrt"` is missing `"ab"` — it
  should be `".shstrtab"` (9 chars + `\0` = 10 bytes, not 8). This undercounts
  by 2 bytes, so every `sh_name` computed inside `WriteProgramSections`
  points 2 bytes too early in the shstrtab buffer.


## Literal pool REL12_PC — slot se izgubi pre linkera

- [ ] **Literal relokacija ne nosi identitet svog pool slota.**
  `GetLiteralValue` (`src/assembly/Assembly.cpp`) postavlja
  `rel->symbolName = mCurrentSection->name`, a pool indeks pečati u
  displacement instrukcije. Za vrednost simbola postoji simbol koji pamti
  slot (`value`) i preživi merge, ali za literal ne postoji ništa što
  linker može da veže za konkretan slot. Zato REL12_PC resolver
  (`Linker::ResolveRelocations`, ~linija 312-328) čita `value` simbola
  sekcije — istu za sve literale — pa se svi literali razrešavaju kao da
  pokazuju na isti slot.
  Predlog: nosi bajt-offset literala u bazenu kroz `rel->addend`, pomeri ga
  u `MergeSections` za `poolOffset` (bajtovi, kao i ostali pool offseti),
  koristi ga direktno u REL12_PC resolveru. Alternativa: lokalni anonimni
  simbol po pool slotu (uniformnije, ali puni tabelu simbola).

## `Linker::MergeSections` — jedinice `value` vs `poolOffset`

- [ ] **`sym.first->value += poolOffset` meša jedinice.**
  (`src/linker/Linker.cpp:194`) `value` je pool INDEKS (entry broj), a
  `poolOffset` (`dst->literalPool.size()`) je u BAJTOVIMA. Sabiranje daje
  pogrešan slot (npr. bazen prvog fajla od 2 unosa = 8 bajtova, pa se indeks
  0 drugog fajla pomeri na 8 umesto na 2). Fix: `poolOffset / 4`, ili držati
  `value` u bajtovima svuda dosledno.

## `Elf::LoadBinary` — uskladiti sa promenjenim linker pozivom

- [ ] **Izmeniti `Elf::LoadBinary` (`src/elf/Elf.cpp:599`).** Poziv u linkeru
  koji učitava binarni fajl je promenjen — sada učitava i adrese kao prve
  članove niza sa memorijske lokacije. `LoadBinary` treba uskladiti sa tim
  novim formatom (adrese na početku niza pre ELF sadržaja) umesto da čita
  `Elf64_Ehdr` odmah sa offseta 0.
