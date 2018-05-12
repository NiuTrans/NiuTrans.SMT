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
 * basic structure of our decoder; OurChart.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (xiaotong@mail.neu.edu.cn); December 20th, 2010
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "OurChart.h"

namespace smt {

/////////////////////////////////////////////////////
// phrase
Phrase::Phrase(MemPool *m)
{
    this->mem = m;
}

void Phrase::Init(char *str, int aBeg, int aEnd)
{
    this->str = ( char* )mem->Alloc( strlen( str ) + 1 );
    strcpy( this->str, str );
    this->alignBeg = aBeg;
    this->alignEnd = aEnd;

}

/////////////////////////////////////////////////////
// translaton hypothesis for chart parsing (decoding)

CellHypo::CellHypo(Cell * c, MemPool * m, int featNum)
{
    Init(c, m, featNum);
}

CellHypo::~CellHypo()
{
}

void CellHypo::Init(Cell * c, MemPool * m, int featNum)
{
    cell        = c;
    mem         = m;
    modelScore  = 0;
    featValues  = (float*)mem->Alloc(sizeof(float) * featNum);
    //memset(featValues, 0, sizeof(float) * featNum);
    for( int i=0; i < featNum; i++ ) {
        featValues[i] = 0.0f;
    }
    LMScore       = NULL;
    wid           = NULL;
    wCount        = 0;
    translation   = NULL;
    transLength   = 0;
    lc            = NULL;
    rc            = NULL;
    childHypos    = NULL;
    childHypoNum  = 0;
    MERe          = NULL;

    tLeftPhrase = ( Phrase* )mem->Alloc( sizeof(Phrase) );
    tLeftPhrase->mem = mem;
    tRightPhrase = ( Phrase* )mem->Alloc( sizeof(Phrase) );
    tRightPhrase->mem = mem;

    root          = NULL; 
    ruleUsed      = NULL;

    rootFineModel = NULL;
    CSFMCandId    = -1;

    matchedRefPOS = NULL;
    matchedRefPOSCount = 0;

	LMScore2      = NULL;
    wid2          = NULL;
    wCount2       = 0;
}

void CellHypo::Create(const char * trans, Model * model)
{
    transLength = (int)strlen(trans);
    translation = (char*)mem->Alloc(sizeof(char) * (transLength + 1));
    strcpy(translation, trans); // translation

    char * tmpTrans = (char*)mem->Alloc(sizeof(char)* (transLength + 1));
    strcpy(tmpTrans, trans);
    StringUtil::ToLowercase(tmpTrans);

    modelScore = 0;
    model->GetWId(tmpTrans, wid, wCount, mem); // word-id list
    //LMScore = (float*)mem->Malloc(sizeof(float)*wCount);
    //memset(LMScore, 0, sizeof(float) * wCount);

    featValues[model->BiLexCount] = (float)wCount;
}

CellHypo * CellHypo::Copy(CellHypo * ch, Model * model, MemPool * newMem)
{
    CellHypo * newch = (CellHypo*)newMem->Alloc(sizeof(CellHypo));
    memcpy(newch, ch, sizeof(CellHypo));

    newch->mem = newMem;

    newch->featValues = (float*)newMem->Alloc(sizeof(float) * model->featNum);
    memcpy(newch->featValues, ch->featValues, sizeof(float) * model->featNum);

    newch->translation = (char*)newMem->Alloc(sizeof(char) * (ch->transLength + 1));
    memcpy(newch->translation, ch->translation, sizeof(char) * (ch->transLength + 1));

    newch->wid = (int*)newMem->Alloc(sizeof(int) * ch->wCount);
    memcpy(newch->wid, ch->wid, sizeof(int) * ch->wCount);

    newch->LMScore = (float*)newMem->Alloc(sizeof(float) * ch->wCount);
    memcpy(newch->LMScore, ch->LMScore, sizeof(float) * ch->wCount);

    if(ch->MERe != NULL){
        newch->MERe = (MEReFeatInfo*)newMem->Alloc(sizeof(MEReFeatInfo));
        memcpy(newch->MERe, ch->MERe, sizeof(MEReFeatInfo));
    }

    if(ch->root != NULL){
        newch->root = (char*)newMem->Alloc(sizeof(char) * ((int)strlen(ch->root) + 1));
        strcpy(newch->root, ch->root);
    }

    if(ch->rootFineModel != NULL){
        newch->rootFineModel = (char*)newMem->Alloc(sizeof(char) * ((int)strlen(ch->rootFineModel) + 1));
        strcpy(newch->rootFineModel, ch->rootFineModel);
    }

    if(ch->childHypos != NULL){
        newch->childHypos = (CellHypo**)newMem->Alloc(sizeof(CellHypo*) * ch->childHypoNum);
        memcpy(newch->childHypos, ch->childHypos, sizeof(CellHypo*) * ch->childHypoNum);
    }

    if(ch->matchedRefPOS != NULL){
        newch->matchedRefPOS = (int *)newMem->Alloc(sizeof(int) * ch->matchedRefPOSCount);
        memcpy(newch->matchedRefPOS, ch->matchedRefPOS, sizeof(int) * ch->matchedRefPOSCount);
    }

	if(ch->wid2 != NULL){
		newch->wid2 = (int*)newMem->Alloc(sizeof(int) * ch->wCount2);
		memcpy(newch->wid2, ch->wid2, sizeof(int) * ch->wCount2);
	}

	if(ch->LMScore2 != NULL){
		newch->LMScore2 = (float*)newMem->Alloc(sizeof(float) * ch->wCount2);
		memcpy(newch->LMScore2, ch->LMScore2, sizeof(float) * ch->wCount2);
	}
	
    return newch;
}

void CellHypo::CopyContent( CellHypo *src, Model *model, MemPool *newMem )
{
    CellHypo *dst = this;

    memcpy(dst, src, sizeof(CellHypo));

    dst->featValues = (float*)newMem->Alloc(sizeof(float) * model->featNum);
    memcpy(dst->featValues, src->featValues, sizeof(float) * model->featNum);

    dst->translation = (char*)newMem->Alloc(sizeof(char) * (src->transLength + 1));
    memcpy(dst->translation, src->translation, sizeof(char) * (src->transLength + 1));

    dst->wid = (int*)newMem->Alloc(sizeof(int) * src->wCount);
    memcpy(dst->wid, src->wid, sizeof(int) * src->wCount);

    dst->LMScore = (float*)newMem->Alloc(sizeof(float) * src->wCount);
    memcpy(dst->LMScore, src->LMScore, sizeof(float) * src->wCount);

    if(src->MERe != NULL){
        dst->MERe = (MEReFeatInfo*)newMem->Alloc(sizeof(MEReFeatInfo));
        memcpy(dst->MERe, src->MERe, sizeof(MEReFeatInfo));
    }

    if(src->root != NULL){
        dst->root = (char*)newMem->Alloc(sizeof(char) * ((int)strlen(src->root) + 1));
        strcpy(dst->root, src->root);
    }

    if(src->rootFineModel != NULL){
        dst->rootFineModel = (char*)newMem->Alloc(sizeof(char) * ((int)strlen(src->rootFineModel) + 1));
        strcpy(dst->rootFineModel, src->rootFineModel);
    }

    if(src->childHypos != NULL){
        dst->childHypos = (CellHypo**)newMem->Alloc(sizeof(CellHypo*) * src->childHypoNum);
        memcpy(dst->childHypos, src->childHypos, sizeof(CellHypo*) * src->childHypoNum);
    }

    if(src->matchedRefPOS != NULL){
        dst->matchedRefPOS = (int *)newMem->Alloc(sizeof(int) * src->matchedRefPOSCount);
        memcpy(dst->matchedRefPOS, src->matchedRefPOS, sizeof(int) * src->matchedRefPOSCount);
    }

	if(src->wid2 != NULL){
		dst->wid2 = (int*)newMem->Alloc(sizeof(int) * src->wCount2);
		memcpy(dst->wid2, src->wid2, sizeof(int) * src->wCount2);
	}

	if(src->LMScore2 != NULL){
		dst->LMScore2 = (float*)newMem->Alloc(sizeof(float) * src->wCount2);
		memcpy(dst->LMScore2, src->LMScore2, sizeof(float) * src->wCount2);
	}
}

////////////////////////////////////////////
// cell defined for chart parsing (decoding)

int Cell::incompleteHypoNum = 100000;

Cell::Cell()
{
    beg         = 0;
    end         = 0;
    tList       = NULL;
    havePunct   = true;
    haveFactoid = true;
    nList       = NULL;
    n           = 0;
    chheap      = NULL;
    timeStamp   = NULL;
    forcedTrans = false;
    valid       = true;
	spanType    = NORMAL_SPAN;
	decoder     = NULL;

    matchedPatternList  = NULL;
    matchedPatternCount = 0;
    hypoNodeList        = NULL;
    nListWithSymbol     = NULL;
    treeNodes           = NULL;
    cellNodes           = NULL;

    prematchedRules     = NULL;
    viterbiRules        = NULL;
    viterbiDerivationState = 0;
}

Cell::~Cell()
{
    delete tList;
    delete chheap;
    delete[] timeStamp;
    delete matchedPatternList;
    delete hypoNodeList;
    delete nListWithSymbol;
    delete treeNodes;
    delete cellNodes;
    delete prematchedRules;
    delete viterbiRules;
}

void Cell::Init(int b, int e, BaseDecoder * d, int maxTransNum, MemPool * myMem)
{
    mem   = myMem;
    beg   = b;
    end   = e;
    tList = new List(maxTransNum);
    nList = NULL;
    n     = 0;
    chheap= new HypoHeap(maxTransNum, MINHEAP);
    timeStamp = new unsigned int[MAX_WORD_NUM];
    tolHypoUpdateCnt = 0;
    tolSpanUpdateCnt = 0;
    cellExpandOver = false;
    SR_decodingOver = false;
    ruleNum = 0;
	decoder = d;
}

void Cell::Clear()
{
    tList->Clear();
    n = 0;
    chheap->Clear();
    memset( timeStamp, 0, sizeof( unsigned int ) * MAX_WORD_NUM );
    tolHypoUpdateCnt = 0;
    tolSpanUpdateCnt = 0;
    cellExpandOver = false;
    SR_decodingOver = false;
    ruleNum = 0;
    viterbiDerivationState = 0;
}

void Cell::AddCellHypo(CellHypo * ch)
{
#ifdef USE_TRANS_HEAP
    if( ch != NULL )
        chheap->Update(ch);
#else
    if( ch != NULL )
        tList->Add((void*)ch);
#endif
}

void QuickSortWithModeScore(void ** list, int left, int right)
{
      int i = left, j = right;
      void * tmp;
      CellHypo * mid = (CellHypo*)list[(left + right) / 2];
      float midScore = mid->modelScore;

      /* partition */
      while (i <= j) {
          while(((CellHypo*)list[i])->modelScore > midScore && i <= j)
              i++;
          while(((CellHypo*)list[j])->modelScore < midScore && i <= j)
              j--;
          
          if (i <= j) {
              tmp = list[i];
              list[i] = list[j];
              list[j] = tmp;
              i++;
              j--;
          }
      };

      /* recursion */
      if (left < j)
          QuickSortWithModeScore(list, left, j);
      if (i < right)
          QuickSortWithModeScore(list, i, right);
}

void SortWithModeScore(List * list)
{
    int len = list->count;
    for( int i = 0; i < len - 1; i++ ){
        for( int j = len - 1; j > i; j-- ){
            CellHypo * t1 = (CellHypo*)list->items[j - 1];
            CellHypo * t2 = (CellHypo*)list->items[j];
            if( t1->modelScore < t2->modelScore ){
                void * tmp = list->items[j - 1];
                list->items[j - 1] = list->items[j];
                list->items[j] = tmp;
            }
        }
    }
}

int nx = 0;
//int hc[4] = {0, 0, 0, 0}; // total recombination number
//int cc[4] = {0, 0, 0, 0}; // unit->composed recombination number
//int bc[4] = {0, 0, 0, 0}; // unit->composed phrase number
//int ac[4] = {0, 0, 0, 0}; // composed phrase number
//int dc[4] = {0, 0, 0, 0}; // unit beat composed phrase number
unsigned int Cell::CompleteWithBeamPruning(int beamSize, BaseDecoder * decoder, bool withSymbol, bool withFineSymbol, 
                                           bool deepCopy, bool checkRule, MemPool * newMem)
{
	Model * model = decoder->model;

#ifdef USE_TRANS_HEAP
    CompleteWithBeamPruningInHeap(beamSize);
#else
    unsigned int updateCnt = 0;
    ++tolSpanUpdateCnt;
    // sort by model score
    if( tList->count > 0 ){
        QuickSortWithModeScore(tList->items, 0, tList->count - 1);
        //SortWithModeScore(tList);
    }

    int ntmp = tList->count;
    if( beamSize > 0 && tList->count > beamSize )
        ntmp = beamSize;

    if( ntmp > n )
    {
        nList = (CellHypo**)newMem->Alloc(sizeof(CellHypo*) * ntmp);
        memset( nList, 0, sizeof( CellHypo* ) * ntmp );
    }

    n = ntmp;

    int incompleteHypoNum = 0;
    int maxIncompleteHypoNum = (int)(tList->count > n ? n * model->incompleteHypoRate : n); // max number of hypothese rooting at virtual NTs
    int maxSymbolNum = decoder->maxNumOfSymbolWithSameTrans;
    int maxFineSymbolNum = decoder->maxNumOfFineSymbolWithSameTrans;

    bool ok = false;

    CellHypo** ntmpList = new CellHypo*[n];
    int c = 0;
    for( int i = 0; i < tList->count; i++ ){
        CellHypo * cch = (CellHypo*)tList->items[i];
        int symbolTransCount = 0;
        int fineSymbolTransCount = 0;

        if(checkRule && !CheckHypoAvailability(cch, decoder))
            continue; // throw away hypothese with the rules will not be used in the further decoder steps

        bool duplicated = false;
        for( int j = 0; j < c; j++ ){
            if( strcmp(cch->translation, ntmpList[j]->translation) == 0 ){

                if(!withSymbol || symbolTransCount++ >= maxSymbolNum){
                    duplicated = true;
                    break;
                }
                
                if(!strcmp(cch->root, ntmpList[j]->root)){

                    if(!withFineSymbol || fineSymbolTransCount++ >= maxFineSymbolNum || 
                        !strcmp(cch->rootFineModel, ntmpList[j]->rootFineModel) ){
                        duplicated =true;
                        break;
                    }
                }
                
                //if(!withSymbol || !strcmp(cch->root, ntmpList[j]->root)){ // modified by xiaotong, 08/07/2011
    //                duplicated = true;
    //                break;
    //            }
            }
        }

        if( !duplicated ){
            if(withSymbol && !SynchronousGrammar::IsComplete(cch->root)){
                if(incompleteHypoNum++ > maxIncompleteHypoNum)
                    continue;
            }

            //nList[c++] = CellHypo::Copy(cch, model, newMem);
            ntmpList[c++] = cch;
        }
        if( c >= n )
            break;
    }
    for( int i=c-1; i >= 0; i-- ) {
        CellHypo* cch = ntmpList[i];
        if( nList[i] == NULL )
        {
            nList[i] = CellHypo::Copy( cch, model, newMem );
            ++updateCnt;
            ++tolHypoUpdateCnt;
        }
        else if( nList[i] == cch )
        {
            nList[i] = cch;
        }
        else
        {
            if(!deepCopy)
                nList[i]->CopyContent( cch, model, newMem );
            else
                nList[i] = CellHypo::Copy( cch, model, newMem );
            ++updateCnt;
            ++tolHypoUpdateCnt;
        }
    }
    delete[] ntmpList;

    if( n > c )
        n = c;

    for( int i = 0; i < n; i++ )
        tList->items[i] = (void*)nList[i];
    tList->count = n;

    return updateCnt;
#endif
}

void Cell::CompleteWithBeamPruningInHeap(int beamSize, BaseDecoder * decoder, MemPool * newMem)
{
	Model * model = decoder->model;
    CellHypo * ch = NULL;
    int count = chheap->count, c = 0;

    if( beamSize < 0 || count < beamSize )
        beamSize = count;

    nList = (CellHypo**)newMem->Alloc(sizeof(CellHypo*) * beamSize);

    while(chheap->count > 0){
        ch = chheap->Pop();
        if(count - c <= beamSize)
            nList[beamSize - c - 1] = CellHypo::Copy(ch, model, newMem);
        c++;
    }
    n = beamSize;

    for( int i = 0; i < n; i++ )
        tList->items[i] = (void*)nList[i];
    tList->count = n;
}

void Cell::ReassignCellToHyposInBeam(Cell * c)
{
    for(int i = 0; i < n; i++){
        nList[i]->cell = c;
    }
}

bool Cell::CheckHypoAvailability(CellHypo * ch, BaseDecoder * decoder)
{
    if(ch == NULL)
        return false;

    if(ch->ruleUsed == NULL)
        return true;
    else{
        UnitRule * urule = (UnitRule *)ch->ruleUsed;
        char * src = urule->parentRule->src;
        return decoder->CheckAvialability(src);
    }
}

int CompareHypoModelScore(const void * ch1, const void * ch2)
{
    float score1 = (*((CellHypo **)ch1))->modelScore;
    float score2 = (*((CellHypo **)ch2))->modelScore;

    if(score1 > score2)
        return -1;
    else if(score1 < score2)
        return 1;
    else
        return 0;
}

// sort hypotheses by model score
void Cell::SortHyposInBeam()
{
    qsort(nList, n, sizeof(CellHypo*), CompareHypoModelScore);
}

/////////////////////////////////////////////////////
// count ngrams for a given sentence

RefNGram::RefNGram(int maxSentLength, int ngram)
{
    ngramBase     = new int[maxSentLength * (ngram + 1)];
    ngramNum      = 0;
    n             = ngram;
    ngramUnitSize = ngram + 2;

}

RefNGram::~RefNGram()
{
    Clear();
    delete[] ngramBase;
}

void RefNGram::Clear()
{
    ngramNum = 0;
}

void RefNGram::SetNGram(int ngram)
{
    n = ngram;
    ngramUnitSize = ngram + 2;
}

int CompareNGram(const void * ngram1, const void * ngram2)
{
    int n1 = *((int*)ngram1);
    int n2 = *((int*)ngram2);

    if(n1 == n2){
        return memcmp((int*)ngram1 + 2, (int*)ngram2 + 2, sizeof(int) * n1);
    }
    else{
        fprintf(stderr, "ERROR! n1 != n2 in ngram comparison!\n");
        return 0;
    }
}

void RefNGram::Load(char * ref, Model * model)
{
    char  newRef[MAX_SENT_LENGTH];
    int * wid;
    int   wCount;
    int   i, j;

    sprintf(newRef, "<s> %s </s>", ref);
    model->GetWId(newRef, wid, wCount, NULL); // string -> word id sequence

    for( int i = 0; i < wCount; i++){
        for( int j = 0; j < n && i + j < wCount; j++){
            Add(wid + i, j + 1, 1);
        }
    }

    // sort those ngrams
    qsort(ngramBase, ngramNum, sizeof(int) * ngramUnitSize, CompareNGram); 

    // count those ngrams
    int trueNGramNum = 0;
    for(i = 0; i < ngramNum; i++){
        int * curNGram = ngramBase + i * ngramUnitSize;
        int count = curNGram[1];
        for(j = i + 1; j < ngramNum; j++){
            int * followingNGram = ngramBase + j * ngramUnitSize;
            //fprintf(stderr, "%d %d %d %d\n", followingNGram[2], followingNGram[3], followingNGram[4], followingNGram[5]);
            if(!CompareNGram(curNGram, followingNGram)){ // i-th ngram == j-th ngram ?
                count += followingNGram[1];
            }
            else
                break;
        }

        curNGram[1] = count;
        memcpy(ngramBase + trueNGramNum * ngramUnitSize, curNGram, sizeof(int) * ngramUnitSize);
        trueNGramNum++;
        i = j - 1;
    }

    ngramNum = trueNGramNum;

}

void RefNGram::Add(int * wid, int ngram, int count)
{
    int w[MAX_NGRAM];

    memset(w, -1, sizeof(int) * ngramUnitSize);
    w[0] = n;     // ngram order
    w[1] = count; // frequency
    memcpy(w + 2, wid, sizeof(int) * ngram);
    
    // copy the input ngram
    memcpy(ngramBase + ngramNum * ngramUnitSize, w, sizeof(int) * ngramUnitSize); 
    ngramNum++;
}

int RefNGram::Find(int * wid, int ngram) // return the frequency for an input ngram
{
    int * result = NULL;
    int w[MAX_NGRAM];

     memset(w, -1, sizeof(int) * ngramUnitSize);
     w[0] = n;     // ngram order
     w[1] = 0;
     memcpy(w + 2, wid, sizeof(int) * ngram);
     result = (int *)bsearch(w, ngramBase, ngramNum, sizeof(int) * ngramUnitSize, CompareNGram);


     if(result != NULL && result[1] <= 0){
         fprintf(stderr, "ERROR! invalid ngram!\n");
     }

     if(result != NULL)
         return result[1];
     else
         return -1;
}

/////////////////////////////////////////////////////
// heap

HypoHeap::HypoHeap(int mySize, HEAPTYPE myType)
{
    type = myType;
    size = mySize;
    items = new CellHypo*[mySize];
    count = 0;
    memset(items, 0, sizeof(CellHypo*) * mySize);
}

HypoHeap::~HypoHeap()
{
    delete[] items;
}

void HypoHeap::Clear()
{
    count = 0;
    memset(items, 0, sizeof(CellHypo*) * size);
}

void HypoHeap::Update(CellHypo * ch)
{
    if (type == MAXHEAP){
        fprintf(stderr, "ERROR: udpate operation is unavailable for max-heap\n");
        return;
    }

    if( count == size && items[0]->modelScore >= ch->modelScore)
        return;

    // find and update the translation
    for( int i = 0; i < count; i++ ){
        if(strcmp(items[i]->translation, ch->translation))
            continue;
        if(items[i]->modelScore < ch->modelScore){
            items[i] = ch;
            Down(i);
        }
        return;
    }

    if( count < size ){
        Push(ch);
        return;
    }

    items[0] = ch;
    Down(0);
}

bool HypoHeap::Compare(int i, int j)
{
    if (type == MINHEAP)
        return items[i]->modelScore < items[j]->modelScore;
    else
        return items[j]->modelScore < items[i]->modelScore;
}

void HypoHeap::Push(CellHypo * ch)
{
    if (count >= size){
        fprintf(stderr, "ERROR: Heap is full!\n");
        exit(-1);
    }

    items[count] = ch;
    Up(count);
    count++;
}


CellHypo * HypoHeap::Pop()
{
    if (size == 0){
        fprintf(stderr, "ERROR: empty heap!\n");
        exit(-1);
    }

    CellHypo * ch = items[0];
    items[0] = items[count - 1];
    count--;
    items[count] = NULL;
    Down(0);
    return ch;
}

void HypoHeap::Down(int k)
{
    int i = k;

    while (2 * i + 1 < count)
    {
        int l = 2 * i + 1, r = 2 * i + 2;
        int m = (r >= count || Compare(l, r)) ? l : r;
        if (Compare(i, m))
            break;
        CellHypo * tmp = items[i];
        items[i] = items[m];
        items[m] = tmp;
        i = m;
    }
}
void HypoHeap::Up(int k)
{
    int i = k;
    int parent = (i - 1) / 2;
    while (i > 0 && !Compare(parent, i))
    {
        CellHypo * tmp = items[i];
        items[i] = items[parent];
        items[parent] = tmp;
        i = parent;
        parent = (i - 1) / 2;
    }
}

}

