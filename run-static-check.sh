#!/bin/sh

# --suppress=unusedStructMember -- Not sure if Rack's cppcheck also runs with this or not.

cppcheck src \
    -isrc/dep \
    -isrc/tests \
    --std=c++11 \
    --max-configs=1 \
    --enable=warning,performance,portability \
    --suppress=*:*/dep/* \
    --suppress=*:*/tests/* \
    --suppress=missingIncludeSystem \
    --suppress=unusedStructMember \
    --suppress=*:*parser.hh \
    --suppress=*:*parser.cc \
    --suppress=*:*scanner.cc \
    -I src/parser \
    -I src/parser-venn \
    -j $(nproc) \
    -q > local-static-checks.txt 2>&1
