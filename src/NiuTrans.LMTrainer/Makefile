NiuTrans.LMTrainer: Main.cpp
	g++ Main.cpp -o ../../bin/NiuTrans.LMTrainer -ldl

NiuTrans.LMTrainer.i386: Main.cpp
	g++ -D__MACHINE_TYPE_32__ Main.cpp -o ../../bin/NiuTrans.LMTrainer -ldl -m32

.PHONY: clean

clean:
	-rm ../../bin/NiuTrans.LMTrainer
