#!/bin/bash

G_SLICE=always-malloc;

valgrind --demangle=no --num-callers=50 --leak-check=full --suppressions=tests/suppressions/linux.txt make/build/jb_test -d -G "resources/cfg/defaults_general.cfg" -G "resources/cfg/defaults_linux.cfg" -s "src/jb_test.lua" -g "local/test.cfg" -t 1
