# This Makefile is designed to be simple and readable.  It does not
# aim at portability.  It requires GNU Make.

# This is for parsing Venn configurations.

BISON = bison
CXX = g++
FLEX = flex
FLEXFLAGS = -i -P vv
BISONFLAGS = -Wcounterexamples -p vv

%.cc %.hh %.html %.gv: %.yy
	$(BISON) $(BISONFLAGS) --html --graph -o $*.cc $<

%.cc: %.ll
	$(FLEX) $(FLEXFLAGS) -o$@ $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o$@ $<

parser.o: parser.hh
scanner.o: parser.hh

CLEANFILES =										\
  *.o										\
  parser.hh parser.cc parser.output parser.xml parser.html parser.gv location.hh	\
  scanner.cc
clean:
	rm -f $(CLEANFILES)
