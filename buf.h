// (c) 2026 shdown
// This code is licensed under MIT license (see LICENSE.MIT for details)

#pragma once

#include "compdep.h"

void buf_reset(void);

ATTR_PRINTF(1, 2)
void buf_appendf(const char *fmt, ...);

void buf_flush_to_stderr(void);
