CXXFLAGS = -O2

OBJS = Config.o Global.o Interface.o Main.o MemManager.o Model.o OurChart.o OurDecoder.o OurDecoder_Phrase_ITG.o OurDecoder_Phrase_Stack.o OurDecoder_SCFG.o OurDecoder_SCFG_CSFM.o OurDecoder_SCFG_Tree.o OurInputSentence.o OurLM.o OurSharedStructure.o OurTrainer.o OurTree.o TrainingAndDecoding.o Utilities.o DataCheck.o OurDecoder_Skeleton.o

# smt platform
NiuTrans.Decoder: $(OBJS)
	g++ -o ../../bin/$@ $^ -lpthread -ldl

NiuTrans.Decoder.i386: $(OBJS)
	g++ -D__MACHINE_TYPE_32__ -o ../../bin/NiuTrans.Decoder $^ -lpthread -ldl -m32

$(OBJS): %.o:%.cpp
	g++ $(CXXFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	-rm $(OBJS) ../../bin/NiuTrans.Decoder
