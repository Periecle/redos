#ifndef _STDINT_H
#define _STDINT_H

#include <bits/types.h>
#include <bits/wchar.h>

// ----------------------------------------------------------------------------
// Type definitions.
// ----------------------------------------------------------------------------

// Fixed-width (signed).
typedef __int8  int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;

// Fixed-width (unsigned).
typedef __uint8  uint8_t;
typedef __uint16 uint16_t;
typedef __uint32 uint32_t;
typedef __uint64 uint64_t;

// Least-width (signed).
typedef __int8  int_least8_t;
typedef __int16 int_least16_t;
typedef __int32 int_least32_t;
typedef __int64 int_least64_t;

// Least-width (unsigned).
typedef __uint8  uint_least8_t;
typedef __uint16 uint_least16_t;
typedef __uint32 uint_least32_t;
typedef __uint64 uint_least64_t;

// Fast-width (signed).
typedef __int_fast8  int_fast8_t;
typedef __int_fast16 int_fast16_t;
typedef __int_fast32 int_fast32_t;
typedef __int_fast64 int_fast64_t;

// Fast-width (unsigned).
typedef __uint_fast8  uint_fast8_t;
typedef __uint_fast16 uint_fast16_t;
typedef __uint_fast32 uint_fast32_t;
typedef __uint_fast64 uint_fast64_t;

// Miscellaneous (signed).
typedef __intmax intmax_t;
typedef __intptr intptr_t;

// Miscellaneous (unsigned).
typedef __uintmax uintmax_t;
typedef __uintptr uintptr_t;

// ----------------------------------------------------------------------------
// Constants.
// ----------------------------------------------------------------------------

// Fixed-width (signed).
#define INT8_C(x)  __INT8_C(x)
#define INT16_C(x) __INT16_C(x)
#define INT32_C(x) __INT32_C(x)
#define INT64_C(x) __INT64_C(x)
#define INTMAX_C(x) __INTMAX_C(x)

// Fixed-width (unsigned).
#define UINT8_C(x)  __UINT8_C(x)
#define UINT16_C(x) __UINT16_C(x)
#define UINT32_C(x) __UINT32_C(x)
#define UINT64_C(x) __UINT64_C(x)
#define UINTMAX_C(x) __UINTMAX_C(x)

// ----------------------------------------------------------------------------
// Limits.
// ----------------------------------------------------------------------------

// Fixed-width (signed).
#define INT8_MAX  __INT8_MAX
#define INT16_MAX __INT16_MAX
#define INT32_MAX __INT32_MAX
#define INT64_MAX __INT64_MAX

#define INT8_MIN  __INT8_MIN
#define INT16_MIN __INT16_MIN
#define INT32_MIN __INT32_MIN
#define INT64_MIN __INT64_MIN

// Fixed-width (unsigned).
#define UINT8_MAX  __UINT8_MAX
#define UINT16_MAX __UINT16_MAX
#define UINT32_MAX __UINT32_MAX
#define UINT64_MAX __UINT64_MAX

// Least-width (signed).
#define INT_LEAST8_MAX  __INT8_MAX
#define INT_LEAST16_MAX __INT16_MAX
#define INT_LEAST32_MAX __INT32_MAX
#define INT_LEAST64_MAX __INT64_MAX

#define INT_LEAST8_MIN  __INT8_MIN
#define INT_LEAST16_MIN __INT16_MIN
#define INT_LEAST32_MIN __INT32_MIN
#define INT_LEAST64_MIN __INT64_MIN

// Least-width (unsigned).
#define UINT_LEAST8_MAX  __UINT8_MAX
#define UINT_LEAST16_MAX __UINT16_MAX
#define UINT_LEAST32_MAX __UINT32_MAX
#define UINT_LEAST64_MAX __UINT64_MAX

// Fast-width (signed).
#define INT_FAST8_MAX  __INT_FAST8_MAX
#define INT_FAST16_MAX __INT_FAST16_MAX
#define INT_FAST32_MAX __INT_FAST32_MAX
#define INT_FAST64_MAX __INT_FAST64_MAX

#define INT_FAST8_MIN  __INT_FAST8_MIN
#define INT_FAST16_MIN __INT_FAST16_MIN
#define INT_FAST32_MIN __INT_FAST32_MIN
#define INT_FAST64_MIN __INT_FAST64_MIN

// Fast-width (unsigned).
#define UINT_FAST8_MAX  __UINT_FAST8_MAX
#define UINT_FAST16_MAX __UINT_FAST16_MAX
#define UINT_FAST32_MAX __UINT_FAST32_MAX
#define UINT_FAST64_MAX __UINT_FAST64_MAX

// Miscellaneous (signed).
#define INTMAX_MAX __INTMAX_MAX
#define INTPTR_MAX __INTPTR_MAX

#define INTMAX_MIN __INTMAX_MIN
#define INTPTR_MIN __INTPTR_MIN

// Miscellaneous (unsigned).
#define UINTMAX_MAX __UINTMAX_MAX
#define UINTPTR_MAX __UINTPTR_MAX

// Other limits (signed).
#define PTRDIFF_MAX    __PTRDIFF_MAX
#define PTRDIFF_MIN    __PTRDIFF_MIN
#define SIG_ATOMIC_MAX __SIG_ATOMIC_MAX
#define SIG_ATOMIC_MIN __SIG_ATOMIC_MIN
#define WINT_MAX       __WINT_MAX
#define WINT_MIN       __WINT_MIN

// Other limits (unsigned).
#define SIZE_MAX __SIZE_MAX

#endif // _STDINT_H
