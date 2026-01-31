#include "gaia_log.hpp"
#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    vp::logger::logInitFromMain(argc, argv);

    return RUN_ALL_TESTS();
}