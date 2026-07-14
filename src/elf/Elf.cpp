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
|------------------------------------|
|            SHSTR table             |
|------------------------------------| - ovde se sekcije zavrsavaju
|            SHDR Table              | - opciono, ako je executable
|------------------------------------|
*/

void Elf::LoadLinkable(const SymbolTable &symTable, const SectionTable &sections)
{
    // postavi elf header
    this->symTable = symTable;
    this->sections = sections;
    this->relocations = relocations;

    // treba nam prvo  velicina
    int size = GetElfSize();
    content = std::vector<uint8_t>(size, 0);

    Elf64_Ehdr ehdr = InitEhdr();

    std::vector<Elf64_Shdr> shdrt = std::vector<Elf64_Shdr>(); // section header table
    std::vector<Elf64_Phdr> phdrt = std::vector<Elf64_Phdr>(); // program header table

    // sections: symtab, strtab, program sections, relocation sections
    std::vector<uint8_t> sectionData = std::vector<uint8_t>();
    std::vector<uint8_t> strt = std::vector<uint8_t>();
    std::vector<uint8_t> shstrt = std::vector<uint8_t>();

    std::vector<Elf64_Rela> relt;

    Elf64_Shdr shdr;

    LoadSymbolTable(shdrt, shstrt);



    shstrt.insert(shstrt.end(), sec->name.begin(), sec->name.end());
    shstrt.push_back('\0');

    std::string shstrtabname = ".shstrtab";
    shstrt.insert(shstrt.end(), shstrtabname.begin(), shstrtabname.end());
    shstrt.push_back('\0');

    // pravimo sekciju symtab i strtab paralelno
    std::vector<Elf64_Sym> symt;
    strt = {};


    
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
    relocations = std::vector<Relocation::s_ptr>();
    
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

    // ucitaj relokacije
    for (const auto& shdr : shdrt)
    {
        if (shdr.sh_type != SHT_RELA)
            continue;

        std::vector<Elf64_Rela> tbl = ElfIO::ReadTable<Elf64_Rela>(shdr.sh_offset, shdr.sh_size / shdr.sh_entsize, content);
        Elf64_Shdr sec_shdr = shdrt[shdr.sh_info];

        for (const auto& rel : tbl)
        {
            Relocation::s_ptr curr = std::make_shared<Relocation>();
            curr->symbolName = ReadString(strt, symt[rel.r_info >> 32].st_name);
            curr->sectionName = ReadString(shstrt, sec_shdr.sh_name);
            curr->addend = rel.r_addend;
            curr->type = (eRelocationType)(rel.r_info & 0xFFFFFFFF);
            curr->offset = rel.r_offset;

            relocations.push_back(curr);
        }
    }
}

void Elf::LoadSymbolTable(std::vector<Elf64_Shdr>& shdrt, std::vector<uint8_t>& shstrt)
{
    std::vector<uint8_t> strt;
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

    std::string symtabname = ".symtab";
    shstrt.insert(shstrt.end(), symtabname.begin(), symtabname.end());
    shstrt.push_back('\0');

    std::string strtname = ".strtab";
    shstrt.insert(shstrt.end(), strtname.begin(), strtname.end());
    shstrt.push_back('\0');

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

    for (const auto& s: local)
    {
        Elf64_Sym sym = {};
        sym.st_name = strt.size();
        sym.st_info = ELF64_ST_INFO(STB_LOCAL, 0); // vezivanje
        sym.st_shndx = GetSectionIndex(sections, s->section); // trenutno nula, azuriramo posle
        sym.st_value = s->value;
        sym.st_size = 0;

        strt.insert(strt.end(), s->name.begin(), s->name.end());

        symt.push_back(sym);
    }

    for (const auto& s: global)
    {
        Elf64_Sym sym = {};
        sym.st_name = strt.size();
        sym.st_info = ELF64_ST_INFO(STB_GLOBAL, 0); // vezivanje
        sym.st_shndx = GetSectionIndex(sections, s->section); // trenutno nula, azuriramo posle
        sym.st_value = s->value;
        sym.st_size = 0;

        strt.insert(strt.end(), s->name.begin(), s->name.end());

        symt.push_back(sym);
    }

    ElfIO::WriteTable<Elf64_Sym>(shdr.sh_offset, content, symt);
    ElfIO::WriteTable<uint8_t>(shdr.sh_offset + symt.size() * sizeof(Elf64_Sym), content, strt);
}

void Elf::LoadSectionData(std::vector<Elf64_Shdr> &shdrt, int startOffset, int shstrtOffset)
{
    std::vector<uint8_t> sectionData;
    std::vector<uint8_t> poolsData;
    std::vector<uint8_t> sectionRelocationData;
    std::vector<uint8_t> poolRelocationData;

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
        shdr.sh_link = i + 2; // dodajemo dva zbog .symtab i .strtab
        shdr.sh_flags = Elf64_Shdr_Flags::SHF_WRITE | Elf64_Shdr_Flags::SHF_EXECINSTR | Elf64_Shdr_Flags::SHF_ALLOC;

        poolsData.insert(poolsData.end(), sections[i]->literalPool.begin(), sections[i]->literalPool.end());

        shstrtOffset += sections[i]->name.size() + /*.pool*/ 5 + /*\0*/1;
    }

    int relaCount = 0;

    for (int i = 0; i < sections.size(); i++)
    {
        if (sections[i]->sectionRelocations.empty())
            continue;

        relaCount++;

        Elf64_Shdr shdr = {};
        shdr.sh_name = shstrtOffset;
        shdr.sh_offset = sectionRelocationData.size() + poolsData.size() + sectionData.size() + startOffset;
        shdr.sh_addr = 0;
        shdr.sh_addralign = sizeof(uint8_t);
        shdr.sh_size = sizeof(uint8_t) * sections[i]->sectionRelocations.size();
        shdr.sh_entsize = sizeof(uint8_t);
        shdr.sh_type = Elf64_Shdr_Type::SHT_RELA;
        shdr.sh_link = i + 2; // dodajemo dva zbog .symtab i .strtab
        shdr.sh_flags = Elf64_Shdr_Flags::SHF_WRITE | Elf64_Shdr_Flags::SHF_EXECINSTR | Elf64_Shdr_Flags::SHF_ALLOC;

        sectionRelocationData.insert(sectionRelocationData.end(), sections[i]->sectionRelocations.begin(), sections[i]->sectionRelocations.end());

        shstrtOffset += sections[i]->name.size() + /*.rela*/ 5 + /*\0*/1;
    }

    for (int i = 0; i < sections.size(); i++)
    {
        if (sections[i]->poolRelocations.empty())
            continue;

        Elf64_Shdr shdr = {};
        shdr.sh_name = shstrtOffset;
        shdr.sh_offset = poolRelocationData.size() + sectionRelocationData.size() + poolsData.size() + sectionData.size() + startOffset;
        shdr.sh_addr = 0;
        shdr.sh_addralign = sizeof(uint8_t);
        shdr.sh_size = sizeof(uint8_t) * sections[i]->poolRelocations.size();
        shdr.sh_entsize = sizeof(uint8_t);
        shdr.sh_type = Elf64_Shdr_Type::SHT_RELA;
        shdr.sh_link = relaCount + sections.size() + 2; // .symtab, .strtab, sve sekcije i na kraju bazeni literala za sekcije
        shdr.sh_flags = Elf64_Shdr_Flags::SHF_WRITE | Elf64_Shdr_Flags::SHF_EXECINSTR | Elf64_Shdr_Flags::SHF_ALLOC;

        poolRelocationData.insert(poolRelocationData.end(), sections[i]->poolRelocations.begin(), sections[i]->poolRelocations.end());

        shstrtOffset += sections[i]->name.size() + /*.pool.rela*/ 10 + /*\0*/1;
    }

    int offset = startOffset;
    ElfIO::WriteTable<uint8_t>(offset, content, sectionData);

    offset += sectionData.size();
    ElfIO::WriteTable<uint8_t>(offset, content, poolsData);

    offset += poolsData.size();
    ElfIO::WriteTable<uint8_t>(offset, content, sectionRelocationData);

    offset += sectionRelocationData.size();
    ElfIO::WriteTable<uint8_t>(offset, content, poolRelocationData);
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

std::vector<uint8_t> Elf::GetSectionContent(int shdrtOffset) const
{
    auto shdrt = GetSectionHeaderTable();
    Elf64_Shdr strtShdr = GetShdrByIndex(shdrt, shdrtOffset);

    if (ElfIO::IsDefaultValue<Elf64_Shdr>(strtShdr))
        return {};

    if (strtShdr.sh_size == 0 || strtShdr.sh_offset + strtShdr.sh_size > content.size())
        return {};

    return ElfIO::ReadTable<uint8_t>(strtShdr.sh_offset, strtShdr.sh_size, content);
}

bool Elf::UpdateSectionContent(int shdrtOffset, const std::vector<uint8_t> &src)
{
    auto shdrt = GetSectionHeaderTable();

    if (shdrtOffset > shdrt.size() || shdrtOffset < 0)
        return false;

    Elf64_Shdr shdr = shdrt[shdrtOffset];
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

int Elf::GetElfSize() const
{
    int size = 0;

    size += sizeof(Elf64_Ehdr);

    // .symtab + .strtab
    int strt_size = 1; // vodeći \0
    if (!symTable.empty())
    {
        size += sizeof(Elf64_Sym) * symTable.size();
        for (const auto& s : symTable)
        {
            if (s->name == s->section)
                continue;
            strt_size += s->name.size() + 1;
        }
        size += (strt_size + 7) / 8 * 8;
    }

    // shstrtab: fiksni nazivi + nazivi sekcija + nazivi .pool sekcija + nazivi .rela sekcija
    int strsht_size = 1; // vodeći \0
    strsht_size += sizeof(".symtab");   // includes \0
    strsht_size += sizeof(".strtab");
    strsht_size += sizeof(".shstrtab");

    // program sekcije
    if (!sections.empty())
    {
        for (const auto& section : sections)
        {
            size += section->data.size();
            strsht_size += section->name.size() + 1;

            if (!section->literalPool.empty())
            {
                size += section->literalPool.size();
                strsht_size += section->name.size() + sizeof(".pool"); // "name.pool\0"
            }
        }
    }

    // RELA sekcije
    std::set<std::string> relaSections;
    if (!relocations.empty())
    {
        size += sizeof(Elf64_Rela) * relocations.size();
        for (const auto& r : relocations)
            relaSections.insert(r->sectionName);
        for (const auto& name : relaSections)
            strsht_size += sizeof(".rela.") - 1 + name.size() + 1; // ".rela.name\0"
    }

    size += (strsht_size + 7) / 8 * 8;

    // Section Header Table
    int num_sections = 1; // null sekcija [0]
    if (!symTable.empty()) num_sections += 2; // .symtab + .strtab
    for (const auto& section : sections)
    {
        num_sections++;
        if (!section->literalPool.empty())
            num_sections++;
    }
    num_sections += relaSections.size();
    num_sections++; // .shstrtab

    size += sizeof(Elf64_Shdr) * num_sections;

    return size;
}

void Elf::HexDump()
{
}

