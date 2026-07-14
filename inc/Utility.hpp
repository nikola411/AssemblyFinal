#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "ParserTypes.hpp"

#include <string>
#include <vector>
#include <cstdint>

// enum helpers
eGPR GPRStringToEnum(std::string reg);
eCSR CSRStringToEnum(std::string csr);

// int helpers
std::vector<uint8_t> IntToByteArray(const uint32_t& value);
std::vector<uint8_t> ShortToByteArray(const uint16_t& value);

// string helpers
uint32_t LiteralStringToInt(const std::string& value);
bool Compare(const std::vector<uint8_t>& first, const std::vector<uint8_t>& second);
bool StartsWith(const std::string& input, const std::string& c1);
bool EndsWith(const std::string& in, const std::string& c1);
std::vector<std::string> Split(const std::string& input, char delim);

// file helpers
std::vector<uint8_t> ReadBinaryFile(const std::string& file);

#endif