# Edit according to your needs

# WHAT
PROGRAM = lazar 

#OPTIONS
FEAT_GEN = rex linfrag smarts-features
TOOLS = chisq-filter #pcprop
OBJ = feature.o lazmol.o io.o rutils.o
HEADERS = lazmolvect.h feature.h lazmol.h io.h feature-generation.h rutils.h
CC            = g++
 


ifeq ($(OS), Windows_NT) # assume MinGW/Windows

INCLUDE       = -I/include/openbabel-2.0/ -I/include/gsl-1.8/include/ -I/include/boost_1_38_0 -I/C/R/R-2.8.1/include
CXXFLAGS      = -g $(INCLUDE) -Wall
LDFLAGS       = -L/lib/  -L/include/gsl-1.8/bin  -L/C/R/R-2.8.1/bin #  -L/usr/local/bin -L/usr/local/lib
LIBS	      = -lm -llibopenbabel-3 -llibgsl -llibgslcblas -lRblas -lRlapack -lR

#RPATH         = -Wl,-rpath=/home/am/validations/libfmine


.PHONY:
all: $(PROGRAM) $(FEAT_GEN) $(TOOLS)

.PHONY:
doc: Doxyfile
	doxygen Doxyfile

lazar: $(OBJ)  lazar.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o lazar $(OBJ)  lazar.o 

linfrag: $(OBJ) linfrag.o
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS)	$(RPATH) -o linfrag $(OBJ) linfrag.o

pcprop: $(OBJ) pcprop.o
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS)	$(RPATH) -o pcprop $(OBJ) pcprop.o

rex: $(OBJ) rex.o
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o rex $(OBJ) rex.o

chisq-filter: $(OBJ)  chisq-filter.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS)	$(RPATH) -o chisq-filter $(OBJ)  chisq-filter.o 

smarts-features: $(OBJ)  smarts-features.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o smarts-features $(OBJ)  smarts-features.o 

testset: $(OBJ)  testset.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS)	$(RPATH) -o testset $(OBJ)  testset.o 

chisq-filter.o: $(HEADERS) activity-db.h feature-db.h 

rex.o: $(HEADERS) feature-generation.h

linfrag.o: $(HEADERS) feature-generation.h

pcprop.o: $(HEADERS)

smarts-features.o: $(HEADERS) feature-generation.h

lazar.o: $(HEADERS) predictor.h model.h activity-db.h feature-db.h feature-generation.h

lazmol.o: lazmol.h

feature.o: feature.h 

io.o: io.cpp io.h

rutils.o: rutils.h

testset.o: feature-generation.h




else

INCLUDE       = -I/usr/local/include/openbabel-2.0/ -I/usr/share/R/include
CXXFLAGS      = -g $(INCLUDE) -Wall -fPIC
LDFLAGS       = -L/usr/local/lib
LIBS	      = -lm -ldl -lopenbabel -lgslcblas -lgsl -lRblas -lRlapack -lR

#RPATH         = -Wl,-rpath=/home/am/validations/libfminer/
SWIG          = swig
SWIGFLAGS     = -c++ -ruby
RUBY_INC      = -I/usr/local/lib/ruby/1.8/i686-linux/

%.cxx: %.i
	$(SWIG) $(SWIGFLAGS) -o $@ $^
lazar_wrap.o: lazar_wrap.cxx
	$(CC) $(RUBY_INC) $(INCLUDE) -c -o $@ $^
lazar.so: lazar_wrap.o $(OBJ)
	$(CC) -shared $(CXXFLAGS) $(LIBS) $(LDFLAGS) $^ /usr/local/lib/libopenbabel.so /usr/lib/libgsl.so -o $@

.PHONY:
all: $(PROGRAM) $(FEAT_GEN) $(TOOLS)

.PHONY:
doc: Doxyfile
	doxygen Doxyfile

lazar: $(OBJ)  lazar.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o lazar $(OBJ)  lazar.o 

linfrag: $(OBJ) linfrag.o
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o linfrag $(OBJ) linfrag.o

pcprop: $(OBJ) pcprop.o
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o pcprop $(OBJ) pcprop.o

rex: $(OBJ) rex.o
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o rex $(OBJ) rex.o

chisq-filter: $(OBJ)  chisq-filter.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o chisq-filter $(OBJ)  chisq-filter.o 

smarts-features: $(OBJ)  smarts-features.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o smarts-features $(OBJ)  smarts-features.o 

testset: $(OBJ)  testset.o 
	$(CC) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(LDFLAGS) $(RPATH) -o testset $(OBJ)  testset.o 

chisq-filter.o: $(HEADERS) activity-db.h feature-db.h 

rex.o: $(HEADERS) feature-generation.h

linfrag.o: $(HEADERS) feature-generation.h

pcprop.o: $(HEADERS)

smarts-features.o: $(HEADERS) feature-generation.h

lazar.o: $(HEADERS) predictor.h model.h activity-db.h feature-db.h feature-generation.h

lazmol.o: lazmol.h

feature.o: feature.h 

io.o: io.cpp io.h

rutils.o: rutils.h

testset.o: feature-generation.h

endif

.PHONY:
clean:
	-rm -rf *.o $(PROGRAM) $(TOOLS) $(FEAT_GEN) lazar.so *.exe 
