#include "gaia_lru_cache.hpp"
#include <benchmark/benchmark.h>
#include <iostream>
#include <random>
#include <sstream>

// Legacy Cache에 대한 벤치마크 코드는 풀 리퀘스트 승인 후 최종 병합 전에 삭제 예정
namespace autocrypt
{
class GaiaLruCacheFixture : public benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State &) override
    {
    }

    void TearDown(const ::benchmark::State &) override
    {
    }
};
// END: Anonymous namespace for benchmark

BENCHMARK_DEFINE_F(GaiaLruCacheFixture, put16667Gaia)

(benchmark::State &state)
{
    try
    {
        double cnt = 0;

        std::random_device random_device{};
        std::mt19937 prng_engine{random_device()};
        std::uniform_int_distribution<uint64_t> random_number(0, UINT64_MAX);
        std::vector<std::pair<uint64_t, std::string>> insert_entries;
        size_t max_size;

        switch (state.range(0))
        {
        case 0: // 꽉 차지 않는 경우
            max_size = 10000000;
            break;
        case 1: // 꽉 채우고 더 밀어넣는 경우
            max_size = 16667;
            break;
        }

        LruCache<uint64_t, std::string> test_map{max_size};

        size_t i = 1;
        do
        {
            i *= 2;
            i %= 100003;

            std::string random_string_256_bit{};
            for (size_t index = 0; index < 4; ++index)
            {
                std::ostringstream ss;
                ss << std::hex << random_number(prng_engine);
            }
            insert_entries.push_back(std::make_pair(i, random_string_256_bit));
        } while (i != 1);

        for (auto _ : state)
        {
            (void)_;
            (void)_;
            for (const auto &entry : insert_entries)
            {
                cnt++;
                test_map.put(entry.first, entry.second);
            }
        }

        state.counters["OPS"] = benchmark::Counter(cnt, benchmark::Counter::kIsRate);
        state.counters["Latency"] = benchmark::Counter(cnt, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    }
    catch (const std::exception &e)
    {
        state.SkipWithError(e.what());
    }
}
BENCHMARK_REGISTER_F(GaiaLruCacheFixture, put16667Gaia)
    ->DenseRange(0, 1)
    ->UseRealTime();

BENCHMARK_DEFINE_F(GaiaLruCacheFixture, get16667Gaia)
(benchmark::State &state)
{
    try
    {
        double cnt = 0;

        std::random_device random_device{};
        std::mt19937 prng_engine{random_device()};
        std::uniform_int_distribution<uint64_t> random_number(0, UINT64_MAX);
        LruCache<uint64_t, std::string> test_map{16667};
        std::vector<std::pair<uint64_t, std::string>> insert_entries;

        size_t i = 1;
        do
        {
            i *= 2;
            i %= 100003;

            std::string random_string_256_bit{};
            for (size_t index = 0; index < 4; ++index)
            {
                std::ostringstream ss;
                ss << std::hex << random_number(prng_engine);
            }
            insert_entries.push_back(std::make_pair(i, random_string_256_bit));
        } while (i != 1);

        for (const auto &entry : insert_entries)
        {
            test_map.put(entry.first, entry.second);
        }

        for (auto _ : state)
        {
            (void)_;
            (void)_;
            for (auto &entry : insert_entries)
            {
                cnt++;
                test_map.get(random_number(prng_engine) % 33334);
            }
        }

        state.counters["OPS"] = benchmark::Counter(cnt, benchmark::Counter::kIsRate);
        state.counters["Latency"] = benchmark::Counter(cnt, benchmark::Counter::kIsRate | benchmark::Counter::kInvert);
    }
    catch (const std::exception &e)
    {
        state.SkipWithError(e.what());
    }
}
BENCHMARK_REGISTER_F(GaiaLruCacheFixture, get16667Gaia)
    ->UseRealTime();
} // namespace autocrypt