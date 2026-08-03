/*
 * Unit tests for the linker's section-merge step (Section::MergeSections)
 * and the relocation-update helpers it relies on.
 *
 * These cover the core guarantees of Linker::MergeSections step 2 ("spoji
 * sekcije") from src/linker/Linker.cpp:
 *   - data / pool concatenation and location counter
 *   - section relocation OFFSET shift (only `second`, `first` untouched)
 *   - section relocation ADDEND shift for REL12_PC (only `second`, only PC12)
 *   - pool relocation OFFSET shift (only `second`)
 *   - pool / displacement helper units
 *
 * The higher-level Linker members (MergeSections, ResolveRelocations, ...)
 * read private state (absFiles, symt, sect) and need integration tests with
 * real .o files, so they are not exercised here.
 *
 * Groups:
 *   [1] Section::MergeSections   – data / pool / location counter
 *   [2] Section relocation OFFSET shift
 *   [3] Section relocation ADDEND shift (REL12_PC)
 *   [4] Pool relocation OFFSET shift
 *   [5] Pool + displacement helper units
 */

#include "Section.hpp"
#include "Utility.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// ── tiny test harness ─────────────────────────────────────────────────────────

static int g_passed = 0;
static int g_failed = 0;

#define RUN(fn)                                                                 \
    do {                                                                        \
        std::cout << "  " << #fn << " ... ";                                    \
        try {                                                                   \
            fn();                                                               \
            std::cout << "\033[32mPASS\033[0m\n";                               \
            g_passed++;                                                         \
        } catch (const std::exception& _e) {                                    \
            std::cout << "\033[31mFAIL\033[0m: " << _e.what() << "\n";          \
            g_failed++;                                                         \
        }                                                                       \
    } while (0)

#define ASSERT(cond)                                                            \
    do {                                                                        \
        if (!(cond))                                                            \
            throw std::runtime_error("assert(" #cond ") at line "               \
                                     + std::to_string(__LINE__));               \
    } while (0)

#define ASSERT_EQ(a, b)                                                         \
    do {                                                                        \
        auto _a = (a);                                                          \
        auto _b = (b);                                                          \
        if (_a != _b)                                                           \
            throw std::runtime_error(                                           \
                std::string("assert_eq(") + #a + ", " + #b + ") -> ("          \
                + std::to_string((long long)_a) + " != "                        \
                + std::to_string((long long)_b) + ") at line "                  \
                + std::to_string(__LINE__));                                    \
    } while (0)

// ── builders ──────────────────────────────────────────────────────────────────

// Build a section and drive data through AppendData so locationCounter tracks it.
static Section::s_ptr make_section(const std::string& name,
                                   std::vector<uint8_t> data = {})
{
    auto s  = std::make_shared<Section>();
    s->name = name;
    if (!data.empty())
        s->AppendData(data);
    return s;
}

// Append `count` 4-byte pool entries, each holding `value`.
static void add_pool_entries(const Section::s_ptr& s, int count, uint32_t value = 0)
{
    for (int i = 0; i < count; i++)
        s->InsertLiteralInPool(value);
}

static Relocation::s_ptr find_rel(const RelocationTable& table,
                                  const std::string& name)
{
    for (const auto& r : table)
        if (r->symbolName == name)
            return r;
    return nullptr;
}

// ── [1] MergeSections: data / pool / location counter ─────────────────────────

static void test_merge_concatenates_data()
{
    auto first  = make_section("text", { 0xDE, 0xAD });
    auto second = make_section("text", { 0xBE, 0xEF });

    auto result = Section::MergeSections(first, second);

    ASSERT_EQ(result->data.size(), 4u);
    ASSERT_EQ(result->data[0], 0xDE);
    ASSERT_EQ(result->data[1], 0xAD);
    ASSERT_EQ(result->data[2], 0xBE);
    ASSERT_EQ(result->data[3], 0xEF);
}

static void test_merge_location_counter_is_sum()
{
    auto first  = make_section("text", { 1, 2, 3, 4 });     // loc = 4
    auto second = make_section("text", { 5, 6 });           // loc = 2

    auto result = Section::MergeSections(first, second);

    ASSERT_EQ(result->locationCounter, 6u);
}

static void test_merge_concatenates_pool_content()
{
    auto first  = make_section("text");
    auto second = make_section("text");
    first->InsertLiteralInPool(0x11111111);
    second->InsertLiteralInPool(0x22222222);

    auto result = Section::MergeSections(first, second);

    ASSERT_EQ(result->literalPool.size(), 8u);
    ASSERT_EQ(result->ReadPoolEntry(0), 0x11111111u);
    ASSERT_EQ(result->ReadPoolEntry(1), 0x22222222u);
}

// ── [2] Section relocation OFFSET shift ───────────────────────────────────────

static void test_merge_shifts_second_section_reloc_offset()
{
    auto first  = make_section("text", std::vector<uint8_t>(8, 0)); // data size 8
    auto second = make_section("text");
    second->AddSectionRelocation(eRelocationType::REL32_ABS, "B", 0);

    auto result = Section::MergeSections(first, second);

    auto b = find_rel(result->sectionRelocations, "B");
    ASSERT(b != nullptr);
    ASSERT_EQ(b->offset, 8u);   // 0 + first->data.size()
}

static void test_merge_keeps_first_section_reloc_offset()
{
    auto first = make_section("text", std::vector<uint8_t>(8, 0));
    first->AddSectionRelocation(eRelocationType::REL32_ABS, "A", 4);
    auto second = make_section("text", std::vector<uint8_t>(4, 0));

    auto result = Section::MergeSections(first, second);

    auto a = find_rel(result->sectionRelocations, "A");
    ASSERT(a != nullptr);
    ASSERT_EQ(a->offset, 4u);   // first's offset must not move
}

// ── [3] Section relocation ADDEND shift (REL12_PC only, second only) ──────────

static void test_merge_shifts_second_rel12_addend_by_first_pool()
{
    auto first = make_section("text");
    add_pool_entries(first, 2);                              // pool size = 8
    auto second = make_section("text");
    second->AddSectionRelocation(eRelocationType::REL12_PC, "B", 0, /*addend=*/4);

    auto result = Section::MergeSections(first, second);

    auto b = find_rel(result->sectionRelocations, "B");
    ASSERT(b != nullptr);
    ASSERT_EQ(b->addend, 12);   // 4 + first->literalPool.size()
}

// Regression: the old code shifted addends on `result` (first + second),
// wrongly bumping first's REL12_PC addend on every merge.
static void test_merge_keeps_first_rel12_addend()
{
    auto first = make_section("text");
    add_pool_entries(first, 2);                              // pool size = 8
    first->AddSectionRelocation(eRelocationType::REL12_PC, "A", 0, /*addend=*/4);
    auto second = make_section("text");
    second->AddSectionRelocation(eRelocationType::REL12_PC, "B", 0, /*addend=*/0);

    auto result = Section::MergeSections(first, second);

    auto a = find_rel(result->sectionRelocations, "A");
    auto b = find_rel(result->sectionRelocations, "B");
    ASSERT(a != nullptr && b != nullptr);
    ASSERT_EQ(a->addend, 4);    // first: unchanged
    ASSERT_EQ(b->addend, 8);    // second: shifted by first pool size
}

static void test_merge_does_not_shift_rel32_addend()
{
    auto first = make_section("text");
    add_pool_entries(first, 2);                              // pool size = 8
    auto second = make_section("text");
    second->AddSectionRelocation(eRelocationType::REL32_ABS, "C", 0, /*addend=*/7);

    auto result = Section::MergeSections(first, second);

    auto c = find_rel(result->sectionRelocations, "C");
    ASSERT(c != nullptr);
    ASSERT_EQ(c->addend, 7);    // REL32_ABS addend is left alone
}

// ── [4] Pool relocation OFFSET shift ──────────────────────────────────────────

static void test_merge_shifts_second_pool_reloc_offset()
{
    auto first = make_section("text");
    add_pool_entries(first, 2);                              // pool size = 8
    auto second = make_section("text");
    second->AddPoolRelocation(eRelocationType::REL32_ABS, "B", 0);

    auto result = Section::MergeSections(first, second);

    auto b = find_rel(result->poolRelocations, "B");
    ASSERT(b != nullptr);
    ASSERT_EQ(b->offset, 8u);   // 0 + first->literalPool.size()
}

static void test_merge_keeps_first_pool_reloc_offset()
{
    auto first = make_section("text");
    add_pool_entries(first, 2);
    first->AddPoolRelocation(eRelocationType::REL32_ABS, "A", 0);
    auto second = make_section("text");

    auto result = Section::MergeSections(first, second);

    auto a = find_rel(result->poolRelocations, "A");
    ASSERT(a != nullptr);
    ASSERT_EQ(a->offset, 0u);   // first's pool reloc must not move
}

// ── [5] pool + displacement helper units ──────────────────────────────────────

static void test_insert_and_read_pool_entry_roundtrip()
{
    auto s = make_section("text");
    auto idx0 = s->InsertLiteralInPool(0xAABBCCDD);
    auto idx1 = s->InsertLiteralInPool(0x01020304);

    ASSERT_EQ(idx0, 0u);
    ASSERT_EQ(idx1, 1u);
    ASSERT_EQ(s->ReadPoolEntry(idx0), 0xAABBCCDDu);
    ASSERT_EQ(s->ReadPoolEntry(idx1), 0x01020304u);
}

static void test_pool_entry_address_roundtrip()
{
    ASSERT_EQ(Section::PoolEntryToAddress(2), 8u);
    ASSERT_EQ(Section::AddressToPoolEntry(8), 2u);
}

static void test_is_literal_present_in_pool()
{
    auto s = make_section("text");
    s->InsertLiteralInPool(0x11223344);

    // sentinel is -1 but the return type is unsigned; real callers store it in
    // an int first (see Assembly::GetLiteralValue), so mirror that here.
    int present = s->IsLiteralPresentInPool(0x11223344);
    int absent  = s->IsLiteralPresentInPool(0xDEADBEEF);
    ASSERT_EQ(present, 0);    // first entry
    ASSERT_EQ(absent, -1);    // absent
}

static void test_write_instruction_displacement_low_byte()
{
    // jmp-style instruction bytes; write displacement 0x005 (cf. e2e 05)
    auto s = make_section("text", { 0x00, 0x00, 0xF0, 0x38 });
    s->WriteInstructionDisplacement(0, 0x005);

    ASSERT_EQ(s->data[0], 0x05);
    ASSERT_EQ(s->data[1], 0x00);
    ASSERT_EQ(s->data[2], 0xF0);
    ASSERT_EQ(s->data[3], 0x38);
}

static void test_write_instruction_displacement_high_nibble()
{
    auto s = make_section("text", { 0x00, 0x00, 0xF0, 0x38 });
    s->WriteInstructionDisplacement(0, 0x123);

    ASSERT_EQ(s->data[0], 0x23);   // low byte
    ASSERT_EQ(s->data[1], 0x01);   // high nibble OR'd in
}

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "\n\033[1m[1] Section::MergeSections — data / pool\033[0m\n";
    RUN(test_merge_concatenates_data);
    RUN(test_merge_location_counter_is_sum);
    RUN(test_merge_concatenates_pool_content);

    std::cout << "\n\033[1m[2] Section relocation OFFSET shift\033[0m\n";
    RUN(test_merge_shifts_second_section_reloc_offset);
    RUN(test_merge_keeps_first_section_reloc_offset);

    std::cout << "\n\033[1m[3] Section relocation ADDEND shift (REL12_PC)\033[0m\n";
    RUN(test_merge_shifts_second_rel12_addend_by_first_pool);
    RUN(test_merge_keeps_first_rel12_addend);
    RUN(test_merge_does_not_shift_rel32_addend);

    std::cout << "\n\033[1m[4] Pool relocation OFFSET shift\033[0m\n";
    RUN(test_merge_shifts_second_pool_reloc_offset);
    RUN(test_merge_keeps_first_pool_reloc_offset);

    std::cout << "\n\033[1m[5] pool + displacement helpers\033[0m\n";
    RUN(test_insert_and_read_pool_entry_roundtrip);
    RUN(test_pool_entry_address_roundtrip);
    RUN(test_is_literal_present_in_pool);
    RUN(test_write_instruction_displacement_low_byte);
    RUN(test_write_instruction_displacement_high_nibble);

    int total = g_passed + g_failed;
    std::cout << "\n" << std::string(50, '=') << "\n";
    if (g_failed == 0)
        std::cout << "\033[32m\033[1m";
    else
        std::cout << "\033[31m\033[1m";
    std::cout << "  " << g_passed << " passed  |  "
              << g_failed << " failed  /  " << total << " total"
              << "\033[0m\n\n";

    return g_failed == 0 ? 0 : 1;
}
