######
# Current status for now.
# If updating the parser:
# * cd src/parser
# * make parser.cc scanner.cc
# * cd ../..

# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS += -Isrc/parser -Isrc/parser-venn -Itipsy-encoder/include

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp) $(wildcard src/parser/*.cc) $(wildcard src/parser-venn/*.cc)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += presets
DISTRIBUTABLES += fonts
DISTRIBUTABLES += venn-waves

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

VPATH = .:./src/parser:./src/parser-venn
