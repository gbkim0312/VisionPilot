#pragma once

#include "assembly_config.hpp"
#include <memory>

namespace vp::assembly
{

class AssemplyImpl;

class Assembly
{
public:
    Assembly(const config::AssemblyConfig &config);
    ~Assembly();

    void startService();
    void stopService();

private:
    std::unique_ptr<AssemplyImpl> impl_;
};
} // namespace vp::assembly
