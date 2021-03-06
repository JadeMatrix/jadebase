#!/bin/bash

G_SLICE=always-malloc;

valgrind --tool=massif --demangle=no --num-callers=50 --suppressions=tests/suppressions/linux.suppressions --massif-out-file=local/heap-check.txt make/build/jb_test -d -G "resources/cfg/defaults_general.cfg" -G "resources/cfg/defaults_linux.cfg" -s "src/jb_test.lua" -g "local/test.cfg" --gui-scale 2
ms_print local/heap-check.txt > local/heap-check-msprint.txt
