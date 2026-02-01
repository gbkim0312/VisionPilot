#include "assembly.hpp"
#include "assembly_impl.hpp"
#include "gaia_log.hpp"

namespace vp::assembly
{
Assembly::Assembly(const config::AssemblyConfig &config)
    : impl_(std::make_unique<AssemblyImpl>(config))
{
    LOG_TRA("");
}

Assembly::~Assembly()
{
    LOG_TRA("");
}

void Assembly::startService()
{
    LOG_TRA("");
    impl_->startService();
}

void Assembly::stopService()
{
    LOG_TRA("");
    impl_->stopService();
}
} // namespace vp::assembly