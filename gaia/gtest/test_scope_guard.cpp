#include "gaia_scope_guard.hpp"
#include <gtest/gtest.h>

namespace
{
bool finalized = false;
bool rollback_triggered = false;
} // namespace

namespace autocrypt
{

TEST(ScopeGuardTest, LifeCycleCheck)
{
    {
        auto finalize = MakeGuard([&]
                                  { finalized = true; });
    }

    ASSERT_TRUE(finalized);

    try
    {
        auto rollback = MakeGuard([&]
                                  { rollback_triggered = true; });

        rollback.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    ASSERT_FALSE(rollback_triggered);

    try
    {
        auto rollback = MakeGuard([&] // NOLINT (Unused variable): throw로 인해 tidy에서 잡힘
                                  { rollback_triggered = true; });

        throw std::runtime_error("throw!!!");

        rollback.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    ASSERT_TRUE(rollback_triggered);
}

} // namespace autocrypt