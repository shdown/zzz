// (c) 2026 shdown
// This code is licensed under MIT license (see LICENSE.MIT for details)

#pragma once

#if __GNUC__ >= 2
#   define ATTR_PRINTF(n, m)   __attribute__((format(printf, n, m)))
#   define ATTR_NORETURN       __attribute__((noreturn))
#else
#   define ATTR_PRINTF(n, m)   /*nothing*/
#   define ATTR_NORETURN       /*nothing*/
#endif
