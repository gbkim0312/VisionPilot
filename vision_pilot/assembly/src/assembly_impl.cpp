#include "assembly_impl.hpp"
#include "gaia_log.hpp"

namespace vp::assembly
{
AssemblyImpl::AssemblyImpl(const config::AssemblyConfig &config)
    : config_{config}
{
    LOG_TRA("");
}

AssemblyImpl::~AssemblyImpl()
{
    LOG_TRA("");
}

void AssemblyImpl::startService()
{
    LOG_TRA("");
}

void AssemblyImpl::stopService()
{
    LOG_TRA("");
}
} // namespace vp::assembly