#ifndef _INTERNAL_TYPES_H
#define _INTERNAL_TYPES_H

typedef __UINT8_TYPE__ __uint8;
typedef __UINT16_TYPE__ __uint16;
typedef __UINT32_TYPE__ __uint32;
typedef __UINT64_TYPE__ __uint64;

typedef __INT8_TYPE__ __int8;
typedef __INT16_TYPE__ __int16;
typedef __INT32_TYPE__ __int32;
typedef __INT64_TYPE__ __int64;

/* Remove redefinitions of compiler built-ins */
#define __INT8_MAX  __INT8_MAX__
#define __INT16_MAX __INT16_MAX__
#define __INT32_MAX __INT32_MAX__
#define __INT64_MAX __INT64_MAX__

#define __INT8_MIN  (-__INT8_MAX - 1)
#define __INT16_MIN (-__INT16_MAX - 1)
#define __INT32_MIN (-__INT32_MAX - 1)
#define __INT64_MIN (-__INT64_MAX - 1)

#define __UINT8_MAX  __UINT8_MAX__
#define __UINT16_MAX __UINT16_MAX__
#define __UINT32_MAX __UINT32_MAX__
#define __UINT64_MAX __UINT64_MAX__

// Fast types (signed).

#if defined (__i386__)

typedef __int8 __int_fast8;
#define __INT_FAST8_MAX __INT8_MAX
#define __INT_FAST8_MIN __INT8_MIN

typedef __int32 __int_fast16;
#define __INT_FAST16_MAX __INT32_MAX
#define __INT_FAST16_MIN __INT32_MIN

typedef __int32 __int_fast32;
#define __INT_FAST32_MAX __INT32_MAX
#define __INT_FAST32_MIN __INT32_MIN

typedef __int64 __int_fast64;
#define __INT_FAST64_MAX __INT64_MAX
#define __INT_FAST64_MIN __INT64_MIN

#elif defined (__x86_64__)

typedef __int8 __int_fast8;
#define __INT_FAST8_MAX __INT8_MAX
#define __INT_FAST8_MIN __INT8_MIN

typedef __int64 __int_fast16;
#define __INT_FAST16_MAX __INT64_MAX
#define __INT_FAST16_MIN __INT64_MIN

typedef __int64 __int_fast32;
#define __INT_FAST32_MAX __INT64_MAX
#define __INT_FAST32_MIN __INT64_MIN

typedef __int64 __int_fast64;
#define __INT_FAST64_MAX __INT64_MAX
#define __INT_FAST64_MIN __INT64_MIN

#elif defined (__aarch64__)

typedef __int8 __int_fast8;
#define __INT_FAST8_MAX __INT8_MAX
#define __INT_FAST8_MIN __INT8_MIN

typedef __int64 __int_fast16;
#define __INT_FAST16_MAX __INT64_MAX
#define __INT_FAST16_MIN __INT64_MIN

typedef __int64 __int_fast32;
#define __INT_FAST32_MAX __INT64_MAX
#define __INT_FAST32_MIN __INT64_MIN

typedef __int64 __int_fast64;
#define __INT_FAST64_MAX __INT64_MAX
#define __INT_FAST64_MIN __INT64_MIN

#else
#  error "Missing architecture specific code"
#endif

// Fast types (unsigned).

#if defined (__i386__)

typedef __uint8 __uint_fast8;
#define __UINT_FAST8_MAX __UINT8_MAX
#define __UINT_FAST8_MIN __UINT8_MIN

typedef __uint32 __uint_fast16;
#define __UINT_FAST16_MAX __UINT32_MAX
#define __UINT_FAST16_MIN __UINT32_MIN

typedef __uint32 __uint_fast32;
#define __UINT_FAST32_MAX __UINT32_MAX
#define __UINT_FAST32_MIN __UINT32_MIN

typedef __uint64 __uint_fast64;
#define __UINT_FAST64_MAX __UINT64_MAX
#define __UINT_FAST64_MIN __UINT64_MIN

#elif defined (__x86_64__)

typedef __uint8 __uint_fast8;
#define __UINT_FAST8_MAX __UINT8_MAX
#define __UINT_FAST8_MIN __UINT8_MIN

typedef __uint64 __uint_fast16;
#define __UINT_FAST16_MAX __UINT64_MAX
#define __UINT_FAST16_MIN __UINT64_MIN

typedef __uint64 __uint_fast32;
#define __UINT_FAST32_MAX __UINT64_MAX
#define __UINT_FAST32_MIN __UINT64_MIN

typedef __uint64 __uint_fast64;
#define __UINT_FAST64_MAX __UINT64_MAX
#define __UINT_FAST64_MIN __UINT64_MIN

#elif defined (__aarch64__)

typedef __uint8 __uint_fast8;
#define __UINT_FAST8_MAX __UINT8_MAX
#define __UINT_FAST8_MIN __UINT8_MIN

typedef __uint64 __uint_fast16;
#define __UINT_FAST16_MAX __UINT64_MAX
#define __UINT_FAST16_MIN __UINT64_MIN

typedef __uint64 __uint_fast32;
#define __UINT_FAST32_MAX __UINT64_MAX
#define __UINT_FAST32_MIN __UINT64_MIN

typedef __uint64 __uint_fast64;
#define __UINT_FAST64_MAX __UINT64_MAX
#define __UINT_FAST64_MIN __UINT64_MIN

#else
#  error "Missing architecture specific code"
#endif

// Special types.

typedef __INTMAX_TYPE__ __intmax;
typedef __INTPTR_TYPE__ __intptr;
typedef __PTRDIFF_TYPE__ __ptrdiff;
#define __INTMAX_MAX __INTMAX_MAX__
#define __INTMAX_MIN (-__INTMAX_MAX__ - 1)
#define __INTPTR_MAX __INTPTR_MAX__
#define __INTPTR_MIN (-__INTPTR_MAX__ - 1)
#define __PTRDIFF_MAX __PTRDIFF_MAX__
#define __PTRDIFF_MIN (-__PTRDIFF_MAX__ - 1)

typedef __UINTMAX_TYPE__ __uintmax;
typedef __UINTPTR_TYPE__ __uintptr;
typedef __SIZE_TYPE__ __size;
#define __UINTMAX_MAX __UINTMAX_MAX__
#define __UINTPTR_MAX __UINTPTR_MAX__
#define __SIZE_MAX __SIZE_MAX__

// Other limits.

#define __WCHAR_MAX __WCHAR_MAX__
#define __WCHAR_MIN __WCHAR_MIN__

#define __WINT_MAX __WINT_MAX__
#define __WINT_MIN __WINT_MIN__

#define __SIG_ATOMIC_MAX __SIG_ATOMIC_MAX__
#define __SIG_ATOMIC_MIN __SIG_ATOMIC_MIN__

#endif // _INTERNAL_TYPES_H
