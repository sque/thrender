/*  libsimdpp
    Copyright (C) 2012  Povilas Kanapickas tir5c3@yahoo.co.uk

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DETAIL_NEON_MATH_INT_H
#define LIBSIMDPP_DETAIL_NEON_MATH_INT_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

namespace simdpp {
SIMDPP_ARCH_NAMESPACE_BEGIN
namespace neon {

/** Multiplies the values of two int8x16 vectors and returns the low
    part of the multiplication

    @code
    r0 = low(a0 * b0)
    ...
    r15 = low(a15 * b15)
    @endcode
*/
inline int128 mul_lo(basic_int8x16 a, basic_int8x16 b)
{
    return vmulq_u8(a, b);
}

/** Multiplies the first 8 values of two signed int8x16 vectors and expands the
    results to 16 bits.

    @code
    r0 = a0 * b0
    ...
    r7 = a7 * b7
    @endcode
*/
inline int128 mull_lo(int8x16 a, int8x16 b)
{
    return vmullq_s8(vget_low_s8(a), vget_low_s8(b));
}

/** Multiplies the first 8 values of two unsigned int8x16 vectors and expands the
    results to 16 bits.

    @code
    r0 = a0 * b0
    ...
    r7 = a7 * b7
    @endcode
*/
inline int128 mull_lo(uint8x16 a, uint8x16 b)
{
    return vmullq_u8(vget_low_u8(a), vget_low_u8(b));
}

/** Multiplies the last 8 values of two signed int8x16 vectors and expands the
    results to 16 bits.

    @code
    r0 = a8 * b8
    ...
    r7 = a15 * b15
    @endcode
*/
inline int128 mull_hi(int8x16 a, int8x16 b)
{
    return vmullq_s8(vget_high_s8(a), vget_high_s8(b));
}


/** Multiplies the last 8 values of two unsigned int8x16 vectors and expands
    the results to 16 bits.

    @code
    r0 = a8 * b8
    ...
    r7 = a15 * b15
    @endcode
*/
inline int128 mull_hi(uint8x16 a, uint8x16 b)
{
    return vmullq_u8(vget_high_u8(a), vget_high_u8(b));
}

} // namespace neon
SIMDPP_ARCH_NAMESPACE_END
} // namespace simdpp

#endif