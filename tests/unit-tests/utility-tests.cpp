#include "gtest/gtest.h"
#include <iostream>

#include "Utility.hpp"

namespace UtilityTests
{
    TEST(SymbolTableTests, EntryFound)
    {
        SymbolTable table = {
            std::make_shared<Symbol>("a", "sec_a", 0),
            std::make_shared<Symbol>("b", "sec_b", 0),
            std::make_shared<Symbol>("c", "sec_c", 0)
        };

        auto result = GetSymbol(table, "b");
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->name, "b");
    }

    TEST(SymbolTableTests, EntryNotFound)
    {
        SymbolTable table = {
            std::make_shared<Symbol>("a", "sec_a", 0),
            std::make_shared<Symbol>("b", "sec_b", 0),
            std::make_shared<Symbol>("c", "sec_c", 0)
        };

        auto result = GetSymbol(table, "d");
        EXPECT_EQ(result, nullptr);
    }

    TEST(IntToByteArrayTests, ConvertZero)
    {
        auto result = IntToByteArray(0);
        std::vector<uint8_t> expected = {0, 0, 0, 0};
        EXPECT_EQ(result, expected);
    }

    TEST(IntToByteArrayTests, ConvertMaxValue)
    {
        auto result = IntToByteArray(0xFFFFFFFF);
        std::vector<uint8_t> expected = {0xFF, 0xFF, 0xFF, 0xFF};
        EXPECT_EQ(result, expected);
    }

    TEST(IntToByteArrayTests, ConvertMixedValue)
    {
        auto result = IntToByteArray(0x12345678);
        std::vector<uint8_t> expected = {0x12, 0x34, 0x56, 0x78};
        EXPECT_EQ(result, expected);
    }

    TEST(LiteralStringToIntTests, DecimalString)
    {
        auto result = LiteralStringToInt("12345");
        EXPECT_EQ(result, 12345);
    }

    TEST(LiteralStringToIntTests, HexString)
    {
        auto result = LiteralStringToInt("0x1A3F");
        EXPECT_EQ(result, 0x1A3F);
    }

    TEST(CompareVectorsTests, CompareSameVectors)
    {
        std::vector<uint8_t> toCompare = {1, 1, 1, 1};
        EXPECT_TRUE(Compare(toCompare, toCompare));
    }

    TEST(CompareVectorsTests, CompareDifferentSizeVectors)
    {
        std::vector<uint8_t> first = {1, 2};
        std::vector<uint8_t> second = {1, 2, 3, 4};
        EXPECT_FALSE(Compare(first, second));
    }

    TEST(CompareVectorsTests, CompareDifferentVectorsSameSize)
    {
        std::vector<uint8_t> first = {1, 2, 3, 4};
        std::vector<uint8_t> second = {1, 2, 3, 3};

        EXPECT_FALSE(Compare(first, second));
    }

    TEST(CompareVectorsTests, CompareEmptyVectors)
    {
        EXPECT_TRUE(Compare({}, {}));
    }

} // UtilityTests
