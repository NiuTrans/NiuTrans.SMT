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
 * basic structure of our decoder; OurChart.h
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (xiaotong@mail.neu.edu.cn); December 20th, 2010
 *
 */


#ifndef _OURCHART_H_

#include "Global.h"
#include "Model.h"
#include "OurTree.h"
#include "OurInputSentence.h"
#include "OurDecoder.h"

namespace smt {

enum SPAN_TYPE {NORMAL_SPAN = 0, SKELETON_SPAN = 1, OTHER_SPAN = 2};

class Cell;
class CellHypo;
class Phrase;
class HypoHeap;
class RefNGram;

/////////////////////////////////////////////////////
// phrase
typedef class Phrase
{
public:
    char*        str;
    int          alignBeg;
    int          alignEnd;
    MemPool*   mem;

public:
    Phrase( MemPool* m );
    void    Init( char* str, int aBeg, int aEnd );

}
*pPhrase;

/////////////////////////////////////////////////////
// translaton hypothesis for chart parsing (decoding)
typedef class CellHypo
{
public:
    Cell *             cell;
    float              modelScore;
    float *            featValues;
    float *            LMScore;
    int *              wid;
    int                wCount;
    char *             translation;
    int                transLength;
    CellHypo *         lc;            // left partial translation
    CellHypo *         rc;            // right partial translation
    CellHypo **        childHypos;    // hypotheses made up of the current one (tails of edge)
    short              childHypoNum;
    MemPool *          mem;
    char *             ltgtword;
    char *             rtgtword;
    MEReFeatInfo *     MERe;          // features used in ME-based reordering model
    int                r1, r2;        // for cube-pruning

    Phrase *           tLeftPhrase;
    Phrase *           tRightPhrase; // alignment from target side to source side for MSD re-ordering model

    // for syntax-based decoding
    char *             root;
    void *             ruleUsed;

    // for "coarse-search + fine-modeling"
    char *             rootFineModel;
    int                CSFMCandId;

    // record which pattern is used (for syntax-based system)
    void *             patternUsed;

    int *              matchedRefPOS;
    int                matchedRefPOSCount;

    // loss
    float              beamLoss;
    float              bleuLoss;

	// another LM
	float *            LMScore2;
    int *              wid2;
    int                wCount2;

public:
    CellHypo(Cell * c, MemPool * m, int featNum);
    ~CellHypo();
    void Init(Cell * c, MemPool * m, int featNum);
    void Create(const char * trans, Model * model);
    static CellHypo * Copy(CellHypo * ch, Model * model, MemPool * newMem);
    void CopyContent( CellHypo *src, Model *model, MemPool *newMem );

}* pCellHypo;

class MatchedPattern;

////////////////////////////////////////////
// cell defined for chart parsing (decoding)

class Cell
{
public:
    MemPool *          mem;
    int                beg, end;
    List *             tList;
    bool               havePunct;
    bool               haveFactoid;
    CellHypo **        nList;        // n-best result
    int                n;            // n in n-best
    HypoHeap *         chheap;
    unsigned int*      timeStamp;
    unsigned int       tolHypoUpdateCnt;
    unsigned int       tolSpanUpdateCnt;
    bool               cellExpandOver;
    bool               SR_decodingOver;
    int                ruleNum;
    bool               forcedTrans; // forced decoding for the span
    bool               valid;
	SPAN_TYPE          spanType;    // 0: normal 1: in skeleton 2: remaining compenents
	BaseDecoder *      decoder;
    

public: // for syntax-based system
    List *             matchedPatternList;
    int                matchedPatternCount;
    List *             hypoNodeList;
    HashTable *        nListWithSymbol;
    List *             treeNodes;  // corresponding (source-)tree nodes
    List *             cellNodes;  // corresponding (source-)tree nodes for decoding

public:
    static int         incompleteHypoNum;

public:
    List *             prematchedRules;  // rules that are definitly matched for a given span/tree-node
    List *             viterbiRules;     // rules used in the Viterbi derivation
    short              viterbiDerivationState; // recording state of the Viterbi derivation
                                               // 0: initial, -1: failure, 1: success

public:
    Cell();
    ~Cell();

    void Init(int b, int e, BaseDecoder * d, int maxTransNum, MemPool * myMem);
    void Clear();
    void AddCellHypo(CellHypo * ch);
    unsigned int CompleteWithBeamPruning(int beamSize, BaseDecoder * decoder, bool withSymbol, bool withFineSymbol, 
                                         bool deepCopy, bool checkRule, MemPool * newMem);
    void CompleteWithBeamPruningInHeap(int beamSize, BaseDecoder * decoder, MemPool * newMem);
    void ReassignCellToHyposInBeam(Cell * c);
    bool CheckHypoAvailability(CellHypo * ch, BaseDecoder * decoder);
    void SortHyposInBeam();

public: // for syntax-based system
    void SynInit(int b, int e, int maxTransNum, bool withSymbol, MemPool * myMem);
    void SynClear();
    void AddMatchedPattern(MatchedPattern * pattern);
};

class RefNGram // for calculating loss
{
protected:
    int * ngramBase;
    int   ngramNum;
    int   n;
    int   ngramUnitSize;

public:
    RefNGram(int maxSentLength, int ngram);
    ~RefNGram();
    void Clear();
    void SetNGram(int ngram);
    int Compare(const void * ngram1, const void * ngram2);
    void Load(char * ref, Model * model);
    void Add(int * wid, int ngram, int count);
    int  Find(int * wid, int ngram);
};

/////////////////////////////////////////////////////
// heap

enum HEAPTYPE {MAXHEAP, MINHEAP};

class HypoHeap
{
public:
    HEAPTYPE           type;
    int                size;
    int                count;
    CellHypo **        items;

public:
    HypoHeap(int mySize, HEAPTYPE myType);
    ~HypoHeap();
    void Clear();
    void Update(CellHypo * ch);
    bool Compare(int i, int j);
    void Push(CellHypo * ch);
    CellHypo * Pop();
    void Down(int k);
    void Up(int k);
};

}

#endif

