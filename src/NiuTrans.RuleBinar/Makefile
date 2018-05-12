CPP = g++
COMOBJ = ../NiuTrans.Base/Utilities.o ../NiuTrans.Base/DataStruts.o ../NiuTrans.Base/OServ.o
BINDIR=../../bin
TARGET= ruleBinar
CXXFLAGS = -O2

all: $(TARGET)

ruleBinar: $(COMOBJ) SCFGRuleBinarization.o Main.o
	$(CPP) $(COMOBJ) SCFGRuleBinarization.o Main.o -o $(BINDIR)/NiuTrans.RuleBinar

SCFGRuleBinarization.o: SCFGRuleBinarization.cpp
	$(CPP) -c SCFGRuleBinarization.cpp -o SCFGRuleBinarization.o $(CXXFLAGS)

Main.o: Main.cpp
	$(CPP) -c Main.cpp -o Main.o $(CXXFLAGS)

.PHONY: clean

clean:
	rm -f *.o $(BINDIR)/NiuTrans.RuleBinar

