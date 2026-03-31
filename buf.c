// (c) 2026 shdown
// This code is licensed under MIT license (see LICENSE.MIT for details)

#include "buf.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>

static char my_buf[1024];
static size_t my_pos;

void buf_reset(void)
{
    my_pos = 0;
}

void buf_appendf(const char *fmt, ...)
{
    char *dest = my_buf + my_pos;
    size_t dest_avail = sizeof(my_buf) - my_pos;
    assert(dest_avail);

    va_list vl;
    va_start(vl, fmt);
    int rc = vsnprintf(dest, dest_avail, fmt, vl);
    va_end(vl);

    if (rc < 0) {
        perror("vsnprintf");
        abort();
    }
    my_pos += strlen(dest);
}

static bool full_write(int fd, const char *data, size_t ndata)
{
    size_t nwritten = 0;
    while (nwritten < ndata) {
        ssize_t w = write(fd, data + nwritten, ndata - nwritten);
        if (w < 0) {
            return false;
        }
        nwritten += w;
    }
    return true;
}

void buf_flush_to_stderr(void)
{
    if (!full_write(2, my_buf, my_pos)) {
        perror("write");
        abort();
    }
}
