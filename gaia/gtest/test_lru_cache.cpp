#include "gaia_lru_cache.hpp"
#include <gtest/gtest.h>
#include <type_traits>

namespace autocrypt
{

// 기본 생성자가 제대로 작동하는지 확인하는 테스트
TEST(LruCache, ConstructorWithoutParameter)
{
    LruCache<uint64_t, uint64_t> lru_cache{};
    ASSERT_EQ(lru_cache.size(), 0);
    ASSERT_EQ(lru_cache.capacity(), 1000);
    ASSERT_TRUE(lru_cache.isEmpty());
}

// 캐시의 최대 크기를 인자로 받는 생성자가 제대로 작동하는지 확인하는 테스트
TEST(LruCache, ConstructorWithParameter)
{
    LruCache<uint64_t, uint64_t> lru_cache{2000};
    ASSERT_EQ(lru_cache.size(), 0);
    ASSERT_EQ(lru_cache.capacity(), 2000);
    ASSERT_TRUE(lru_cache.isEmpty());
}

// 꽉 찬 캐시에서, 이미 있는 키의 값을 대체할 수 있는지 보는 테스트
TEST(LruCache, PutcontainsKey)
{
    LruCache<uint64_t, uint64_t> lru_cache{3};
    lru_cache.put(1, 0);
    lru_cache.put(2, 0);
    lru_cache.put(3, 0);
    lru_cache.put(1, 1);

    ASSERT_EQ(lru_cache.size(), 3);

    ASSERT_EQ(*lru_cache.get(1), 1);
    ASSERT_EQ(*lru_cache.get(2), 0);
    ASSERT_EQ(*lru_cache.get(3), 0);

    ASSERT_TRUE(lru_cache.contains(1));
    ASSERT_TRUE(lru_cache.contains(2));
    ASSERT_TRUE(lru_cache.contains(3));

    ASSERT_FALSE(lru_cache.isEmpty());
}

// 꽉 찬 캐시에서, 새로운 키를 넣었을 때 잘 들어가면서 가장 오래된 것이 잘 지워지는지 보는 테스트
TEST(LruCache, DeleteLeastRecentElement)
{
    LruCache<uint64_t, uint64_t> lru_cache{3};
    lru_cache.put(1, 0);
    lru_cache.put(2, 0);
    lru_cache.put(3, 0);
    lru_cache.put(1, 1);
    lru_cache.get(2);
    lru_cache.put(4, 0);

    ASSERT_EQ(lru_cache.size(), 3);

    ASSERT_FALSE(lru_cache.get(3));
    ASSERT_EQ(*lru_cache.get(4), 0);

    ASSERT_TRUE(lru_cache.contains(1));
    ASSERT_TRUE(lru_cache.contains(2));
    ASSERT_FALSE(lru_cache.contains(3));
    ASSERT_TRUE(lru_cache.contains(4));

    ASSERT_FALSE(lru_cache.isEmpty());
}

// get 메서드로 가져온 Value 객체가 const 레퍼런스임을 보이는 테스트
TEST(LruCache, GetConstantReference)
{
    LruCache<uint64_t, uint64_t> lru_cache{3};
    lru_cache.put(1, 42);
    ASSERT_TRUE(std::is_reference<decltype(lru_cache.get(1)->get())>::value);                      // 레퍼런스인가?
    ASSERT_TRUE(std::is_const<std::remove_reference_t<decltype(lru_cache.get(1)->get())>>::value); // 레퍼런스를 떼면 const인가?
}

// 캐시 내용 전체 삭제가 잘 되는지 보는 테스트
TEST(LruCache, Clear)
{
    LruCache<uint64_t, uint64_t> lru_cache{3};
    lru_cache.put(1, 0);
    lru_cache.put(2, 0);
    lru_cache.put(3, 0);
    lru_cache.put(1, 1);
    lru_cache.get(2);
    lru_cache.put(4, 0);

    lru_cache.clear();

    ASSERT_TRUE(lru_cache.isEmpty());
}

TEST(LruCache, RvalueMove)
{
    struct TestClassCallingCount
    {
        uint32_t lvalue_construct_count = 0;
        uint32_t rvalue_construct_count = 0;
        uint32_t lvalue_assign_count = 0;
        uint32_t rvalue_assign_count = 0;
    } test_class_calling_count;

    class TestClass
    {
    public:
        explicit TestClass(TestClassCallingCount &count_ref)
            : count_ref_(count_ref)
        {
        }

        ~TestClass() = default;

        TestClass(const TestClass &other)
            : count_ref_(other.count_ref_)
        {
            count_ref_.lvalue_construct_count++;
        }
        TestClass(TestClass &&other) noexcept
            : count_ref_(other.count_ref_)
        {
            count_ref_.rvalue_construct_count++;
        }

        TestClass &operator=(const TestClass & /* other */)
        {
            count_ref_.lvalue_assign_count++;
            return *this;
        }
        TestClass &operator=(TestClass && /* other */) noexcept
        {
            count_ref_.rvalue_assign_count++;
            return *this;
        }

    private:
        TestClassCallingCount &count_ref_;
    };

    LruCache<uint64_t, TestClass> lru_cache{3};
    lru_cache.put(1, TestClass{test_class_calling_count}); // rvalue construct - rvalue construct
    TestClass test_class{test_class_calling_count};
    lru_cache.put(2, test_class);                          // lvalue construct
    lru_cache.put(3, TestClass{test_class_calling_count}); // rvalue construct - rvalue construct
    lru_cache.put(3, test_class);                          // lvalue assign
    lru_cache.put(2, TestClass{test_class_calling_count}); // rvalue construct - rvalue assign

    ASSERT_EQ(test_class_calling_count.lvalue_construct_count, 1);
    ASSERT_EQ(test_class_calling_count.rvalue_construct_count, 5);
    ASSERT_EQ(test_class_calling_count.lvalue_assign_count, 1);
    ASSERT_EQ(test_class_calling_count.rvalue_assign_count, 1);
}

TEST(LruCache, Remove)
{
    LruCache<uint64_t, uint64_t> lru_cache{2};
    lru_cache.put(1, 0);
    lru_cache.put(2, 0);
    lru_cache.put(3, 0);
    lru_cache.remove(1);
    ASSERT_TRUE(lru_cache.get(1) == std::nullopt);
}

TEST(LruCache, RemoveNonExistentKey)
{
    LruCache<uint64_t, uint64_t> lru_cache{2};
    lru_cache.put(1, 0);
    lru_cache.put(2, 0);
    lru_cache.remove(3);            // 3은 존재하지 않는 키
    ASSERT_EQ(lru_cache.size(), 2); // 크기는 변하지 않아야 함
    ASSERT_TRUE(lru_cache.get(1).has_value());
    ASSERT_TRUE(lru_cache.get(2).has_value());
}
} // namespace autocrypt