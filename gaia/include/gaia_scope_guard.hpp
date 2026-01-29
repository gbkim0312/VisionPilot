#pragma once

#include <utility> // for forward

namespace autocrypt
{

class ScopeGuardBase
{
public:
    ScopeGuardBase() : commit_(false) {}
    void commit() const noexcept { commit_ = true; }

    ScopeGuardBase(const ScopeGuardBase &) = delete;
    ScopeGuardBase &operator=(const ScopeGuardBase &) = delete;

protected:
    ScopeGuardBase(ScopeGuardBase &&other)
        : commit_(other.commit_) { other.commit(); }
    ~ScopeGuardBase() {}

    mutable bool commit_;
};

template <typename Func>
class ScopeGuard : public ScopeGuardBase
{
public:
    ScopeGuard(Func &&func) : func_(func) {}
    ScopeGuard(const Func &func) : func_(func) {}

    ~ScopeGuard()
    {
        if (!commit_)
        {
            func_();
        }
    }
    ScopeGuard(ScopeGuard &&other)
        : ScopeGuardBase(std::move(other)),
          func_(other.func_) {}

private:
    Func func_;
};

template <typename Func>
ScopeGuard<Func> MakeGuard(Func &&func)
{
    return ScopeGuard<Func>(std::forward<Func>(func));
}

} // namespace autocrypt