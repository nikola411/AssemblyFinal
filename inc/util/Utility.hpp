#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "DataTypes.hpp"
// General utility

eGPR GPRStringToEnum(std::string reg);
eCSR CSRStringToEnum(std::string csr);
std::vector<uint8_t> IntToByteArray(const uint32_t& value);
std::vector<uint8_t> ShortToByteArray(const uint16_t& value);
uint32_t LiteralStringToInt(const std::string& value);
bool Compare(const std::vector<uint8_t>& first, const std::vector<uint8_t>& second);

#endif