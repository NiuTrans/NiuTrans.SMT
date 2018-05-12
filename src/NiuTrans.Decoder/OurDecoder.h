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
 * the base class of our decoder; OurDecoder.h
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


#ifndef _OURDECODER_H_
#define _OURDECODER_H_

#include "Global.h"
#include "Config.h"
#include "Model.h"
#include "OurTree.h"
#include "OurInputSentence.h"
#include "OurSharedStructure.h"

namespace smt {

typedef struct UserTranslation
{
    int     beg, end;
    char *  type;
    char *  translation;
}* pUserTranslation;

typedef struct WordInfo
{
    bool    isIiteral;
}* pWordInfo;


////////////////////////////////////////
// basic data structure for our decoders 

class BaseDecoder
{
public:
	MemPool *          mem;
    UserTranslation *  userTrans;
    int                userTransCount;
    DecodingSentence * decodingSent;
    char *             srcSent;
    char **            srcWords;
    int                srcLength;
    int *              srcWordLength;
    WordInfo *         srcWordInfo;
    Tree *             srcTree;  // soruce tree (or forest)
    bool               useForest;
    bool               decodingFailure;
	bool               skeleton; // whether it is running in skeleton translation mode
	int                decoderId;
	int                ngram;    // order of n-gram LM
    int                ngram2;   // order of the 2nd LM

    /* for forced decoding (1-ref) */
    char *             tgtSent;
    char **            tgtWords;
    int                tgtLength;
    int *              tgtWordLength;
    int *              tgtWid;

	/* Maximum Entropy-based reordering */
public:
    char **            MEReFeats;  // features needed in ME-based reordering
    MEReFeatInfo *     MEReFeatsForSrc;

	/* parameters for beam pruning */
public:
    int                maxNumOfSymbolWithSameTrans;
    int                maxNumOfFineSymbolWithSameTrans;
    int                maxNumOfHPhraseHypo;

public:
    Model *            model;
	Model *            skeletonModel;
	Model *            assistantModel;
	Configer *         configer;
	int *              realWordOffsetForSkeleton; // for skeleton-based translation (See "OurDecoder_Skeleton.h/cpp")
	                                              // Note: "skeletonWordOffset" is maintained in "OurDecoder_Skeleton.h/cpp" instead of "OurDecoder.h/cpp"

public:
    BaseDecoder();
    virtual ~BaseDecoder();
    
	virtual void Init(Model * m){model = m; configer = m->configer;};
    virtual void DecodeInput(DecodingSentence * sentence){};                                      // decoding
    virtual int DumpTransResult(int nbest, TransResult * result, DecodingLoger * log){return 0;}; // output decoding result
    virtual void DumpLog(DecodingLoger * log){};

    void CreatBase(Model * m);
    void ClearSentInfo();
    bool LoadInputSentence(char * input);
    bool IsByline(char * type);
    bool IsFactoid(char * type);
    bool IsNE(char * type);
    bool CheckAvialability(char * input);
	char * GetSrcString(int begIndex, int endIndex, MemPool * myMem);
    bool DoesDecodingFail();

    // for forced decoding
    void LoadRef(DecodingSentence * sentence);
    void ClearRefInfo();
    bool FindTransInRef(int * wid, int wCount, int &matchedCount, int * &matchedHeads, MemPool * mem);

};

}

#endif



