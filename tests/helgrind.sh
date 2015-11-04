#!/bin/bash

valgrind --tool=helgrind --demangle=no --num-callers=50 --free-is-write=yes --track-lockorders=no --suppressions=tests/suppressions/linux.suppressions make/build/jb_test -d -G "resources/cfg/defaults_general.cfg" -G "resources/cfg/defaults_linux.cfg" -s "src/jb_test.lua" -g "local/test.cfg" --gui-scale 2

