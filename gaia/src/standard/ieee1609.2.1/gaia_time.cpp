#include "gaia_time.hpp"
#include "gaia_exception.hpp" // for SysException, THROWLOG
namespace
{
constexpr uint32_t i0sec = 347619603;
} // namespace

namespace vp
{
uint32_t iValueToTime32(uint16_t i_value)
{
    uint32_t time32 = i_value;
    time32 *= 60 * 60 * 24 * 7; // 1 week
    time32 += i0sec;
    return time32;
}

uint16_t time32ToIvalue(uint32_t time32)
{
    if (time32 < i0sec)
    {
        THROWLOG(SysException, "time32({}) for IValue must be after 2015-01-06({})", time32, i0sec);
    }

    time32 -= i0sec;
    time32 /= 604800; // 1week : 60 * 60 * 24 * 7
    return time32;
}

} // namespace vp