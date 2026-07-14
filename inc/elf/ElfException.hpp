#ifndef ELF_EXCEPTION_HPP
#define ELF_EXCEPTION_HPP

#include <exception>
#include <string>
#include <map>

enum ElfExceptionType
{
    ELF_BINARY_FORMAT_WRONG,    // fajl nije validan ELF

    ELF_EHDR_NOT_FOUND,         // fajl je prekratak za header
    ELF_EHDR_CORRUPTED,         // magic number ili verzija nisu ispravni

    ELF_SHDRT_NOT_FOUND,        // e_shoff pokazuje van fajla
    ELF_SHDRT_CORRUPTED,        // e_shnum ili e_shentsize nekonzistentni

    ELF_SYMTAB_NOT_FOUND,       // nema SHT_SYMTAB sekcije
    ELF_SYMTAB_CORRUPTED,       // sh_size nije višekratnik od sizeof(Elf64_Sym)

    ELF_STRTAB_NOT_FOUND,       // .strtab sekcija nedostaje
    ELF_STRTAB_OUT_OF_BOUNDS,   // st_name pokazuje van strtab-a

    ELF_SECTION_OUT_OF_BOUNDS,  // sh_offset + sh_size prelazi kraj fajla
    ELF_SECTION_NOT_FOUND,      // tražena sekcija po imenu ne postoji

    ELF_RELA_CORRUPTED,         // sh_size nije višekratnik od sizeof(Elf64_Rela)
    ELF_RELA_SYM_OUT_OF_RANGE,  // indeks simbola u r_info van opsega symtab-a

    ELF_UNDEFINED_EXCEPTION,    // nedefinisana greska, user error
};

static std::map<ElfExceptionType, std::string> ElfExceptionMapping
{
    { ELF_BINARY_FORMAT_WRONG,   "Invalid ELF binary format"                                        },

    { ELF_EHDR_NOT_FOUND,        "File is too short to contain an ELF header"                       },
    { ELF_EHDR_CORRUPTED,        "ELF header is corrupted (invalid magic number or version)"        },

    { ELF_SHDRT_NOT_FOUND,       "Section header table offset (e_shoff) is out of file bounds"      },
    { ELF_SHDRT_CORRUPTED,       "Section header table is inconsistent (e_shnum or e_shentsize)"    },

    { ELF_SYMTAB_NOT_FOUND,      "No SHT_SYMTAB section found in the file"                         },
    { ELF_SYMTAB_CORRUPTED,      "Symbol table size is not a multiple of sizeof(Elf64_Sym)"         },

    { ELF_STRTAB_NOT_FOUND,      "String table (.strtab) section is missing"                       },
    { ELF_STRTAB_OUT_OF_BOUNDS,  "Symbol name offset (st_name) is out of string table bounds"      },

    { ELF_SECTION_OUT_OF_BOUNDS, "Section data (sh_offset + sh_size) exceeds end of file"          },
    { ELF_SECTION_NOT_FOUND,     "Requested section name not found in section header table"        },

    { ELF_RELA_CORRUPTED,        "Relocation table size is not a multiple of sizeof(Elf64_Rela)"   },
    { ELF_RELA_SYM_OUT_OF_RANGE, "Symbol index in relocation entry (r_info) is out of symtab range"},

    { ELF_UNDEFINED_EXCEPTION,   "Undefined exception"},
};


class ElfException : public std::exception
{
public:
    explicit ElfException(const ElfExceptionType& ex) : excpt(ex) {}

    const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override
    {
        if (ElfExceptionMapping.find(excpt) == ElfExceptionMapping.end())
            return ElfExceptionMapping[ELF_UNDEFINED_EXCEPTION].c_str();
        return ElfExceptionMapping[excpt].c_str();
    }

private:
    ElfExceptionType excpt;
};

#endif