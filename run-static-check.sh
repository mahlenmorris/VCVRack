#!/bin/sh

# --suppress=unusedStructMember -- Not sure if Rack's cppcheck also runs with this or not.

cppcheck --enable=all --suppress=missingIncludeSystem --suppress=unusedStructMember --suppress=*:*parser.hh --suppress=*:*parser.cc --suppress=*:*scanner.cc -I src/parser -I src/parser-venn src/  > local-static-checks.txt 2>&1
