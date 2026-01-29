#include <benchmark/benchmark.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    {
        return 1;
    }

    ::benchmark::RunSpecifiedBenchmarks();
}