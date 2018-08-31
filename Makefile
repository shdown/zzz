include config.mk
version := $(shell git rev-parse --short HEAD 2>/dev/null || cat VERSION 2>/dev/null || echo unknown)
CFLAGS := -std=c99 -Wall -Wextra -O2
CPPFLAGS := -D_POSIX_C_SOURCE=200809L -DPROGRAM_VERSION=\"$(version)\"

all: zzz

zzz: zzz.c

install: zzz
	install zzz $(PREFIX)/bin/zzz

uninstall:
	$(RM) $(PREFIX)/bin/zzz

clean:
	$(RM) zzz

.PHONY: all install uninstall clean
