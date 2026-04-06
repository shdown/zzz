// (c) 2026 shdown
// This code is licensed under MIT license (see LICENSE.MIT for details)

#include "my_time.h"
#include "buf.h"
#include "is_term_interactive.h"
#include "compdep.h"
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

static const char *SEQ_CLEAR_LINE = "\033[1K\033[1G";

static bool fflag;
static bool interactive;

static void print_line(uint64_t total_s)
{
    uint64_t rem = total_s;
    int ss = rem % 60; rem /= 60;
    int mm = rem % 60; rem /= 60;
    int hh = rem % 24; rem /= 24;
    uint64_t dd = rem;

    buf_reset();
    if (interactive) {
        buf_appendf("%s", SEQ_CLEAR_LINE);
    }

    bool nonempty = false;
    if (dd || nonempty) {
        buf_appendf("%" PRIu64 "d ", dd);
        nonempty = true;
    }
    if (hh || nonempty) {
        buf_appendf("%dh ", hh);
        nonempty = true;
    }
    if (mm || nonempty) {
        buf_appendf("%dm ", mm);
        nonempty = true;
    }
    buf_appendf("%ds", ss);

    if (!interactive) {
        buf_appendf("\n");
    }

    buf_flush_to_stderr();
}

static void done_printing_lines(void)
{
    if (interactive) {
        buf_reset();
        buf_appendf("%s\n", SEQ_CLEAR_LINE);
        buf_flush_to_stderr();
    }
}

static MyTime add_or_die_on_overflow(MyTime a, MyTime b)
{
    bool overflowed = false;
    MyTime res = my_time_add(a, b, &overflowed);
    if (overflowed) {
        fputs("FATAL: my_time_add() reported overflow. This is a bug.\n", stderr);
        abort();
    }
    return res;
}

void do_zzz(MyTime delta)
{
    MyTime t_start = my_time_now();
    MyTime t_end = add_or_die_on_overflow(t_start, delta);

    for (;;) {
        MyTime t_left = my_time_sub(t_end, my_time_now());
        if (my_time_is0(t_left)) {
            break;
        }

        MyTime to_print = fflag ? my_time_sub(delta, t_left) : t_left;
        print_line(to_print.s);

        if (my_time_lessthan1(t_left)) {
            my_time_sleep(t_left);
        } else {
            my_time_sleep(my_time_from_int(1));
        }
    }
    done_printing_lines();
}

ATTR_NORETURN
static void print_usage_and_exit(void)
{
    fputs(
        "USAGE: zzz [-f] number[suffix]...\n"
        "    -f: count forward, not backward.\n"
        "Supported suffixes:\n"
        "    's' for seconds;\n"
        "    'm' for minutes;\n"
        "    'h' for hours;\n"
        "    'd' for days.\n"
        , stderr);
    exit(2);
}

static double parse_arg_or_die(const char *arg)
{
    if (isspace((unsigned char) arg[0])) {
        goto err_number;
    }
    char *suffix;
    double r = strtod(arg, &suffix);
    if (suffix == arg || isnan(r) || r < 0) {
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
    fprintf(stderr, "E: argument '%s': does not start with number\n", arg);
    print_usage_and_exit();
err_suffix:
    fprintf(stderr, "E: argument '%s': invalid suffix.\n", arg);
    print_usage_and_exit();
}

int main(int argc, char **argv)
{
    for (int c; (c = getopt(argc, argv, "f")) != -1;) {
        switch (c) {
        case 'f':
            fflag = true;
            break;
        default:
            print_usage_and_exit();
        }
    }
    if (argc - optind < 1) {
        fputs("At least one positional argument is required.\n", stderr);
        print_usage_and_exit();
    }

    MyTime total = my_time_from_int(0);
    for (int i = optind; i < argc; ++i) {
        double cur_d = parse_arg_or_die(argv[i]);
        MyTime cur = my_time_from_double(cur_d);
        total = my_time_add(total, cur, NULL);
    }

    if (my_time_make_addsafe(&total)) {
        fputs("W: this amount of time is insane, truncated.\n", stderr);
    }

    interactive = is_term_interactive();
    do_zzz(total);
    return 0;
}
