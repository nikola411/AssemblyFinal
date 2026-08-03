#include "Linker.hpp"

#include <iostream>
#include <fstream>

#include "Helpers.hpp"
#include <algorithm>

Linker::Linker() {}
Linker::~Linker() {}

/*

Linker treba da pokupi ulazne fajlove koje dobije i spoji u jedan izlazni fajl.
Taj izlazni fajl moze da bude hex ili relocatable.
Ako je izlaz hex, fajl mora da bude moguce izvrsiti (izvrsi elf format).
Na ulaz dolaze elf fajlovi koje je obradio asembler, a linker treba da ih ucita.

1. Napravi tabelu simbola, proveri da li se krse pravila:
    1.1 pravila:
        1.1.1 simbol moze biti definisan kao lokalan* u vise fajlova
        1.1.2 simbol ne moze biti definisan kao globalan i kao lokalan*
        1.1.3 simbol koji je definisan kao extern** u nekom fajlu, mora da ima svoju global definiciju u nekom drugom fajlu

    *lokalan = definisan u fajlu i nije globala
    ** extern = nije definisan u fajlu

2. Spojiti sekcije:
    2.1 spojiti data deo
    2.2 azurirati data relokacije
    2.3 spojiti data relokacije
    2.4 spojiti pool deo
    2.5 azurirati pool relokacije
    2.6 spojiti pool relokacije

3. Razresiti relokacije:
    3.1 redosled:
        3.1.1 Elf64_Rela_Type_custom::R_CUSTOM_ABS32
        3.1.2 Elf64_Rela_Type_custom::R_CUSTOM_PC12
        3.1.3 Elf64_Rela_Type_custom::R_CUSTOM_ABS12

        (3.1.1) razresava vrednost simbola u bazenu, a (3.1.2) upisuje tu vrednost u memoriju


*/

void Linker::DoWork()
{
    // ucitaj
    ConsumeInput();
    // Razresi simbole
    ResolveSymbols();
    // spoji sekcije
    MergeSections();

    if (modes &= Modes::HEX)
    {
        PlaceSections();
        ResolveRelocations();
    }

    CreateOutput();
}

void Linker::SetInput(const std::string& input)
{
    inputFiles.push_back(input);
}

void Linker::SetOutput(const std::string& output)
{
    outputFile = output;
}

void Linker::SetPlace(const std::string& sectionName, const std::string& address)
{
    sectionPositions[sectionName] = std::stoul(address, nullptr, 16);
}

void Linker::SetMode(const uint8_t mode)
{
    modes |= mode;
}

void Linker::ConsumeInput()
{
    for (const auto& path: inputFiles)
    {
        auto raw = ReadBinaryFile(path);
        Elf elf = {};
        elf.LoadBinary(raw);

        SymbolTable symt;
        SectionTable sect;

        elf.UnloadLinkable(symt, sect);

        absFiles.push_back(AbstractFile{symt, sect});
    }
}

void Linker::ResolveSymbols()
{
    std::map<std::string, std::vector<std::pair<Symbol::s_ptr, int>>> hashMap = {};

    for (int i = 0; i < absFiles.size(); i++)
    {
        auto currSymt = absFiles[i].symTable;
        for (auto sym : currSymt)
        {
            hashMap[sym->name].push_back(std::pair<Symbol::s_ptr, int>(sym, i));
        }
    }

    for (auto entry : hashMap)
    {
        auto& data = hashMap[entry.first];

        int global = 0;
        int local = 0;
        int xtern = 0;

        for (auto pair : data)
        {
            auto sym = pair.first;

            global += sym->defined && sym->isGlobal;
            local += sym->defined && !sym->isExtern && !sym->isGlobal;
            xtern += !sym->defined && sym->isExtern;
        }

        if (global > 1)
            throw std::exception();

        if (xtern > 0 && global == 0)
            throw std::exception();

        if (local > 0 && global > 0)
        {
            std::vector<std::pair<Symbol::s_ptr, int>> globals, locals;
            std::partition_copy(data.begin(), data.end(),
                std::back_inserter(globals), std::back_inserter(locals),
                [](const std::pair<Symbol::s_ptr, int>& data) { return data.first->isGlobal; });

            auto it = std::remove_if(locals.begin(), locals.end(), [](std::pair<Symbol::s_ptr, int>& data) { return data.first->isExtern && !data.first->defined; });
            locals.erase(it, locals.end());

            for (const auto& glob : globals)
            {
                auto it = std::find_if(locals.begin(), locals.end(),
                    [&](std::pair<Symbol::s_ptr, int>& data) { return data.second == glob.second; });

                if (it != locals.end())
                    throw std::exception();
            }
        }

        for (auto& p : data)
            if (p.first -> defined)
                symt.push_back(p);
    }
}

void Linker::MergeSections()
{
    std::map<std::string, int> hashMap = {};
    for (int i = 0; i < absFiles.size(); i++)
    {
        for (int j = 0; j < absFiles[i].sections.size(); j++)
        {
            auto curr = absFiles[i].sections[j];
            std::string sectionName = curr->name;
            if (hashMap.find(sectionName) == hashMap.end())
            {
                hashMap[sectionName] = sect.size();
                sect.push_back(curr);
                continue;
            }
            /*
            Uzimamo trenutne velicine sekcije i samog bazena kako bazicne offsete za povecanje.
            PoolOffset se dodaje samo simbolima koji koriste bazen (isBig == true)
            */
            auto& dst = sect[hashMap[sectionName]];
            int offset = dst->data.size();
            int poolOffset = dst->literalPool.size();

            for (auto& sym : symt)
            {
                if (sym.second == i && sym.first->section == sectionName)
                {
                    sym.first->offset += offset;
                    if (!sym.first->isConstant)
                    {
                        sym.first->value += poolOffset;
                    }
                }
            }

            sect[hashMap[sectionName]] = Section::MergeSections(sect[hashMap[sectionName]], curr);
        }
    }
}

void Linker::PlaceSections()
{
    int maxAddress = 0;
    for (auto& sec : sect)
    {
        // skip sekcija koje nisu eksplicitno postavljene kroz argumente linkera
        if (sectionPositions.find(sec->name) == sectionPositions.end())
            continue;

        uint32_t sectionStartAddress = sectionPositions[sec->name];
        uint32_t sectionEndAddress = sec->startAddress + sec->data.size() + sec->literalPool.size();
        // gledamo da li je place komanda iskoriscena pogresno, tj. da li pokusavamo da smestimo neku sekciju u sred neke druge sekcije
        auto lookupIter = std::find_if(sectionPositions.begin(), sectionPositions.end(),
            [&](std::pair<std::string, int>& keyValue) { return keyValue.second < sectionEndAddress && keyValue.second > sectionStartAddress; });

        if (lookupIter != sectionPositions.end())
            throw std::exception();

        maxAddress = std::max<uint32_t>(maxAddress, sectionEndAddress);
        sec->startAddress = sectionStartAddress;
    }

    for (auto& sec : sect)
    {
        // skip sekcija koje su postavljene u prethodnoj petlji
        if (sectionPositions.find(sec->name) != sectionPositions.end())
            continue;

        // redjamo preostale sekcije redom nakon najvise
        sec->startAddress = maxAddress;
        maxAddress += sec->data.size() + sec->literalPool.size();
    }

    // nakon sto su sekcije dobile nove startne adrese, treba da azuriramo sve simbole
    for (auto& sym : symt)
    {
        auto sectionIter = std::find_if(sect.begin(), sect.end(),
            [&](Section::s_ptr& sec) { return sec->name == sym.first->section; });

        sym.first->offset += (*sectionIter)->startAddress;
    }
}

void Linker::ResolveRelocations()
{
    for (auto& sec : sect)
    {
        // pool relokacije su uvek tipa REL32_ABS i uvek je rel->sectionName != currentSection->name
        for (auto& rel : sec->poolRelocations)
        {
            auto symbolIt = std::find_if(symt.begin(), symt.end(), [&](std::pair<Symbol::s_ptr, int>& data) { return data.first->name == rel->symbolName; });
            if (symbolIt == symt.end())
                throw std::exception();

            auto sectionIt = std::find_if(sect.begin(), sect.end(), [&](const Section::s_ptr& s) { return s->name == symbolIt->first->section; });
            if (sectionIt == sect.end())
                throw std::exception();

            uint32_t value = (*sectionIt)->ReadPoolEntry(symbolIt->first->value);
            uint32_t entry = Section::AddressToPoolEntry(rel->offset);
            sec->WritePoolEntry(value, entry);
            symbolIt->first->value = entry;
        }

        for (auto& rel : sec->sectionRelocations)
        {
            /// pronadji simbol u agregiranoj tabeli simbola
            auto symbolIt = std::find_if(symt.begin(), symt.end(), [&](std::pair<Symbol::s_ptr, int>& data) { return data.first->name == rel->symbolName; });
            if (symbolIt == symt.end())
                throw std::exception();

            Symbol::s_ptr symbol = symbolIt->first;
            // nadji sekciju u kojoj se nalazi simbol
            auto sectionIt = std::find_if(sect.begin(), sect.end(), [&](const Section::s_ptr& s) { return s->name == symbolIt->first->section; });
            if (sectionIt == sect.end())
                throw std::exception();

            Section::s_ptr symbolSection = *sectionIt;

            switch (rel->type)
            {
                case eRelocationType::REL12_PC:
                {
                    // relokaciju koju pravimo kada treba da upisemo offset do ulaza u bazen literala
                    uint32_t poolAddress = rel->addend;
                    uint32_t displacement = sec->data.size() - rel->offset + poolAddress - 4;

                    sec->WriteInstructionDisplacement(rel->offset, displacement);
                    break;
                }
                case eRelocationType::REL32_ABS:
                {
                    if (symbol->isConstant)
                    {
                        std::vector<uint8_t> data = IntToByteArray(symbol->value);
                        sec->WriteData(rel->offset, data);

                        break;
                    }

                    uint32_t value = symbolSection->ReadPoolEntry(symbol->value);
                    std::vector<uint8_t> data = IntToByteArray(value);
                    sec->WriteData(rel->offset, data);

                    break;
                }
                default:
                    throw std::exception();
            }
        }
    }
}

void Linker::CreateOutput()
{
    
}
