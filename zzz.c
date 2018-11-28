#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

static const char *PROGRAM_NAME = "zzz";

static const char *SEQ_CLEAR_LINE = "\033[1K\033[1G";

static const int TERM_FD = 1;

static bool interactive;

static char buf[1024];

static
bool
is_term_interactive(void)
{
    if (isatty(TERM_FD) != 1) {
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

// Assumes the argument fits in time_t.
static
struct timespec
double_to_ts(double seconds)
{
    struct timespec ts;
    ts.tv_sec = seconds;
    ts.tv_nsec = (seconds - ts.tv_sec) * 1e9;
    return ts;
}

static
struct timespec
ts_sub(struct timespec a, struct timespec b)
{
    if ((a.tv_nsec -= b.tv_nsec) < 0) {
        a.tv_nsec += 1e9;
        if ((--a.tv_sec) == (time_t) -1) {
            return (struct timespec) {0};
        }
    }
    if (a.tv_sec < b.tv_sec) {
        return (struct timespec) {0};
    }
    a.tv_sec -= b.tv_sec;
    return a;
}

static
struct timespec
ts_add(struct timespec a, struct timespec b)
{
    if ((a.tv_nsec += b.tv_nsec) >= 1e9) {
        a.tv_nsec -= 1e9;
        ++a.tv_sec;
    }
    a.tv_sec += b.tv_sec;
    return a;
}

static
struct timespec
monotonic(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
        perror("clock_gettime");
        exit(1);
    }
    return ts;
}

static
struct timespec
try_sleep(struct timespec ts)
{
    struct timespec rem;
    if (nanosleep(&ts, &rem) < 0) {
        if (errno == EINTR) {
            return rem;
        } else {
            perror("nanosleep");
            exit(1);
        }
    }
    return (struct timespec) {0};
}

static
void
write_buf(size_t n)
{
    for (size_t written = 0; written != n; ) {
        ssize_t w = write(TERM_FD, buf + written, n - written);
        if (w < 0) {
            perror("write");
            exit(1);
        }
        written += w;
    }
}

static
void
term_update(struct timespec ts)
{
    size_t n = 0;
#define APPENDF(...) (n += snprintf(buf + n, sizeof(buf) - n, __VA_ARGS__))
    if (interactive) {
        APPENDF("%s", SEQ_CLEAR_LINE);
    }

    // round (ts.tv_sec + ts.tv_nsec / 1e9) to the nearest integer
    unsigned long long cur = ts.tv_sec + ts.tv_nsec / (long) (1e9 / 2);
    const int s = cur % 60;
    cur /= 60;
    const int m = cur % 60;
    cur /= 60;
    const int h = cur % 24;
    const unsigned long long d = cur / 24;

    bool printed = false;
    if (d || printed) {
        APPENDF("%llud ", d);
        printed = true;
    }
    if (h || printed) {
        APPENDF("%dh ", h);
        printed = true;
    }
    if (m || printed) {
        APPENDF("%dm ", m);
        printed = true;
    }
    APPENDF("%ds", s);

    if (!interactive) {
        APPENDF("\n");
    }
    write_buf(n);
#undef APPENDF
}

static
void
term_update_end(void)
{
    if (interactive) {
        write_buf(snprintf(buf, sizeof(buf), "%s\n", SEQ_CLEAR_LINE));
    }
}

static
void
interactive_sleep(const struct timespec amount)
{
    const struct timespec second = {.tv_sec = 1};
    const struct timespec start = monotonic();
    struct timespec left = amount;
    term_update(left);
    while (left.tv_sec) {
        const struct timespec rem = try_sleep(second);
        term_update(ts_add(ts_sub(left, second), rem));
        left = ts_sub(amount, ts_sub(monotonic(), start));
    }
    while (left.tv_nsec) {
        left = try_sleep(left);
    }
    term_update_end();
}

static
void
print_usage_and_exit(void)
{
    fprintf(stderr, "USAGE: %s number[suffix]...\n", PROGRAM_NAME);
    fputs("Supported suffixes:\n"
          "    's' for seconds;\n"
          "    'm' for minutes;\n"
          "    'h' for hours;\n"
          "    'd' for days.\n"
          , stderr);
    exit(2);
}

static
double
parse_arg_or_die(const char *arg)
{
    if (isspace(arg[0])) {
        goto err_number;
    }
    char *suffix;
    double r = strtod(arg, &suffix);
    if (suffix == arg || r < 0) {
        goto err_number;
    }
    switch (*suffix) {
    case 'd':
        r *= 24;
        // fall thru
    case 'h':
        r *= 60;
        // fall thru
    case 'm':
        r *= 60;
        // fall thru
    case 's':
        if (suffix[1] != '\0') {
            goto err_suffix;
        }
        break;
    case '\0':
        break;
    default:
        goto err_suffix;
    }
    return r;

err_number:
    fprintf(stderr, "Argument '%s': does not start with number\n", arg);
    exit(1);
err_suffix:
    fprintf(stderr, "Argument '%s': invalid suffix.\n", arg);
    exit(1);
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        fputs("At least one argument is required.\n", stderr);
        print_usage_and_exit();
    }
    double seconds = 0;
    for (int i = 1; i < argc; ++i) {
        seconds += parse_arg_or_die(argv[i]);
    }
    if (seconds >= (1UL << 31)) {
        fprintf(stderr, "That amount of time is insane.\n");
        exit(1);
    }
    interactive = is_term_interactive();
    interactive_sleep(double_to_ts(seconds));
    return 0;
}
