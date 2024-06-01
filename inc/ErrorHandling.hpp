#ifndef ERROR_HANDLING_HPP
#define ERROR_HANDLING_HPP

#include <exception>

class AssemblyException : public std::exception
{
    const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW;
};


#endif