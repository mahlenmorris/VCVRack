# Directives for other Makefile's to read to know how to build the parser
# code.
# NOT held consistant with the Makefile in this directory (yet).

BISON = bison
CXX = g++
FLEX = flex

%.cc %.hh %.html %.gv: %.yy tree.hh environment.h
	$(BISON) $(BISONFLAGS) --html --graph -o $*.cc $<

%.cc: %.ll
	$(FLEX) $(FLEXFLAGS) -o$@ $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o$@ $<

#$(BASE): $(BASE).o driver.o parser.o scanner.o
#	$(CXX) -o $@ $^

#$(BASE).o: parser.hh
parser.o: parser.hh tree.hh
scanner.o: parser.hh

CLEANFILES =										\
  *.o	parser.d									\
  parser.hh parser.cc parser.output parser.xml parser.html parser.gv location.hh	\
  scanner.cc
# TODO: Fix this so it removes files from the right directory each time.
clean_parser:
	rm -f $(CLEANFILES)
