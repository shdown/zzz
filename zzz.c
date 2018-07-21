#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <limits.h>

static const char *PROGRAM_NAME = "zzz";

static const char *SEQ_CLEAR_LINE = "\033[1K\033[1G";

static bool interactive;

static char buf[1024];

static const int TERM_FD = 1;

static bool is_term_interactive(void) {
    if (isatty(TERM_FD) != 1) {
        return false;
    }
    char *term = getenv("TERM");
    if (!term
        || strcmp(term, "") == 0
        || strcmp(term, "dumb") == 0)
    {
        return false;
    }
    return true;
}

static inline double ts_to_double(struct timespec ts) {
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static inline struct timespec double_to_ts(double seconds) {
    struct timespec ts = {.tv_sec = (time_t) seconds};
    ts.tv_nsec = (long) ((seconds - ts.tv_sec) * 1e9);
    return ts;
}

static double monotonic(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
        perror("clock_gettime");
        exit(1);
    }
    return ts_to_double(ts);
}

static double zzz(double seconds) {
    struct timespec ts = double_to_ts(seconds);
    struct timespec rem;
    if (nanosleep(&ts, &rem) < 0) {
        if (errno == EINTR) {
            return ts_to_double(rem);
        } else {
            perror("nanosleep");
            exit(1);
        }
    }
    return seconds;
}

static void write_buf(size_t n) {
    for (size_t written = 0; written != n; ) {
        ssize_t w = write(TERM_FD, buf + written, n - written);
        if (w < 0) {
            perror("write");
            exit(1);
        }
        written += w;
    }
}

static void term_update(double seconds_left) {
    size_t n = 0;
#define APPENDF(...) (n += snprintf(buf + n, sizeof(buf) - n, __VA_ARGS__))
    if (interactive) {
        APPENDF("%s", SEQ_CLEAR_LINE);
    }

    unsigned long long cur = (unsigned long long) (seconds_left + 0.5);
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

static void term_update_end(void) {
    if (interactive) {
        write_buf(snprintf(buf, sizeof(buf), "%s\n", SEQ_CLEAR_LINE));
    }
}

static void loop(double seconds, double blinktime) {
    const double start = monotonic();
    double left = seconds;
    term_update(left);
    while (left > blinktime) {
        const double slept = zzz(blinktime);
        term_update(left - slept);
        left = seconds - (monotonic() - start);
    }
    if (left > 0) {
        zzz(left);
    }
    term_update_end();
}

static void help(void) {
    fprintf(stderr, "USAGE: %s [options] number[suffix]...\n", PROGRAM_NAME);
    fprintf(stderr, "       %s -h\n", PROGRAM_NAME);
    fprintf(stderr, "       %s -v\n", PROGRAM_NAME);
    fputs("Supported suffixes:\n"
          "    's' for seconds;\n"
          "    'm' for minutes;\n"
          "    'h' for hours;\n"
          "    'd' for days.\n"
          "Supported options:\n"
          "    -a: alert on finish.\n"
          "Run with '-h' for help, with '-v' for version.\n"
          , stderr);
    exit(2);
}

static void version(void) {
    fprintf(stderr, "This is %s %s.\n", PROGRAM_NAME, PROGRAM_VERSION);
    exit(2);
}

static void usage(void) {
    fprintf(stderr, "Run '%s -h' for help.\n", PROGRAM_NAME);
    exit(2);
}

static inline bool my_is_digit(char c) {
    return '0' <= c && c <= '9';
}

static inline bool my_is_period(char c) {
    return c == '.';
}

static double parse_arg(const char *arg) {
    const char *s = arg;
    // skip the integer part
    while (my_is_digit(*s)) {
        ++s;
    }
    // no digits?
    if (s == arg) {
        fprintf(stderr, "Argument '%s': does not start with a digit\n", arg);
        exit(1);
    }
    // skip the fractional part, if any
    if (my_is_period(*s)) {
        do {
            ++s;
        } while (my_is_digit(*s));
    }
    // parse the suffix, if any
    double mul = 1;
    switch (*s) {
    case 'd':
        mul *= 24;
        // fallthrough
    case 'h':
        mul *= 60;
        // fallthrough
    case 'm':
        mul *= 60;
        // fallthrough
    case '\0':
    case 's':
        break;
    default:
        fprintf(stderr, "Argument '%s': unknown suffix\n", arg);
        exit(1);
    }
    // suffix is present and is longer than one character?
    if (*s && s[1]) {
        fprintf(stderr, "Argument '%s': extra characters after suffix\n", arg);
        exit(1);
    }
    // parse the number part
    errno = 0;
    double result = strtod(arg, NULL);
    if (errno) {
        fprintf(stderr, "Argument '%s': something's wrong with number\n", arg);
        exit(1);
    }
    return result * mul;
}

int main(int argc, char **argv) {
    bool alert_on_fin = false;
    for (int c; (c = getopt(argc, argv, "ahv")) != -1;) {
        switch (c) {
        case 'a':
            alert_on_fin = true;
            break;
        case 'h':
            help();
            break;
        case 'v':
            version();
            break;
        case '?':
            usage();
            break;
        }
    }

    if (optind == argc) {
        fprintf(stderr, "No arguments provided.\n");
        usage();
    }
    double seconds = 0;
    for (int i = optind; i < argc; ++i) {
        seconds += parse_arg(argv[i]);
    }
    if (!isfinite(seconds) || seconds > (double) ULLONG_MAX) {
        fprintf(stderr, "That amount of time is insane.\n");
        exit(1);
    }
    interactive = is_term_interactive();
    loop(seconds, 1);
    if (alert_on_fin) {
        buf[0] = '\a';
        write_buf(1);
    }
    return 0;
}
