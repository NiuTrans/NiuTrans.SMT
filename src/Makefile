PROJS = NiuTrans.Base \
	NiuTrans.Decoder \
	NiuTrans.LMTrainer \
	NiuTrans.MEReorder \
	NiuTrans.MSDReorder \
	NiuTrans.PhraseExtractor \
	NiuTrans.SyntaxRuleEx \
	NiuTrans.RuleBinar \
	NiuTrans.SymAlignment

all: $(PROJS)
	cd NiuTrans.Base && make
	cd NiuTrans.Decoder && make
	cd NiuTrans.LMTrainer && make
	cd NiuTrans.MEReorder && make
	cd NiuTrans.MSDReorder && make
	cd NiuTrans.PhraseExtractor && make
	cd NiuTrans.SyntaxRuleEx && make
	cd NiuTrans.RuleBinar && make
	cd NiuTrans.SymAlignment && make

all32: $(PROJS)
	cd NiuTrans.Base && make
	cd NiuTrans.Decoder && make BITS=32
	cd NiuTrans.LMTrainer && make NiuTrans.LMTrainer.i386
	cd NiuTrans.MEReorder && make
	cd NiuTrans.MSDReorder && make
	cd NiuTrans.PhraseExtractor && make
	cd NiuTrans.SyntaxRuleEx && make
	cd NiuTrans.RuleBinar && make
	cd NiuTrans.SymAlignment && make

clean: $(PROJS)
	cd NiuTrans.Base && make clean
	cd NiuTrans.Decoder && make clean
	cd NiuTrans.LMTrainer && make clean
	cd NiuTrans.MEReorder && make clean
	cd NiuTrans.MSDReorder && make clean
	cd NiuTrans.PhraseExtractor && make clean
	cd NiuTrans.SyntaxRuleEx && make clean
	cd NiuTrans.RuleBinar && make clean
	cd NiuTrans.SymAlignment && make clean

