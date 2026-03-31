// (c) 2026 shdown
// This code is licensed under MIT license (see LICENSE.MIT for details)

#include "my_time.h"
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

enum { NSEC_IN_SEC = 1000 * 1000 * 1000 };

MyTime my_time_now(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
        perror("clock_gettime");
        abort();
    }
    return (MyTime) {
        .s = ts.tv_sec,
        .ns = ts.tv_nsec,
    };
}

MyTime my_time_from_int(uint64_t s)
{
    return (MyTime) {.s = s, .ns = 0};
}

MyTime my_time_from_double(double d)
{
    assert(!isnan(d));
    assert(d >= 0);

    if (d > /*2^63*/ 9223372036854775808.0) {
        return (MyTime) {-1, -1};
    }

    uint64_t res_s = (uint64_t) d;
    uint32_t res_ns = (d - floor(d)) * 1e9;
    assert(res_ns < NSEC_IN_SEC);

    return (MyTime) {.s  = res_s, .ns = res_ns};
}

MyTime my_time_add(MyTime a, MyTime b, bool *overflowed)
{
    uint32_t res_ns = a.ns + b.ns;
    bool carry = res_ns >= NSEC_IN_SEC;

    if (carry) {
        res_ns -= NSEC_IN_SEC;

        ++a.s;
        if (!a.s) {
            goto overflow;
        }
    }
    uint64_t res_s = a.s + b.s;
    if (res_s < a.s) {
        goto overflow;
    }
    return (MyTime) {.s = res_s, .ns = res_ns};

overflow:
    if (overflowed) {
        *overflowed = true;
    }
    return (MyTime) {-1, -1};
}

MyTime my_time_sub(MyTime a, MyTime b)
{
    uint32_t res_ns = a.ns - b.ns;
    bool borrow = a.ns < b.ns;

    if (borrow) {
        res_ns += NSEC_IN_SEC;

        if (!a.s) {
            goto underflow;
        }
        --a.s;
    }

    if (a.s < b.s) {
        goto underflow;
    }
    uint64_t res_s = a.s - b.s;
    return (MyTime) {.s = res_s, .ns = res_ns};

underflow:
    return (MyTime) {0};
}

bool my_time_is0(MyTime a)
{
    return a.s == 0 && a.ns == 0;
}

bool my_time_lessthan1(MyTime a)
{
    return a.s == 0;
}

void my_time_sleep(MyTime delta)
{
    struct timespec duration = {
        .tv_sec = delta.s < INT32_MAX ? delta.s : INT32_MAX,
        .tv_nsec = delta.ns,
    };
    struct timespec rem;

    int rc;
    while ((rc = nanosleep(&duration, &rem)) < 0 && errno == EINTR) {
        duration = rem;
    }
    if (rc < 0) {
        perror("nanosleep");
        abort();
    }
}

bool my_time_make_addsafe(MyTime *a)
{
    static const uint64_t THRES = UINT64_MAX / 2 - 10;

    if (a->s > THRES) {
        a->s = THRES;
        return true;
    }
    return false;
}
