#include "assembly_config.hpp"

namespace vp::assembly
{
class AssemblyImpl
{
public:
    AssemblyImpl(const config::AssemblyConfig &config);
    ~AssemblyImpl();

    void startService();
    void stopService();

private:
    const config::AssemblyConfig &config_;
};
} // namespace vp::assembly