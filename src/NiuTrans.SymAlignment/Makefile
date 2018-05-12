OBJS = Alignment.o

NiuTrans.SymAlignment: $(OBJS)
	g++ -o ../../bin/$@ $^

Alignment.o: Alignment.cpp


.PHONY: clean

clean:
	-rm $(OBJS) ../../bin/NiuTrans.SymAlignment
