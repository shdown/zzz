*zzz: sleep with feedback!*

**zzz** is a simple command-line sleep utility with live countdown/elapsed display.

Usage
===

```
zzz [-f] number[suffix]...
```

`-f` means count **forward** (elapsed time) instead of backward (remaining time)

Supported suffixes
---

| Suffix | Unit |
|--------|------|
| `s`    | Seconds |
| `m`    | Minutes |
| `h`    | Hours |
| `d`    | Days |

If no suffix is provided, the value is interpreted as seconds.

Examples
===

```bash
# Sleep for 30 seconds with a live countdown
zzz 30s

# Sleep for 1 hour and 30 minutes
zzz 1h 30m

# Count forward (elapsed time display)
zzz -f 5m
```

Building and installing
===

It requires CMake and a GNU C-compatible compiler (either GCC or Clang).

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

License
===

MIT; see [LICENSE.MIT](LICENSE.MIT).
