#include <initrb.h>

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef INITRB_PLATFORM_H
#define INITRB_PLATFORM_H

/**
 * Basic platform detection
 */
#define INITRB_PLATFORM_LINUX 0

#if defined(__GNUC__) && defined(__linux__)
    #define INITRB_PLATFORM INITRB_PLATFORM_LINUX
    #define INITRB_PLATFORM_IS_GNU 1
#else
    #error "Don't know how to build on your platform!"
#endif

/**
 * Limits
 */
#if CHAR_MAX == INT8_MAX || CHAR_MAX == UINT8_MAX
    #define INITRB_INT8_BITS 8
    #define INITRB_CHAR_BITS 8
    #define INITRB_CHAR_BYTES 1
#else
    #error "invalid CHAR_MAX value"
#endif

#if SHRT_MAX == INT16_MAX || SHRT_MAX == UINT16_MAX
    #define INITRB_INT16_BITS 16
    #define INITRB_SHORT_BITS 16
    #define INITRB_SHORT_BYTES 2
#else
    #error "invalid SHRT_MAX value"
#endif

#if INT_MAX == INT32_MAX || INT_MAX == UINT32_MAX
    #define INITRB_INT32_BITS 32
    #define INITRB_INT_BITS 32
    #define INITRB_INT_BYTES 4
#else
    #error "invalid INT_MAX value"
#endif

#if LLONG_MAX == INT64_MAX || LLONG_MAX == UINT64_MAX
    #define INITRB_INT64_BITS 64
    #define INITRB_LLONG_BITS 64
    #define INITRB_LLONG_BYTES 8
#else
    #error "invalid LLONG_MAX value"
#endif

#if SIZE_MAX == INT32_MAX || SIZE_MAX == UINT32_MAX
    #define INITRB_SIZE_BITS 32
    #define INITRB_SIZE_BYTES 4
#elif SIZE_MAX == INT64_MAX || SIZE_MAX == UINT64_MAX
    #define INITRB_SIZE_BITS 64
    #define INITRB_SIZE_BYTES 8
#else
    #error "invalid SIZE_MAX value"
#endif

/**
 * Expect/noexpect
 */
#if INITRB_PLATFORM_IS_GNU
    #define INITRB_EXPECT(expr) (__builtin_expect((expr), true))
    #define INITRB_NOEXPECT(expr) (__builtin_expect((expr), false))
#endif

/**
 * Bit operations
 */
#if INITRB_PLATFORM_IS_GNU
    #if INITRB_INT_BITS == 32
        #define INITRB_CLZ_32(x) ((uint32_t)__builtin_clz((x)))
    #else
        #error "invalid INITRB_INT_BITS value"
    #endif

    #if INITRB_LLONG_BITS == 64
        #define INITRB_CLZ_64(x) ((uint64_t)__builtin_clzll((x)))
    #else
        #error "invalid INITRB_LLONG_BITS value"
    #endif
#endif

#if INITRB_SIZE_BITS == 32
    #define INITRB_CLZ_SIZE(x) INITRB_CLZ_32(x)
#elif INITRB_SIZE_BITS == 64
    #define INITRB_CLZ_SIZE(x) INITRB_CLZ_64(x)
#else
    #error "invalid INITRB_SIZE_BITS value"
#endif

/**
 * Token concatenation
 */
#define INITRB_CONCAT2(a, b) a ## b
#define INITRB_CONCAT(a, b) INITRB_CONCAT2(a, b)

/**
 * Attributes
 */
#define INITRB_PACK __attribute__ ((packed))
#define INITRB_NORETURN __attribute__ ((noreturn))

/**
 * Array counts. From Chromium source.
 */
#define INITRB_COUNT(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

/**
 * Endianness
 */
#define INITRB_HOST64(value) INITRB_ENDIAN_SWAP64(value)
#define INITRB_NET64(value) INITRB_ENDIAN_SWAP64(value)
#define INITRB_HOST32(value) INITRB_ENDIAN_SWAP32(value)
#define INITRB_NET32(value) INITRB_ENDIAN_SWAP32(value)
#define INITRB_HOST16(value) INITRB_ENDIAN_SWAP16(value)
#define INITRB_NET16(value) INITRB_ENDIAN_SWAP16(value)

#define INITRB_ENDIAN_LITTLE 0
#define INITRB_ENDIAN_BIG 1
#define INITRB_ENDIAN_NOP(value) do {value = value;} while (0)
#define INITRB_ENDIAN_CAST(value, type) (*((type *)(&value)))

#if INITRB_PLATFORM_IS_GNU
    #if __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define INITRB_ENDIAN INITRB_ENDIAN_BIG
    #elif __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define INITRB_ENDIAN INITRB_ENDIAN_LITTLE
    #elif __BYTE_ORDER__ && __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
        #error "Please don't build libinitrb on a PDP."
    #else
        #error "Invalid endianness for a gnuc system"
    #endif

    #if INITRB_ENDIAN == INITRB_ENDIAN_BIG
        #define INITRB_ENDIAN_SWAP64(value) INITRB_ENDIAN_NOP(value)
        #define INITRB_ENDIAN_SWAP32(value) INITRB_ENDIAN_NOP(value)
        #define INITRB_ENDIAN_SWAP16(value) INITRB_ENDIAN_NOP(value)
    #elif INITRB_ENDIAN == INITRB_ENDIAN_LITTLE
        #define INITRB_ENDIAN_SWAP64(value) do {INITRB_ENDIAN_CAST(value, uint64_t) = __builtin_bswap64(INITRB_ENDIAN_CAST(value, uint64_t));} while (0)
        #define INITRB_ENDIAN_SWAP32(value) do {INITRB_ENDIAN_CAST(value, uint32_t) = __builtin_bswap32(INITRB_ENDIAN_CAST(value, uint32_t));} while (0)
        #define INITRB_ENDIAN_SWAP16(value) do {INITRB_ENDIAN_CAST(value, uint16_t) = __builtin_bswap16(INITRB_ENDIAN_CAST(value, uint16_t));} while (0)
    #endif
#elif INITRB_PLATFORM == INITRB_PLATFORM_WINDOWS
    #define INITRB_ENDIAN INITRB_ENDIAN_LITTLE
    #if INITRB_ENDIAN == INITRB_ENDIAN_LITTLE
        #define INITRB_ENDIAN_SWAP64(value) do {INITRB_ENDIAN_CAST(value, uint64_t) = _byteswap_uint64(INITRB_ENDIAN_CAST(value, uint64_t));} while (0)
        #define INITRB_ENDIAN_SWAP32(value) do {INITRB_ENDIAN_CAST(value, uint32_t) = _byteswap_ulong(INITRB_ENDIAN_CAST(value, uint32_t));} while (0)
        #define INITRB_ENDIAN_SWAP16(value) do {INITRB_ENDIAN_CAST(value, uint16_t) = _byteswap_ushort(INITRB_ENDIAN_CAST(value, uint16_t));} while (0)
    #else
        #error "Invalid endianness for a Windows system"
    #endif
#endif

/**
 * printf definitions
 */
#define INITRB_PRINT_U8 "%"PRIu8
#define INITRB_PRINT_S8 "%"PRId8
#define INITRB_PRINT_U16 "%"PRIu16
#define INITRB_PRINT_S16 "%"PRId16
#define INITRB_PRINT_U32 "%"PRIu32
#define INITRB_PRINT_S32 "%"PRId32
#define INITRB_PRINT_U64 "%"PRIu64
#define INITRB_PRINT_S64 "%"PRId64
#define INITRB_PRINT_TIME INITRB_PRINT_S64

#if INITRB_PLATFORM_IS_GNU
    #define INITRB_PRINT_SIZE  "%zu"
    #define INITRB_PRINT_SSIZE "%zd"
#endif

#endif /* INITRB_PLATFORM_H */
