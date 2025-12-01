#pragma once

#include <intrin.h>
#include <cstdint>
#include <stdexcept>

struct uint128_t {
    uint64_t lo;
    uint64_t hi;

    // ---- Constructors ----
    constexpr uint128_t(uint64_t low = 0) noexcept : lo(low), hi(0) {}
    constexpr uint128_t(uint64_t high, uint64_t low) noexcept : lo(low), hi(high) {}

    // ---- Basic conversions ----
    constexpr explicit operator bool() const noexcept { return (hi | lo) != 0; }

    constexpr explicit operator uint64_t() const noexcept {
        return lo;
    }

    // ---- Comparisons ----
    friend constexpr bool operator==(const uint128_t& a, const uint128_t& b) noexcept {
        return a.hi == b.hi && a.lo == b.lo;
    }
    friend constexpr bool operator!=(const uint128_t& a, const uint128_t& b) noexcept {
        return !(a == b);
    }
    friend constexpr bool operator<(const uint128_t& a, const uint128_t& b) noexcept {
        return (a.hi < b.hi) || (a.hi == b.hi && a.lo < b.lo);
    }
    friend constexpr bool operator>(const uint128_t& a, const uint128_t& b) noexcept {
        return b < a;
    }
    friend constexpr bool operator<=(const uint128_t& a, const uint128_t& b) noexcept {
        return !(b < a);
    }
    friend constexpr bool operator>=(const uint128_t& a, const uint128_t& b) noexcept {
        return !(a < b);
    }

    // ---- Addition / Subtraction ----
    friend inline uint128_t operator+(const uint128_t& a, const uint128_t& b) noexcept {
        uint64_t lo, carry;
        carry = _addcarry_u64(0, a.lo, b.lo, &lo);
        uint64_t hi = a.hi + b.hi + carry;
        return { hi, lo };
    }

    friend inline uint128_t operator-(const uint128_t& a, const uint128_t& b) noexcept {
        uint64_t lo, borrow;
        borrow = _subborrow_u64(0, a.lo, b.lo, &lo);
        uint64_t hi = a.hi - b.hi - borrow;
        return { hi, lo };
    }

    // ---- Multiplication ----
    friend inline uint128_t operator*(const uint128_t& a, const uint128_t& b) noexcept {
        uint64_t hi_part;
        uint64_t lo_part = _umul128(a.lo, b.lo, &hi_part);
        hi_part += a.lo * b.hi + a.hi * b.lo;
        return { hi_part, lo_part };
    }

    // ---- Division / Modulo ----
    // Only supports division by 64-bit divisor. For full 128/128 division, extend manually.
    friend inline uint128_t operator/(const uint128_t& a, const uint128_t& b) {
        if (b.hi == 0 && b.lo == 0)
            throw std::overflow_error("division by zero");
        if (b.hi != 0)
            throw std::runtime_error("128-bit divisor not supported on MSVC path");
        uint64_t rem;
        uint64_t q = _udiv128(a.hi, a.lo, b.lo, &rem);
        return { 0ULL, q };
    }

    friend inline uint128_t operator%(const uint128_t& a, const uint128_t& b) {
        if (b.hi == 0 && b.lo == 0)
            throw std::overflow_error("division by zero");
        if (b.hi != 0)
            throw std::runtime_error("128-bit divisor not supported on MSVC path");
        uint64_t rem;
        _udiv128(a.hi, a.lo, b.lo, &rem);
        return { 0ULL, rem };
    }

    // ---- Bitwise ----
    friend constexpr uint128_t operator&(const uint128_t& a, const uint128_t& b) noexcept {
        return { a.hi & b.hi, a.lo & b.lo };
    }
    friend constexpr uint128_t operator|(const uint128_t& a, const uint128_t& b) noexcept {
        return { a.hi | b.hi, a.lo | b.lo };
    }
    friend constexpr uint128_t operator^(const uint128_t& a, const uint128_t& b) noexcept {
        return { a.hi ^ b.hi, a.lo ^ b.lo };
    }
    friend constexpr uint128_t operator~(const uint128_t& a) noexcept {
        return { ~a.hi, ~a.lo };
    }

    // ---- Shifts ----
    friend constexpr uint128_t operator<<(const uint128_t& a, unsigned s) noexcept {
        if (s == 0) return a;
        if (s >= 128) return { 0, 0 };
        if (s >= 64) return { a.lo << (s - 64), 0 };
        return { (a.hi << s) | (a.lo >> (64 - s)), a.lo << s };
    }

    friend constexpr uint128_t operator>>(const uint128_t& a, unsigned s) noexcept {
        if (s == 0) return a;
        if (s >= 128) return { 0, 0 };
        if (s >= 64) return { 0, a.hi >> (s - 64) };
        return { a.hi >> s, (a.lo >> s) | (a.hi << (64 - s)) };
    }

    // ---- Compound assignment ----
#define DEFINE_OP_ASSIGN(op) \
        uint128_t& operator op##=(const uint128_t& o) noexcept { *this = *this op o; return *this; }
    DEFINE_OP_ASSIGN(+)
        DEFINE_OP_ASSIGN(-)
        DEFINE_OP_ASSIGN(*)
        DEFINE_OP_ASSIGN(/)
        DEFINE_OP_ASSIGN(%)
        DEFINE_OP_ASSIGN(&)
        DEFINE_OP_ASSIGN(|)
        DEFINE_OP_ASSIGN(^)
        //DEFINE_OP_ASSIGN(<<)
        //DEFINE_OP_ASSIGN(>>)
#undef DEFINE_OP_ASSIGN
};
