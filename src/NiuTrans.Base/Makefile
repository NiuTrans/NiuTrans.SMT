CPP = g++
CXXFLAGS = -O2
COMOBJS = OServ.o Utilities.o DataStruts.o

all: $(COMOBJS)

OServ.o: OServ.h OServ.cpp
	$(CPP) -c OServ.cpp -o OServ.o $(CXXFLAGS)

Utilities.o: Utilities.h Utilities.cpp
	$(CPP) -c Utilities.cpp -o Utilities.o $(CXXFLAGS)

DataStruts.o: DataStruts.h DataStruts.cpp
	$(CPP) -c DataStruts.cpp -o DataStruts.o $(CXXFLAGS)

.PHONY: clean

clean:
	-rm $(COMOBJS)

