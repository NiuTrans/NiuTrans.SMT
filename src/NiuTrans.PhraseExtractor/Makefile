CXXFLAGS = -O2

OBJS = basic_method.o dispatcher.o hierarchyrule_extractor.o lexicaltable_extractor.o lock.o main.o multi_thread.o n_ary_tree.o parameter_controller.o phrasetable_extractor.o ruletable_filter.o ruletable_scorer.o syntaxrule_scorer.o word_alignment.o

NiuTrans.PhraseExtractor: $(OBJS)
	g++ -o ../../bin/$@ $^ -lpthread

$(OBJS): %.o:%.cpp
	g++ $(CXXFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	-rm $(OBJS) ../../bin/NiuTrans.PhraseExtractor
