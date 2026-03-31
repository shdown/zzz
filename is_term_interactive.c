// (c) 2026 shdown
// This code is licensed under MIT license (see LICENSE.MIT for details)

#include "is_term_interactive.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

bool is_term_interactive(void)
{
    if (isatty(2) != 1) {
        return false;
    }
    const char *term = getenv("TERM");
    if (!term
        || strcmp(term, "") == 0
        || strcmp(term, "dumb") == 0)
    {
        return false;
    }
    return true;
}
