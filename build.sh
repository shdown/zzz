#!/usr/bin/env bash

set -e
set -x

gcc -Wall -Wextra -O3 ./*.c -o zzz -lm
