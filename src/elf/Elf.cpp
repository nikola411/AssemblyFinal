#include "Elf.hpp"

#include "ElfHelpers.hpp"

#include <vector>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <set>

/*
- pravila: 
    - jedna tabela simbola po fajlu
    - nakon tabele simbola, tabela stringova za tu tabelu simbola
    - bazen literala na kraju sekcije ako je potreban (treba dodati jmp naredbu u tom slucaju) 
|------------------------------------|
|             ELF header             |
|------------------------------------|
|            PHDR Table              | - opciono, ako je linkable
|------------------------------------| - formalno, ovde pocinju sekcije
|            Symbol table            |
|------------------------------------|
|            String table            |
|------------------------------------|
|            SHSTR table             | - zbog organizacije funkcija prilikom konverzije 
|------------------------------------| - pocetak formalnih sekcija
|            Section 1               |
|------------------------------------|
|               ...                  |
|------------------------------------|
|            Section n               |
|------------------------------------|
|            Relocations 1           |
|------------------------------------|
|               ...                  |
|------------------------------------|
|            Relocations n           |
|------------------------------------| - ovde se sekcije zavrsavaju
|            SHDR Table              | - opciono, ako je executable
|------------------------------------|
*/

size_t Elf::WriteSymtabSection(std::vector<Elf64_Shdr>& shdrt)
{
    // u ovom trenutku, shdrt je prazan i psotavljamo prvu sekciju tu i pisemo u fajl
    // odmah nakon elf headera

    std::vector<Elf64_Sym> symt;

    Elf64_Shdr shdr = {};
    shdr.sh_name = 0;
    shdr.sh_offset = sizeof(Elf64_Ehdr) + 1;
    shdr.sh_addr = 0;
    shdr.sh_addralign = sizeof(Elf64_Sym);
    shdr.sh_size = sizeof(Elf64_Sym) * symTable.size();
    shdr.sh_entsize = sizeof(Elf64_Sym);
    shdr.sh_type = Elf64_Shdr_Type::SHT_SYMTAB;
    shdr.sh_link = 1; // odmah nakon ide tablela stringova
    shdr.sh_flags = 0;

    shdrt.push_back(shdr);

        // potrebne su nam dve dodatne podtabele, lokalni i globalni simboli
    SymbolTable local;
    SymbolTable global;
    std::partition_copy(
        symTable.begin(),
        symTable.end(),
        std::back_inserter(local),
        std::back_inserter(global),
        [](const Symbol::s_ptr& sym){ return !sym->isGlobal; }
    );

    uint32_t strt_offset = 0;

    for (const auto& s: local)
    {
        Elf64_Sym sym = {};
        sym.st_name = strt_offset;
        sym.st_info = ELF64_ST_INFO(STB_LOCAL, 0); // vezivanje
        sym.st_shndx = GetSectionIndex(sections, s->section); // trenutno nula, azuriramo posle
        sym.st_value = s->value;
        sym.st_size = 0;

        strt_offset += s->name.size() + 1;

        symt.push_back(sym);
    }

    for (const auto& s: global)
    {
        Elf64_Sym sym = {};
        sym.st_name = strt_offset;
        sym.st_info = ELF64_ST_INFO(STB_GLOBAL, 0); // vezivanje
        sym.st_shndx = GetSectionIndex(sections, s->section); // trenutno nula, azuriramo posle
        sym.st_value = s->value;
        sym.st_size = 0;

        strt_offset += s->name.size() + 1;

        symt.push_back(sym);
    }

    ElfIO::WriteTable<Elf64_Sym>(shdr.sh_offset, content, symt);

    return sizeof(Elf64_Sym) * symt.size();
}

size_t Elf::WriteProgramSections(std::vector<Elf64_Shdr> &shdrt, int startOffset, int shstrtOffset)
{
    // u ovom trenutku, shdrt ima dva ulazaa - .symtab i .strtab koji su upisani u fajl
    std::vector<uint8_t> sectionData;
    std::vector<uint8_t> poolsData;
    std::vector<Elf64_Rela> sectionRelocationData;
    std::vector<Elf64_Rela> poolRelocationData;

    for (const auto& sec: sections)
    {
        Elf64_Shdr shdr = {};
        shdr.sh_name = shstrtOffset;
        shdr.sh_offset = sectionData.size() + startOffset;
        shdr.sh_addr = 0;
        shdr.sh_addralign = sizeof(uint8_t);
        shdr.sh_size = sizeof(uint8_t) * sec->data.size();
        shdr.sh_entsize = sizeof(uint8_t);
        shdr.sh_type = Elf64_Shdr_Type::SHT_PROGBITS;
        shdr.sh_link = Elf64_SHN::SHN_UNDEF;
        shdr.sh_info = 0; // po specifikaciji elf-a
        shdr.sh_flags = Elf64_Shdr_Flags::SHF_WRITE | Elf64_Shdr_Flags::SHF_EXECINSTR | Elf64_Shdr_Flags::SHF_ALLOC;
        shdrt.push_back(shdr);

        sectionData.insert(sectionData.end(), sec->data.begin(), sec->data.end());

        shstrtOffset += sec->name.size() + 1;
    }

    for (int i = 0; i < sections.size(); i++)
    {
        if (sections[i]->literalPool.empty())
            continue;

        Elf64_Shdr shdr = {};
        shdr.sh_name = shstrtOffset;
        shdr.sh_offset = poolsData.size() + sectionData.size() + startOffset;
        shdr.sh_addr = 0;
        shdr.sh_addralign = sizeof(uint8_t);
        shdr.sh_size = sizeof(uint8_t) * sections[i]->literalPool.size();
        shdr.sh_entsize = sizeof(uint8_t);
        shdr.sh_type = Elf64_Shdr_Type::SHT_LITPOOL;
        shdr.sh_link = 0; // indeks zaglavlja pridruzene tabele simbola
        shdr.sh_info = i + 3;// dodajemo dva zbog .symtab, .strtab i .shstrt (gledamo unazad, posto smo isli redom za sekcije tako idemo redom i za bazene)
        shdr.sh_flags = Elf64_Shdr_Flags::SHF_WRITE | Elf64_Shdr_Flags::SHF_EXECINSTR | Elf64_Shdr_Flags::SHF_ALLOC;
        shdrt.push_back(shdr);

        poolsData.insert(poolsData.end(), sections[i]->literalPool.begin(), sections[i]->literalPool.end());

        shstrtOffset += sections[i]->name.size() + /*.pool*/ 5 + /*\0*/1;
    }

    for (int i = 0; i < sections.size(); i++)
    {
        if (sections[i]->sectionRelocations.empty())
            continue;

        Elf64_Shdr shdr = {};
        shdr.sh_name = shstrtOffset;
        shdr.sh_offset = sectionRelocationData.size() * sizeof(Elf64_Rela) + poolsData.size() + sectionData.size() + startOffset;
        shdr.sh_addr = 0;
        shdr.sh_addralign = sizeof(Elf64_Rela);
        shdr.sh_size = sizeof(Elf64_Rela) * sections[i]->sectionRelocations.size();
        shdr.sh_entsize = sizeof(Elf64_Rela);
        shdr.sh_type = Elf64_Shdr_Type::SHT_RELA;
        shdr.sh_link = 0; // .symtab je prva sekcija u shdrt
        shdr.sh_info = i + 3; // dodajemo 3 zbog .symtab i .strtab i .shstrt
        shdr.sh_flags = Elf64_Shdr_Flags::SHF_WRITE | Elf64_Shdr_Flags::SHF_EXECINSTR | Elf64_Shdr_Flags::SHF_ALLOC;
        shdrt.push_back(shdr);

        std::vector<Elf64_Rela> relaTable;
        for (auto r : sections[i]->sectionRelocations)
        {
            relaTable.push_back(ConvertRelocationToRela(*r, *this));
        }

        sectionRelocationData.insert(sectionRelocationData.end(), relaTable.begin(), relaTable.end());

        shstrtOffset += sections[i]->name.size() + /*.rela*/ 5 + /*\0*/1;
    }

    for (int i = 0; i < sections.size(); i++)
    {
        if (sections[i]->poolRelocations.empty())
            continue;

        Elf64_Shdr shdr = {};
        shdr.sh_name = shstrtOffset;
        shdr.sh_offset = (poolRelocationData.size() + sectionRelocationData.size()) * sizeof(Elf64_Rela) + poolsData.size() + sectionData.size() + startOffset;
        shdr.sh_addr = 0;
        shdr.sh_addralign = sizeof(Elf64_Rela);
        shdr.sh_size = sizeof(Elf64_Rela) * sections[i]->poolRelocations.size();
        shdr.sh_entsize = sizeof(Elf64_Rela);
        shdr.sh_type = Elf64_Shdr_Type::SHT_LITPOOL_RELA;
        shdr.sh_link = 0; // .symtab je prva sekcija u shdrt
        shdr.sh_info = i + 3; // indeks sekcije na ciji bazen literala ce se primenjivati ove relokacije
        shdr.sh_flags = Elf64_Shdr_Flags::SHF_WRITE | Elf64_Shdr_Flags::SHF_EXECINSTR | Elf64_Shdr_Flags::SHF_ALLOC;
        shdrt.push_back(shdr);

        std::vector<Elf64_Rela> relaTable;
        for (auto r : sections[i]->poolRelocations)
        {
            relaTable.push_back(ConvertRelocationToRela(*r, *this));
        }

        poolRelocationData.insert(poolRelocationData.end(), relaTable.begin(), relaTable.end());

        shstrtOffset += sections[i]->name.size() + /*.pool.rela*/ 10 + /*\0*/1;
    }

    size_t size = 0;

    ElfIO::WriteTable<uint8_t>(startOffset + size, content, sectionData);
    size += sectionData.size();

    ElfIO::WriteTable<uint8_t>(startOffset + size, content, poolsData);
    size += poolsData.size();

    ElfIO::WriteTable<Elf64_Rela>(startOffset + size, content, sectionRelocationData);
    size += sectionRelocationData.size() * sizeof(Elf64_Rela);

    ElfIO::WriteTable<Elf64_Rela>(startOffset + size, content, poolRelocationData);
    size += poolRelocationData.size() * sizeof(Elf64_Rela);

    return size;
}

// odnosi se na .symtab tabelu stringova
size_t Elf::WriteStrtabSection(std::vector<Elf64_Shdr> &shdrt)
{
    // u ovom trenutku shdrt sadrzi jedan ulaz - .symtab shdr
    Elf64_Shdr symt_shdr = GetShdrByType(shdrt, Elf64_Shdr_Type::SHT_SYMTAB);

    std::vector<uint8_t> strt = {};

    for (auto sym : symTable)
    {
        strt.insert(strt.end(), sym->name.begin(), sym->name.end());
        strt.push_back('\0');
    }

    int strt_offset = symt_shdr.sh_offset + symt_shdr.sh_size;

    Elf64_Shdr shdr = {};
    shdr.sh_name = 8; // doci ce odmah posle .symtab (size = 8)
    shdr.sh_offset = strt_offset;
    shdr.sh_addr = 0;
    shdr.sh_addralign = sizeof(uint8_t);
    shdr.sh_size = sizeof(uint8_t) * strt.size();
    shdr.sh_entsize = 0;
    shdr.sh_type = Elf64_Shdr_Type::SHT_STRTAB;
    shdr.sh_link = 0; // pisemo 0 jer po elf standardu ovde ne referenciramo nista
    shdr.sh_info = 0; // isto kao gore
    shdr.sh_flags = 0;

    shdrt.push_back(shdr);

    ElfIO::WriteTable(strt_offset, content, strt);

    return sizeof(uint8_t) * strt.size();
}

size_t Elf::WriteShstrtabSection(std::vector<Elf64_Shdr> &shdrt)
{
    // u ovom trenutku, shdrt sadrzi .symtab, .strtab i sve sekcije
    std::vector<uint8_t> shstrt = {};

    std::string symtab_name = ".symtab";
    shstrt.insert(shstrt.end(), symtab_name.begin(), symtab_name.end());
    shstrt.push_back('\0');

    std::string strtab_name = ".strtab";
    shstrt.insert(shstrt.end(), strtab_name.begin(), strtab_name.end());
    shstrt.push_back('\0');

    std::string shstrt_name = ".shstrtab";
    shstrt.insert(shstrt.end(), shstrt_name.begin(), shstrt_name.end());
    shstrt.push_back('\0');

    for (const auto s : sections)
    {
        shstrt.insert(shstrt.end(), s->name.begin(), s->name.end());
        shstrt.push_back('\0');
    }

    for (const auto s : sections)
    {
        if (s->literalPool.empty())
            continue;

        std::string poolSectionName = s->name + ".pool";
        shstrt.insert(shstrt.end(), poolSectionName.begin(), poolSectionName.end());
        shstrt.push_back('\0');
    }

    for (const auto s : sections)
    {
        if (s->sectionRelocations.empty())
            continue;

        std::string relocationSectionName = s->name + ".rela";
        shstrt.insert(shstrt.end(), relocationSectionName.begin(), relocationSectionName.end());
        shstrt.push_back('\0');
    }

    for (const auto s : sections)
    {
        if (s->poolRelocations.empty())
            continue;

        std::string poolRelocationSectionName = s->name + ".pool.rela";
        shstrt.insert(shstrt.end(), poolRelocationSectionName.begin(), poolRelocationSectionName.end());
        shstrt.push_back('\0');
    }

    Elf64_Shdr shdr = {};
    shdr.sh_name = sizeof(".symtab") + sizeof(".strtab");
    shdr.sh_offset = shdrt.back().sh_offset + shdrt.back().sh_size;
    shdr.sh_addr = 0;
    shdr.sh_addralign = sizeof(uint8_t);
    shdr.sh_size = sizeof(uint8_t) * shstrt.size();
    shdr.sh_entsize = 0; // nema standardan entitet, pa zbog toga i ne definisemo entsize
    shdr.sh_type = Elf64_Shdr_Type::SHT_STRTAB;
    shdr.sh_link = Elf64_SHN::SHN_UNDEF;
    shdr.sh_info = 0;
    shdr.sh_flags = 0;

    shdrt.push_back(shdr);

    ElfIO::WriteTable<uint8_t>(shdr.sh_offset, content, shstrt);

    Elf64_Ehdr ehdr = GetElfHeader();
    ehdr.e_shstrndx = 2;
    SetElfHeader(ehdr);

    return sizeof(uint8_t) * shstrt.size();
}

size_t Elf::WriteShdrtSection(const std::vector<Elf64_Shdr> &shdrt, int offset)
{
    Elf64_Ehdr ehdr  = GetElfHeader();
    ehdr.e_shnum = shdrt.size();
    ehdr.e_shoff = offset;
    ehdr.e_shentsize = sizeof(Elf64_Shdr);

    SetElfHeader(ehdr);
    ElfIO::WriteTable<Elf64_Shdr>(offset, content, shdrt);

    return offset + shdrt.size() * sizeof(Elf64_Shdr);
}

Elf64_Ehdr Elf::GetElfHeader() const
{
    return ElfIO::ReadObject<Elf64_Ehdr>(0, content);
}

bool Elf::SetElfHeader(Elf64_Ehdr ehdr)
{
    ElfIO::WriteObject<Elf64_Ehdr>(0, content, ehdr);
    return true;
}

std::vector<Elf64_Phdr> Elf::GetProgramHeaderTable() const
{
    Elf64_Ehdr ehdr = GetElfHeader();

    Elf64_Off start = ehdr.e_phoff;

    return ElfIO::ReadTable<Elf64_Phdr>(start, ehdr.e_phnum, content);
}

bool Elf::SetProgramHeaderTable(const std::vector<Elf64_Phdr>& phdr)
{
    Elf64_Ehdr ehdr = GetElfHeader();

    Elf64_Off start = ehdr.e_phoff;
    ElfIO::WriteTable<Elf64_Phdr>(start, content, phdr);

    return true;
}

std::vector<Elf64_Shdr> Elf::GetSectionHeaderTable() const
{
    Elf64_Ehdr ehdr = GetElfHeader();

    Elf64_Off start = ehdr.e_shoff;

    return ElfIO::ReadTable<Elf64_Shdr>(start, ehdr.e_shnum, content);
}

bool Elf::SetSectionHeaderTable(const std::vector<Elf64_Shdr>& shdr)
{
    Elf64_Ehdr ehdr = GetElfHeader();

    Elf64_Off start = ehdr.e_shoff;
    ElfIO::WriteTable<Elf64_Shdr>(start, content, shdr);

    return true;
}

std::vector<Elf64_Sym> Elf::GetSymbolTable() const
{
    auto shdrt = GetSectionHeaderTable();
    Elf64_Shdr symtShdr = GetShdrByType(shdrt, Elf64_Shdr_Type::SHT_SYMTAB);

    if (ElfIO::IsDefaultValue<Elf64_Shdr>(symtShdr))
        return {};

    if (symtShdr.sh_size == 0 ||
        symtShdr.sh_size % sizeof(Elf64_Sym) != 0 ||
        symtShdr.sh_offset + symtShdr.sh_size > content.size())
    {
        return {};
    }

    return ElfIO::ReadTable<Elf64_Sym>(symtShdr.sh_offset, symtShdr.sh_size / sizeof(Elf64_Sym), content);
}

std::vector<uint8_t> Elf::GetStringTable() const
{
    auto shdrt = GetSectionHeaderTable();
    Elf64_Shdr symstrt_shdr = GetShdrByType(shdrt, Elf64_Shdr_Type::SHT_STRTAB);

    return ElfIO::ReadTable<uint8_t>(symstrt_shdr.sh_offset, symstrt_shdr.sh_size / sizeof(uint8_t), content);
}

std::vector<uint8_t> Elf::GetSectionContent(int shdrtIndex) const
{
    auto shdrt = GetSectionHeaderTable();
    Elf64_Shdr strtShdr = GetShdrByIndex(shdrt, shdrtIndex);

    if (ElfIO::IsDefaultValue<Elf64_Shdr>(strtShdr))
        return {};

    if (strtShdr.sh_size == 0 || strtShdr.sh_offset + strtShdr.sh_size > content.size())
        return {};

    return ElfIO::ReadTable<uint8_t>(strtShdr.sh_offset, strtShdr.sh_size, content);
}

Elf64_Rela Elf::ConvertRelocationToRela(const Relocation &rel, const Elf &elf)
{
    Elf64_Sym sym = elf.GetSymbolByName(rel.symbolName);
    if (ElfIO::IsDefaultValue<Elf64_Sym>(sym))
        return Elf64_Rela();

    int32_t ind = elf.GetSymbolIndexInSymbolTable(sym);
    if (ind == -1)
        return Elf64_Rela();

    Elf64_Rela ret = {};
    ret.r_info = ind << 32 | (Elf64_Rela_Type_custom)rel.type;
    ret.r_offset = rel.offset;
    ret.r_addend = rel.addend;

    return ret;
}

Relocation Elf::ConvertRelaToRelocation(const Elf64_Rela &rel, const Elf &elf)
{
    Relocation ret = {};

    Elf64_Sym sym = elf.GetSymbolByIndex(rel.r_info >> 32);
    Elf64_Sym def = {};
    if (std::memcmp(&sym, &def, sizeof(Elf64_Sym)) == 0)
        return ret;

    ret.offset = rel.r_offset;
    ret.addend = rel.r_addend;
    ret.type = (eRelocationType)((rel.r_info  << 32) >> 32);
    ret.symbolName = elf.GetSymbolName(sym);

    return ret;
}

bool Elf::UpdateSectionContent(int shdrtIndex, const std::vector<uint8_t> &src)
{
    auto shdrt = GetSectionHeaderTable();

    if (shdrtIndex > shdrt.size() || shdrtIndex < 0)
        return false;

    Elf64_Shdr shdr = shdrt[shdrtIndex];
    if (shdr.sh_size != src.size())
        return false;

    ElfIO::WriteTable<uint8_t>(shdr.sh_offset, content, src);

    return true;
}

std::string Elf::GetSymbolName(const Elf64_Sym& sym) const
{
    auto shdrt = GetSectionHeaderTable();
    Elf64_Shdr symtShdr = GetShdrByType(shdrt, Elf64_Shdr_Type::SHT_SYMTAB);
    if (ElfIO::IsDefaultValue<Elf64_Shdr>(symtShdr))
        return {};

    Elf64_Shdr strtShdr = GetShdrByIndex(shdrt, symtShdr.sh_link);
    if (ElfIO::IsDefaultValue<Elf64_Shdr>(strtShdr))
        return {};

    std::string name;
    size_t offset = strtShdr.sh_offset + sym.st_name;

    while (offset < content.size() && content[offset] != '\0')
    {
        name += content[offset];
        offset++;
    }

    return name;
}

Elf64_Sym Elf::GetSymbolByName(const std::string &name) const
{
    std::vector<Elf64_Sym> symt = GetSymbolTable();
    std::vector<uint8_t> strt = GetStringTable();

    for (auto sym : symt)
    {
        std::string symName = ReadString(strt, sym.st_name);
        if (symName == name)
            return sym;
    }

    return Elf64_Sym();
}

uint32_t Elf::GetSymbolIndexInSymbolTable(const Elf64_Sym &sym) const
{
    std::vector<Elf64_Sym> symt = GetSymbolTable();

    for (uint32_t i = 0; i < symt.size(); i++)
    {
        if (std::memcmp((void*)&symt[i], (void*)&sym, sizeof(Elf64_Sym)) == 0)
            return i;
    }

    return -1;
}

Elf64_Sym Elf::GetSymbolByIndex(const uint32_t &index) const
{
    std::vector<Elf64_Sym> symt = GetSymbolTable();
    if (index > symt.size() || index < 0)
        return Elf64_Sym();

    return symt[index];
}

void Elf::HexDump()
{
}

void Elf::LoadLinkable(const SymbolTable &symTable, const SectionTable &sections)
{
    // postavi elf header
    this->symTable = symTable;
    this->sections = sections;
    this->relocations = relocations;

    Elf64_Ehdr ehdr = InitEhdr();
    SetElfHeader(ehdr);

    std::vector<Elf64_Shdr> shdrt = std::vector<Elf64_Shdr>(); // section header table
    std::vector<Elf64_Phdr> phdrt = std::vector<Elf64_Phdr>(); // program header table

    // sections: symtab, strtab, program sections, relocation sections
    std::vector<uint8_t> sectionData = std::vector<uint8_t>();
    std::vector<uint8_t> strt = std::vector<uint8_t>();
    std::vector<uint8_t> shstrt = std::vector<uint8_t>();

    std::vector<Elf64_Rela> relt;

    Elf64_Shdr shdr;

    size_t symtSize = WriteSymtabSection(shdrt);
    size_t strtSize = WriteStrtabSection(shdrt);
    size_t shstrtSize = WriteShstrtabSection(shdrt);

    size_t shstrtOffset = sizeof(".symtab") + sizeof(".strtab") + sizeof(".shstrtab");
    size_t startOffset = sizeof(Elf64_Ehdr) + symtSize + strtSize + shstrtSize;
    size_t sectionsSize = WriteProgramSections(shdrt, startOffset, shstrtOffset);

    size_t elfSize = WriteShdrtSection(shdrt, startOffset + sectionsSize);
}

void Elf::LoadBinary(const std::vector<uint8_t> &data)
{
    this->content = data;
    Elf64_Ehdr ehdr = ElfIO::ReadObject<Elf64_Ehdr>(0, content);
    ValidateEhdr(ehdr, content.size());
}

void Elf::UnloadLinkable(SymbolTable &symTable, SectionTable &sections)
{
    symTable = std::vector<Symbol::s_ptr>();
    sections = std::vector<Section::s_ptr>();

    Elf64_Ehdr ehdr = GetElfHeader();

    std::vector<Elf64_Shdr> shdrt = ElfIO::ReadTable<Elf64_Shdr>(ehdr.e_shoff, ehdr.e_shnum, content);

    // ucitaj tabelu simbola; potrebne su nam dve sekcije: .symtab i .strtab; indeksi 0 i 1
    Elf64_Shdr symt_shdr = shdrt[0];
    Elf64_Shdr strt_shdr = shdrt[1];
    Elf64_Shdr shstrtab_shdr = shdrt[ehdr.e_shstrndx];

    std::vector<Elf64_Sym> symt = ElfIO::ReadTable<Elf64_Sym>(symt_shdr.sh_offset, symt_shdr.sh_size / symt_shdr.sh_entsize, content);
    std::vector<uint8_t> strt = ElfIO::ReadTable<uint8_t>(strt_shdr.sh_offset, strt_shdr.sh_size, content);
    std::vector<uint8_t> shstrt = ElfIO::ReadTable<uint8_t>(shstrtab_shdr.sh_offset, shstrtab_shdr.sh_size, content);

    for (const auto& sym : symt)
    {
        Symbol::s_ptr curr = std::make_shared<Symbol>();
        curr->defined = sym.st_shndx != Elf64_SHN::SHN_UNDEF;
        curr->isBig = sym.st_size > 2;
        curr->isGlobal = ELF64_ST_BIND(sym.st_info) == Elf64_Sym_Binding::STB_GLOBAL;
        curr->isExtern = ELF64_ST_BIND(sym.st_info) == Elf64_Sym_Binding::STB_WEAK;
        curr->offset = sym.st_value;
        curr->value = sym.st_value;

        curr->section = ReadString(shstrt, shdrt[sym.st_shndx].sh_name);
        curr->name = ReadString(strt, sym.st_name);

        symTable.push_back(curr);
    }

    // ucitaj sekcije
    for (const auto& shdr : shdrt)
    {
        if (shdr.sh_type != SHT_PROGBITS)
            continue;

        Section::s_ptr curr = std::make_shared<Section>();
        curr->data = ElfIO::ReadTable<uint8_t>(shdr.sh_offset, shdr.sh_size, content);
        curr->name = ReadString(shstrt, shdr.sh_name);

        sections.push_back(curr);
    }

    for (const auto& shdr : shdrt)
    {
        if (shdr.sh_type != SHT_LITPOOL)
            continue;

        sections[shdr.sh_info - 3]->literalPool = ElfIO::ReadTable<uint8_t>(shdr.sh_offset, shdr.sh_size, content);
    }

    // ucitaj relokacije
    for (const auto& shdr : shdrt)
    {
        if (shdr.sh_type != SHT_RELA)
            continue;

        std::vector<Elf64_Rela> tbl = ElfIO::ReadTable<Elf64_Rela>(shdr.sh_offset, shdr.sh_size / shdr.sh_entsize, content);

        for (const auto& rel : tbl)
        {
            Relocation curr = ConvertRelaToRelocation(rel, *this);
            sections[shdr.sh_info - 3]->sectionRelocations.push_back(std::make_shared<Relocation>(curr));
        }
    }

    for (const auto& shdr : shdrt)
    {
        if (shdr.sh_type != SHT_LITPOOL_RELA)
            continue;

        std::vector<Elf64_Rela> tbl = ElfIO::ReadTable<Elf64_Rela>(shdr.sh_offset, shdr.sh_size / shdr.sh_entsize, content);

        for (const auto& rel : tbl)
        {
            Relocation curr = ConvertRelaToRelocation(rel, *this);
            sections[shdr.sh_info - 3]->poolRelocations.push_back(std::make_shared<Relocation>(curr));
        }
    }
}

