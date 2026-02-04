# Compiler and Flags
CXX      := g++
CXXFLAGS := -std=c++17 -O3 -Wall

# MSYS2 Pathing (Standard for MinGW 64-bit)
# Usually /mingw64/include and /mingw64/lib are in the default search path,
# but we add them explicitly to be safe.
INCLUDE_PATHS := -I/mingw64/include
LIBRARY_PATHS := -L/mingw64/lib

# Libraries to link
# CGAL 5.0+ is mostly header-only, but it requires GMP and MPFR for math
LIBS := -lgmp -lmpfr

# Targets
TARGET := interpolation.exe
SRC    := interpolation.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) $(SRC) $(LIBRARY_PATHS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)
