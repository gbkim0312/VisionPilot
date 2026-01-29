#include "gaia_geography.hpp"
#include "gaia_log.hpp"
#include "gaia_octstr.hpp"
#include "gaia_time.hpp"
#include <gtest/gtest.h>

namespace
{
constexpr auto kLatLonScale = 1e7;
constexpr auto kCountryDbPath = "country21.bin";
} // namespace

namespace vp
{

TEST(Geography, RectangularRegion)
{
    std::vector<Rectangle> testRectangleVector{};
    testRectangleVector.push_back(Rectangle{42, 20, 30, 1});
    testRectangleVector.push_back(Rectangle{60, 12, 93, 26});
    testRectangleVector.push_back(Rectangle{72, 37, 55, 18});

    ASSERT_TRUE(insideRectangleIndicator(testRectangleVector, 40, 29));    // 세 사각형에 모두 포함
    ASSERT_TRUE(insideRectangleIndicator(testRectangleVector, 40, 40));    // 두 사각형에 포함
    ASSERT_TRUE(insideRectangleIndicator(testRectangleVector, 40, 80));    // 한 사각형에 포함
    ASSERT_TRUE(insideRectangleIndicator(testRectangleVector, 60, 93));    // 한 사각형의 경계에 있음
    ASSERT_FALSE(insideRectangleIndicator(testRectangleVector, 100, 100)); // 모든 사각형에 포함되지 않음
}

TEST(Geography, PolygonalRegion)
{
    std::vector<std::pair<int32_t, int32_t>> vertexOfPolygon{};
    vertexOfPolygon.push_back(std::make_pair(200000000, 1000000000));
    vertexOfPolygon.push_back(std::make_pair(400000000, 1000000000));
    vertexOfPolygon.push_back(std::make_pair(500000000, 1100000000));
    vertexOfPolygon.push_back(std::make_pair(450000000, 1250000000));
    vertexOfPolygon.push_back(std::make_pair(350000000, 1150000000));
    vertexOfPolygon.push_back(std::make_pair(300000000, 1300000000));
    vertexOfPolygon.push_back(std::make_pair(150000000, 1200000000));

    // 다각형의 변이 경선의 일부일 경우 : 위치에 따라 내부와 외부가 갈린다.
    EXPECT_FALSE(insidePolygonIndicator(vertexOfPolygon, 100000000, 1000000000)); // 2(수는 교차횟수, bool은 내부 여부)
    EXPECT_TRUE(insidePolygonIndicator(vertexOfPolygon, 200000000, 1000000000));  // true
    EXPECT_TRUE(insidePolygonIndicator(vertexOfPolygon, 300000000, 1000000000));  // true
    EXPECT_TRUE(insidePolygonIndicator(vertexOfPolygon, 400000000, 1000000000));  // true
    EXPECT_FALSE(insidePolygonIndicator(vertexOfPolygon, 500000000, 1000000000)); // 0

    // 다각형 내부의 점과 북극을 잇는 선분이 다각형의 변과 교차하는 경우 : 내부
    EXPECT_TRUE(insidePolygonIndicator(vertexOfPolygon, 350000000, 1050000000)); // 1

    // 다각형 내부의 점과 북극을 잇는 선분이 다각형의 꼭지점과 교차하는 경우 : 내부
    EXPECT_TRUE(insidePolygonIndicator(vertexOfPolygon, 200000000, 1100000000)); // 1

    // 다각형 외부의 점과 북극을 잇는 선분이 다각형과 교차하는 경우 : 외부
    EXPECT_FALSE(insidePolygonIndicator(vertexOfPolygon, 100000000, 1050000000)); // 2
    EXPECT_FALSE(insidePolygonIndicator(vertexOfPolygon, 100000000, 1100000000)); // 2

    // 다각형 내부의 점과 북극을 잇는 선분이 다각형에 접하는 경우 : 내부
    EXPECT_TRUE(insidePolygonIndicator(vertexOfPolygon, 200000000, 1150000000)); // 1

    // 다각형 외부의 점과 북극을 잇는 선분이 다각형에 접하는 경우 : 전부 외부여야 한다.
    EXPECT_FALSE(insidePolygonIndicator(vertexOfPolygon, 100000000, 1150000000)); // 2
    EXPECT_FALSE(insidePolygonIndicator(vertexOfPolygon, 200000000, 1300000000)); // 0

    // 현 위치가 다각형의 꼭지점인 경우 : 전부 내부여야 한다.
    EXPECT_TRUE(insidePolygonIndicator(vertexOfPolygon, 350000000, 1150000000)); // true
    EXPECT_TRUE(insidePolygonIndicator(vertexOfPolygon, 300000000, 1300000000)); // true

    // 오목다각형의 경우
    EXPECT_TRUE(insidePolygonIndicator(vertexOfPolygon, 300000000, 1180000000));  // 내부 // 3
    EXPECT_TRUE(insidePolygonIndicator(vertexOfPolygon, 340000000, 1180000000));  // 경계는 내부 // true
    EXPECT_FALSE(insidePolygonIndicator(vertexOfPolygon, 350000000, 1180000000)); // 외부 // 2
}

TEST(Geography, PolygonInclusion)
{
    std::vector<std::pair<int32_t, int32_t>> vertexOfOuterPolygon{};
    vertexOfOuterPolygon.push_back(std::make_pair(1, 6));
    vertexOfOuterPolygon.push_back(std::make_pair(4, 8));
    vertexOfOuterPolygon.push_back(std::make_pair(4, 6));
    vertexOfOuterPolygon.push_back(std::make_pair(6, 8));
    vertexOfOuterPolygon.push_back(std::make_pair(3, 2));
    vertexOfOuterPolygon.push_back(std::make_pair(3, 5));
    vertexOfOuterPolygon.push_back(std::make_pair(1, 4));
    vertexOfOuterPolygon.push_back(std::make_pair(0, 5));
    std::vector<std::pair<int32_t, int32_t>> vertexOfInnerPolygon1{};
    vertexOfInnerPolygon1.push_back(std::make_pair(2, 3));
    vertexOfInnerPolygon1.push_back(std::make_pair(3, 8));
    vertexOfInnerPolygon1.push_back(std::make_pair(4, 2));
    std::vector<std::pair<int32_t, int32_t>> vertexOfInnerPolygon2{};
    vertexOfInnerPolygon2.push_back(std::make_pair(1, 5));
    vertexOfInnerPolygon2.push_back(std::make_pair(4, 5));
    vertexOfInnerPolygon2.push_back(std::make_pair(3, 7));
    std::vector<std::pair<int32_t, int32_t>> vertexOfInnerPolygon3{};
    vertexOfInnerPolygon3.push_back(std::make_pair(1, 6));
    vertexOfInnerPolygon3.push_back(std::make_pair(4, 6));
    vertexOfInnerPolygon3.push_back(std::make_pair(4, 8));
    std::vector<std::pair<int32_t, int32_t>> vertexOfInnerPolygon4{};
    vertexOfInnerPolygon4.push_back(std::make_pair(2, 6));
    vertexOfInnerPolygon4.push_back(std::make_pair(3, 6));
    vertexOfInnerPolygon4.push_back(std::make_pair(4, 5));
    std::vector<std::pair<int32_t, int32_t>> arrowpolygon{};
    arrowpolygon.push_back(std::make_pair(0, 0));
    arrowpolygon.push_back(std::make_pair(-3, 2));
    arrowpolygon.push_back(std::make_pair(6, 0));
    arrowpolygon.push_back(std::make_pair(-3, -2));
    std::vector<std::pair<int32_t, int32_t>> rectangle{};
    rectangle.push_back(std::make_pair(-1, -1));
    rectangle.push_back(std::make_pair(1, -1));
    rectangle.push_back(std::make_pair(1, 1));
    rectangle.push_back(std::make_pair(-1, 1));

    EXPECT_FALSE(polygonInsidePolygonIndicator(vertexOfOuterPolygon, vertexOfInnerPolygon1)); // 오버랩되지만 포함하지 않음
    EXPECT_TRUE(polygonInsidePolygonIndicator(vertexOfOuterPolygon, vertexOfInnerPolygon2));  // 점 하나가 겹친 채 포함됨
    EXPECT_TRUE(polygonInsidePolygonIndicator(vertexOfOuterPolygon, vertexOfInnerPolygon3));  // 두 변이 겹친 채 포함됨
    EXPECT_TRUE(polygonInsidePolygonIndicator(vertexOfOuterPolygon, vertexOfInnerPolygon4));  // 완전하게 포함됨
    EXPECT_FALSE(polygonInsidePolygonIndicator(arrowpolygon, rectangle));
}

TEST(Geography, CircleInsideCountryAt_Jeju_PositiveAndNegative)
{
    constexpr auto kKorM49 = 410;
    constexpr auto kCenterLatI7 = 334527000;  // 33.4527000
    constexpr auto kCenterLonI7 = 1268470000; // 126.8470000
    constexpr auto kRadiusInsideCountry = 500.0;
    constexpr auto kEpsilon = 1.0;

    // 반지름 500m → safezone 빠른판정에서 통과(또는 근접 시 정밀판정 성공) 기대
    {
        const double r_small_m = 500.0;
        const double epsilon_m = 1.0;          // 경계 여유
        const double promote_threshold = 50.0; // 여유 작으면 정밀로 승격
        auto is_inside = circleInsideCountryAt(kCenterLatI7, kCenterLonI7,
                                               kKorM49, r_small_m, kCountryDbPath,
                                               epsilon_m, promote_threshold);
        ASSERT_TRUE(is_inside);
    }

    // 반지름 20km → 실패 기대 (섬 경계에 걸려 나가도록)
    {
        const double r_large_m = 20000.0;
        const double epsilon_m = 1.0;
        const double promote_threshold = 50.0;
        auto is_inside = circleInsideCountryAt(kCenterLatI7, kCenterLonI7,
                                               kKorM49, r_large_m, kCountryDbPath,
                                               epsilon_m, promote_threshold);
        ASSERT_FALSE(is_inside);
    }
}

TEST(Geography, CircleInsideCountryAt_Jeju_PositiveAndNegative_Precise)
{
    constexpr auto kKorM49 = 410;
    constexpr auto kCenterLatI7 = 334527000;  // 33.4527000
    constexpr auto kCenterLonI7 = 1268470000; // 126.8470000

    // 반지름 500m → safezone 빠른판정에서 통과
    {
        const double r_small_m = 500.0;
        const double epsilon_m = 1.0;          // 경계 여유
        const double promote_threshold = 1e12; // 여유 작으면 정밀로 승격
        auto is_inside = circleInsideCountryAt(kCenterLatI7, kCenterLonI7,
                                               kKorM49, r_small_m, kCountryDbPath,
                                               epsilon_m, promote_threshold);
        ASSERT_TRUE(is_inside);
    }

    // 반지름 20km -> 실패 기대
    {
        const double r_large_m = 20000.0;
        const double epsilon_m = 1.0;
        const double promote_threshold = 1e12;
        auto is_inside = circleInsideCountryAt(kCenterLatI7, kCenterLonI7,
                                               kKorM49, r_large_m, kCountryDbPath,
                                               epsilon_m, promote_threshold);
        ASSERT_FALSE(is_inside);
    }
}

TEST(Geography, CircleInsidePolygon_Inside)
{
    std::vector<std::pair<double, double>> locations;
    // 경복궁 근정전
    auto center_lat = 37.5784286 * kLatLonScale;
    auto center_lon = 126.9765772 * kLatLonScale;

    locations.emplace_back(37.5579397, 126.8027436); // 김포공항
    locations.emplace_back(37.5776322, 127.1356105); // 남구리IC
    locations.emplace_back(37.6419351, 127.1281132); // 별내역
    locations.emplace_back(37.6185158, 126.8212504); // 능곡역

    std::vector<std::pair<int32_t, int32_t>> locations_ie7;
    locations_ie7.reserve(locations.size());
    for (const auto &location : locations)
    {
        locations_ie7.emplace_back(location.first * kLatLonScale, location.second * kLatLonScale);
    }

    constexpr auto kRadiusInside = 500;
    auto result = circleInsidePolygon(locations_ie7, center_lat, center_lon, kRadiusInside);
    ASSERT_TRUE(result);

    constexpr auto kRadiusOutOfBorder = 3000;
    result = circleInsidePolygon(locations_ie7, center_lat, center_lon, kRadiusOutOfBorder);
    ASSERT_FALSE(result);
}

} // namespace vp