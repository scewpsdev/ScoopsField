#pragma once


template <typename F>
struct invoke_on_dtor : F
{
    ~invoke_on_dtor() { static_cast<F&>(*this)(); }
};

#define CONCAT(a, b) a ## b
#define CONCAT2(a, b) CONCAT(a, b)

#define defer(x) \
    auto CONCAT2(scope_exit_, __LINE__) = ::invoke_on_dtor{[&] { x; } }
