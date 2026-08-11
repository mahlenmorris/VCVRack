#!/bin/sh
cppcheck --enable=all --suppress=missingIncludeSystem -I src/parser -I src/parser-venn src/  > local-static-checks.txt 2>&1
