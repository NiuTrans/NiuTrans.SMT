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
 * ITG-Based decoder for phrase-based SMT; OurDecoder_Phrase_ITG.h
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (xiaotong@mail.neu.edu.cn); December 20th, 2010
 * Hao Zhang (email: zhanghao1216@gmail.com); January 21th, 2011
 *
 * $Last Modified by:
 * Tong Xiao (xiaotong@mail.neu.edu.cn); June 19th, 2011
 * Hao Zhang (email: zhanghao1216@gmail.com); May 9th, 2011
 *
 */


#ifndef _OURDECODER_PHRASE_ITG_H_
#define _OURDECODER_PHRASE_ITG_H_

#include "Global.h"
#include "Model.h"
#include "OurTree.h"
#include "OurInputSentence.h"
#include "OurChart.h"
#include "OurDecoder.h"

namespace smt {

/* for search */
typedef struct BaseSearchNode
{
    CellHypo * ch;
}* pBaseSearchNode;

/* for search */
typedef struct ExploredSimpleNode : public BaseSearchNode
{
    CellHypo * ch2;
	short      bundle;
	short *    offsets;
}* pExploredSimpleNode;

class NodeHeapForSearch;

////////////////////////////////////////
// phrase-based decoder

class Decoder_Phrase_ITG : public BaseDecoder
{
public:
    Cell **            cell;
    int                maxWordNumInPhrase;
    int                nbest;
    int                maxDistortionDistance;
    int                beamSize;
    float              posteriorAlpha;
    int                featNum;
    char               nullString[1];

    UserTranslation ** partialTrans;
    char **            factoids;
    int                factoidCount;

    bool               dumpCell;
	bool               smallMem;
	int                memBlockSize;
	int                memBlockNum;
	int                minMemSize;
    bool               multiLMs;

protected:
    unsigned int       ignoreComposeNum;
    unsigned int       totalComposeNum;

protected:
    int **             explored;
	bool *             explored2;
    HypoHeap *         hypoQueue;
	NodeHeapForSearch * heapForSearch;   // a heap used in heuristics-based search

    float              beamScale;

    bool               outputOOV; // output OOV or not?
    bool               labelOOV;  // label OOV words in the form <OOV>
    bool               toEnglish; // the target language is English
    bool               allowSequentialNULLTrans;
    bool               dumpLeftHypo;
    bool               normalizeOutput;
    bool               serverDisplay;
    char *             serverLog;
    bool               dumpWordTrans;
    bool               useUppercase;

	/* pruning settings */
    bool               usePuncPruning;
    int                boundaryWordNumForPunctPruning;
    bool               useCubePruning;
	bool               useCubePruningFaster;
	int                maxBundleNum;
	int                maxItemNumInCubePruning;
	

	/* reordering settings */
protected:
    bool               useMEReordering;
    bool               useMSDReordering;

protected:
    bool               doRecasing;

    /* more decoding settings */
protected:
    bool               forcedDecoding;
    bool               lossAugumentedDecoding;
    bool               beamLoss; // for loss augumented decoding
    bool               bleuLoss; // for loss augumented decoding
    int                bleuLossNGram; // for loss augumented decoding
    int                beamLossK;     // k is a parameter in the beam loss
                                      // for candidates with rank <= k, loss = k - rank
                                      // otherwise, loss = - 2^(rank - k)
    float              beamLossWeight; // weight of beam loss;
    float              bleuLossWeight; // weight of bleu loss;
    int                refNum; // number of reference translations
    RefNGram **        refNGram;

protected:


public:
    Decoder_Phrase_ITG();
    ~Decoder_Phrase_ITG();

    void Init(Model * m);
    void InitCell();
    void InitRefNGram();
    void DecodeInput(DecodingSentence * sentence);
    int DumpTransResult(int nbest, TransResult * result, DecodingLoger * log);
    int DumpWordAlignment(TransResult * result, CellHypo * ch);

    void Unload();
    
    void ResetTransInfo();
    void LoadSettings();
    void SetUserTranslations(DECODER_TYPE decoderType, NEToNTSymbol * NESymbol = NULL, NEToNTSymbol * NESymbolFineModel = NULL);
    void ComputeModelScore(CellHypo * ch);
    void Compute2ndLMScore(CellHypo * ch);
    void MatchPhrases();
    void AddPhrase(Cell * c, char * srcPhrase, bool haveFoctoid);
    void RefineOptions(Cell * c, int inListSize, CellHypo * ch);
    void AddCharNGramLM(Cell * c, CellHypo * ch, char * trans);
    char * ReplaceFoctoidTrans(Cell * c, char * trans, MemPool * myMem);
    char * RecaseLiteral(int iword, char * trans, MemPool * myMem);

protected:
    void MatchSrcReorderFeat(Model * m);
    void GenerateTrans(Cell * c);
	Cell * GetCell(int beg, int end);
    void ComposeTwoSpans(Cell * c, Cell * sub1, Cell * sub2, MemPool * tmpMem);
    void ComposeTwoSpans_Naive(Cell * c, Cell * sub1, Cell * sub2, MemPool * tmpMem);
    void ComposeTwoSpans_CubePruning_AlongOneEdge(Cell * c, Cell * sub1, Cell * sub2, MemPool * tmpMem);
	void ComposeSpans_CubePruning_ForAllIncomingEdges(Cell * c, MemPool * tmpMem);
    CellHypo * ComposeTwoHypotheses(Cell * c, CellHypo * ch1, CellHypo * ch2, DECODER_TYPE d, MemPool * tmpMem);
    void Compute2ndLMScoreWhenComposeTwoHypotheses(CellHypo * rch, CellHypo * ch1, CellHypo * ch2, MemPool * tmpMem);
	CellHypo * ComposeTwoHypothesesForSkeleton(Cell * c, CellHypo * ch1, CellHypo * ch2, DECODER_TYPE d, MemPool * tmpMem); // this method is explained in "OurDecoder_Skeleton.cpp"
	bool ComposeTwoHypothesesWithTheFullModel(Cell * c, CellHypo * ch1, CellHypo * ch2, DECODER_TYPE d, 
		                                      CellHypo * &result1, CellHypo * &result2, bool canBeReversed, MemPool * tmpMem);
    bool CanBeReversed(Cell * c, Cell * sub1, Cell * sub2);
    void SetForcedTrans(int beg, int end);
    void SetBoundaryTgtWords(CellHypo * ch, bool LM2, MemPool * curMem);
    void SetBoundaryTgtWords(CellHypo * rch, CellHypo * ch1, CellHypo * ch2, bool moreLMs, MemPool * curMem);
    void CalculateReordering(CellHypo * ch1, CellHypo * ch2, CellHypo * chMono, CellHypo * chRever);
    float GetMEReorderingScore(CellHypo * ch1, CellHypo * ch2); // monotonic translation. 
                                                                // in the case of reversed translation, please use 1 - "score" intead
    void AddFeatValue(CellHypo * ch, int featDim, float featValue);
    bool CheckForPunctPruning(int beg, int end);
    void ClearCubePruningData();
    void EnQueue( Cell* c, CellHypo** tlist1, CellHypo** tlist2, int r1, int r2, bool canBeReversed, MemPool *tmpMem );
	void EnQueue2( Cell* c, CellHypo** tlist1, CellHypo** tlist2, int mid, short bundle, short r1, short r2, bool canBeReversed, MemPool *tmpMem );
    void ClearExplored();
    inline bool CheckExplored(int i, int j);
    inline void SetExplored(int i, int j);
	inline bool CheckExplored2(int bundle, int i, int j);
    inline void SetExplored2(int bundle, int i, int j);
    void AddMSDReorderingScore( Cell* c, CellHypo* lt, CellHypo* rt, CellHypo* ch );
    bool CheckHypoInRef(CellHypo * ch, RefSents * refs);
    char* GenDecodingLog( CellHypo* ch );
    void DumpSpanTransForServerDisplay(char * log, Cell * c);

    // for the CKY decoding algorithm
    void CKYDecoding(DecodingSentence * sent);

    // for "forced" decoding
    void ComposeTwoSpansWithForcedDecoding(Cell * c, Cell * sub1, Cell * sub2, MemPool * tmpMem);
    CellHypo * TryToComopseTwoHyposWithForcedDecoding(CellHypo * ch1, CellHypo * ch2, MemPool * tmpMem);
    CellHypo * ComposeTwoHypothesesForForcedDecoding(Cell * c, CellHypo * ch1, CellHypo * ch2, DECODER_TYPE d, MemPool * tmpMem);

    // for loss augumented decoding
    void LoadRefForLossAugumentedDecoding(DecodingSentence * sentence);
    void RerankingWithLossAugumentedScore(Cell * c);
    void AssignLossAugumentedScoreInTList(Cell * c);
    void ComputeLoss(CellHypo * ch, int rank);
    bool InRef(int * wid, int ngram);
};

//////////////////////////////////////////////////////////////
// Why do we need to define "heap" structure again

class NodeHeapForSearch
{
public:
    HEAPTYPE           type;
    int                size;
    int                count;
    BaseSearchNode **  items;

public:
    NodeHeapForSearch(int mySize, HEAPTYPE myType);
    ~NodeHeapForSearch();
    void Clear();
    void Update(BaseSearchNode * node);
    bool Compare(int i, int j);
    void Push(BaseSearchNode * node);
    BaseSearchNode * Pop();
    void Down(int k);
    void Up(int k);
};


}

#endif

