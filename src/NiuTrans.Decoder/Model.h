/* NiuTrans - SMT platform
 * Copyright (C) 2011, NEU-NLPLab (http://www.nlplab.com/). All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/*
 * $Id:
 * SMT System's Model; Model.h
 * This header file defines the "model" of our translation system.
 * The model is constructed by several components, including
 * "translation model (TM)", "language model (LM)" and "re-ordering models".
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn); Jan 2, 2013, add "associated models" into class "Model"
 * Hao Zhang (email: zhanghao1216@gmail.com); June 24th, 2011; fix a bug for "word deletion" in function "PhraseTable::ParseTransOption"
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn); June 19th, 2011
 * Hao Zhang (email: zhanghao1216@gmail.com); April 13th, 2011;
 *
 */


#ifndef __MODEL_H__
#define __MODEL_H__

#include "Config.h"
#include "Utilities.h"
#include "MemManager.h"
#include "OurTrainer.h"
#include "OurLM.h"
using namespace utilities;
using namespace memmanager;
using namespace utilities;

namespace smt {

    class PhraseTable;
    class MEReorderModel;
    class MSDReorderModel;
    class SynchronousGrammar;
    class NEToNTSymbol;
    class NTMappingProb;

    ////////////////////////////////////////
    // resources required by our decoder 

    class Model
    {
    public:
        static const int NGramLM = 0;
        static const int WordCount = 1;
        static const int PrF2E = 2;
        static const int PrF2ELex = 3;
        static const int PrE2F = 4;
        static const int PrE2FLex = 5;
        static const int PhraseCount = 6;
        static const int BiLexCount = 7;
        static const int PenaltyNull = 8;
        static const int MEReorder = 9;
        //static const int GlueRuleCount = 10;

        static const int MosRe_L2R_M = 11;
        static const int MosRe_L2R_S = 12;
        static const int MosRe_L2R_D = 13;
        static const int MosRe_R2L_M = 14;
        static const int MosRe_R2L_S = 15;
        static const int MosRe_R2L_D = 16;

        /* for heiro */
        static const int RuleCount = 6;
        static const int PhrasalRuleCount = 9; // rules without variables invloved
        static const int GlueRuleCount = 10;

        /* for syntax-based system */
        static const int PrBiCFG = 11;
        static const int PrCFG2 = 12;
        static const int IsLexRule = 13;
        static const int IsComposedRule = 14;
        static const int IsLowFreqRule = 15;

        /* another language model */
        static const int NGramLM2 = 17;
        static const int WordCount2 = 18;

    public:
        static int TableFeatBeg;
        static int MaxTableFeatNum;
        static int TableFeatBegCoarse;
        static int TableFeatEndCoarse;

        static int MinFeatNum;
        static int MainTableBeg;
        static int MainTableEnd;

        static int CSFMSurrogateRuleCount;
        static int CSFMMatchedNTCount;
        static int CSFMMatchedSrcNTCount;
        static int CSFMMatchedTgtNTCount;
        static int CSFMUnmatchedNTCount;
        static int CSFMUnmatchedSrcNTCount;
        static int CSFMUnmatchedTgtNTCount;
        
        static int RuleSoringDimen; // dimension for sorting SCFG rules

	public:
		static int associatedModelFeatBeg;
        
    public:
        int           featNum;
        int           tableFeatNum;
        int           tableFeatNumPhraseBased;
        int           tableFeatNumSyntaxBased;
        float *       featWeight; // features and feature weights in log-linear model

    public:
        
		Configer *           configer;
        ParaInfo *           paras;         // feature weights' properties for MERTraing in class "OurSystem"

        PhraseTable *        pTable;        // phrase table
        MEReorderModel *     MEReModel;     // ME-based reordering model
        OurLM *              ngramLM;       // n-gram language model
		OurLM *              ngramLM2;      // the 2nd language models
        int                  ngram;
        MSDReorderModel*     msdReModel;    // MSD target-side re-ordering model
        //TO BE ADDED: MSDReorderModel* msdReModel_S; // MSD source-side re-ordering model

        SynchronousGrammar * SCFG;          // sychronous context-free grammar
        SynchronousGrammar * fineSCFG;      // sychronous context-free grammar for (fine-grained modeling)

        NEToNTSymbol *       NE2NT;         // NE's syntactic label
        NEToNTSymbol *       NE2NTFineModel;// NE's syntactic label (for the fine-grained model)
        PhraseTable *        vocabManager;  // tgt vocab
        DECODER_TYPE         type;
        int                  treeBasedModel;// 1: string-to-tree 2: tree-to-string 3: tree-to-tree
        NTMappingProb *      NTProb;

        bool                 xtsearch;      // different modeling on different search stages
        int                  firstspan;     // for xtsearch

		/* misc */
    public:
        bool                 noFiltering;   // does not filter out any phrase/rule 
        bool                 useForest;     // forest-based decoding

		/* second (character-based) language model */
	public:
		bool                 useCharLM;

        /* for decdoding */
    public:
        float                incompleteHypoRate;
        float                backoffToHiero;  // backoff weight
        float                backoffToSyntax; // backoff weight
        float                baseBiCFGProb;

        bool                 CSFMMode;        // "coarse-search + fine-modeling" mode

	public:
		List *               associatedModels;
		List *               associatedConfigs;
		Model *              skeletonModel;
		Model *              assistantModel;
		Configer *           skeletonConfiger;
		Configer *           assistantConfiger;

    public:
        Model(DECODER_TYPE type, Configer * c = &ConfigManager);
        ~Model();
        // initialize model
        void Init();
		void InitForSkeletonTranslationModel();
        void InitFeatNum();
        void InitForDecoding();
        void InitNTProb();
        void LoadPara();
        void LoadNESymbol();
		void LoadLMs();

        // word -> id (akso for chen rushan)
        int GetWId(const char * trans);
        int GetWId2ndVocab(const char * trans);
        void GetWId(char * trans, int * &wId, int &wCount, MemPool * myMem);
        void GetWId2ndVocab(char * trans, int * &wId, int &wCount, MemPool * myMem);
        char * GetWord(int wId);
        char * GetWord2ndVocab(int wId);

        // rule table
        List * FindRuleList(const char * src);
        List * FindRuleListWithSymbol(const char * src);

        // export functions from phrase-table (also for chen rushan!!!)
        List * FindOptionList(const char * src);

        // export functions from language model (also for chen rushan!!!)
        float GetNGramLMProb(int beg, int end, int * wid);
        float GetCharNGramLMProb(int beg, int end, int * wid);	
        
        // export functions from ME-reorder model
        float GetMEReorderScore(char ** feats);

        // export functions from MSD-reorder model
        float GetMSDReoderingFeatVal( char* fStr, char* tStr, const char* featType );

        const char * GetFeatString(int featDim, DECODER_TYPE dtype, bool CSFM);

        const char * GetCSFMInfo();

        bool IsSCFGWithNTSymbol();

		DECODER_TYPE GetModelType(const char * type);

		void UpdateAssociatedModelFeatureWeights();
    };

    // translation option
    typedef struct PhraseTransOption
    {
        char *       src;         // source-side
        char *       tgt;         // target-side (translation)
        float *      feat;        // features (value)
        int *        wid;         // word-id (for n-gram LM)
        int          wCount;      // number of target words
    }* pPhraseTransOption;

    ////////////////////////////////////////
    // phrase-table needed in deocding

    class PhraseTable
    {
    protected:
        DECODER_TYPE type;        // type of the model
		Configer *   configer;
        MemPool *    mem;         // memory (in blocks)
        HashTable *  entry;       // entry of the phrase table
        HashTable *  srcDict;
        HashTable *  tgtDict;
        HashTable *  tgtDict2;
        HashTable *  NULLTransDict;
        char **      srcVocab;
        char **      tgtVocab;
        char **      tgtVocab2;
        int          srcVocabSize;
        int          tgtVocabSize;
        int          tgtVocabSize2;
        int          featNum;
        float *      featList;    // tmp structure
        bool         useNULLTrans;
        bool         useLowercase;
        bool         use2ndVocab; // for the 2nd language model
        bool         outputNULL; // output NULL translation
        bool         patentMT;   // trigger for ntcir-9 patent MT task
        bool         noFiltering; // does not filter out any phrase/rule 
        bool         freeFeature;
        int          tableFeatNum;
        PhraseTable * baseTable;

    public:
        int          unkWId;
        int          unkWId2;

    public:
        PhraseTable(int entryNum, int featNum, int tableFeatNum, DECODER_TYPE type, PhraseTable * baseTable, Configer * c = &ConfigManager);
        ~PhraseTable();
        void LoadTable();

    protected:
        void LoadPara();
        int LoadPhraseTable();
        int LoadDict(HashTable *  dict, const char * dictfn, const char * name, char ** vocab, int &vocabSize);
        PhraseTransOption * ParseTransOption(char * src, char * tgt, char * feat);

    public:
        int GetWId(const char * trans);
        int GetWId2ndVocab(const char * trans);
        void GetWId(char * trans, int * &wId, int &wCount, int vocabId, MemPool * myMem);
        void GetWId(char * trans, int * &wId, int &wCount);
        char * GetWord(int wId);
        char * GetWord2ndVocab(int wId);
        List * FindOptionList(const char * src);
        void GetTerms(char * line, char ** terms, int &termCount);
        bool IsValid(const char * src, const char * tgt);
        bool IsValidForFoctoid(const char * src, const char * tgt);
        bool IsValid(const char * src, const char * tgt, const char * type);
        bool IsValidForPatentMT(const char * src, const char * tgt);
        bool CheckLiteralMatching(const char * src, const char * tgt);

    protected:
        void AddOptionList(char * src, void ** optionList, int optionCount, int initListSize);
        void AddOption(const char * src, void * option,  int initListSize);
        void ParseFeatValues(float * featP, char * feat, int maxFeatNum);
        void AddMetaEntry(const char * src);
        void AddMetaPhrases();
    };

    //////////////////////////////////////////////////////////////
    // Maximum Entropy-based reordering model (Xiong et al., 2006)

    typedef struct MEReFeatInfo{
    public:
        float feats[8];
        // 0: class-0 left-1
        // 1: class-0 right-1
        // 2: class-1 left-1
        // 3: class-1 right-1
        // 4: class-0 left-2
        // 5: class-0 right-2
        // 6: class-1 left-2
        // 7: class-1 right-2
    }* pMEReFeatInfo;

    enum MEReorderFeatType {SLL, SLR, SRL, SRR, TLL, TLR, TRL, TRR}; // eight features used in the model

    class MEReorderModel
    {
    private:
		Configer *   configer;
        MemPool *    mem;
        HashTable *  entry;
        HashTable ** entryFeat;

    public:
        MEReorderModel(Configer * c = &ConfigManager);
        ~MEReorderModel();

        void LoadModel();
        float Calculate(char ** reorderFeat);
        float GetFeatWeight(int classId, int featId, char * feat);
        float GetFeatWeight(char * feat);
    };


    /* definition of MSD re-ordering model */
    typedef struct MSDReFeatInfo {
        float feats[6];
    }
    * pMSDReFeatInfo;

    class MSDReorderModel {

    private:
		Configer *  configer;
        MemPool *   pMem;
        HashTable * entryList;
        bool        onSrcSide;

    public:
        pMSDReFeatInfo    defaultFeatInfo;

    public:
        MSDReorderModel(Configer * c = &ConfigManager);
        MSDReorderModel(bool onSourceSide, Configer * c = &ConfigManager);
        ~MSDReorderModel();

        void LoadMSDReorderModel();
        void GetTerms(char * line, char ** terms, int &termCount);
        void ParseFeatValues( char* feats, pMSDReFeatInfo opt );
        pMSDReFeatInfo GetFeatList( char* fStr, char* tStr );

    };


    /********************************************
    *
    * NE's syntactic label
    *
    *********************************************/

    class NEToNTSymbol
    {
    protected:
        HashTable *   NTDict;
        char **       symbolVocab;
        int           symbolCount;
        int           maxSymbolNum;

    public:
        NEToNTSymbol();
        ~NEToNTSymbol();
        void Add(const char * NE, const char * symbol);
        char * GetSymbol(const char * NE);
        char * GetSymbolByDefault(const char * NE);
        void Load(const char * fn);
    };


    /********************************************
    *
    * non-terminal to non-terminal mapping prob
    *
    *********************************************/

    struct NTMappingNode{
        char * nt;
        float  prob;
    };

    class NTMappingProb
    {
    protected:
        HashTable *       srcNTDict;
        HashTable *       tgtNTDict;
        NTMappingNode **  srcToTgt;
        NTMappingNode **  tgtToSrc;
        int *             srcMappingNum;
        int *             tgtMappingNum;
        int               srcEntryCount;
        int               tgtEntryCount;

        NTMappingNode *   srcToTgtDefault;
        NTMappingNode *   tgtToSrcDefault;
        NTMappingNode *   jointDefault;
        int               srcToTgtDefaultNum;
        int               tgtToSrcDefaultNum;
        int               jointDefaultNum;

    public:
        NTMappingProb();
        ~NTMappingProb();
        void Load(const char * fn_s2t, const char * fn_t2s);

    protected:
        void Load(const char * fn, HashTable * dict, NTMappingNode ** mappingProb, int * mappingNum, int &entryCount);
        void CreateMappingNode(NTMappingNode * node, const char * NT, float prob);
        void AddMappingNode(char * srcNT, NTMappingNode * mappingBufProb, int mappingBufCount, 
                            HashTable * dict, NTMappingNode ** mappingProb, int * mappingNum, int &entryCount);
        
    public:
        NTMappingNode * GetMappingList(int direction, char * srcNT, int &mappingNum);
        int GetTopNMapping(int direction, char * srcNT, int n, NTMappingNode * &result);
        int GetMappingAboveP(int direction, char * srcNT, int n, float p, NTMappingNode * &result);
    };

    /********************************************
    *
    * Synchronous Context-Free Grammar
    *
    *********************************************/

    // non-terminal
    typedef struct Nonterminal
    {
        char *       symbol;     // non-terminal symbol
        char *       symbolSrc;  // symbol of the source-langauge side non-terminal
        char *       symbolTgt;  // symbol of the source-langauge side non-terminal
    }* pNonterminal;

    struct UnitRule;

    // basic structure for SCFG rule
    typedef struct BasicRule
    {
        char *        root;     // root symbol
        char *        rootSrc;  // root symbol of the source-langauge side
        char *        rootTgt;  // root symbol of the target-langauge side

        char *        src;      // source-language side
        char *        tgt;      // target-language side (translation)
        char *        alignment;// block-to-block alignment
                                // block: word sequence or variable

        Nonterminal * NT;       // nonterminal sequence
        short         NTCount;  // # of non-terminals


        float *       feat;     // features (value)
        int *         wid;      // word-id (for n-gram LM)
        int           wCount;   // # of target words
        char *        key;         // for indexing

        // no alignment information
    }* pBasicRule;

    // sychronous grammar rule
    typedef struct SCFGRule : public BasicRule
    {
        UnitRule **   ruleList;  // unit rules
        int           ruleCount; // number of the unit rules

        SCFGRule **   CSFMRules; // pointers to the correcponding rules in coarse-grained grammar (for search) or
                                 // fine grained grammar (for modeling)
        int           CSFMRuleNum;
    }* pSCFGRule;

    // unit rule used for decoding
    typedef struct UnitRule : public BasicRule // rule used for chart-parsing
    {
        bool          isComplete;  // not root at a virtual non-terminal
        bool          isLexRule;   // is a lexical rule in the Lexical Normal Form; othewise is a non-lexical rule
        SCFGRule *    parentRule;  // a pointer to the original SCFG rule
        short         id;          // index the unit rule in a given SCFG rule
    }* pUnitRule;

    extern int CompareUnitRules(const void * rule1, const void * rule2);

    class SynchronousGrammar : public PhraseTable
    {
    protected:
        bool          withoutGrammarEncoding; // enabled when no grammar encoding (e.g. binarization) is required

    public:
        Model *       model;
        HashTable *   symbolDict;     // non-terminal symbols
        HashTable *   srcSymbolDict;  // source-side non-terminal symbols
        HashTable *   tgtSymbolDict;  // target-side non-terminal symbols
        char **       symbolVocab;
        char **       srcSymbolVocab;
        char **       tgtSymbolVocab;
        int           symbolCount;
        int           srcSymbolCount;
        int           tgtSymbolCount;

        Nonterminal * tmpNTs;
        int           tmpNTCount;
        UnitRule **   tmpList;
        int           tmpCount;
        List *        ruleBase;       // where are the rules
        int           ruleCount;
        List *        sourceSideBase;

        HashTable *   entryWithSymbol;       // index the rules with non-terminal symbols
        DECODER_TYPE  type;                  // 1: hiero or 2: syntax-based
        bool          withSymbolEntry;
        bool          strictNTLabeling;      // true when punctuations (such as "," are ".") are excluded.
        bool          allowNonlexReordering;
        bool          forTreeParsing;        // using source-side syntax for indexing (for source-tree-based decoding)
        bool          generateTreeStructure; // false: pure SCFG rule (no STSG rule is involved)
        bool          dumpRule;              // the rules will also be outputed for certain purpose

    public:
        SynchronousGrammar(int entryNum, int featNum, int tableFeatNum, Model * model, DECODER_TYPE type, PhraseTable * baseTable, Configer * c = &ConfigManager);
        ~SynchronousGrammar();

        void LoadGrammarPara(DECODER_TYPE type);
        int LoadSCFG(const char * fn = NULL, bool CSFMMode = false);
        //SCFGRule * ParseSCFGRule(char * root, char * src, char * tgt, char * feat, bool keyWithSymbol);
        SCFGRule * ParseSCFGRule(char ** terms, int termCount, bool keyWithSymbol);
        void AddSCFGRule(SCFGRule * rule);
        static bool IsMetaSymbol(char * symbol);
        List * FindOptionListWithSymbol(const char * src);

        static bool IsNonLexial(char * src);
        static bool IsComplete(char * root);

    protected:
        void AddMetaSymbol();
        void ParseSymbol(char * symbol, char * &root, char * &rootSrc, char * &rootTgt);
        char * AddSymbol(const char * symbol, HashTable * sdict, char** symbolVocab, int &symbolCount);
        void ParseSourceSide(char * src, BasicRule * rule, char * &key, bool generateKey, bool indexForSourceTreeParsing);
        void GetNTNum(BasicRule * rule);
        void PostProcessingSourceSide(char * &key, int &keyLen);
        void GenerateUnitRules(SCFGRule * rule, bool keyWithSymbol);
        int  LoadUnitRules(FILE * f, SCFGRule * rule, UnitRule ** uruleBuf, char * lineBuf, char ** termsBuf, bool keyWithSymbol);
        void AddMetaRules();
        SCFGRule * AddMetaRuleEntry(const char * symbol);
        void AddOptionWithSymbol(char * src, void * option, int initListSize);
        void AddOptionWithRuleSourceSide(char * src, UnitRule * urule, int initListSize);
        void AddSourceSideForUnitRules(SCFGRule * rule);
        void SortRules();
        bool IsValidForSCFG(const char * src, const char * tgt);
        bool IsValidSymbol(const char * symbol);
        bool IsValidWidForSCFG(BasicRule * rule);
        bool IsUnaryProduction(const char * src);
        bool IsUnaryProductionForSourceSide(char * srcSymbol, char * ruleSymbol);
        bool IsUnaryProductionForTargetSide(char * tgtSymbol, char * ruleSymbol);
        bool InformalCheck(const char * src, const char * tgt, SCFGRule * rule);
        void PreprocessRootLabel(char * root);

        // for coarse-gained search and fine-grained modeling
    public:
        static void LoadCSFM(const char * cfn, const char * ffn, SynchronousGrammar * coarseGrammar, SynchronousGrammar * fineGrammar);
        void LoadCSFMRulePointers(char * pline, SCFGRule * rule);
    };

}


#endif

