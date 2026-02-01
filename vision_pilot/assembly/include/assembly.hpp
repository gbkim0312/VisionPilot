#pragma once

#include "assembly_config.hpp"
#include <memory>

namespace vp::assembly
{

class AssemblyImpl;

class Assembly
{
public:
    Assembly(const config::AssemblyConfig &config);
    ~Assembly();

    void startService();
    void stopService();

private:
    std::unique_ptr<AssemblyImpl> impl_;
};
} // namespace vp::assembly
