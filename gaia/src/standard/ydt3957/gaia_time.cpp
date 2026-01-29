#include "gaia_time.hpp"
#include "gaia_exception.hpp" // for SysException, THROWLOG
namespace
{
// 1578340800 - 1072915200
constexpr uint32_t chinaI0sec = 505425600;
} // namespace

namespace vp
{
uint32_t iValueToTime32(uint16_t i_value)
{
    uint32_t time32 = i_value;
    time32 *= 60 * 60 * 24 * 7; // 1 week
    time32 += chinaI0sec;
    return time32;
}

uint16_t time32ToIvalue(uint32_t time32)
{
    if (time32 < chinaI0sec)
    {
        THROWLOG(SysException,
                 "time32({}) for ChinaIValue must be after 2020-01-07 4AM(CST)({})", time32, chinaI0sec);
    }

    time32 -= chinaI0sec;
    time32 /= 604800; // 1week : 60 * 60 * 24 * 7
    return time32;
}
} // namespace vp