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
 * Model parameters trainer; OurTrainer.h
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


#ifndef _OURTRAINER_H_
#define _OURTRAINER_H_

#include <string.h>
#include "Global.h"
#include "Utilities.h"
#include "OurSharedStructure.h"
using namespace utilities;


namespace smt {

// types of BLEU
// 0: NIST version BLEU (using the shortest reference length) - default
// 1: IBM version BLEU (using the closest reference length)
// 2: BLEU-SBP (Chiang et al., EMNLP2008)
// 3: f1 measure
enum BLEU_TYPE {NIST_BLEU, IBM_BLEU, BLEU_SBP, F1};
extern char BLEU_TYPE_NAME[4][20];

// evaluation metrics (e.g. BLEU used in defining error function)
class Evaluator
{
public:
    static float CalculateBP(float matchedNGram[], float statNGram[], float bestRefLen, 
                             float shortestRefLen, float SBPRefLen, int ngram, BLEU_TYPE type = NIST_BLEU);
    static float CalculateBLEU(float matchedNGram[], float statNGram[], float bestRefLen, 
                               float shortestRefLen, float SBPRefLen, int ngram, BLEU_TYPE type = NIST_BLEU);
    static float CalculateBLEUSmoothed(float matchedNGram[], float statNGram[], float bestRefLen,
                                       float shortestRefLen, float SBPRefLen, int ngram, BLEU_TYPE type = NIST_BLEU);
};

////////////////////////////////////////
// Basic structure used in training

class Sentence
{
public:
    char *          str;
    char **         words;
    int             wordCount;

public:
    Sentence();
    Sentence(char * line);
    ~Sentence();
    void Init();

public:
    void Create(char * line);    // create a candidate from a char string
};

typedef struct FeatureNode{
    int             id;
    float          value;
}* pFeatureNode;

// word alignment
class WordAlignment
{
public:
    List **          srcAlign;     // alignments from source words
    List **          tgtAlign;     // alignments from target words
    int              srcWordCount; // number of source words
    int              tgtWordCount; // number of target words

public:
    WordAlignment();
    ~WordAlignment();
    int Create(int srcLength, int tgtLength, char * alignment);
};

class WordTranslation
{
public:
    List *           translations;

public:
    WordTranslation() ;
    ~WordTranslation();
    void Create(int srcLength);
    bool Add(int srcWId, char * translation);
    WordTranslation * Copy();
};

// translation candidate
class TransCand
{
public:
    Sentence *      source;      // source sentence
    Sentence *      translation; // target sentence
    FeatureNode *   feats;       // accosiated features
    int             featCount;   // number of features
    float *         nGramMatch;
    WordTranslation * wordTrans;

    // model-score(trans-cand) = slope * w + b
    float           slope;
    float           b;

    float           bestRefLen;     // effective length for IBM-BLEU
    float           shortestRefLen; // effective length for NIST-BLEU
    float           SBPRefLen;         // effective length for BLEU-SBP

    float           precision;
    float           recall;
    float           f1;
    float           correct;
    float           predicted;
    float           truth;
    float           anchorLoss;
    float           anchorCount;

public:
    TransCand();
    TransCand(char * line);
    TransCand(char * trans, float * featValues, int featNum);
    ~TransCand();
    void Init();
    void InitWithNULL(int featNum);

public:
    void Clear();
    void Create(char * line);    // create a candidate from a char string
    void Create(char * trans, float * featValues, int featNum);    // create a candidate from a pair of (translation, features)
    void CreateF1(char * line);  // create a candidate from a feature vector
    void Deliver(TransCand * target);
    int GetFeatIndex(int id);
};

// reference translation
class RefSents
{    
public:
    Sentence *       refs;       // reference translations
	WordAlignment *  aligns;     // (words) alignment for each reference
    int              refCount;   // number of reference translations
    HashTable *      ngramMatch; // ngram hash
    int              shortestRefLen;  // shortest reference length

public:
    RefSents();
    RefSents(char ** refSent, int refCount, int ngram);
    ~RefSents();
    void Init();
	void CreateAlignment(char ** alignment, int srcLength, int refCount);
    
public:
    float * GetNgramMatchStat(Sentence * sent, int ngram);
};

////////////////////////////////////////
// Trainer

// training sample
class TrainingSample
{
public:
    Sentence *       srcSent;     // source sentence
    TransCand *      Cands;       // list of candidates
    int              CandCount;
    RefSents *       ref;

public:
    TrainingSample();
    ~TrainingSample();
    void Init();

public:
    void Deliver(TrainingSample * target);
    void Clear();
    void ClearCands();
    void CaculateAnchorLoss(TransCand * cand);
};

// training set
class TrainingSet
{
public:
    TrainingSample * samples;
    int              sampleCount;
    int              ngram;

public:
    TrainingSet();
    TrainingSet(const char * refFileName, int refNum, int ngram, int maxSentNum);
    ~TrainingSet();
    void Init();

public:
    void Clear();
    void LoadRefData(const char * refFileName, int refNum, int ngram, int maxSentNum);
    void LoadTrainingData(const char * transFileName, bool accumlative);
    void LoadTrainingData(TransResult * rlist, int rCount, int featNum, 
                            int sampleId, bool accumlative);
	void LoadWordAlignment(const char * alignFileName, int refNum, int maxSentNum);
    TrainingSample * GetSample(int i);
    RefSents * GetRef(int i);
};

class ParaInfo
{
public:
    float            weight;
    float            minValue;
    float            maxValue;
    bool             isFixed;

public:
    ParaInfo();
};

class ParaBase
{
public:
    ParaInfo **     paraList;
    int *           paraLen;
    float *         bleuList;
    /*ParaInfo *      paraList[10];
    int             paraLen[10];
    float           bleuList[10];*/
    int             paraListLen;

public:
    ParaBase(int maxNum);
    ~ParaBase();
    void Add(ParaInfo * para, int paraCount, float bleu); // add a para-vector
    static char * ToStr(ParaInfo * para, int paraCount);

};

enum METHOD{MERT,PERCEPTRON, MIRA, RANKPAIRWISE};
extern char METHOD_NAME[4][20];

// intersection (for MERT)
class Intersection
{
public:
    float           coord;
    float *         deltaMatch;
    float *         deltaNGram;
    float           deltaBestRefLen;
    float           deltaShortestRefLen;
    float           deltaSBPRefLen;
    float           deltaPrecision;
    float           deltaRecall;
    float           deltaF1;
    float           deltaCorrect;
    float           deltaPredicted;
    float           deltaTruth;
    float           deltaAnchorLoss;
    float           deltaAnchorCount;

public:
    Intersection();
    ~Intersection();
    void Create(int ngram);
};

// trainer
class OurTrainer
{
public:
    ParaInfo *       para;
    ParaInfo *       finalPara;
    int              paraCount;
    int              round;
    bool             isPAveraged; // for perceptron-based training
    float            pScale;      // update scale

public:
    OurTrainer();
    ~OurTrainer();
    void Init();

public:
    void Clear();
    void CreatePara(int weightNum);
    void LoadPara(const char * configFileName);
    void LoadParaFromModel(ParaInfo * p, int pCount);
    void SetParaInfo(char * paraLine);
    void OptimizeWeights(TrainingSet * ts, ParaInfo * para, int paraCount, 
                         int ngram, BLEU_TYPE BLEUType, float anchorAlpha, METHOD method);
    void OptimzieWeightsWithMERT(TrainingSet * ts, int ngram, BLEU_TYPE BLEUType, float anchorAlpha);
    float GetScore(TrainingSet * ts, int ngram, BLEU_TYPE BLEUType, float anchorAlpha);
    float GetBP(TrainingSet * ts, int ngram, BLEU_TYPE BLEUType, float anchorAlpha);

protected:
    float GetModelScore(ParaInfo * para, int paraCount, TransCand * cand);
    TransCand * GetTop1(ParaInfo * para, int paraCount, TransCand * cands, int candCount);
    float GetBLEU(TransCand ** transList, int transCount, TrainingSet * ts, 
                  int ngram, BLEU_TYPE type = NIST_BLEU, bool BPOnly = false);
    float GetF1(TransCand ** transList, int transCount, TrainingSet * ts);
    float GetAnchorLoss(TransCand ** transList, int transCount, TrainingSet * ts);
    float GetScore(TransCand ** transList, int transCount, TrainingSet * ts, 
                  int ngram, BLEU_TYPE type = NIST_BLEU, float anchorAlpha = 0);
    float GetBP(TransCand ** transList, int transCount, TrainingSet * ts, 
                  int ngram, BLEU_TYPE type = NIST_BLEU, float anchorAlpha = 0);
    void DumpPara(const char * fileName, ParaInfo * para, int paraCount, char * msg);
    //int SortIntersectionWithW( const void * arg1, const void * arg2 );

// minimum error rate training
public:
    int ComputeIntersections(TrainingSample * sample, int dim, Intersection * intersections, 
                             int &num, int ngram);
    int MERTWithLineSearch(TrainingSet * ts, ParaInfo * para, ParaInfo * finalPara, int paraCount, 
                           int nRoundConverged, int ngram, BLEU_TYPE BLEUType, float anchorAlpha);

// perceptron
public:
    int PeceptronBasedTraining(TrainingSet * ts, ParaInfo * para, ParaInfo * finalPara, int paraCount, 
                           int nRoundConverged, int ngram, BLEU_TYPE BLEUType);
};

}

#endif

