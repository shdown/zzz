// (c) 2026 shdown
// This code is licensed under MIT license (see LICENSE.MIT for details)

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t s;
    uint32_t ns;
} MyTime;

MyTime my_time_now(void);

MyTime my_time_from_int(uint64_t s);

// Assumes: d >= 0, d is not NaN.
MyTime my_time_from_double(double d);

MyTime my_time_add(MyTime a, MyTime b, bool *overflowed);

MyTime my_time_sub(MyTime a, MyTime b);

bool my_time_is0(MyTime a);

bool my_time_lessthan1(MyTime a);

void my_time_sleep(MyTime a);

bool my_time_make_addsafe(MyTime *a);
