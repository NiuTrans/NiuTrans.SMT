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
 * an SCFG-based decoder; OurDecoder_SCFG.h
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 *
 * $Last Modified by:
 *
 */


#ifndef _OURDECODER_SCFG_H_
#define _OURDECODER_SCFG_H_

#include <string.h>
#include <stdlib.h>
#include "Global.h"
#include "Model.h"
#include "OurInputSentence.h"
#include "OurDecoder_Phrase_ITG.h"
#include "OurTree.h"

namespace smt {

//////////////////////////////////////////////////////////////
// record the boundaries of a non-terminal

typedef struct NTBoundary
{
    short        left;
    short        right;
    Cell *       cellNode;     // for tree-parsing only
}* pNTBoundary;


//////////////////////////////////////////////////////////////
// structure defined for rule matching

class MatchedPattern
{
public:
    char *       key;          // for indexing
    NTBoundary * matching;     // matched variables
    short        matchingNum;  // # of matched variables
    List *       ruleList;

public:
    void Init(const char * myKey, int keyLen, MemPool * myMem);
    static MatchedPattern * GeneratePatternWithRightEnd(MatchedPattern * prePattern, char * key, 
                                                        int keyLen, int varLeftEnd, int varRightEnd,
                                                        MemPool * myMem);
    static MatchedPattern * GeneratePatternWithLeftEnd(MatchedPattern * prePattern, char * key, 
                                                        int keyLen, int varLeftEnd, int varRightEnd,
                                                        MemPool * myMem);
};

//////////////////////////////////////////////////////////////
//// data structure used in SCFG-based decoder
typedef struct SlotInformation
{
    int              slotNum;         // how many variables
    CellHypo **      slotHypoPointer;
    CellHypo ***     slotHypoList;
    int *            hypoNum;
    int *            invertedNTIndex; // point to the variable on the source-language side
    int *            wordIndex;
    int              wordNum;
    List *           ruleList;        // matched rule list for a given source-language pattern
    Nonterminal *    NT;              // nonterminal sequence
    MatchedPattern * pattern;
    

public:
    void Init(int slotNum, MemPool * tmpMem);

}* pSlotInformation;

typedef struct ExploredNode : public BaseSearchNode
{
    int *      offsets;
    int        lastUpdated;
    int        lastUpdatedSlot; // used for non-lexicalized expansion only
    int        ruleOffset;
    List *     ruleList;
}* pExploredNode;

//////////////////////////////////////////////////////////////
//// SCFG-based decoder

class HypoCandPool;

class Decoder_SCFG : public Decoder_Phrase_ITG
{
protected:
    SynchronousGrammar *  SCFG;    // grammar
    int                   maxWordNumInARule;
    int                   maxBlockNum;
    int                   maxVarSpan;
    bool                  allowVarBounary; // allows variables to appear at the boundares of a span
    bool                  useGlueRule;
    bool                  fastDecoding;
    bool                  allowNULLSubstitution;
    bool                  allowUnaryProduction;
    bool                  replaceFoctoid;
    bool                  removeInfeasibleRule;
    bool                  dealWithDecodingFailure;
    bool                  dumpDefeatedViterbiRule;
    bool                  dumpUsedRule;
    CellHypo *            defaultHypo;

// syntax-based decoding
protected:
    DECODER_TYPE          dtype;      // 1: hiero or 2: syntax-based
    char *                transBuf;
    int *                 widBuf;
    float *               LMScoreBuf;
    int *                 wordIndexBuf;
    bool                  withNTSymbol;
    bool                  withFineNTSymbol;
    char *                sourceSideBuf;
    char *                defaultSymbol;
    char *                defaultSymbolFineModel;
    char *                defaultOneSideSymbolFineModel;
    char *                hieroSymbol;

// "coarse-grained search + fine-grained modeling"
protected:
    CellHypo **           tmpHypoPool;
    bool                  CSFMMode;      // "coarse-search + fine-modeling" mode
    HypoCandPool *        candPool;      // "coarse-search + fine-modeling" mode
    bool                  fuzzyMatching;
    bool                  coarseHeuristic;
    bool                  useCoarseModelAlso;
    float *               tmpFeatValues;
    int                   topNDefaultNT; // use the top-n non-terminal symbols to create surrogate rules
    float                 pDefaultNT;    // use the non-terminal symbols with prob > p to create surrogate rules
    int                   directionForCSFMDecoding;

// tree-parsing (source-tree-based decoding)
protected:
    bool                  treeParsing;
    Cell *                cellNodes;
    int                   cellNodeNum;
    bool                  generateTreeStructure;

// fuzzy decoding
    bool                  hieroMatchAll;

// for paper work
protected:
    float                 epsilonViterbiScore;

public:
	Decoder_SCFG();
    Decoder_SCFG(DECODER_TYPE type);
    ~Decoder_SCFG();
    void Init(Model * m);
    void ResetTransInfo();

    void DecodeInput(DecodingSentence * sentence);
    bool DeosDecodingFail();

protected:

    // get basic information for decoding
    void MatchSCFGRules();
    void AddBasicRules();
    void HandleUnknownWord(Cell * c, char * phrase, char * label);
    void AddCellTrans(Cell * c, UnitRule * rule, bool outputUnknown, char * label);
    void GenerateRuleKey(MatchedPattern * preKey, int keyLen, int leftEnd, int beg, int wordCount, int blockCount);
    void AddMatchedPattern(int beg, int end, MatchedPattern * pattern);
    void SetDefaultSymbol();

    // for decoding
    void CKYDecoding(DecodingSentence * sent);
    void GenerateTrans(Cell * c);
    void ApplyMatchedPattern(Cell * c, List * ruleList, MatchedPattern * pattern, bool haveFoctoid, SlotInformation * slotInfo, MemPool * tmpMem);
    void ApplyTranslationRule(Cell * c, List * ruleList, MatchedPattern * pattern, bool haveFoctoid, SlotInformation * slotInfo, MemPool * tmpMem);
    bool InitSlotInfo(MatchedPattern * pattern, BasicRule * rule, SlotInformation * slotInfo, MemPool * tmpMem);
    CellHypo * GenerateHypothesisWithTheRule(Cell * c, UnitRule * rule, SlotInformation * slotInfo, bool haveFoctoid, MemPool * tmpMem);
    void GenerateTranslationWithTheRule(UnitRule * rule, SlotInformation * slotInfo, char * &trans, int &transLen, MemPool * tmpMem);
    bool HaveFoctoid(int beg, int end, MatchedPattern * pattern, SlotInformation * slotInfo, MemPool * myMem);
    char * ReplaceFoctoidTrans(int * wordIndex, int wordNum, char * trans, MemPool * myMem);
    void ApplyNonlexRules(Cell * c, MemPool * tmpMem);
    void ApplyNonlexRules(Cell * c, Cell * subc1, Cell * subc2, SlotInformation * slottInfo, MemPool * tmpMem);
    List * GetRuleListForSlotInfo(SlotInformation * slotInfo);
    void CompleteCell(Cell * c);
    void RecordChildHypo(CellHypo * ch, SlotInformation * slotInfo, MemPool * tmpMem);
    UnitRule * CreateGlueRule(Cell * c, SlotInformation * slotInfo, MemPool * tmpMem);

    // for heuristics-based search
    void HeuristicSearch(Cell * c, UnitRule * rule, SlotInformation * slotInfo, 
                         bool haveFoctoid, bool forNonlex, MemPool * tmpMem);
    void EnQueue(Cell * c, ExploredNode * exploredNode, UnitRule * rule, SlotInformation * slotInfo, 
                 bool haveFoctoid, bool forNonlex, MemPool * tmpMem);
    ExploredNode * CopyExploredNode(ExploredNode * me, int NTCount, MemPool * tmpMem);
    ExploredNode * CopyExploredNode(ExploredNode * me);
    inline bool CheckExplored(ExploredNode * me);
    inline void SetExplored(ExploredNode * me);

    // glue rule
    void ApplyGlueRulesForSpan(Cell * c, MemPool * tmpMem);
    void ApplyGlueRulesForEndSpan(Cell * c, MemPool * tmpMem);
    void GlueTwoCells(Cell * c, Cell * subc1, Cell * subc2, SlotInformation * slotInfo, MemPool * tmpMem);

    // syntax-based models
    char * GetNonterminalSymbol(Nonterminal * NT);
    void CompleteSpanForNTSymbols(Cell * c, MemPool * mem);
    void AssignRuleAndSyntacticLabel(CellHypo * ch, CellHypo * subc1, CellHypo * subc2);
    bool IsSentEnd(Cell * c);
    bool IsAcceptable(CellHypo * ch);
    void ApplyUnaryRulesInBeam(Cell * c, MemPool * tmpMem);
    bool HaveValidNULLTrans(SlotInformation * slotInfo);
    void AddRuleProbForImcompleteHypo(CellHypo * newch, CellHypo * subc1, CellHypo * subc2);

    // "coarse-grained search + fine-grained modeling"
    CellHypo * FineModelHypo(CellHypo * ch, UnitRule * rule, SlotInformation * slotInfo, MemPool * tmpMem);
    int CheckNTConsistency(CellHypo * ch, UnitRule * rule, SlotInformation * slotInfo, float * featForUnmatchedNT);
    bool GetFeatValueForUnmatchedNT(char * symbol1, char * symbol2, float * featForUnmatchedNT);
    CellHypo * GetBaseScoreUsingFineGrainedModel(CellHypo * coarseHypo, UnitRule * coarseRule, SlotInformation * slotInfo, MemPool * tmpMem);
    void GenerateNewHypoUsingFineGrainedModel(UnitRule * rule, SlotInformation * slotInfo, MemPool * tmpMem);
    float ScoreHypoUsingFineGrainedModel(CellHypo * ch, UnitRule * rule, SlotInformation * slotInfo, MemPool * tmpMem);
    float ScoreHypoUsingDefaultModel(CellHypo * ch, UnitRule * rule, SlotInformation * slotInfo, char * root, MemPool * tmpMem);
    float GetLabelProb(char * symbol);
    char * GenerateFineGrainedSymbol(char * coarseSymbol, char * suggestedSymbol, UnitRule * coarseRule, MemPool * tmpMem);
    char * GenerateFineGrainedSymbol(char * coarseSymbol, UnitRule * coarseRule, MemPool * tmpMem); // to be updated
    void AddCellHypoForCSFM(Cell * c, CellHypo * ch);

    // tree-parsing (source-tree-based decoding)
    void SetTreeNode(Tree * tree);
    void SetTreeNode(TreeNode * root);
    void SetTreeNodeNoRecursion(TreeNode * root);
    void MatchRulesForTreeParsing(MemPool * myMem);
    void MatchRulesForTreeParsing(Cell * c, MemPool * myMem);
    bool AddMatchedPatternUsingTreeFragment(Cell * c, TreeFrag * frag, MemPool * myMem);
    void TreeParsing();
    void GenerateNodeTrans(Cell * c);
    void GlueMultipleNodeTrans(Cell * target, List * cNodes, MemPool * myMem);
    char * GetDefaultRootLabel(Cell * c, const char * tgtSymbol, MemPool * myMem);
    char * GetDefaultRootLabel(TreeNode * treeNode, const char * tgtSymbol, MemPool * myMem);
    void SetUserTransForTreeParsing();

    // to build a reachable derivation when decoding fails
    void CreateTransUsingTreeSequence();
    List * GetBestSpanSequenceForTreeParsing();
    float GetBestSpanSequenceForTreeParsing(List * sequence, TreeNode * root);
	List * GetBestSpanSequenceForParsing();

    // to add rules using pre-matched patterns
    void LoadPrematchedRules(DecodingSentence * sentence);
    void AddPrematchedRules(Cell * c, SimpleRule * rule);
    bool IsMatched(TreeNode * rootNode, MatchedPattern * pattern, SimpleRule * rule); // whether a rule can be mapped onto a given pattern and span
    bool IsMatched(TreeNode * node, char * targetLabel);
    MatchedPattern * GenerateMatchedPattern(SimpleRule * rule, MemPool * myMem);

    // check various issues
    void CheckMe();

    // to add and check Viterbi rules
    void LoadViterbiRules(DecodingSentence * sentence);
    void CheckViterbiRules();
    bool CheckViterbiRulesForTreeParsing(TreeNode * rootNode); // check for tree node (hyper-node in source-side tree/forest)
    bool CheckViterbiRulesForTreeParsing(Cell * c); // check for a cell (hyper-node in translation forest)
    bool CheckViterbiRuleForHypothesis(SimpleRule * vRule, CellHypo * ch); // check whether a (Viterbi) rule is used in generating a given hypothsis
    bool CheckViterbiRuleUsage(SimpleRule * vRule, CellHypo * ch);
    void FindCausesForDefeatedViterbiRule(CellHypo * baseHypo, SimpleRule * vRule, List * defeatedRuleList);
    void FindTMCauseForDefeatedViterbiRule(CellHypo * baseHypo, SimpleRule * vRule, List * defeatedRuleList);
    List * FindDefeatedRulesForChildNodes(CellHypo * ch);

    // log
    void DumpLog(DecodingLoger * log);
    void DumpViterbiLog(Cell * c, DecodingLoger * log);
    void DumpUsedRuleLog(CellHypo * ch, DecodingLoger * log);
    void DumpUsedRuleSlots(CellHypo * ch, char * log);
    void DumpUsedRuleScore(CellHypo * ch, char * log);

    // misc
    float GetTranslationModelScore(CellHypo * ch);
    float GetNGramLMModelScore(CellHypo * ch);
    float GetSMTModelScore(CellHypo * ch);

};

typedef struct HypoList{
    void ** list;
    int     n;
}* pHypoList;

class HypoCandPool // for "coarse-search + fine-modeling" only
{
    List * p;
    List * n;
    List * pool;
    int    maxCandNum;
    int    cur;

public:
    HypoCandPool(int max, MemPool * mem);
    ~HypoCandPool();
    void Clear();
    int Add(long num, CellHypo ** head);
    void Get(int id, int &num, CellHypo ** &head);
};

}

#endif

