VPATH = ../NiuTrans.Decoder/
COMOBJS = ../NiuTrans.Base/DataStruts.o ../NiuTrans.Base/OServ.o ../NiuTrans.Base/Utilities.o
OBJS = main.o MeReorderExtract.o

GXXFLAGS = -O2

NiuTrans.MEReorder: $(COMOBJS) $(OBJS)
	g++ -o ../../bin/$@ $^

main.o:	main.cpp

MeReorderExtract.o: MeReorderExtract.cpp

.PHONY: clean

clean:
	-rm $(OBJS) ../../bin/NiuTrans.MEReorder
