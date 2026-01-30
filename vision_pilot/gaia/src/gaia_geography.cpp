#include "gaia_geography.hpp"
#include "gaia_dir.hpp"
#include "gaia_exception.hpp" // for SysException, THROWLOG
#include "gaia_log.hpp"       // for LOG_INF, LOG_DBG
#include "zonedetect.h"       // for ZDCloseDatabase, ZoneDetect, ZDHelperSimple...
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
namespace
{
constexpr double kLatLonScale = 1e7;
constexpr double kMetersPerDegLat = 111320.0; //    위도 1도 ≈ 111,320m (보수적). radius_padding_m 만큼 여유를 반지름에 더해 비교.
constexpr size_t kMinPolygonPoints = 3;

constexpr double kMinLat = -90.0;
constexpr double kMaxLat = 90.0;
constexpr double kMinLon = -180.0;
constexpr double kMaxLon = 180.0;

constexpr double kDegToRadFactor = M_PI / 180.0;

constexpr auto kAlpha3String = "Alpha3";
constexpr auto kAlpha3StringLegacy = "CountryAlpha3";

enum class FastCheckResult
{
    SUCCESS,
    NEED_TO_PRECISE_CHECK,
    OUT_OF_BORDER
};

struct Point
{
    double x = 0.0;
    double y = 0.0;
};

std::unique_ptr<ZoneDetect, decltype(&ZDCloseDatabase)>
    _country_db(nullptr, &ZDCloseDatabase);
std::once_flag _db_once;

inline void dbInit(const std::string &country_db_path)
{

    if (!vp::isFileExist(country_db_path))
    {
        THROWLOG(vp::SysException,
                 "dbInit: database file not found: {}", country_db_path);
    }

    std::call_once(_db_once, [&]
                   {
        _country_db.reset(ZDOpenDatabase(country_db_path.c_str()));
        if (!_country_db)
        {
            THROWLOG(vp::SysException, "getCountryCodeFromLatLon: incorrect database path: {}", country_db_path);
        } });
}

inline double toDegree(int32_t value)
{
    return value / kLatLonScale;
}

inline bool isAlpha3Key(const std::string &key)
{
    return key == kAlpha3String || key == kAlpha3StringLegacy;
}

// 점 P(px, py) 에서 선분 AB A(ax, ay), B(bx,by)까지의 최소 거리의 제곱값 반환
inline double pointSegmentDistSquared(const Point &p,
                                      const Point &a,
                                      const Point &b)
{
    // v: 선분 AB의 벡터 (v-> = vx-> + vy->)
    const auto vx = b.x - a.x;
    const auto vy = b.y - a.y;
    const auto vv = vx * vx + vy * vy;

    // w: A에서 P로 향하는 벡터
    const auto wx = p.x - a.x;
    const auto wy = p.y - a.y;

    // 내적 (정사영) : C =  {(AP * AB)/ |AB|^2 } * AB
    auto t = (vv > 0.0) ? ((wx * vx + wy * vy) / vv) : 0.0;

    // t를 [0,1] 구간으로 제한 (선분 위로 clamp)
    // 정사영 T가 0보다 작거나 크면 선분 AB의 연장선 위에 있음
    if (t < 0.0)
    {
        t = 0.0;
    }
    else if (t > 1.0)
    {
        t = 1.0;
    }

    // 선분 AB 위에서 P와 가장 가까운 점 C
    const auto cx = a.x + t * vx;
    const auto cy = a.y + t * vy;

    // 점 P와 A의 거리 제곱
    const auto dx = p.x - cx;
    const auto dy = p.y - cy;
    return dx * dx + dy * dy;
}

int8_t ccw(int64_t p1_latitude, int64_t p1_longitude,
           int64_t p2_latitude, int64_t p2_longitude,
           int64_t p3_latitude, int64_t p3_longitude)
{
    int64_t cross_product = (p2_latitude - p1_latitude) * (p3_longitude - p1_longitude) -
                            (p3_latitude - p1_latitude) * (p2_longitude - p1_longitude);

    if (cross_product > 0)
    {
        return 1;
    }
    else if (cross_product < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

std::pair<bool, uint8_t> lineSegmentIntersection(int64_t segment_previous_start_longitude,
                                                 int64_t segment_start_latitude, int64_t segment_start_longitude,
                                                 int64_t segment_end_latitude, int64_t segment_end_longitude,
                                                 int64_t segment_next_end_longitude,
                                                 int64_t current_location_latitude, int64_t current_location_longitude)
{
    std::pair<bool, uint8_t> result = std::make_pair(false, 0);
    if ((ccw(segment_start_latitude, segment_start_longitude, segment_end_latitude, segment_end_longitude, current_location_latitude, current_location_longitude) * ccw(segment_start_latitude, segment_start_longitude, segment_end_latitude, segment_end_longitude, 900000000, current_location_longitude) == 0) &&
        (ccw(current_location_latitude, current_location_longitude, 900000000, current_location_longitude, segment_start_latitude, segment_start_longitude) * ccw(current_location_latitude, current_location_longitude, 900000000, current_location_longitude, segment_end_latitude, segment_end_longitude) == 0)) // 4점이 일직선인 경우
    {
        if (segment_start_latitude < segment_end_latitude) // 끝점이 시점보다 고위도일 때
        {
            if (current_location_latitude >= segment_start_latitude && current_location_latitude <= segment_end_latitude) // 선분 내부에 있으면 내부
            {
                result.first = true;
            }
            if (current_location_latitude < segment_start_latitude) // 현재 지점이 시점보다 저위도일 때는
            {
                if ((current_location_longitude - segment_previous_start_longitude) * (segment_next_end_longitude - current_location_longitude) < 0) // 안에서 안으로, 밖에서 밖으로 가는 것이면 +1해서 다음 선의 시점과 합쳐서 +2되는 것으로 하고
                {
                    result.second = 1;
                }
                else // 아니면 +0 해서 다음 선의 시점과 합쳐서 +1되는 것으로 한다.
                {
                    result.second = 0;
                }
            }
            else // 현재 시점이 종점과 같거나 더 고위도일 때는 카운트하지 않는다.
            {
                result.second = 0;
            }
        }
        else // 끝점이 시점보다 저위도일 때
        {
            if (current_location_latitude >= segment_end_latitude && current_location_latitude <= segment_start_latitude) // 선분 내부에 있으면 내부
            {
                result.first = true;
            }
            if (current_location_latitude <= segment_end_latitude) // 현재 지점이 종점과 같거나 더 저위도일 때는
            {
                if ((current_location_longitude - segment_previous_start_longitude) * (segment_next_end_longitude - current_location_longitude) < 0) // 안에서 안으로, 밖에서 밖으로 가는 것이면 +1해서 다음 선의 시점과 합쳐서 +2되는 것으로 하고
                {
                    result.second = 1;
                }
                else // 아니면 +0 해서 다음 선의 시점과 합쳐서 +1되는 것으로 한다.
                {
                    result.second = 0;
                }
            }
            else // 현재 시점이 시점보다 더 고위도일 때는 카운트하지 않는다.
            {
                result.second = 0;
            }
        }
    }
    else
    {
        if (ccw(segment_start_latitude, segment_start_longitude, segment_end_latitude, segment_end_longitude, current_location_latitude, current_location_longitude) * ccw(segment_start_latitude, segment_start_longitude, segment_end_latitude, segment_end_longitude, 900000000, current_location_longitude) < 0 &&
            ccw(current_location_latitude, current_location_longitude, 900000000, current_location_longitude, segment_start_latitude, segment_start_longitude) * ccw(current_location_latitude, current_location_longitude, 900000000, current_location_longitude, segment_end_latitude, segment_end_longitude) < 0) // 2점만 일직선인 경우에 선분의 교차 여부 판별
        {
            result.second = 1;
        }
        else if (ccw(segment_start_latitude, segment_start_longitude, segment_end_latitude, segment_end_longitude, current_location_latitude, current_location_longitude) == 0) // 3점이 일직선인 경우 중 직선이 현 위치(경선의 시점)에 접할 때
        {
            if (segment_start_latitude == current_location_latitude && segment_start_longitude == current_location_longitude) // 현 위치가 선분의 시점과 만나면 도형 내부에 속하는 것으로 처리
            {
                result.first = true;
            }
            else if (segment_end_latitude == current_location_latitude && segment_end_longitude == current_location_longitude) // 현 위치가 선분의 종점과 만나면 만나지 않는 것으로 처리
            {
                result.second = 0;
            }
            else
            {
                if ((segment_start_latitude < segment_end_latitude && current_location_latitude >= segment_start_latitude && current_location_latitude < segment_end_latitude) ||
                    (segment_start_latitude > segment_end_latitude && current_location_latitude < segment_start_latitude && current_location_latitude >= segment_end_latitude) ||
                    (segment_start_latitude == segment_end_latitude && segment_start_latitude == current_location_latitude)) // 현 위치가 선분의 내부에 있으면 도형 내부에 속하는 것으로 처리
                {
                    result.first = true;
                }
                else // 현 위치가 선분의 외부에 있으면 만나지 않는 것으로 처리
                {
                    result.second = 0;
                }
            }
        }
        else if (ccw(current_location_latitude, current_location_longitude, 900000000, current_location_longitude, segment_start_latitude, segment_start_longitude) == 0) // 3점이 일직선인 경우 중 경선이 선분의 시점에 접할 때
        {
            if (segment_start_latitude == current_location_latitude && segment_start_longitude == current_location_longitude) // 선분의 시점이 현 위치와 만나면 도형 내부에 속하는 것으로 처리
            {
                result.first = true;
            }
            else
            {
                if (current_location_latitude <= segment_start_latitude) // 선분의 시점이 경선 선분 내부에 있으면 만나는 것으로 처리. 단, 그 점이 접점이라면 만나지 않는 것으로 처리
                {
                    if ((segment_end_longitude - segment_start_longitude) * (segment_start_longitude - segment_previous_start_longitude) < 0)
                    {
                        result.second = 0;
                    }
                    else
                    {
                        result.second = 1;
                    }
                }
                else // 선분의 시점이 경선 선분 외부에 있으면 만나지 않는 것으로 처리
                {
                    result.second = 0;
                }
            }
        }
        else // 2점만 일직선인데 서로 만나지 않는 경우
        {
            result.second = 0;
        }
    }

    return result;
}

bool lineInPolygon(const std::vector<std::pair<int32_t, int32_t>> &locations, int32_t start_longitude, int32_t start_latitude, int32_t end_longitude, int32_t end_latitude)
{ // 한 선분이 다각형 안에 완전히 포함되는지 여부 반환

    if (!vp::insidePolygonIndicator(locations, start_latitude, start_longitude) || !vp::insidePolygonIndicator(locations, end_latitude, end_longitude))
    {
        return false;
    }

    for (uint64_t i = 0; i < locations.size(); i++)
    {
        std::pair<int32_t, int32_t> here{};
        std::pair<int32_t, int32_t> there{};
        if (i == locations.size() - 1)
        {
            here = locations[i];
            there = locations[0];
        }
        else
        {
            here = locations[i];
            there = locations[i + 1];
        }
        double denominator = static_cast<double>(there.first - here.first) / (there.second - here.second) - static_cast<double>(end_latitude - start_latitude) / (end_longitude - start_longitude);
        if (denominator != 0.0)
        {
            auto numerator = end_latitude - there.first + static_cast<double>(there.first - here.first) / (there.second - here.second) * there.second - static_cast<double>(end_latitude - start_latitude) / (end_longitude - start_longitude) * end_longitude;
            auto intersection_x = static_cast<double>(numerator / denominator);
            if (intersection_x > std::max({std::min({there.second, here.second}), std::min({end_longitude, start_longitude})}) && intersection_x < std::min({std::max({there.second, here.second}), std::max({end_longitude, start_longitude})}))
            {
                return false;
            }
        }
    }

    return true;
}

uint32_t getPolygonId(ZoneDetectResult *zd_result, const std::string &alpha3_code)
{
    for (auto *r = zd_result; r != nullptr && r->lookupResult != ZD_LOOKUP_END; ++r) // NOLINT: 외부 C 라이브러리
    {
        // 지점이 해당 폴리곤 안/경계(정점/변)인 경우만 대상
        switch (r->lookupResult)
        {
        case ZD_LOOKUP_IN_ZONE:
        case ZD_LOOKUP_ON_BORDER_VERTEX:
        case ZD_LOOKUP_ON_BORDER_SEGMENT:
            break;
        default:
            continue;
        }

        if (r->fieldNames == nullptr || r->data == nullptr)
        {
            continue;
        }

        for (uint8_t i = 0; i < r->numFields; ++i)
        {
            // 복사. 속도 중요할 시 char * 로 쓰는것도 검토 필요
            std::string key = {r->fieldNames[i]}; // NOLINT: 외부 C API
            std::string value = {r->data[i]};     // NOLINT: 외부 C API

            if (key.empty() || value.empty())
            {
                continue;
            }

            if (::isAlpha3Key(key) && alpha3_code == value)
            {
                return r->polygonId;
            }
        }
    }

    return 0;
}

// 원 중심이 Polygon 내에 있다는것을 확인한 뒤 호출 필요
// 원의 반지름이 다각형의 가장 가까운 선분 위의 점과 원의 중심
bool checkCircleInsidePolygonDegRaw(const double *latlon_deg_interleaved,
                                    size_t count_pairs,
                                    double center_lat_deg,
                                    double center_lon_deg,
                                    double radius_m)
{
    if (count_pairs < kMinPolygonPoints || latlon_deg_interleaved == nullptr || !(radius_m > 0.0))
    {
        LOG_DBG("invalid input. count_pairs: {}, radius_m: {}", count_pairs, radius_m);
        return false;
    }

    // 위도/경도 → 거리 환산 (지역 평면 근사)
    const auto meter_per_degree_lat = kMetersPerDegLat;
    const auto meter_per_degree_lon = meter_per_degree_lat * std::cos(center_lat_deg * kDegToRadFactor);

    // 원 반지름의 제곱
    const auto r2 = radius_m * radius_m;

    // 현재까지 찾은 중심→경계 최소거리의 제곱을 저장하기 위한 변수
    auto min_distance2 = std::numeric_limits<double>::infinity();

    // 다각형의 각 변(선분)에 대해 검사
    // 중심을 (0,0)으로 옮겨서 좌표 계산 (미터 단위)
    for (size_t index = 0; index < count_pairs; ++index)
    {
        // 다음 꼭지점의 인덱스, 마지막이 아니면 index + 1, 마지막이면 0
        const size_t next_index = (index + 1 != count_pairs) ? (index + 1) : 0;

        // 꼭짓점 좌표 (위도/경도)
        const auto lat_i = latlon_deg_interleaved[2 * index + 0];      // NOLINT: 외부 C Library
        const auto lon_i = latlon_deg_interleaved[2 * index + 1];      // NOLINT: 외부 C Library
        const auto lat_j = latlon_deg_interleaved[2 * next_index + 0]; // /NOLINT: 외부 C Library
        const auto lon_j = latlon_deg_interleaved[2 * next_index + 1]; // /NOLINT: 외부 C Library

        // 중심을 원점(0,0)으로 두는 지역 평면 좌표 변환 (단위: m) - 각 좌표에 원의 중심값을 빼고 위도 경도를 m로 환산
        const auto ax = (lon_i - center_lon_deg) * meter_per_degree_lon; // x
        const auto ay = (lat_i - center_lat_deg) * meter_per_degree_lat; // y
        const auto bx = (lon_j - center_lon_deg) * meter_per_degree_lon;
        const auto by = (lat_j - center_lat_deg) * meter_per_degree_lat;

        // 원 중심(0,0)에서 선분 AB까지의 최소 거리의 제곱
        min_distance2 = std::min(min_distance2, ::pointSegmentDistSquared({0.0, 0.0}, {ax, ay}, {bx, by}));

        // 만약 경계까지 최소 거리가 반지름보다 짧으면 원이 밖으로 나감
        if (min_distance2 < r2)
        {
            return false;
        }
    }

    return min_distance2 >= r2;
}

// 원 중심이 Polygon 내에 있다는것을 확인한 뒤 호출 필요
bool checkCircleInsidePolygonDeg(const std::vector<Point> &locations,
                                 double center_lat_deg,
                                 double center_lon_deg,
                                 double radius_m)
{
    if (locations.size() < kMinPolygonPoints)
    {
        return false;
    }

    // 일반적으론 한 번의 힙 할당으로 interleaved 버퍼를 만든 뒤 raw 버전을 호출
    std::vector<double> interleaved;
    interleaved.reserve(locations.size() * 2);
    for (const auto &p : locations)
    {
        interleaved.push_back(p.x); // lat
        interleaved.push_back(p.y); // lon
    }

    return checkCircleInsidePolygonDegRaw(interleaved.data(),
                                          locations.size(),
                                          center_lat_deg,
                                          center_lon_deg,
                                          radius_m);
}

FastCheckResult circleInsideCountryFastCheck(double radius_m, double radius_padding_m, double safezone_deg, double promote_threshold_m)
{
    const auto meters_per_deg_lat = kMetersPerDegLat;
    const auto radius_deg_lat = (radius_m + radius_padding_m) / meters_per_deg_lat;
    const auto safezone_deg_d = safezone_deg;

    LOG_DBG("radius degree: {} | safezone degree: {}", radius_deg_lat, safezone_deg_d);
    if (radius_deg_lat <= safezone_deg_d)
    {
        const double margin_m = (safezone_deg_d - radius_deg_lat) * meters_per_deg_lat;
        LOG_DBG("margin (m): {} | promote_threshold (m): {} ", margin_m, promote_threshold_m);
        if (margin_m >= promote_threshold_m)
        {
            LOG_DBG("SUCCESS");
            return FastCheckResult::SUCCESS;
        }
        else
        {
            // 여유가 너무 작으면 오탐 방지를 위해 정밀판정 승격
            LOG_DBG("Need precise checking...");
            return FastCheckResult::NEED_TO_PRECISE_CHECK;
        }
    }

    // 반지름이 가장 가까운 경계를 벗어남
    LOG_DBG("The circle crosses the national border.");
    return FastCheckResult::OUT_OF_BORDER;
}

bool circleInsideCountryPreciseCheck(uint32_t polygon_id, double center_lat_deg, double center_lon_deg, double radius_m)
{
    LOG_DBG("Precise checking...");

    size_t raw_n = 0;
    std::unique_ptr<float, decltype(&std::free)> raw(
        ZDPolygonToList(_country_db.get(), polygon_id, &raw_n),
        &std::free);

    constexpr auto kMinRawN = 6; // 최소 3점(=6값) 필요
    if (raw == nullptr || raw_n < kMinRawN)
    {
        return false;
    }

    std::vector<Point> ring_double;
    ring_double.reserve(raw_n / 2);

    for (size_t i = 0; i + 1 < raw_n; i += 2) // NOLINT
    {
        const auto lat_i = static_cast<double>(raw.get()[i]);     // NOLINT: 외부 라이브러리
        const auto lon_i = static_cast<double>(raw.get()[i + 1]); // NOLINT: 외부 라이브러리
        ring_double.emplace_back(Point{lat_i, lon_i});
    }

    return ::checkCircleInsidePolygonDeg(ring_double, center_lat_deg, center_lon_deg, radius_m);
}
} // namespace

namespace vp
{

std::string getCountryCodeFromLatLon(float latitude,
                                     float longitude,
                                     const std::string &country_db_path)
{
    ::dbInit(country_db_path);

    std::unique_ptr<char, decltype(&std::free)> country_a3_ptr{
        ZDHelperSimpleLookupString(_country_db.get(), latitude, longitude),
        &std::free};

    std::string country_a3_code;
    if (country_a3_ptr)
    {
        country_a3_code = country_a3_ptr.get();
    }
    return country_a3_code;
}

bool insidePolygonIndicator(const std::vector<std::pair<int32_t, int32_t>> &locations,
                            int32_t current_location_latitude, int32_t current_location_longitude)
{
    int64_t number_of_cross = 0;

    for (size_t index = 0; index < locations.size() - 3; ++index)
    {
        if (lineSegmentIntersection(locations[index].second,
                                    locations[index + 1].first, locations[index + 1].second,
                                    locations[index + 2].first, locations[index + 2].second,
                                    locations[index + 3].second,
                                    current_location_latitude, current_location_longitude)
                .first)
        {
            return true;
        }
        number_of_cross += lineSegmentIntersection(locations[index].second,
                                                   locations[index + 1].first, locations[index + 1].second,
                                                   locations[index + 2].first, locations[index + 2].second,
                                                   locations[index + 3].second,
                                                   current_location_latitude, current_location_longitude)
                               .second;
    }

    if (lineSegmentIntersection(locations[locations.size() - 3].second,
                                locations[locations.size() - 2].first, locations[locations.size() - 2].second,
                                locations[locations.size() - 1].first, locations[locations.size() - 1].second,
                                locations[0].second,
                                current_location_latitude, current_location_longitude)
            .first)
    {
        return true;
    }
    if (lineSegmentIntersection(locations[locations.size() - 2].second,
                                locations[locations.size() - 1].first, locations[locations.size() - 1].second,
                                locations[0].first, locations[0].second,
                                locations[1].second,
                                current_location_latitude, current_location_longitude)
            .first)
    {
        return true;
    }
    if (lineSegmentIntersection(locations[locations.size() - 1].second,
                                locations[0].first, locations[0].second,
                                locations[1].first, locations[1].second,
                                locations[2].second,
                                current_location_latitude, current_location_longitude)
            .first)
    {
        return true;
    }
    number_of_cross += lineSegmentIntersection(locations[locations.size() - 3].second,
                                               locations[locations.size() - 2].first, locations[locations.size() - 2].second,
                                               locations[locations.size() - 1].first, locations[locations.size() - 1].second,
                                               locations[0].second,
                                               current_location_latitude, current_location_longitude)
                           .second;

    number_of_cross += lineSegmentIntersection(locations[locations.size() - 2].second,
                                               locations[locations.size() - 1].first, locations[locations.size() - 1].second,
                                               locations[0].first, locations[0].second,
                                               locations[1].second,
                                               current_location_latitude, current_location_longitude)
                           .second;
    number_of_cross += lineSegmentIntersection(locations[locations.size() - 1].second,
                                               locations[0].first, locations[0].second,
                                               locations[1].first, locations[1].second,
                                               locations[2].second,
                                               current_location_latitude, current_location_longitude)
                           .second;

    LOG_DBG("number of cross : {}", number_of_cross);

    if (number_of_cross % 2 == 0)
    {
        LOG_INF("The given gen_location is out of the polygonal region : lat({}), lon({})", current_location_latitude, current_location_longitude);
    }

    return static_cast<bool>(number_of_cross % 2);
}

bool polygonInsidePolygonIndicator(const std::vector<std::pair<int32_t, int32_t>> &locations,
                                   const std::vector<std::pair<int32_t, int32_t>> &otherlocations)
{
    for (uint64_t i = 0; i < otherlocations.size() - 1; i++)
    {
        const auto &start = otherlocations[i];
        const auto &end = otherlocations[i + 1];
        if (!lineInPolygon(locations, start.second, start.first, end.second, end.first))
        {
            return false;
        }
    }

    auto last_vertex_number = otherlocations.size();

    return lineInPolygon(locations, otherlocations[0].second, otherlocations[0].first, otherlocations[last_vertex_number - 1].second, otherlocations[last_vertex_number - 1].first);
}

bool insideRectangleIndicator(const std::vector<Rectangle> &rectangles,
                              int32_t current_location_latitude, int32_t current_location_longitude)
{
    for (const auto &rectangle : rectangles)
    {
        if (current_location_latitude <= rectangle.northLimit && current_location_latitude >= rectangle.southLimit &&
            current_location_longitude <= rectangle.eastLimit && current_location_longitude >= rectangle.westLimit)
        {
            return true;
        }
    }

    LOG_DBG("gen_location    : lat({}), lon({})", current_location_latitude, current_location_longitude);
    for (const auto &rectangle : rectangles)
    {
        LOG_DBG("region    : northWest({}, {}), southEast({}, {})",
                rectangle.northLimit, rectangle.westLimit,
                rectangle.southLimit, rectangle.eastLimit);
    }
    return false;
}

bool circleInsidePolygon(const std::vector<std::pair<int32_t, int32_t>> &locations, int32_t center_lat, int32_t center_lon, double radius_m)
{
    if (locations.size() < 3)
    {
        return false;
    }

    // 1) 중심이 폴리곤 내부인지 확인
    if (!vp::insidePolygonIndicator(locations, center_lat, center_lon))
    {
        return false;
    }

    // 2) deg로 변환
    std::vector<Point> ring_deg;

    ring_deg.reserve(locations.size());
    for (const auto &p : locations)
    {
        // pair<double,double> = {lat_deg, lon_deg}
        ring_deg.emplace_back(Point{::toDegree(p.first), ::toDegree(p.second)});
    }

    const double center_lat_deg = ::toDegree(center_lat);
    const double center_lon_deg = ::toDegree(center_lon);

    // 3) 실제 판단
    return checkCircleInsidePolygonDeg(ring_deg, center_lat_deg, center_lon_deg, radius_m);
}

// (2) 빠른판정 + 정밀판정
bool circleInsideCountryAt(int32_t center_lat,
                           int32_t center_lon,
                           uint16_t m49_country_code,
                           double radius_m,
                           const std::string &country_db_path,
                           double radius_padding_m,   // 경계 여유(미터): 반지름에 더해 보수적으로 판단
                           double promote_threshold_m // margin이 이 값 미만이면 정밀판정으로 승격
)
{
    ::dbInit(country_db_path);

    const auto alpha3 = m49ToAlpha3(m49_country_code);
    if (alpha3.empty())
    {
        THROWLOG(SysException, "circleInsideCountryAt: invalid country_code");
    }

    // normalize된 위경도(도 단위)로 변환
    const auto center_lat_deg = ::toDegree(center_lat);
    const auto center_lon_deg = ::toDegree(center_lon);

    if (center_lat_deg < kMinLat || center_lat_deg > kMaxLat ||
        center_lon_deg < kMinLon || center_lon_deg > kMaxLon)
    {
        LOG_DBG("Invalid Latitude or Longitude. lat(raw): {} | lon(raw): {}", center_lat, center_lon);
        return false;
    }

    // 1) ZDLookup: safezone(위도 도) 및 해당 지점의 국가 폴리곤 확보
    auto safezone_deg = 0.0F;

    std::unique_ptr<ZoneDetectResult, decltype(&ZDFreeResults)> zd_result(
        ZDLookup(_country_db.get(), center_lat_deg, center_lon_deg, &safezone_deg), &ZDFreeResults);

    if (!zd_result)
    {
        LOG_DBG("ZDLookup returned nullptr. lat: {} | lon: {}", center_lat_deg, center_lon_deg);
        return false;
    }

    const auto polygon_id = ::getPolygonId(zd_result.get(), alpha3);
    if (polygon_id == 0)
    {
        LOG_DBG("Cannot find polygon id for country {}", alpha3);
        return false;
    }

    // 2) 빠른 판정
    const auto fast_check_result = ::circleInsideCountryFastCheck(radius_m, radius_padding_m, safezone_deg, promote_threshold_m);

    switch (fast_check_result)
    {
    case FastCheckResult::SUCCESS:
        return true;
    case FastCheckResult::OUT_OF_BORDER:
        return false;
    case FastCheckResult::NEED_TO_PRECISE_CHECK:
    default:
        break;
    }

    // 3) 정밀판정
    LOG_DBG("Precise checking... polygon_id: {}, lat: {}, lon: {}, r(m): {}",
            polygon_id, center_lat_deg, center_lon_deg, radius_m);

    return ::circleInsideCountryPreciseCheck(polygon_id, center_lat_deg, center_lon_deg, radius_m);
}

} // namespace vp