#pragma once

#include "spdlog/fmt/fmt.h" // for format_parse_context, formatter
#include <algorithm>        // for reverse
#include <array>            // for avoid conflict with android std::__ndk1::array<char>
#include <cctype>           // for isprint, isspace, tolower
#include <cstddef>          // for size_t
#include <cstdint>          // for uint8_t, uint16_t, uint32_t
#include <functional>
#include <iostream> // for size_t, ostream
#include <string>   // for basic_string, string, hash

namespace vp
{

using Bytes = std::basic_string<uint8_t>;
static_assert(std::is_same<Bytes::value_type, uint8_t>{}, "Is not a container consist of uint8_t"); // 내부 데이터가 uint8_t의 컨테이너인지 검사한다. 통과하지 못하면 컴파일 오류.

template <typename Container, typename InputIt>
struct HasInputIteratorConstructor
{
private:
    template <typename C, typename I,
              typename = decltype(C(std::declval<I>(), std::declval<I>())),
              typename = std::enable_if_t<std::is_convertible<typename std::iterator_traits<I>::iterator_category, std::input_iterator_tag>::value>,
              typename = std::enable_if_t<std::is_same<typename std::iterator_traits<InputIt>::value_type, Bytes::value_type>::value>>
    static std::true_type Test(int);

    template <typename C, typename I>
    static std::false_type Test(...);

public:
    static constexpr bool value = decltype(Test<Container, InputIt>(0))::value;
};

template <typename T>
struct HasDataAndSize
{
    template <typename U,
              typename = std::enable_if_t<std::is_same<decltype(std::declval<U>().data()), Bytes::const_pointer>::value>,
              typename = std::enable_if_t<std::is_same<decltype(std::declval<U>().size()), Bytes::size_type>::value>>
    static auto test(int) -> decltype(std::declval<U>().data(), std::declval<U>().size(), std::true_type());

    template <typename U>
    static std::false_type test(...);

    using type = decltype(test<T>(0));
    static constexpr bool value = type::value;
};

template <typename T>
struct HasBeginAndEnd // begin의 리턴 타입과 end의 리턴 타입이 모두 iterator 타입이어야 한다. 그리고, begin이 리턴하는 iterator와 end가 리턴하는 iterator는 uint8_t나 int8_t를 가리켜야 한다.
{
    template <typename U,
              typename = std::enable_if_t<std::is_same<decltype(std::declval<U>().begin()), typename U::const_iterator>::value || std::is_same<decltype(std::declval<U>().begin()), typename U::iterator>::value>,
              typename = std::enable_if_t<std::is_same<decltype(std::declval<U>().end()), typename U::const_iterator>::value || std::is_same<decltype(std::declval<U>().end()), typename U::iterator>::value>,
              typename = std::enable_if_t<std::is_convertible<typename std::iterator_traits<typename U::const_iterator>::iterator_category, std::input_iterator_tag>::value || std::is_convertible<typename std::iterator_traits<typename U::iterator>::iterator_category, std::input_iterator_tag>::value>,
              typename = std::enable_if_t<std::is_scalar<typename std::iterator_traits<typename U::iterator>::value_type>::value>,
              typename = std::enable_if_t<std::is_same<std::make_unsigned_t<typename std::iterator_traits<typename U::iterator>::value_type>, Bytes::value_type>::value>>
    static auto test(int) -> decltype(std::declval<U>().begin(), std::declval<U>().end(), std::true_type());

    template <typename U>
    static std::false_type test(...);

    using type = decltype(test<T>(0));
    static constexpr bool value = type::value;
};

class OctStr
{
public:
    // 모든 생성자가(비어있는 것을 만드는 생성자 합쳐서) 짧은 OctStr에 대한 최적화를 해 주어야 한다.
    OctStr();

    OctStr(Bytes bytes);

    // data와 size가 있는 경우
    template <typename ByteContainer,
              typename = std::enable_if_t<!std::is_same<Bytes, ByteContainer>::value>,
              typename = std::enable_if_t<HasDataAndSize<ByteContainer>::value>>
    OctStr(const ByteContainer &bytes) : bytes_{bytes.data(), bytes.size()}
    {
    }

    // data와 size가 없는데, begin과 end는 있는 경우
    template <typename ByteContainer,
              typename = std::enable_if_t<!std::is_same<Bytes, ByteContainer>::value>,
              typename = std::enable_if_t<!HasDataAndSize<ByteContainer>::value>,
              typename = std::enable_if_t<HasBeginAndEnd<ByteContainer>::value>>
    OctStr(const ByteContainer &bytes) : bytes_{bytes.begin(), bytes.end()}
    {
    }

    OctStr(Bytes::const_pointer str, Bytes::size_type s);

    OctStr(const void *ptr, Bytes::size_type s);

    template <class InputIt,
              typename = std::enable_if_t<std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category, std::input_iterator_tag>::value>,
              typename = std::enable_if_t<std::is_same<std::make_unsigned_t<typename std::iterator_traits<InputIt>::value_type>, Bytes::value_type>::value>>
    OctStr(InputIt first, InputIt last) : bytes_{first, last}
    {
    }

    explicit OctStr(uint8_t other);

    explicit OctStr(uint16_t other);

    explicit OctStr(uint32_t other);

    explicit OctStr(std::initializer_list<Bytes::value_type> ilist);

    OctStr &operator=(std::initializer_list<Bytes::value_type> ilist);

    void assign(Bytes::size_type count, Bytes::const_reference value);

    template <class InputIt,
              typename = std::enable_if_t<std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category, std::input_iterator_tag>::value>,
              typename = std::enable_if_t<std::is_same<std::make_unsigned_t<typename std::iterator_traits<InputIt>::value_type>, Bytes::value_type>::value>>
    void assign(InputIt first, InputIt last)
    {
        bytes_.assign(first, last);
    }

    void assign(std::initializer_list<Bytes::value_type> ilist);

    // Element access
    Bytes::reference at(Bytes::size_type pos);

    Bytes::const_reference at(Bytes::size_type pos) const;

    Bytes::reference operator[](Bytes::size_type pos) noexcept;

    Bytes::const_reference operator[](Bytes::size_type pos) const noexcept;

    Bytes::reference front() noexcept;

    Bytes::const_reference front() const noexcept;

    Bytes::reference back() noexcept;

    Bytes::const_reference back() const noexcept;

    Bytes::pointer data() noexcept;

    Bytes::const_pointer data() const noexcept;

    const Bytes &bytes() const noexcept;

    // Iterators
    Bytes::iterator begin() noexcept;

    Bytes::const_iterator begin() const noexcept;

    Bytes::const_iterator cbegin() const noexcept;

    Bytes::iterator end() noexcept;

    Bytes::const_iterator end() const noexcept;

    Bytes::const_iterator cend() const noexcept;

    Bytes::reverse_iterator rbegin() noexcept;

    Bytes::const_reverse_iterator rbegin() const noexcept;

    Bytes::const_reverse_iterator crbegin() const noexcept;

    Bytes::reverse_iterator rend() noexcept;

    Bytes::const_reverse_iterator rend() const noexcept;

    Bytes::const_reverse_iterator crend() const noexcept;

    // Capacity
    bool empty() const;

    Bytes::size_type size() const noexcept;

    Bytes::size_type max_size() const noexcept;

    void reserve(Bytes::size_type n = 0);

    Bytes::size_type capacity() const noexcept;

    // Modifiers
    void clear() noexcept;

    Bytes::iterator insert(Bytes::const_iterator pos, Bytes::value_type ch);

    Bytes::iterator insert(Bytes::const_iterator pos, Bytes::size_type count, Bytes::value_type value);

    Bytes::iterator insert(Bytes::const_iterator pos, Bytes::const_iterator first, Bytes::const_iterator last);

    template <class InputIt,
              typename = std::enable_if_t<std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category, std::input_iterator_tag>::value>,
              typename = std::enable_if_t<std::is_same<std::make_unsigned_t<typename std::iterator_traits<InputIt>::value_type>, Bytes::value_type>::value>>
    Bytes::iterator insert(Bytes::const_iterator pos, InputIt first, InputIt last)
    {
        return bytes_.insert(pos, first, last);
    }

    // Bytes::iterator insert(Bytes::const_iterator pos, std::initializer_list<Bytes::value_type> ilist)
    // {
    //     return bytes_.insert(pos, ilist);
    // }

    Bytes::iterator erase(Bytes::const_iterator pos);

    Bytes::iterator erase(Bytes::const_iterator first, Bytes::const_iterator last);

    void push_back(Bytes::value_type ch);

    void pop_back();

    void resize(Bytes::size_type n);

    void resize(Bytes::size_type n, Bytes::value_type c);

    void swap(OctStr &other) noexcept;

    // string-like 함수

    // string-like insert
    OctStr &insert(Bytes::size_type index, Bytes::size_type count, Bytes::value_type ch);

    OctStr &insert(Bytes::size_type index, Bytes::const_pointer s, Bytes::size_type count);

    OctStr &insert(Bytes::size_type index, const OctStr &str);

    OctStr &insert(Bytes::size_type index, const OctStr &str, Bytes::size_type index_str, Bytes::size_type count = npos);

    OctStr &erase(Bytes::size_type index = 0, Bytes::size_type count = npos); // string-like erase

    OctStr &append(Bytes::size_type count, Bytes::value_type ch);

    OctStr &append(const OctStr &str);

    OctStr &append(const OctStr &str, Bytes::size_type pos, Bytes::size_type count = npos);

    OctStr &append(Bytes::const_pointer s, Bytes::size_type count);

    template <class InputIt,
              typename = std::enable_if_t<std::is_convertible<typename std::iterator_traits<InputIt>::iterator_category, std::input_iterator_tag>::value>,
              typename = std::enable_if_t<std::is_same<std::make_unsigned_t<typename std::iterator_traits<InputIt>::value_type>, Bytes::value_type>::value>>
    OctStr &append(InputIt first, InputIt last)
    {
        bytes_.append(first, last);
        return *this;
    }

    OctStr &append(std::initializer_list<Bytes::value_type> ilist);

    inline OctStr &operator+=(OctStr &&str)
    {
        bytes_.append(str.bytes());
        return *this;
    }

    inline OctStr &operator+=(const OctStr &str)
    {
        bytes_.append(str.bytes());
        return *this;
    }

    inline OctStr &operator+=(Bytes::value_type ch)
    {
        bytes_.push_back(ch);
        return *this;
    }

    inline OctStr &operator+=(std::initializer_list<Bytes::value_type> ilist)
    {
        bytes_.append(ilist);
        return *this;
    }

    int compare(const OctStr &other) const noexcept;

    OctStr substr(Bytes::size_type pos = 0UL, Bytes::size_type count = npos) const;

    Bytes::size_type copy(Bytes::pointer dest, Bytes::size_type count, Bytes::size_type pos = 0) const;

    OctStr operator+(const OctStr &b);

    // 비트 연산 함수
    OctStr operator^(const OctStr &b) const;

    OctStr operator<<(Bytes::size_type n) const;

    OctStr operator>>(Bytes::size_type n) const;

    // 기타
    operator Bytes() const { return bytes_; }

    // ByteContainer에 맨 앞 포인터와 크기로 생성하는 생성자가 있어야 함
    template <typename ByteContainer,
              typename = std::enable_if_t<!std::is_same<Bytes, ByteContainer>::value>,
              typename = std::enable_if_t<std::is_same<typename ByteContainer::value_type, Bytes::value_type>::value>,
              typename = std::enable_if_t<std::is_trivially_constructible<ByteContainer, Bytes::const_pointer, Bytes::size_type>::value>>
    operator ByteContainer() const
    {
        return ByteContainer{data(), size()};
    }

    // ByteContainer에 inputit으로 생성하는 생성자가 있어야 함
    template <typename ByteContainer,
              typename = std::enable_if_t<!std::is_same<Bytes, ByteContainer>::value>,
              typename = std::enable_if_t<std::is_same<typename ByteContainer::value_type, Bytes::value_type>::value>,
              typename = std::enable_if_t<!std::is_trivially_constructible<ByteContainer, Bytes::const_pointer, Bytes::size_type>::value>,
              typename = std::enable_if_t<HasInputIteratorConstructor<ByteContainer, typename ByteContainer::iterator>::value>>
    operator ByteContainer() const
    {
        return ByteContainer{begin(), end()};
    }

    std::string hexlify() const;

    static constexpr Bytes::size_type npos = Bytes::size_type(-1);

private:
    Bytes bytes_;
};

// non-member octstr function
inline OctStr operator+(const OctStr &a, const OctStr &b)
{
    return OctStr{a}.append(b);
}

inline bool operator==(const OctStr &lhs, const OctStr &rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

inline bool operator!=(const OctStr &lhs, const OctStr &rhs) noexcept
{
    return lhs.compare(rhs) != 0;
}

inline bool operator<(const OctStr &lhs, const OctStr &rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

inline bool operator<=(const OctStr &lhs, const OctStr &rhs) noexcept
{
    return lhs.compare(rhs) <= 0;
}

inline bool operator>(const OctStr &lhs, const OctStr &rhs) noexcept
{
    return lhs.compare(rhs) > 0;
}

inline bool operator>=(const OctStr &lhs, const OctStr &rhs) noexcept
{
    return lhs.compare(rhs) >= 0;
}

inline std::ostream &operator<<(std::ostream &strm, const OctStr &str)
{
    return strm << str.hexlify();
}

inline std::string hexlify(const OctStr &bin)
{
    return bin.hexlify();
}

inline OctStr unhexlify(const std::string &hex)
{
    constexpr std::array<signed char, 256> Tmap{
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    OctStr bin;
    bin.reserve(hex.size() / 2);

    bool even = true;
    uint8_t value = 0;
    for (char ch : hex)
    {
        auto c = static_cast<uint8_t>(ch);
        if (Tmap[c] == -1)
        {
            continue;
        }

        value = (value << 4U) + Tmap[c];
        even = !even;

        if (even)
        {
            bin.push_back(value);
        }
    }
    return bin;
}

// BEGIN: OctStr related
inline namespace literals
{
inline OctStr operator"" _hex(const char *c, size_t size)
{
    return unhexlify(std::string(c, size));
}
} // namespace literals

} // namespace vp

namespace std
{
// std::hash specialization for vp::OctStr
// This allows vp::OctStr to be used as a key in unordered_map, unordered_set, etc.
// The hash function is designed to be efficient and to minimize collisions.
template <>
struct hash<vp::OctStr>
{
    size_t operator()(vp::OctStr const &s) const noexcept
    {
        size_t h = 146527U; // Magic prime number
        for (auto byte : s.bytes())
        {
            h ^= byte;
            h *= 1099511628211U; // Magic prime number
        }
        return h;
    }
};
} // namespace std

namespace fmt
{
template <>
struct formatter<vp::OctStr>
{
    constexpr auto parse(format_parse_context &ctx) const -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const vp::OctStr &value, FormatContext &ctx) -> decltype(ctx.out())
    {
        return format_to(ctx.out(), value.hexlify());
    }
};
} // namespace fmt