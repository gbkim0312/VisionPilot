#include "gaia_octstr.hpp"

namespace vp
{

// 모든 생성자가(비어있는 것을 만드는 생성자 합쳐서) 짧은 OctStr에 대한 최적화를 해 주어야 한다.
OctStr::OctStr() : bytes_{}
{
}

OctStr::OctStr(Bytes bytes) : bytes_{std::move(bytes)}
{
}

OctStr::OctStr(Bytes::const_pointer str, Bytes::size_type s) : bytes_{str, s}
{
}

OctStr::OctStr(const void *ptr, Bytes::size_type s) : bytes_{static_cast<Bytes::const_pointer>(ptr), s}
{
}

OctStr::OctStr(uint8_t other) : bytes_{reinterpret_cast<Bytes::const_pointer>(&other), sizeof(other)}
{
}

OctStr::OctStr(uint16_t other) : bytes_{reinterpret_cast<Bytes::const_pointer>(&other), sizeof(other)}
{
    std::reverse(bytes_.begin(), bytes_.end());
}

OctStr::OctStr(uint32_t other) : bytes_{reinterpret_cast<Bytes::const_pointer>(&other), sizeof(other)}
{
    std::reverse(bytes_.begin(), bytes_.end());
}

OctStr::OctStr(std::initializer_list<Bytes::value_type> ilist) : bytes_{ilist}
{
}

OctStr &OctStr::operator=(std::initializer_list<Bytes::value_type> ilist)
{
    bytes_.assign(ilist);
    return *this;
}

void OctStr::assign(Bytes::size_type count, Bytes::const_reference value)
{
    bytes_.assign(count, value);
}

void OctStr::assign(std::initializer_list<Bytes::value_type> ilist)
{
    bytes_.assign(ilist);
}

// Element access
Bytes::reference OctStr::at(Bytes::size_type pos)
{
    return bytes_.at(pos);
}

Bytes::const_reference OctStr::at(Bytes::size_type pos) const
{
    return bytes_.at(pos);
}

Bytes::reference OctStr::operator[](Bytes::size_type pos) noexcept
{
    return bytes_[pos];
}

Bytes::const_reference OctStr::operator[](Bytes::size_type pos) const noexcept
{
    return bytes_[pos];
}

Bytes::reference OctStr::front() noexcept
{
    return bytes_.front();
}

Bytes::const_reference OctStr::front() const noexcept
{
    return bytes_.front();
}

Bytes::reference OctStr::back() noexcept
{
    return bytes_.back();
}

Bytes::const_reference OctStr::back() const noexcept
{
    return bytes_.back();
}

Bytes::pointer OctStr::data() noexcept
{
    return const_cast<Bytes::pointer>(bytes_.data());
}

Bytes::const_pointer OctStr::data() const noexcept
{
    return bytes_.data();
}

const Bytes &OctStr::bytes() const noexcept
{
    return bytes_;
}

// Iterators
Bytes::iterator OctStr::begin() noexcept
{
    return bytes_.begin();
}

Bytes::const_iterator OctStr::begin() const noexcept
{
    return bytes_.begin();
}

Bytes::const_iterator OctStr::cbegin() const noexcept
{
    return bytes_.cbegin();
}

Bytes::iterator OctStr::end() noexcept
{
    return bytes_.end();
}

Bytes::const_iterator OctStr::end() const noexcept
{
    return bytes_.end();
}

Bytes::const_iterator OctStr::cend() const noexcept
{
    return bytes_.cend();
}

Bytes::reverse_iterator OctStr::rbegin() noexcept
{
    return bytes_.rbegin();
}

Bytes::const_reverse_iterator OctStr::rbegin() const noexcept
{
    return bytes_.rbegin();
}

Bytes::const_reverse_iterator OctStr::crbegin() const noexcept
{
    return bytes_.crbegin();
}

Bytes::reverse_iterator OctStr::rend() noexcept
{
    return bytes_.rend();
}

Bytes::const_reverse_iterator OctStr::rend() const noexcept
{
    return bytes_.rend();
}

Bytes::const_reverse_iterator OctStr::crend() const noexcept
{
    return bytes_.crend();
}

// Capacity
bool OctStr::empty() const
{
    return bytes_.empty();
}

Bytes::size_type OctStr::size() const noexcept
{
    return bytes_.size();
}

Bytes::size_type OctStr::max_size() const noexcept
{
    return bytes_.max_size();
}

void OctStr::reserve(Bytes::size_type n)
{
    bytes_.reserve(n);
}

Bytes::size_type OctStr::capacity() const noexcept
{
    return bytes_.capacity();
}

// Modifiers
void OctStr::clear() noexcept
{
    bytes_.clear();
}

Bytes::iterator OctStr::insert(Bytes::const_iterator pos, Bytes::value_type ch)
{
    return bytes_.insert(pos, ch);
}

Bytes::iterator OctStr::insert(Bytes::const_iterator pos, Bytes::size_type count, Bytes::value_type value)
{
    return bytes_.insert(pos, count, value);
}

Bytes::iterator OctStr::insert(Bytes::const_iterator pos, Bytes::const_iterator first, Bytes::const_iterator last)
{
    return bytes_.insert(pos, first, last);
}

Bytes::iterator OctStr::erase(Bytes::const_iterator pos)
{
    return bytes_.erase(pos, pos + 1);
}

Bytes::iterator OctStr::erase(Bytes::const_iterator first, Bytes::const_iterator last)
{
    return bytes_.erase(first, last - 1);
}

void OctStr::push_back(Bytes::value_type ch)
{
    bytes_.push_back(ch);
}

void OctStr::pop_back()
{
    bytes_.pop_back();
}

void OctStr::resize(Bytes::size_type n)
{
    bytes_.resize(n);
}

void OctStr::resize(Bytes::size_type n, Bytes::value_type c)
{
    bytes_.resize(n, c);
}

void OctStr::swap(OctStr &other) noexcept
{
    bytes_.swap(other.bytes_);
}

// string-like 함수

// string-like insert
OctStr &OctStr::insert(Bytes::size_type index, Bytes::size_type count, Bytes::value_type ch)
{
    bytes_.insert(index, count, ch);
    return *this;
}

OctStr &OctStr::insert(Bytes::size_type index, Bytes::const_pointer s, Bytes::size_type count)
{
    bytes_.insert(index, s, count);
    return *this;
}

OctStr &OctStr::insert(Bytes::size_type index, const OctStr &str)
{
    bytes_.insert(index, str);
    return *this;
}

OctStr &OctStr::insert(Bytes::size_type index, const OctStr &str, Bytes::size_type index_str, Bytes::size_type count)
{
    bytes_.insert(index, str, index_str, count);
    return *this;
}

OctStr &OctStr::erase(Bytes::size_type index, Bytes::size_type count)
{
    bytes_.erase(index, count);
    return *this;
} // string-like erase

OctStr &OctStr::append(Bytes::size_type count, Bytes::value_type ch)
{
    bytes_.append(count, ch);
    return *this;
}

OctStr &OctStr::append(const OctStr &str)
{
    bytes_.append(str.bytes_);
    return *this;
}

OctStr &OctStr::append(const OctStr &str, Bytes::size_type pos, Bytes::size_type count)
{
    bytes_.append(str, pos, count);
    return *this;
}

OctStr &OctStr::append(Bytes::const_pointer s, Bytes::size_type count)
{
    bytes_.append(s, count);
    return *this;
}

OctStr &OctStr::append(std::initializer_list<Bytes::value_type> ilist)
{
    bytes_.append(ilist);
    return *this;
}

int OctStr::compare(const OctStr &other) const noexcept
{
    auto result_pair = std::mismatch(bytes_.begin(), bytes_.end(), other.bytes_.begin(), other.bytes_.end());

    if (result_pair.first == bytes_.end() && result_pair.second == other.bytes_.end())
    {
        return 0;
    }

    if (result_pair.first == bytes_.end())
    {
        return -1;
    }

    if (result_pair.second == other.bytes_.end())
    {
        return 1;
    }

    return (*result_pair.first < *result_pair.second) ? -1 : 1;
}

OctStr OctStr::substr(Bytes::size_type pos, Bytes::size_type count) const
{
    return OctStr{bytes_.substr(pos, count)};
}

Bytes::size_type OctStr::copy(Bytes::pointer dest, Bytes::size_type count, Bytes::size_type pos) const
{
    return bytes_.copy(dest, count, pos);
}

OctStr OctStr::operator+(const OctStr &b)
{
    OctStr concat{bytes_};
    concat.bytes_.append(b.bytes_);
    return concat;
}

// 비트 연산 함수
OctStr OctStr::operator^(const OctStr &b) const
{
    OctStr r;
    r.bytes_.reserve(bytes_.size());
    for (size_t i = 0; i < bytes_.size(); i++)
    {
        r.bytes_.push_back(bytes_.at(i) ^ b.bytes_.at(i));
    }
    return r;
}

OctStr OctStr::operator<<(Bytes::size_type n) const
{
    size_t n_shift_bytes = n / 8;
    size_t n_shift_bits = n % 8;

    auto result = substr(n_shift_bytes);

    for (auto itr = result.begin(); itr != result.end(); ++itr)
    {
        *itr <<= n_shift_bits; // NOLINT
        if (itr + 1 != result.end())
        {
            *itr |= static_cast<size_t>(*(itr + 1) >> (8U - n_shift_bits)); // NOLINT
        }
    }

    result.insert(result.size(), n_shift_bytes, 0);

    return result;
}

OctStr OctStr::operator>>(Bytes::size_type n) const
{
    size_t n_shift_bytes = n / 8;
    size_t n_shift_bits = n % 8;

    auto result = substr(n_shift_bytes);

    for (auto itr = result.rbegin(); itr != result.rend(); ++itr)
    {
        *itr >>= n_shift_bits; // NOLINT
        if (itr + 1 != result.rend())
        {
            *itr |= static_cast<size_t>(*(itr + 1) << (8U - n_shift_bits)); // NOLINT
        }
    }

    result.insert(0, n_shift_bytes, 0);

    return result;
}

std::string OctStr::hexlify() const
{
    constexpr std::array<char, 16> T = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    std::string hex;
    hex.reserve(2 * bytes_.size());

    for (uint8_t c : bytes_)
    {
        hex.push_back(T[static_cast<uint8_t>(c >> 4U) & 0x0fU]);
        hex.push_back(T[c & 15U]);
    }
    return hex;
}

} // namespace vp
