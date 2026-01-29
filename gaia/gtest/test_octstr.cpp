#include <gtest/gtest.h>

#include "gaia_octstr.hpp"
#include "small_vector.hpp"
#include <deque>
#include <list>
#include <random>
#include <unordered_map>
#include <unordered_set>

namespace
{
uint8_t test_byte1 = 1U;
uint8_t test_byte2 = 2U;
uint8_t test_byte3 = 4U;
uint8_t test_byte4 = 8U;
uint8_t test_byte5 = 16U;
uint8_t test_byte6 = 32U;
uint8_t test_byte7 = 64U;
uint8_t test_byte8 = 128U;
uint8_t test_byte9 = 0U;
uint8_t test_byte10 = 250U;

std::random_device rd{};
std::mt19937_64 mt{rd()};
} // namespace
namespace autocrypt
{

TEST(OctStr, MakeOctStrWithBytes)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    Bytes test_string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, OctStr{test_string});
}

TEST(OctStr, MakeOctStrWithUint8Vector)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::vector<uint8_t> test_string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, OctStr{test_string});
}

TEST(OctStr, MakeOctStrWithBasicStringUint8)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::basic_string<uint8_t> test_string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, OctStr{test_string});
}

TEST(OctStr, MakeOctStrWithOtherVectorLikeContainer)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    gch::small_vector<uint8_t, 64> test_string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, OctStr{test_string});
}

TEST(OctStr, MakeOctStrWithUint8Deque)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::deque<uint8_t> test_string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, OctStr{test_string});
}

TEST(OctStr, MakeOctStrWithUint8List)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::list<uint8_t> test_string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, OctStr{test_string});
}

TEST(OctStr, MakeOctStrWithUint8Array)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::array<uint8_t, 10> test_string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, OctStr{test_string});
}

TEST(OctStr, MakeOctStrWithString)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    const std::array<std::uint8_t, 10> bytes{
        1, 2, 4, 8, 16, 32, 64, 0x80, 0, 0xFA
    };

    const auto test_string =
        std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size());

    ASSERT_EQ(test_oct, OctStr{test_string});
}

TEST(OctStr, MakeOctStrWithBytesSSO)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct.capacity(), 15);
}

TEST(OctStr, MakeOctStrWithConstPointerAndSize)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    Bytes test_string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, OctStr(test_string.data(), 10));
}

TEST(OctStr, MakeOctStrWithVoidPointerAndSize)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    std::vector<uint16_t> test_string{513U, 2052U, 8208U, 32832U, 64000U};
    ASSERT_EQ(test_oct, OctStr(test_string.data(), 10));
}

TEST(OctStr, MakeOctStrWithCharPointerAndSize)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    const std::array<std::uint8_t, 10> bytes{
        1, 2, 4, 8, 16, 32, 64, 0x80, 0, 0xFA
    };

    const auto test_string =
        std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size());

    ASSERT_EQ(test_oct, OctStr(test_string.data(), test_string.size()));
}

TEST(OctStr, MakeOctStrWithIterators)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, (OctStr{test_oct.begin(), test_oct.end()}));
}

TEST(OctStr, MakeOctStrWithOtherIterators)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::deque<uint8_t> test_oct_deque{test_oct};
    ASSERT_EQ(test_oct, (OctStr{test_oct_deque.begin(), test_oct_deque.end()}));
}

TEST(OctStr, MakeOctStrWithSingleUint8)
{
    OctStr test_single_uint8_oct{test_byte2};
    uint8_t single_uint8 = 2U;
    ASSERT_EQ(test_single_uint8_oct, OctStr(single_uint8));
}

TEST(OctStr, MakeOctStrWithSingleUint16)
{
    OctStr test_single_uint16_oct{test_byte1, test_byte2};
    ASSERT_EQ(test_single_uint16_oct, OctStr(static_cast<uint16_t>(258U)));
}

TEST(OctStr, MakeOctStrWithSingleUint32)
{
    OctStr test_single_uin32_oct{test_byte1, test_byte2, test_byte3, test_byte4};
    ASSERT_EQ(test_single_uin32_oct, OctStr(static_cast<uint32_t>(16909320U)));
}

TEST(OctStr, AssignmentOperatorWithBytes)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    Bytes test_string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    OctStr string = test_string;
    ASSERT_EQ(test_oct, string);
}

TEST(OctStr, AssignmentOperatorWithUint8Vector)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    std::vector<uint8_t> test_string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    OctStr string = test_string;
    ASSERT_EQ(test_oct, string);
}

TEST(OctStr, AssignmentOperatorWithInitializerList)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{};
    string = {test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, string);
}

TEST(OctStr, AssignCountAndValue)
{
    OctStr string{};
    OctStr answer{test_byte5, test_byte5, test_byte5};
    string.assign(3, test_byte5);
    ASSERT_EQ(answer, string);
}

TEST(OctStr, AssignInputIterator)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{};
    OctStr answer{test_byte3, test_byte4, test_byte5, test_byte6, test_byte7};
    string.assign(test_oct.begin() + 2, test_oct.begin() + 7);
    ASSERT_EQ(answer, string);
}

TEST(OctStr, AssignInitializerList)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{};
    string.assign({test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10});
    ASSERT_EQ(test_oct, string);
}

TEST(OctStr, AtNoThrow)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(250, test_oct.at(9));
}

TEST(OctStr, AtThrow)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    EXPECT_ANY_THROW(test_oct.at(10));
}

TEST(OctStr, AccessOperator)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(250, test_oct[9]);
}

TEST(OctStr, AccessOperatorNoThrow)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_NO_THROW(test_oct[10]);
}

TEST(OctStr, AccessOperatorEqualsToAtIfNoThrow)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    std::uniform_int_distribution<uint64_t> dist(0, 9);
    auto random_number = dist(mt);
    ASSERT_EQ(test_oct.at(random_number), test_oct[random_number]);
}

TEST(OctStr, Front)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(1U, test_oct.front());
}

TEST(OctStr, Back)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(250U, test_oct.back());
}

TEST(OctStr, Data)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct, (OctStr{test_oct.data(), test_oct.size()}));
}

TEST(OctStr, Byte)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct, OctStr{test_oct.bytes()});
}

TEST(OctStr, Begin)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.begin(), test_oct.bytes().begin());
}

TEST(OctStr, Cbegin)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.cbegin(), test_oct.bytes().cbegin());
}

TEST(OctStr, End)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.end(), test_oct.bytes().end());
}

TEST(OctStr, Cend)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.cend(), test_oct.bytes().cend());
}

TEST(OctStr, Rbegin)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.rbegin(), test_oct.bytes().rbegin());
}

TEST(OctStr, Crbegin)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.crbegin(), test_oct.bytes().crbegin());
}

TEST(OctStr, Rend)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.rend(), test_oct.bytes().rend());
}

TEST(OctStr, Crend)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.crend(), test_oct.bytes().crend());
}

TEST(OctStr, EmptyPositive)
{
    OctStr string{};
    string.empty();
    ASSERT_TRUE(string.empty());
}

TEST(OctStr, EmptyNegative)
{
    OctStr string{test_byte1};
    ASSERT_FALSE(string.empty());
}

TEST(OctStr, Size)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.size(), 10);
}

TEST(OctStr, Reserve)
{
    OctStr string{};
    string.reserve(10000);
    ASSERT_EQ(string.capacity(), 10000);
}

TEST(OctStr, ReserveSmallerThanInitialCapacity)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.capacity(), 15);
}

TEST(OctStr, Capacity)
{
    OctStr string{};
    string.insert(0, 10000, 42);
    ASSERT_EQ(string.capacity(), 10000);
}

TEST(OctStr, CapacitySmallerThanInitialCapacity)
{
    OctStr string{};
    ASSERT_EQ(string.capacity(), 15);
}

TEST(OctStr, Clear)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    test_oct.clear();
    ASSERT_TRUE(test_oct.empty());
}

TEST(OctStr, InsertSingleElementIterator)
{
    OctStr string{203U, 233U, 105U, 152U};
    auto itr1 = string.insert(string.begin() + 1, 255U);
    auto itr2 = string.begin() + 1;
    ASSERT_EQ(itr1, itr2);
}

TEST(OctStr, InsertSingleElement)
{
    OctStr string{203U, 233U, 105U, 152U};
    string.insert(string.begin() + 1, 255U);
    OctStr answer{203U, 255U, 233U, 105U, 152U};
    ASSERT_EQ(string, answer);
}

TEST(OctStr, InsertMultipleSingleElementIterator)
{
    OctStr string{203U, 233U, 105U, 152U};
    auto itr1 = string.insert(string.begin() + 1, 3, 255U);
    auto itr2 = string.begin() + 1;
    ASSERT_EQ(itr1, itr2);
}

TEST(OctStr, InsertMultipleSingleElement)
{
    OctStr string{203U, 233U, 105U, 152U};
    string.insert(string.begin() + 1, 3, 255U);
    OctStr answer{203U, 255U, 255U, 255U, 233U, 105U, 152U};
    ASSERT_EQ(string, answer);
}

TEST(OctStr, InsertInputIterator)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{203U, 233U, 105U, 152U};
    string.insert(string.begin() + 1, test_oct.begin(), test_oct.end());
    OctStr answer{203U, test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10, 233U, 105U, 152U};
    ASSERT_EQ(string, answer);
}

TEST(OctStr, EraseSingleElement)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    test_oct.erase(test_oct.begin() + 4);
    OctStr answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, answer);
}

TEST(OctStr, EraseMultipleElement)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    test_oct.erase(test_oct.begin() + 4, test_oct.begin() + 8);
    OctStr answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, answer);
}

TEST(OctStr, Push_back)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    test_oct.push_back(0U);
    OctStr answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10, 0U};
    ASSERT_EQ(test_oct, answer);
}

TEST(OctStr, Pop_back)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    test_oct.pop_back();
    OctStr answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9};
    ASSERT_EQ(test_oct, answer);
}

TEST(OctStr, ResizeDefaultValue)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    test_oct.resize(12);
    OctStr answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10, 0U, 0U};
    ASSERT_EQ(test_oct, answer);
}

TEST(OctStr, ResizeCustomValue)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    test_oct.resize(12, 74U);
    OctStr answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10, 74U, 74U};
    ASSERT_EQ(test_oct, answer);
}

TEST(OctStr, ResizeSmaller)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    test_oct.resize(4);
    OctStr answer{test_byte1, test_byte2, test_byte3, test_byte4};
    ASSERT_EQ(test_oct, answer);
}

TEST(OctStr, ResizeSmallerCustomValue)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    test_oct.resize(4, 74U);
    OctStr answer{test_byte1, test_byte2, test_byte3, test_byte4};
    ASSERT_EQ(test_oct, answer);
}

TEST(OctStr, Swap)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr other{1U, 100U};
    OctStr answer{Bytes(other)};
    test_oct.swap(other);
    ASSERT_EQ(test_oct, answer);
}

TEST(OctStr, InsertIndexCount)
{
    OctStr string{203U, 233U, 105U, 152U};
    OctStr answer{203U, 74U, 74U, 74U, 233U, 105U, 152U};
    ASSERT_EQ(string.insert(1, 3, 74U), answer);
}

TEST(OctStr, InsertConstPointer)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{203U, 233U, 105U, 152U};
    OctStr answer{203U, test_byte2, 233U, 105U, 152U};
    string.insert(string.begin() + 1, test_byte2);
    ASSERT_EQ(string, answer);
}

TEST(OctStr, InsertConstPointerAndCount)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{203U, 233U, 105U, 152U};
    OctStr answer{203U, test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, 233U, 105U, 152U};
    ASSERT_EQ(string.insert(1, test_oct.data(), 5), answer);
}

TEST(OctStr, InsertOtherOctStr)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{203U, 233U, 105U, 152U};
    OctStr answer{203U, test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10, 233U, 105U, 152U};
    ASSERT_EQ(string.insert(1, test_oct), answer);
}

TEST(OctStr, InsertOtherOctStrAndCount)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{203U, 233U, 105U, 152U};
    OctStr answer{203U, test_byte2, test_byte3, test_byte4, test_byte5, 233U, 105U, 152U};
    ASSERT_EQ(string.insert(1, test_oct, 1, 4), answer);
}

TEST(OctStr, EraseIndexCount)
{
    OctStr string{203U, 233U, 105U, 152U};
    OctStr answer{203U};
    ASSERT_EQ(string.erase(1), answer);
}

TEST(OctStr, AppendMultipleElement)
{
    OctStr string{203U};
    OctStr answer{203U, 24U, 24U, 24U};
    ASSERT_EQ(string.append(3, 24U), answer);
}

TEST(OctStr, AppendOctStr)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    OctStr the_other_string{test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, string.append(the_other_string));
}

TEST(OctStr, AppendOctStrWithPos)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    ASSERT_EQ(test_oct, string.append(test_oct, 4));
}

TEST(OctStr, AppendConstPointerAndCount)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr empty{};
    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    ASSERT_EQ(string, empty.append(test_oct.data(), 4));
}

TEST(OctStr, AppendConstPointer)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    OctStr the_other_string{test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, string.append(the_other_string.data(), the_other_string.size()));
}

TEST(OctStr, AppendWithIterator)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    ASSERT_EQ(test_oct, string.append(test_oct.begin() + 4, test_oct.end()));
}

TEST(OctStr, AppendInitializerList)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    ASSERT_EQ(test_oct, string.append({test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10}));
}

TEST(OctStr, AppendOperatorRvalue)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    ASSERT_EQ(test_oct, (string += Bytes{test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10}));
}

TEST(OctStr, AppendOperatorLvalue)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    OctStr the_other_string{test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, string += the_other_string);
}

TEST(OctStr, AppendOperatorSingleUint8)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9};
    ASSERT_EQ(test_oct, string += 250U);
}

TEST(OctStr, AppendOperatorInitializerList)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6};
    ASSERT_EQ(test_oct, (string += {test_byte7, test_byte8, test_byte9, test_byte10}));
}

TEST(OctStr, SubStrNoCount)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr expect{test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    EXPECT_EQ(test_oct.substr(5), expect);
}

TEST(OctStr, SubStrNormal)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr expect{test_byte3, test_byte4, test_byte5, test_byte6};
    EXPECT_EQ(test_oct.substr(2, 4), expect);
}

TEST(OctStr, SubStrPosOutOfRange)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr expect{test_byte7, test_byte8, test_byte9, test_byte10};
    EXPECT_ANY_THROW(test_oct.substr(50, 1));
}

TEST(OctStr, SubStrPosPlusCountOutOfRange)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr expect{test_byte7, test_byte8, test_byte9, test_byte10};
    EXPECT_EQ(test_oct.substr(6, 50), expect);
}

TEST(OctStr, SubStrPosInRangeCountOutOfRange)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr expect{test_byte8, test_byte9, test_byte10};
    EXPECT_EQ(test_oct.substr(test_oct.size() - 3, 20), expect);
}

TEST(OctStr, SubStrPosOutOfRangeCountOutOfRange)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    EXPECT_ANY_THROW(test_oct.substr(test_oct.size() + 3, 20));
}

TEST(OctStr, CompareShotertoLonger)
{
    OctStr byte_test1{test_byte1, test_byte2, test_byte3};
    OctStr byte_test2{test_byte1, test_byte2, test_byte3, test_byte1, test_byte2, test_byte3};

    auto res_byte = byte_test2.compare(byte_test1);

    EXPECT_GT(res_byte, 0);
    EXPECT_TRUE(byte_test1 < byte_test2);
    EXPECT_TRUE(byte_test1 <= byte_test2);

    EXPECT_TRUE(byte_test2 > byte_test1);
    EXPECT_TRUE(byte_test2 >= byte_test1);

    EXPECT_TRUE(byte_test2 != byte_test1);
    EXPECT_FALSE(byte_test2 == byte_test1);
}

TEST(OctStr, CompareLongertoShorter)
{
    OctStr byte_test1{test_byte1, test_byte2, test_byte3};
    OctStr byte_test2{test_byte1, test_byte2, test_byte3, test_byte1, test_byte2, test_byte3};

    auto res_byte = byte_test1.compare(byte_test2);

    EXPECT_LT(res_byte, 0);
}

TEST(OctStr, CompareSame)
{
    OctStr byte_test1{test_byte1, test_byte2, test_byte3};
    OctStr byte_test2{test_byte1, test_byte2, test_byte3};

    auto res_byte = byte_test1.compare(byte_test2);

    EXPECT_EQ(res_byte, 0);
}

TEST(OctStr, CompareDifferent)
{
    OctStr byte_test1{test_byte1, test_byte2, test_byte4};
    OctStr byte_test2{test_byte1, test_byte2, test_byte3};

    auto res_byte = byte_test1.compare(byte_test2);

    EXPECT_GT(res_byte, 0);

    EXPECT_TRUE(byte_test1 > byte_test2);
    EXPECT_TRUE(byte_test1 >= byte_test2);

    EXPECT_TRUE(byte_test2 < byte_test1);
    EXPECT_TRUE(byte_test2 <= byte_test1);

    EXPECT_TRUE(byte_test2 != byte_test1);
    EXPECT_FALSE(byte_test2 == byte_test1);
}

TEST(OctStr, Xor)
{
    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    OctStr the_other_string{test_byte5, test_byte6, test_byte7, test_byte8};
    OctStr answer{17U, 34U, 68U, 136U};
    ASSERT_EQ(answer, string ^ the_other_string);
}

TEST(OctStr, BitLeftShift)
{
    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    OctStr answer{test_byte3, test_byte4, test_byte5, test_byte6};
    ASSERT_EQ(answer, string << 2);
}

TEST(OctStr, BitRightShift)
{
    OctStr string{test_byte3, test_byte4, test_byte5, test_byte6};
    OctStr answer{test_byte1, test_byte2, test_byte3, test_byte4};
    ASSERT_EQ(answer, string >> 2);
}

TEST(OctStr, Concatenate)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    OctStr string{test_byte1, test_byte2, test_byte3, test_byte4};
    OctStr the_other_string{test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    ASSERT_EQ(test_oct, string + the_other_string);
}

// TEST(OctStr, Release)
// {
//     OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

//     Bytes *byte;
//     {
//         OctStr string = test_oct.bytes();
//         byte = string.release();
//     }
//     ASSERT_EQ(test_oct, OctStr{*byte});
// }

TEST(OctStr, Bytes)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(test_oct.bytes(), test_oct);
}

TEST(OctStr, Uint8Vector)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::vector<uint8_t> answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(answer, test_oct);
}

TEST(OctStr, BasicStringUint8)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::basic_string<uint8_t> answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(answer, test_oct);
}

TEST(OctStr, VectorLikeContainer)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    gch::small_vector<uint8_t, 64> answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(answer, test_oct);
}

TEST(OctStr, Uint8Deque)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::deque<uint8_t> answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(answer, test_oct);
}

TEST(OctStr, Uint8List)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::list<uint8_t> answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(answer, test_oct);
}

TEST(OctStr, Uint8Array)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};
    std::array<uint8_t, 10> answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    ASSERT_EQ(answer, test_oct);
}

TEST(OctStr, HexlifyOctetString)
{
    OctStr string{250U, 206U}; // "FACE"_hex
    ASSERT_EQ(string.hexlify(), std::string("FACE"));
}

TEST(OctStr, HexlifyOctetStringFreeFunction)
{
    OctStr string{250U, 206U}; // "FACE"_hex
    ASSERT_EQ(hexlify(string), std::string("FACE"));
}

TEST(OctStr, UnhexlifyString)
{
    std::string string{"FACE"};
    OctStr hex_string{250U, 206U};
    ASSERT_EQ(unhexlify(string), hex_string);
}

TEST(OctStr, CopyNoPos)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    Bytes destination;
    destination.resize(7);
    test_oct.copy(const_cast<Bytes::pointer>(destination.data()), 7);
    OctStr answer{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7};
    ASSERT_EQ(OctStr(destination), answer);
}

TEST(OctStr, CopyNormal)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    Bytes destination;
    destination.resize(5);
    test_oct.copy(const_cast<Bytes::pointer>(destination.data()), 5, 2);
    OctStr answer{test_byte3, test_byte4, test_byte5, test_byte6, test_byte7};
    ASSERT_EQ(OctStr(destination), answer);
}

TEST(OctStr, CopyRightCut)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    Bytes destination;
    destination.resize(2);
    test_oct.copy(const_cast<Bytes::pointer>(destination.data()), 2, 8);
    OctStr answer{test_byte9, test_byte10};
    ASSERT_EQ(OctStr(destination), answer);
}

TEST(OctStr, CopyOutOfRange)
{
    OctStr test_oct{test_byte1, test_byte2, test_byte3, test_byte4, test_byte5, test_byte6, test_byte7, test_byte8, test_byte9, test_byte10};

    Bytes destination;
    destination.resize(7);
    ASSERT_ANY_THROW(test_oct.copy(const_cast<Bytes::pointer>(destination.data()), 5, 11));
}

TEST(OctStr, HexLiteral)
{
    OctStr hex_string{250U, 206U};
    ASSERT_EQ("FACE"_hex, hex_string);
}

TEST(OctStrHashTest, BasicEqualityAndHashing)
{
    OctStr a{0x01, 0x02, 0x03};
    OctStr b{0x01, 0x02, 0x03};
    OctStr c{0x04, 0x05, 0x06};

    // operator== works
    ASSERT_EQ(a, b);
    ASSERT_NE(a, c);

    // hash values are consistent
    std::hash<OctStr> hasher;
    EXPECT_EQ(hasher(a), hasher(b));
    EXPECT_NE(hasher(a), hasher(c));

    // noexcept check (static)
    static_assert(noexcept(hasher(a)), "std::hash<OctStr> must be noexcept");
}

TEST(OctStrHashTest, UnorderedMapUsage)
{
    std::unordered_map<OctStr, std::string> map;

    OctStr key1{0x01, 0x02};
    OctStr key2{0x03, 0x04};

    map[key1] = "first";
    map[key2] = "second";

    EXPECT_EQ(map[key1], "first");
    EXPECT_EQ(map[key2], "second");
}

TEST(OctStrHashTest, UnorderedSetUsage)
{
    std::unordered_set<OctStr> set;
    set.insert(OctStr{0xAA, 0xBB});
    set.insert(OctStr{0xCC, 0xDD});

    EXPECT_TRUE(set.find(OctStr{0xAA, 0xBB}) != set.end());
    EXPECT_TRUE(set.find(OctStr{0xCC, 0xDD}) != set.end());
    EXPECT_TRUE(set.find(OctStr{0x00, 0x00}) == set.end());
}

} // namespace autocrypt