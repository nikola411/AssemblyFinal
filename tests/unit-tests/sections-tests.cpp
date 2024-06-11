#include "gtest/gtest.h"
#include <iostream>

#include "Utility.hpp"

static bool isEqual(const std::vector<uint8_t>& first, const std::vector<uint8_t>& second)
{
    if (first.size() != second.size())
        return false;

    for (long unsigned int i = 0; i < first.size(); ++i)
    {
        if (first[i] != second[i])
            return false;
    }

    return true;
}

TEST(SectionTests, ConstructorTest)
{
    ASSERT_NO_THROW(new Section());
}

TEST(SectionTests, SectionCreation)
{
    Section section;
    
    ASSERT_EQ(section.name, std::string());
    ASSERT_EQ(section.data, std::vector<uint8_t>());
    ASSERT_EQ(section.locationCounter, 0);
    ASSERT_EQ(section.literalPool, std::vector<uint8_t>());
}

TEST(SectionTests, AppendDataValues)
{
    Section section;
    std::vector<uint8_t> dataToInsert = {1, 2, 3, 4, 5, 6, 7, 8};
    section.AppendData(std::vector<uint8_t>(dataToInsert.begin(), dataToInsert.begin() + 4));

    ASSERT_EQ(section.data.size(), 4);
    
    bool isEqual = false;
    for (long unsigned int i = 0; i < section.data.size(); ++i)
        if (!(isEqual = section.data[i] == dataToInsert[i]))
            break;
    
    ASSERT_TRUE(isEqual);
}

TEST(SectionTests, AppendDataLocationCounter)
{
    Section section;

    std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7, 8};
    section.AppendData(data);

    ASSERT_EQ(section.locationCounter, 8);
}

TEST(SectionTests, WriteDataValues)
{
    Section section;
    std::vector<uint8_t> startingData = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    section.data = startingData;

    section.WriteData(0, {1, 1, 1, 1});
    ASSERT_TRUE(isEqual(section.data, {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));

    section.WriteData(8, {8, 8, 8, 8});
    ASSERT_TRUE(isEqual(section.data, {1, 1, 1, 1, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0}));

    section.data = {};
    ASSERT_THROW(section.WriteData(18, {1, 1, 1, 1}), std::exception);
}

TEST(SectionTests, AddressAndPoolEntry)
{
    std::pair<int, int> addressEntryPair = {0, 0};
    auto entry = Section::AddressToPoolEntry(addressEntryPair.first);
    auto address = Section::PoolEntryToAddress(addressEntryPair.second);
    ASSERT_EQ(entry, addressEntryPair.second);
    ASSERT_EQ(address, addressEntryPair.first);

    addressEntryPair = {4, 1};
    entry = Section::AddressToPoolEntry(addressEntryPair.first);
    address = Section::PoolEntryToAddress(addressEntryPair.second);
    ASSERT_EQ(entry, addressEntryPair.second);
    ASSERT_EQ(address, addressEntryPair.first);

    addressEntryPair = {5, 1};
    entry = Section::AddressToPoolEntry(addressEntryPair.first);
    address = Section::PoolEntryToAddress(addressEntryPair.second);
    ASSERT_EQ(entry, addressEntryPair.second);
    ASSERT_EQ(address, 4);

    addressEntryPair = {15, 3};
    entry = Section::AddressToPoolEntry(addressEntryPair.first);
    address = Section::PoolEntryToAddress(addressEntryPair.second);
    ASSERT_EQ(entry, addressEntryPair.second);
    ASSERT_EQ(address, 12);

    addressEntryPair = {14, 3};
    entry = Section::AddressToPoolEntry(addressEntryPair.first);
    address = Section::PoolEntryToAddress(addressEntryPair.second);
    ASSERT_EQ(entry, addressEntryPair.second);
    ASSERT_EQ(address, 12);
}

TEST(SectionTests, InsertLiteralInPool)
{
    Section section;
    std::vector<uint8_t> startingPool = {};
    section.literalPool = startingPool;

    section.InsertLiteralInPool(0xEEFF);
    ASSERT_TRUE(isEqual(section.literalPool, { 0x00, 0x00, 0xEE, 0xFF }));

    section.InsertLiteralInPool(0x8888);
    ASSERT_TRUE(isEqual(section.literalPool, { 0x00, 0x00, 0xEE, 0xFF, 0x00, 0x00, 0x88, 0x88}));
}

TEST(SectionTests, IsLiteralPresentInPool)
{
    Section section;
    std::vector<uint8_t> startingPool = {};
    section.literalPool = startingPool;

    ASSERT_EQ(section.IsLiteralPresentInPool(0xFFFF), -1);

    section.literalPool = {0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x88, 0x88, 0x00, 0x00, 0x92, 0x4F };
    ASSERT_EQ(section.IsLiteralPresentInPool(0xFFFF), 0);
    ASSERT_EQ(section.IsLiteralPresentInPool(0x8888), 4);
    ASSERT_EQ(section.IsLiteralPresentInPool(0xFF88), -1);
    ASSERT_EQ(section.IsLiteralPresentInPool(0x924F), 8);
}