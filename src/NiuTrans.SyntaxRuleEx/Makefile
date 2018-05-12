CPP = g++
COMOBJ = ../NiuTrans.Base/Utilities.o ../NiuTrans.Base/DataStruts.o ../NiuTrans.Base/OServ.o
BINDIR=../../bin
TARGET= syntaxEx
CXXFLAGS = -O2

all: $(TARGET)

syntaxEx: $(COMOBJ) SyntaxRuleExtractor.o Main.o
	$(CPP) $(COMOBJ) SyntaxRuleExtractor.o Main.o -o $(BINDIR)/NiuTrans.SyntaxRuleEx

SyntaxRuleExtractor.o: SyntaxRuleExtractor.cpp
	$(CPP) -c SyntaxRuleExtractor.cpp -o SyntaxRuleExtractor.o $(CXXFLAGS)

Main.o: Main.cpp
	$(CPP) -c Main.cpp -o Main.o $(CXXFLAGS)

.PHONY: clean

clean:
	rm -f *.o $(BINDIR)/NiuTrans.SyntaxRuleEx

