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
 * Model parameters trainer; OurTrainer.cpp
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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include "OurTrainer.h"

#ifndef    WIN32
#include <sys/time.h>
#endif

namespace smt {

#define MAX_CAND_NUM         1000000
#define MAX_SAMPLE_NUM       MAX_SENT_NUM
#define MAX_REF_NUM          100

#ifndef FLT_MIN
#define FLT_MIN              -3.4e38F
#endif

////////////////////////////////////////
// evaluation metrics
//
float Evaluator::CalculateBP(float matchedNGram[], float statNGram[], float bestRefLen, 
                               float shortestRefLen, float SBPRefLen, int ngram, BLEU_TYPE type)
{
    float BP    = 0;

    if( type == NIST_BLEU )
        BP = 1 - shortestRefLen / statNGram[0];
    else if( type == IBM_BLEU )
        BP = 1 - bestRefLen / statNGram[0];
    else if( type == BLEU_SBP )
        BP = 1 - shortestRefLen / SBPRefLen;
    if (BP > 0)
        BP = 0;
    return exp(BP);
}

float Evaluator::CalculateBLEU(float matchedNGram[], float statNGram[], float bestRefLen, 
                               float shortestRefLen, float SBPRefLen, int ngram, BLEU_TYPE type)
{
    float BP    = 0;
    float score = 0;

    if( type == NIST_BLEU )
        BP = 1 - shortestRefLen / statNGram[0];
    else if( type == IBM_BLEU )
        BP = 1 - bestRefLen / statNGram[0];
    else if( type == BLEU_SBP )
        BP = 1 - shortestRefLen / SBPRefLen;
    if (BP > 0)
        BP = 0;

    for ( int i = 0; i < ngram; i++ ){
        if (statNGram[i] == 0)
            continue;
        if (matchedNGram[i] != 0)
            score += log(matchedNGram[i] / statNGram[i]);
        else
            score += log(0.5 / statNGram[i]);
    }

    return exp(score / ngram + BP);
}

float Evaluator::CalculateBLEUSmoothed(float matchedNGram[], float statNGram[], float bestRefLen, 
                                       float shortestRefLen, float SBPRefLen, int ngram, BLEU_TYPE type)
{
    int    n;
    float bleu = 0;
    float factor = 2;
    
    for( n = 1; n <= ngram; n++ )
        factor *= 2;
    for( n = 1; n <= ngram; n++ ){
        factor /= 2;
        bleu += CalculateBLEU(matchedNGram, statNGram, bestRefLen, shortestRefLen, SBPRefLen, ngram, type);
    }

    return bleu;
}


////////////////////////////////////////
// Basic structure used in training

// sentence

Sentence::Sentence()
{
    Init();
}

Sentence::Sentence(char * line)
{
    Init();
    Create(line);
}

Sentence::~Sentence()
{
    delete[] str;
    for( int i = 0; i < wordCount; i++ )
        delete[] words[i];
    delete[] words;
}

void Sentence::Init()
{
    str         = NULL;
    words       = NULL;
    wordCount   = 0;
}

void Sentence::Create(char * line)
{
    int len = (int)strlen(line);
    str = new char[len + 1];
    strcpy(str, line);
    //Util::TrimRight(str);
    StringUtil::TrimRight( str );

    //wordCount = Util::SplitWithSpace(str, words);
    wordCount = (int)StringUtil::Split( str, " ", words );
}

TransCand::TransCand()
{
    Init();
}

TransCand::TransCand(char * line)
{
    Init();
    if(GlobalVar::useF1Criterion)
        CreateF1(line);
    else
        Create(line);
}

// word alignment
WordAlignment::WordAlignment()
{
    srcAlign = NULL;
    tgtAlign = NULL;
    srcWordCount = 0;
    tgtWordCount = 0;
}

WordAlignment::~WordAlignment()
{
    for(int i = 0; i < srcWordCount; i++){
        if(srcAlign[i] != NULL)
            delete srcAlign[i];
    }
    delete[] srcAlign;

    for(int i = 0; i < tgtWordCount; i++){
        if(tgtAlign[i] != NULL)
            delete tgtAlign[i];
    }
    delete[] tgtAlign;
}

int WordAlignment::Create(int srcLength, int tgtLength, char * alignment)
{
    int p = 0, q = 0;
    int src, tgt;
    int acount = 0;

    srcWordCount = srcLength;
    srcAlign = new List*[srcWordCount];
    for(int i = 0; i < srcWordCount; i++){
        srcAlign[i] = new List();
    }
    tgtWordCount = tgtLength;
    tgtAlign = new List*[tgtWordCount];
    for(int i = 0; i < tgtWordCount; i++){
        tgtAlign[i] = new List();
    }

    while(alignment[p] != '\0' && alignment[p] != '\r' && alignment[p] != '\n'){
        while(alignment[p] == ' ' || alignment[p] == '\t')
            p++;

        q = p;
        
        while(alignment[q] != ' ' && alignment[q] != '\0')
            q++;

        if(sscanf(alignment + p, "%d-%d", &src, &tgt) == 2){
            if(src >= 0 && src < srcWordCount && tgt >= 0 && tgt < tgtWordCount){
                srcAlign[src]->Add((void*)(long)tgt);
                tgtAlign[tgt]->Add((void*)(long)src);
                acount++;
            }
        }

        p = q + 1;
    }

    return acount;
}

// word translation
WordTranslation::WordTranslation() 
{
    translations = NULL;
}

WordTranslation::~WordTranslation()
{
    for(int i = 0; i < translations->count; i++){
        char * trans = (char *)translations->GetItem(i);
        delete[] trans;
    }
    delete translations;
}

void WordTranslation::Create(int srcLength)
{
    translations = new List(srcLength);
	for(int i = 0; i < srcLength; i++)
		translations->Add(NULL);
}

bool WordTranslation::Add(int srcWId, char * translation)
{
    char * trans = (char *)translations->GetItem(srcWId);
    if(trans != NULL)
        delete[] trans;
    trans = new char[strlen(translation) + 1];
    strcpy(trans, translation);
	translations->SetItem(srcWId, trans);
	return true;
}

WordTranslation * WordTranslation::Copy()
{
    WordTranslation * newTrans = new WordTranslation(); // NOTE: it is not niuTrans
                                                        //       but the name of NiuTrans is inspired by something like "new":)
    newTrans->Create(translations->count);
    for(int i = 0; i < translations->count; i++){
        char * ctransOld = (char *)translations->GetItem(i);
        if(ctransOld == NULL)
            newTrans->translations->SetItem(i, NULL);
        else{
            char * ctransNew = new char[strlen(ctransOld) + 1];
            strcpy(ctransNew, ctransOld);
		    newTrans->translations->SetItem(i, (void*)ctransNew);
        }
    }
    return newTrans;
}


// translation candidate

TransCand::TransCand(char * trans, float * featValues, int featNum)
{
    Init();
    Create(trans, featValues, featNum);
}

TransCand::~TransCand()
{
    delete source;
    delete translation;
    delete[] feats;
    delete[] nGramMatch;
    delete wordTrans;
}

void TransCand::Init()
{
    source         = NULL;
    translation    = NULL;
    feats          = NULL;
    featCount      = 0;
    nGramMatch     = NULL;
    wordTrans      = NULL;
    slope          = 0;
    b              = 0;
    bestRefLen     = MAX_SENT_LENGTH;
    shortestRefLen = MAX_SENT_LENGTH;
    SBPRefLen      = MAX_SENT_LENGTH;
    precision      = 0;
    recall         = 0;
    f1             = 0;
    correct        = 0;
    predicted      = 0;
    truth          = 0;
    anchorLoss     = 0;
    anchorCount    = 0;
}

void TransCand::InitWithNULL(int featNum)
{
    Init();
    translation = new Sentence(GlobalVar::nullString);

    feats = new FeatureNode[featNum + 1];
    for( int i = 0; i < featNum; i++){
        feats[i + 1].id = i + 1;
        feats[i + 1].value = 0;
    }
    featCount = featNum + 1;
}

// format of line:
// source ||| feat1 feat2 feat3
void TransCand::Create(char * line)
{
    int termCount;
    char ** terms;

    //>>>termCount = Util::Split(line, " |||| ", terms); // terms[0] : source-sentence; terms[1]: feature values
    termCount = (int)StringUtil::Split( line, " |||| ", terms );

    if( termCount > 1 ){

        if( GlobalVar::allLowerCase )
            //>>>Util::ToLowerCase(terms[0]);
            StringUtil::ToLowercase( terms[0] );

        // load translation
        if( GlobalVar::normalizeText ){
            char * s = StringUtil::NormalizeText(terms[0]);
            if( GlobalVar::internationalTokenization ){
                char * s2 = StringUtil::InternationalTokenization(s);
                delete[] s;
                s = s2;
            }
            translation = new Sentence(s);
            delete[] s;
        }
        else
            translation = new Sentence(terms[0]);

        // load feature id and feature value
        char ** featTerms;
        //>>>featCount = Util::Split(terms[1], " ", featTerms);
        featCount = (int)StringUtil::Split(terms[1], " ", featTerms );
        
        featCount++;
        feats = new FeatureNode[featCount+1];

        for( int i = 0; i < featCount - 1; i++ ){
            sscanf(featTerms[i], "%d:%f", &feats[i+1].id, &feats[i+1].value );
            delete[] featTerms[i];
        }

        delete[] featTerms;
    }

    for( int i = 0; i < termCount; i++ )
        delete[] terms[i];
    delete[] terms;
}

// translation: word1 word2 word3 ...
// featValues:  feat1 feat2 feat3 ...
void TransCand::Create(char * trans, float * featValues, int featNum)
{
    char * s = NULL;

    // load translation
    if( GlobalVar::normalizeText ){
        s = StringUtil::NormalizeText(trans);
    }
    else{
        s = new char[strlen(trans) + 1];
        strcpy(s, trans);
    }

    if( GlobalVar::allLowerCase )
        StringUtil::ToLowercase(s);

    if( GlobalVar::internationalTokenization ){
        char * s2 = StringUtil::InternationalTokenization(s);
        delete[] s;
        s = s2;
    }

    translation = new Sentence(s);

    delete[] s;

    feats = new FeatureNode[featNum + 1];
    for( int i = 0; i < featNum; i++){
        feats[i + 1].id = i + 1;
        feats[i + 1].value = featValues[i];
    }
    featCount = featNum + 1;

    //if( GlobalVar::allLowerCase )
    //    StringUtil::ToLowercase(trans);

    //// load translation
    //if( GlobalVar::normalizeText ){
    //    char * s = StringUtil::NormalizeText(trans);
    //    translation = new Sentence(s);
    //    delete[] s;
    //}
    //else
    //    translation = new Sentence(trans);

    //feats = new FeatureNode[featNum + 1];
    //for( int i = 0; i < featNum; i++){
    //    feats[i + 1].id = i + 1;
    //    feats[i + 1].value = featValues[i];
    //}
    //featCount = featNum + 1;
}

// format of line:
// feat1 feat2 feat3 ||| p r f1
void TransCand::CreateF1(char * line)
{
    int termCount;
    char ** terms;

    //>>>termCount = Util::Split(line, " |||| ", terms); // terms[0] : source-sentence; terms[1]: feature values
    termCount = (int)StringUtil::Split( line, " |||| ", terms );

    if( termCount > 1 ){

        // load feature id and feature value
        char ** featTerms;
        //>>>featCount = Util::Split(terms[0], " ", featTerms);
        featCount = (int)StringUtil::Split( terms[0], " ", featTerms );
        
        featCount++;
        feats = new FeatureNode[featCount+1];

        for( int i = 0; i < featCount - 1; i++ ){
            sscanf(featTerms[i], "%d:%f", &feats[i+1].id, &feats[i+1].value );
            delete[] featTerms[i];
        }

        delete[] featTerms;

        // load p, r and f1
        if( sscanf(terms[1], "%f %f %f %f %f %f", &precision, &recall, &f1, &correct, &predicted, &truth) < 6 )
            fprintf(stderr, "invalide line: %s\n", line);
    }

    for( int i = 0; i < termCount; i++ )
        delete[] terms[i];
    delete[] terms;
}

void TransCand::Deliver(TransCand * target)
{
    *target     = *this;
    Init();
}

int TransCand::GetFeatIndex(int id)
{
    if( id < featCount && id == feats[id].id )
        return id;
    else{
        for( int i = 1; i < featCount; i++ )
            if( feats[i].id == id )
                return i;
    }

    return -1;
}

// reference translation
RefSents::RefSents()
{
    Init();
}

RefSents::RefSents(char ** myRefSent, int myRefCount, int myNGram)
{
    char nGramStr[MAX_WORD_LENGTH];

    Init();    
    refs = new Sentence[myRefCount];

    for( int i = 0; i < myRefCount; i++ ){
        refs[i].Create(myRefSent[i]);

        // reference length
        if( refs[i].wordCount < shortestRefLen )
            shortestRefLen = refs[i].wordCount;

        // count n-gram
        HashTable* hashSentNGram = new HashTable(NGRAM_MIN_NUM);

        for( int j = 0; j < refs[i].wordCount; j++ ){

            nGramStr[0] = '\0';

            for( int k = 0; k < myNGram && k + j < refs[i].wordCount; k++){
                if( k > 0 )
                    strcat(nGramStr, " ");
                strcat(nGramStr, refs[i].words[k+j]);

                int wid = hashSentNGram->GetInt( nGramStr );

                if( wid == -1 ) {
                    wid = 1;
                }
                else {
                    ++wid;
                }
                hashSentNGram->AddInt( nGramStr, wid );
            }
        }

        for( int j = 0; j < hashSentNGram->GetKeyCnt(); j++ ){
            char * key = ( char* ) hashSentNGram->GetKey( j );
            int indexSent  = hashSentNGram->GetInt( key );
            int indexMatch = ngramMatch->GetInt( key );

            if( indexMatch == -1 )
                ngramMatch->AddInt( key, indexSent );
            else if( indexSent > indexMatch )
                ngramMatch->AddInt( key, indexSent );
        }

        delete hashSentNGram;

        refCount++;
    }
}

RefSents::~RefSents()
{
    delete[] refs;
	delete[] aligns;
    delete ngramMatch;
}

void RefSents::Init()
{
    refs        = NULL;
    aligns      = NULL;
    refCount    = 0;
    ngramMatch  = new HashTable(NGRAM_MIN_NUM);
    shortestRefLen  = MAX_SENT_LENGTH;
}

float * RefSents::GetNgramMatchStat(Sentence * sent, int ngram)
{
    char nGramStr[MAX_WORD_LENGTH];
    float * nGramStat = new float[ngram];
    memset(nGramStat, 0, sizeof(float) * ngram);
    HashTable* hashSentNGram = new HashTable(NGRAM_MIN_NUM);
    
    for( int n = 0; n < ngram; n++ ){
        hashSentNGram->Clear();
        for( int i = 0; i < sent->wordCount - n; i++ ){
            nGramStr[0] = '\0';
            for( int j = 0; j <= n && i + j < sent->wordCount; j++ ){
                if( j > 0 )
                    strcat(nGramStr, " ");
                strcat(nGramStr, sent->words[i+j]);
            }

            int index = hashSentNGram->GetInt( nGramStr );
            if( index == -1 )
                index = 1;
            else
                ++index;
            hashSentNGram->AddInt( nGramStr, index );
        }

        for( int j = 0; j < hashSentNGram->GetKeyCnt(); j++ ){
            char * key = ( char* ) hashSentNGram->GetKey( j );
            int indexSent  = hashSentNGram->GetInt( key );
            int indexMatch = ngramMatch->GetInt( key );

            if( indexMatch == -1 )
                continue;

            if( indexSent < indexMatch )
                nGramStat[n] += indexSent;
            else
                nGramStat[n] += indexMatch;
        }
    }

    delete hashSentNGram;

    return nGramStat;
}

void RefSents::CreateAlignment(char ** alignment, int srcLength, int refCount)
{
	if(aligns != NULL)
		delete[] aligns;

	aligns = new WordAlignment[refCount];
	for(int i = 0; i < refCount; i++){
		aligns[i].Create(srcLength, refs[i].wordCount, alignment[i]);
	}
}

TrainingSample::TrainingSample()
{
    Init();
}

TrainingSample::~TrainingSample()
{
    Clear();
}

void TrainingSample::Init()
{
    srcSent        = NULL;
    Cands          = NULL;
    CandCount      = 0;
    ref            = NULL;
    
}

void TrainingSample::Clear()
{
    delete   srcSent;
    srcSent        = NULL;
    delete[] Cands;
    Cands          = NULL;
    CandCount      = 0;
    delete   ref;
    ref            = NULL;
}

void TrainingSample::ClearCands()
{
    delete[] Cands;
    Cands     = NULL;
    CandCount = 0;
}

void TrainingSample::Deliver(TrainingSample * target)
{
    *target      = *this;
    Init();
}

void TrainingSample::CaculateAnchorLoss(TransCand * cand)
{
    List * wordTrans = cand->wordTrans->translations;
    float score = 0;
    int anchorCount = 0;

    for(int i = 0; i < wordTrans->count; i++){
        char * trans = (char *)wordTrans->GetItem(i);
        bool isAnchor = false;
        float bestScore = 0;

        for(int r = 0; r < ref->refCount; r++){

            Sentence * refTrans  = ref->refs + r;
            WordAlignment * align = ref->aligns + r;
            List * tgtIdList = align->srcAlign[i];
            int matched = 0;

            if(tgtIdList->count > 0){
                for(int j = 0; j < tgtIdList->count; j++){ // for each word in reference translation
                    long id = (long)tgtIdList->GetItem(j);
                    char * cTrans = refTrans->words[id];
                    if(strstr(trans, cTrans) != NULL)
                        matched++;
                }

                isAnchor = true;
            }

            if(tgtIdList->count > 0 && bestScore < (float)matched/tgtIdList->count)
                bestScore = (float)matched/tgtIdList->count; // score = #matched / #total
        }

        if(isAnchor){
            score += bestScore;
            anchorCount++;
        }
    }

    cand->anchorLoss  = -score;
    cand->anchorCount = anchorCount;
}

// training set

TrainingSet::TrainingSet()
{
    Init();
}

TrainingSet::TrainingSet(const char * refFileName, int refNum, int ngram, int maxSentNum)
{
    Init();
    LoadRefData(refFileName, refNum, ngram, maxSentNum);
}

TrainingSet::~TrainingSet()
{
    Clear();
}

void TrainingSet::Init()
{
    samples     = NULL;
    sampleCount = 0;
    ngram       = 0;
}

void TrainingSet::Clear()
{
    delete[] samples;
    samples     = NULL;
    sampleCount = 0;
    ngram       = 0;
}

void TrainingSet::LoadRefData(const char * refFileName, int refNum, int ngram, int maxSentNum)
{
    char **           refs;
    char *            line = new char[MAX_LINE_LENGTH];
    TrainingSample ** tSamples = new TrainingSample*[MAX_SAMPLE_NUM];  // training samples
    int               tSampleCount = 0;
    bool              breakFlag = false;

    this->ngram = ngram;

    fprintf( stderr, "Loading %s\n", "Reference Translations" );
    fprintf( stderr, "  >> From File: %s ...\n", refFileName );
    fprintf( stderr, "  >> " );

    refs = new char*[MAX_REF_NUM];
    for( int i = 0; i < MAX_REF_NUM; i++ )
        refs[i] = new char[MAX_SENT_LENGTH];

    FILE * file = fopen(refFileName, "r");

    if( file == NULL ){
        delete[] line;
        fprintf( stderr, "cannot open file \"%s\"!\n", refFileName );
        exit(1);
    }

    while(fgets(line, MAX_LINE_LENGTH - 1, file )){
        TrainingSample * newSample = new TrainingSample();

        char ** terms;
        int termCount = (int)StringUtil::Split( line, " |||| ", terms );

        if( GlobalVar::allLowerCase )
            StringUtil::ToLowercase( terms[0] );

        // load source sentence
        if( GlobalVar::normalizeText ){
            char * s = StringUtil::NormalizeText(terms[0]);
            newSample->srcSent = new Sentence(s);
            delete[] s;
        }
        else
            newSample->srcSent = new Sentence(terms[0]);

        for( int i = 0; i < termCount; i++ )
            delete[] terms[i];
        delete[] terms;

        // space line
        if( !fgets(line, MAX_LINE_LENGTH, file ) ){
            delete newSample;
            break;
        }

        int refCount = 0;
        for( int i = 0; i < refNum; i++ ){
            if( !fgets(line, MAX_LINE_LENGTH, file ) ){
                breakFlag = true;
                break;
            }

            StringUtil::TrimRight( line );

            if( GlobalVar::allLowerCase )
                StringUtil::ToLowercase( line );

            if( GlobalVar::normalizeText ){
                char * s = StringUtil::NormalizeText(line);
                if( GlobalVar::internationalTokenization ){
                        char * s2 = StringUtil::InternationalTokenization(s);
                        delete[] s;
                        s = s2;
                }
                strcpy(refs[i], s);
                delete[] s;
            }
            else
                strcpy(refs[i], line);
            refCount++;
        }

        newSample->ref = new RefSents(refs, refCount, ngram);

        if( breakFlag ){
            delete newSample;
            break;
        }

        tSamples[tSampleCount++] = newSample;

        if(tSampleCount >= maxSentNum)
            break;

    }

    fclose(file);
    delete[] line;
    fprintf( stderr, "\nDone [%d sentence(s)]\n", tSampleCount );
    //fprintf( stderr, "\n" );

    sampleCount = tSampleCount;
    samples = new TrainingSample[sampleCount];
    for( int i = 0; i < tSampleCount; i++ ){
        tSamples[i]->Deliver(samples + i);
        delete tSamples[i];
    }
    delete[] tSamples;

    for( int i = 0; i < MAX_REF_NUM; i++ )
        delete[] refs[i];
    delete[] refs;
    
    
}

void TrainingSet::LoadTrainingData(const char * transFileName, bool accumlative)
{
    char *            line = new char[MAX_LINE_LENGTH];
    TransCand **      tCands = new TransCand*[MAX_CAND_NUM];      // translation candidates for each source sentence
    int               tCandCount = 0, lineCount = 0, tmpCount = 0;
    int               sampleId = 0;

    FILE * file = fopen(transFileName, "r");

    if( file == NULL ){
        fprintf( stderr, "cannot open file \"%s\"!\n", transFileName );
        return;
    }

    fprintf( stderr, "\rloading training samples ...");

    // create sample list
    if( GlobalVar::useF1Criterion )
    {
        //fprintf( stderr, "preparing ...");

        sampleCount = 0;
        while(fgets(line, MAX_LINE_LENGTH, file )){
			bool seg = strlen(line) >= 3 && line[0] == '=' && line[1] == '=' && line[2] == '=';
            if(seg)
                sampleCount++;
			lineCount++;
        }

        samples = new TrainingSample[sampleCount];
        fclose(file);
        file = fopen(transFileName, "r");
    }

    while(fgets(line, MAX_LINE_LENGTH, file ) && sampleId < sampleCount){
        TrainingSample * sample = samples + sampleId;
        bool seg = strlen(line) >= 3 && line[0] == '=' && line[1] == '=' && line[2] == '=';

        if(seg){ // segmented by "==="
            if(sample->CandCount > 0 && accumlative){
                TransCand * candsCopy = sample->Cands;
                sample->Cands = new TransCand[tCandCount + sample->CandCount];    // Resize
                for( int i = 0; i < sample->CandCount; i++ )
                    //candsCopy[i].Deliver(sample->Cands + i);
                    candsCopy[i].Deliver(sample->Cands + i + tCandCount);
                delete[] candsCopy;
            }
            else{
                sample->ClearCands();
                sample->Cands = new TransCand[tCandCount];
            }

            /*for( int i = 0; i < tCandCount; i++ ){
                tCands[i]->Deliver(sample->Cands + sample->CandCount);
                sample->CandCount++;
                delete tCands[i];
            }*/

            for( int i = 0; i < tCandCount; i++ ){
                tCands[i]->Deliver(sample->Cands + i);
                delete tCands[i];
            }
            sample->CandCount += tCandCount;

            sampleId++;
            tCandCount = 0;
            continue;
        }

        RefSents *  ref  = sample->ref;
        TransCand * cand = new TransCand(line);

        // for bleu only
        if( !GlobalVar::useF1Criterion ){
            cand->nGramMatch = ref->GetNgramMatchStat(cand->translation, ngram);

            cand->shortestRefLen = (float)sample->ref->shortestRefLen;           // shortest length (NIST-BLEU)
            for(int i = 0; i < sample->ref->refCount; i++){                      // "best" length (IBM-BLEU)
                int refLen = sample->ref->refs[i].wordCount; // length of the i-th reference translation
                if( abs((int)(refLen - cand->translation->wordCount)) < \
                                    abs((int)(cand->bestRefLen - cand->translation->wordCount)) )
                    cand->bestRefLen = (float)refLen;
            }
            cand->SBPRefLen = cand->translation->wordCount < cand->shortestRefLen ? // "best" length (BLEU-SBP)
                cand->translation->wordCount :
                cand->shortestRefLen;
        }

        tCands[tCandCount++] = cand;
    }

    fclose(file);
    fprintf( stderr, "\n" );

    delete[] line;
    delete[] tCands;
}

void TrainingSet::LoadTrainingData(TransResult * rlist, int rCount, int featNum, int sampleId, bool accumlative)
{
    bool decodingFailure    = rCount > 0 ? false : true ;
    if(decodingFailure)
        rCount = 1;

    TrainingSample * sample = samples + sampleId;
    RefSents *  ref         = sample->ref;
    TransCand ** tCands     = new TransCand *[rCount];
    int  tCandCount         = rCount;
    
    for( int k = 0; k < rCount; k++ ){
        TransCand * cand = NULL;
        if(!decodingFailure)
            cand = new TransCand(rlist[k].translation, rlist[k].featValues, featNum);
        else{
            cand = new TransCand();
            cand->InitWithNULL(featNum);
        }

        cand->nGramMatch = ref->GetNgramMatchStat(cand->translation, ngram);

        cand->shortestRefLen = (float)sample->ref->shortestRefLen;           // shortest length (NIST-BLEU)
        for(int i = 0; i < sample->ref->refCount; i++){                      // "best" length (IBM-BLEU)
            int refLen = sample->ref->refs[i].wordCount;                     // length of the i-th reference translation
            if( abs((int)(refLen - cand->translation->wordCount)) < \
                            abs((int)(cand->bestRefLen - cand->translation->wordCount)) )
                cand->bestRefLen = (float)refLen;
        }

        cand->SBPRefLen = cand->translation->wordCount < cand->shortestRefLen ? // "best" length (BLEU-SBP)
                          cand->translation->wordCount :
                          cand->shortestRefLen;

        if(rlist[k].wordTranslation != NULL){
            WordTranslation * wordTrans = (WordTranslation *)rlist[k].wordTranslation; // word translation
            cand->wordTrans = wordTrans->Copy();
            sample->CaculateAnchorLoss(cand);  // loss at anchoring points
            rlist[k].anchorLoss = cand->anchorLoss;
        }

        tCands[k] = cand;
    }

    if(sample->CandCount > 0 && accumlative){
        TransCand * candsCopy = sample->Cands;
        sample->Cands = new TransCand[tCandCount + sample->CandCount];    // Resize it
        for( int i = 0; i < sample->CandCount; i++ )
            //candsCopy[i].Deliver(sample->Cands + i);
            candsCopy[i].Deliver(sample->Cands + i + tCandCount);
        delete[] candsCopy;
    }
    else{
        sample->ClearCands();
        sample->Cands = new TransCand[tCandCount];
    }

    /*for( int i = 0; i < tCandCount; i++ ){
        tCands[i]->Deliver(sample->Cands + sample->CandCount);
        sample->CandCount++;
        delete tCands[i];
    }*/

    for( int i = 0; i < tCandCount; i++ ){
        tCands[i]->Deliver(sample->Cands + i);
        delete tCands[i];
    }
    sample->CandCount += tCandCount;

    delete[] tCands;
}

void TrainingSet::LoadWordAlignment(const char * alignFileName, int refNum, int maxSentNum)
{
	int count   = 0;
	char ** aligns = NULL;
	char * line = new char[MAX_LINE_LENGTH];
	FILE * file = fopen(alignFileName, "r");

    if( file == NULL ){
        fprintf( stderr, "cannot open file \"%s\"!\n", alignFileName );
        return;
    }

	aligns = new char*[MAX_REF_NUM];
    for( int i = 0; i < MAX_REF_NUM; i++ )
        aligns[i] = new char[MAX_SENT_LENGTH];

    fprintf( stderr, "Loading %s\n", "Alignments (for anchor-augmented tuning" );
    fprintf( stderr, "  >> From File: %s ...\n", alignFileName );
    fprintf( stderr, "  >> " );

	while(fgets(line, MAX_LINE_LENGTH, file )){

		TrainingSample * sample = samples + count;

        // space line
        if(!fgets(line, MAX_LINE_LENGTH, file))
            break;

		bool breakFlag = false;
        int refCount = 0;

        for( int i = 0; i < refNum; i++ ){
            if( !fgets(line, MAX_LINE_LENGTH, file ) ){
                breakFlag = true;
                break;
            }

            StringUtil::TrimRight( line );

            if( GlobalVar::allLowerCase )
                StringUtil::ToLowercase(line);

			strcpy(aligns[i], line);
        }

		sample->ref->CreateAlignment(aligns, sample->srcSent->wordCount, sample->ref->refCount);

        if( breakFlag )
            break;

        if(++count >= maxSentNum)
            break;
    }

    fclose(file);
    delete[] line;
    fprintf( stderr, "\nDone [%d sentence(s)]\n", count );

	for( int i = 0; i < MAX_REF_NUM; i++ )
        delete[] aligns[i];
    delete[] aligns;
}

TrainingSample * TrainingSet::GetSample(int i)
{
    if(i < 0 || i >= sampleCount)
        return NULL;
    else
        return samples + i;
}

RefSents * TrainingSet::GetRef(int i)
{
    if(i < 0 || i >= sampleCount)
        return NULL;
    else
        return samples[i].ref;
}

// parameters

ParaInfo::ParaInfo()
{
    weight   = 0;
    minValue = -1;
    maxValue = 1;
    isFixed  = false;
}

ParaBase::ParaBase(int maxNum)
{
    paraList    = new ParaInfo*[maxNum];
    paraLen     = new int[maxNum];
    bleuList    = new float[maxNum];
    paraListLen = 0;
    memset(paraList, 0, sizeof(ParaInfo*) * maxNum);
    memset(paraLen, 0, sizeof(int) * maxNum);
    memset(bleuList, 0, sizeof(float) * maxNum);
    //paraListLen = 0;
}

ParaBase::~ParaBase()
{
    for( int i = 0; i < paraListLen; i++ )
        delete[] paraList[i];
    delete[] paraList;
    delete[] paraLen;
    delete[] bleuList;
}

void ParaBase::Add(ParaInfo * para, int paraCount, float bleu)
{
    bleuList[paraListLen] = bleu;
    paraLen[paraListLen] = paraCount;
    paraList[paraListLen] = new ParaInfo[paraCount];
    for( int i = 0; i < paraCount; i++)
        paraList[paraListLen][i] = para[i];
    paraListLen++;
}

char * ParaBase::ToStr(ParaInfo * para, int paraCount)
{
    char * wStr = new char[paraCount * 20];
    wStr[0] = '\0';
    for( int i = 1; i < paraCount; i++ ){
        if( i >= 1 )
            sprintf(wStr, "%s ", wStr);
        sprintf(wStr, "%s%.8f", wStr, para[i].weight);
    }
    return wStr;
}

Intersection::Intersection()
{
    coord               = 0;
    deltaMatch          = NULL;
    deltaNGram          = NULL;
    deltaBestRefLen     = 0;
    deltaShortestRefLen = 0;
    deltaSBPRefLen      = 0;
    deltaPrecision      = 0;
    deltaRecall         = 0;
    deltaF1             = 0;
    deltaCorrect        = 0;
    deltaPredicted      = 0;
    deltaTruth          = 0;
    deltaAnchorLoss     = 0;
    deltaAnchorCount    = 0;
}

Intersection::~Intersection()
{
    delete[] deltaMatch;
    delete[] deltaNGram;
}

void Intersection::Create(int ngram)
{
    coord               = 0;
    deltaBestRefLen     = 0;
    deltaShortestRefLen = 0;
    deltaSBPRefLen      = 0;
    deltaPrecision      = 0;
    deltaRecall         = 0;
    deltaF1             = 0;
    deltaCorrect        = 0;
    deltaPredicted      = 0;
    deltaTruth          = 0;
    deltaAnchorLoss     = 0;
    deltaAnchorCount    = 0;
    delete[] deltaMatch;
    delete[] deltaNGram;
    deltaMatch = new float[ngram];
    deltaNGram = new float[ngram];
    memset(deltaMatch, 0, sizeof(float) * ngram);
    memset(deltaNGram, 0, sizeof(float) * ngram);
}

////////////////////////////////////////
// Trainer

OurTrainer::OurTrainer()
{
    Init();
}

OurTrainer::~OurTrainer()
{
    Clear();
}

void OurTrainer::Init()
{
    para       = NULL;
    finalPara  = NULL;
    paraCount  = 0;
    round      = 0;
    isPAveraged = 1;
}

void OurTrainer::Clear()
{
    delete[] para;
    delete[] finalPara;
    Init();
}

void OurTrainer::CreatePara(int weightNum)
{
    delete[] para;
    delete[] finalPara;
    paraCount = 0;
    para = new ParaInfo[weightNum + 1];    // index from 1
    finalPara = new ParaInfo[weightNum + 1];
    paraCount = weightNum + 1;
}

void OurTrainer::LoadPara(const char * configFileName)
{
    char * line = new char[MAX_LINE_LENGTH];
    FILE * file = fopen(configFileName, "r");
    

    if( !fgets(line, MAX_LINE_LENGTH - 1, file) )
        return;

    sscanf(line, "%d", &paraCount);
    paraCount++;
    delete[] para;
    delete[] finalPara;
    para = new ParaInfo[paraCount];    // index from 1
    finalPara = new ParaInfo[paraCount];

    while( fgets(line, MAX_LINE_LENGTH - 1, file) )
        SetParaInfo(line);

    fclose(file);
    delete[] line;
}

void OurTrainer::SetParaInfo(char * paraLine)
{
    int id, isFixed;
    float w, min, max;

    if( sscanf(paraLine, "%d %f %f %f %d", &id, &w, &min, &max, &isFixed) < 5 ){
        fprintf( stderr, "invalid para-line \"%s\"\n", paraLine );
        return;
    }

    if( id >= paraCount || id <= 0){
        fprintf( stderr, "invalid para-id %d is not in [1,%d]\n", id, paraCount );
        return;
    }

    para[id].weight   = w;
    para[id].minValue = min;
    para[id].maxValue = max;
    para[id].isFixed  = isFixed == 1 ? true : false;
}

void OurTrainer::LoadParaFromModel(ParaInfo * p, int pCount)
{
    if(para != NULL)
        delete[] para;
    if(finalPara != NULL)
        delete[] finalPara;

    paraCount = pCount + 1;
    para = new ParaInfo[paraCount];    // index from 1
    finalPara = new ParaInfo[paraCount];
    for( int i = 0; i < pCount; i++ ){
        para[i + 1] = p[i];
    }
}

void OurTrainer::OptimizeWeights(TrainingSet * ts, ParaInfo * para, int paraCount,
                                 int ngram, BLEU_TYPE BLEUType, float anchorAlpha, METHOD method)
{
    char * wMsg1;
    char * wMsg2;
    char msg[1024], criterion[1024];
    float oldScore, newScore;
    float oldBP, newBP;
    TransCand ** trans = new TransCand*[ts->sampleCount];

    if( GlobalVar::useF1Criterion )
        strcpy(criterion, "F1");
    else
        sprintf(criterion, "BLEU%d", ngram);

    fprintf(stderr, " ...");

    // BLEU before conducting optimization
    //for( int i = 0; i < ts->sampleCount; i++ )
    //    trans[i] = GetTop1(para, paraCount, ts->samples[i].Cands, ts->samples[i].CandCount);
    for( int i = 0; i < ts->sampleCount; i++ )
        trans[i] = ts->samples[i].Cands;


    oldScore = GetScore(trans, ts->sampleCount, ts, ts->ngram, BLEUType, anchorAlpha);
    oldBP = GetBP(trans, ts->sampleCount, ts, ts->ngram, BLEUType, anchorAlpha);

    if(anchorAlpha == 0)
        fprintf(stderr, "\n>>>> ROUND %-2d: method = \"%s + %s\", %s = %.4f, BP = %.3f", 
			    ++round, METHOD_NAME[(int)method], BLEU_TYPE_NAME[(int)BLEUType], criterion, oldScore, oldBP);
    else{
        float loss = anchorAlpha * GetAnchorLoss(trans, ts->sampleCount, ts);
        fprintf(stderr, "\n>>>> ROUND %-2d: method = \"%s + %s(a)\", %s = %.4f (%.3f + %.3f), BP = %0.3f", 
			    ++round, METHOD_NAME[(int)method], BLEU_TYPE_NAME[(int)BLEUType], criterion, oldScore, 
                            oldScore + loss, -loss, oldBP);
    }

    fprintf(stderr, "\n");

    sprintf(msg, "ROUND %2d %s = %.4f", round, criterion, oldScore);
    DumpPara("MERT.xt.log.txt", para, paraCount, msg);

    if( method == MERT )
        MERTWithLineSearch(ts, para, finalPara, paraCount, 5, ngram, BLEUType, anchorAlpha);

    // BLEU after conducting optimization
    for( int i = 0; i < ts->sampleCount; i++ )
        trans[i] = GetTop1(para, paraCount, ts->samples[i].Cands, ts->samples[i].CandCount);
    newScore = GetScore(trans, ts->sampleCount, ts, ts->ngram, BLEUType, anchorAlpha);
    newBP = GetBP(trans, ts->sampleCount, ts, ts->ngram, BLEUType, anchorAlpha);

    msg[0] = '\0';

    wMsg1 = ParaBase::ToStr(finalPara, paraCount);
    wMsg2 = ParaBase::ToStr(para, paraCount);
    sprintf(msg,"Before optimization - %s: %.4f", criterion, oldScore);
    sprintf(msg,"%s\nAfter optimization - %s: %.4f", msg, criterion, newScore);
    sprintf(msg, "%s\nOptimized Weights : %s", msg, wMsg1);
    sprintf(msg, "%s\nFinal Weights : %s\n", msg, wMsg2);

    FILE * resultF = fopen("weights.txt", "w");
    fprintf( resultF, "%s\n", msg );
    fclose(resultF);

    delete[] trans;
    delete[] wMsg1;
    delete[] wMsg2;
}

void OurTrainer::OptimzieWeightsWithMERT(TrainingSet * ts, int ngram, BLEU_TYPE BLEUType, float anchorAlpha)
{
    OptimizeWeights(ts, para, paraCount, ngram, BLEUType, anchorAlpha, MERT);
}

float OurTrainer::GetModelScore(ParaInfo * para, int paraCount, TransCand * cand)
{
    float score = 0;

    if(cand == NULL)
        return FLT_MIN;
    
    for( int i = 1; i < cand->featCount; i++ ){
        int id = cand->feats[i].id;
        score += cand->feats[i].value * para[id].weight;
    }

    return score;
}

TransCand * OurTrainer::GetTop1(ParaInfo * para, int paraCount, TransCand * cands, int candCount)
{
    float maxScore = FLT_MIN;
    int   maxCand = -1;

    for( int i = 0; i< candCount; i++ ){
        TransCand * cand = cands + i;
        float curScore = GetModelScore(para, paraCount, cand);
        if( curScore > maxScore ){
            maxScore = curScore;
            maxCand = i;
        }
    }

    if( maxCand == -1 )
        return NULL;
    else
        return cands + maxCand;
}

float OurTrainer::GetBLEU(TransCand ** transList, int transCount, TrainingSet * ts, int ngram, BLEU_TYPE type, bool BPOnly)
{

    float bestRefLen     = 0;
    float shortestRefLen = 0;
    float SBPRefLen      = 0;
    float * matchedNGram = new float[ngram];
    float * statNGram = new float[ngram];

    for( int n = 0; n < ngram; n++ ){
        matchedNGram[n] = 0;
        statNGram[n] = 0;
    }

    for( int i = 0; i < transCount; i++ ){
        TransCand * cand = transList[i];
		if(cand == NULL || ts->samples[i].CandCount == 0){
            bestRefLen += ts->samples[i].ref->shortestRefLen;
            shortestRefLen += ts->samples[i].ref->shortestRefLen;
            SBPRefLen += ts->samples[i].ref->shortestRefLen;
            continue;
        }

        int wordCount = cand->translation->wordCount;
        for( int n = 0; n < ngram && n < wordCount; n++ ){
            matchedNGram[n] += cand->nGramMatch[n];
            statNGram[n] += cand->translation->wordCount - n;
        }
        bestRefLen += cand->bestRefLen;
        shortestRefLen += cand->shortestRefLen;
        SBPRefLen += cand->SBPRefLen;

    }

    float score = 0;

    if(!BPOnly)
        score = Evaluator::CalculateBLEU(matchedNGram, statNGram, 
                                         bestRefLen, shortestRefLen, SBPRefLen, ngram, type);
    else
        score = Evaluator::CalculateBP(matchedNGram, statNGram, 
                                       bestRefLen, shortestRefLen, SBPRefLen, ngram, type);

    delete[] matchedNGram;
    delete[] statNGram;

    return score;
}

float OurTrainer::GetF1(TransCand ** transList, int transCount, TrainingSet * ts)
{
    float p = 0, r = 0, f1 = 0;
    float correct = 0, predicted = 0, truth = 0;

    for( int i = 0; i < transCount; i++ ){
        TransCand * cand = transList[i];
        if( cand == NULL )
            continue;

        correct += cand->correct;
        predicted += cand->predicted;
        truth += cand->truth;
    }

    p = correct / predicted;
    r = correct / truth;
    f1 = 2 * p * r / (p + r);
    return f1;
}

float OurTrainer::GetAnchorLoss(TransCand ** transList, int transCount, TrainingSet * ts)
{
    float loss = 0;
    float count = 0;

    for( int i = 0; i < transCount; i++){
		if(ts->samples[i].CandCount == 0)
			continue;
        loss  += transList[i]->anchorLoss;
        count += transList[i]->anchorCount;
    }

    return count > 0 ? loss / count : 0;
}

float OurTrainer::GetScore(TrainingSet * ts, int ngram, BLEU_TYPE BLEUType, float anchorAlpha)
{
	float score = 0;

	TransCand ** trans = new TransCand*[ts->sampleCount];

	for( int i = 0; i < ts->sampleCount; i++ ){
		trans[i] = ts->samples[i].Cands;
	}

	score = GetScore(trans, ts->sampleCount, ts, ngram, BLEUType, anchorAlpha);

	delete[] trans;

	return score;
}

float OurTrainer::GetBP(TrainingSet * ts, int ngram, BLEU_TYPE BLEUType, float anchorAlpha)
{
	float BP = 0;

	TransCand ** trans = new TransCand*[ts->sampleCount];

	for( int i = 0; i < ts->sampleCount; i++ ){
		trans[i] = ts->samples[i].Cands;
	}

	BP = GetBP(trans, ts->sampleCount, ts, ngram, BLEUType, anchorAlpha);

	delete[] trans;

	return BP;
}

float OurTrainer::GetScore(TransCand ** transList, int transCount, TrainingSet * ts, int ngram, BLEU_TYPE type, float anchorAlpha)
{
    if( type < 3 )
        return GetBLEU(transList, transCount, ts, ngram, type) 
               - anchorAlpha * GetAnchorLoss(transList, transCount, ts);
    else
        return GetF1(transList, transCount, ts);
}

float OurTrainer::GetBP(TransCand ** transList, int transCount, TrainingSet * ts, int ngram, BLEU_TYPE type, float anchorAlpha)
{
    if( type < 3 )    
        return GetBLEU(transList, transCount, ts, ngram, type, true);
    else
        return 1;
}

void OurTrainer::DumpPara(const char * fileName, ParaInfo * para, int paraCount, char * msg)
{
    FILE * file = fopen(fileName, "a");

    fprintf(file, "%s\n", msg);
    fprintf(file, "<");
    for(int i = 1; i < paraCount; i++ ){
        if( i > 1 )
            fprintf(file, " ");
        fprintf(file, "%0.4f", para[i].weight);
    }

    fprintf(file, ">\n");

    fclose(file);
}

int OurTrainer::ComputeIntersections(TrainingSample * sample, int dim, Intersection * intersections, int &num, int ngram)
{
    TransCand * cands = sample->Cands;

    int * fId = new int[sample->CandCount];
    float y1 = FLT_MIN, y2 = FLT_MIN;
    float a1 = FLT_MIN, a2 = FLT_MIN;
    int n1 = 0, n2 = 0;
    int count = 0;

    for( int n = 0; n < sample->CandCount; n++ ){
        TransCand * cand = cands + n;

        cand->b = 0;
        for(int i = 1; i < cand->featCount; i++){
            int fId = cand->feats[i].id;
            if( fId != dim )
                cand->b += para[fId].weight * cand->feats[i].value;
            else
                cand->slope = cand->feats[fId].value;
        }

        if(cand->slope * para[dim].minValue + cand->b > y1 ){
            y1 = cand->slope * para[dim].minValue + cand->b;
            a1 = cand->slope;
            n1 = n;
        }

        if(cand->slope * para[dim].maxValue + cand->b > y2 ){
            y2 = cand->slope * para[dim].maxValue + cand->b;
            a2 = cand->slope;
            n2 = n;
        }
    }

    Intersection * isct = intersections + num;
    isct->Create(ngram);

    isct->coord = para[dim].minValue - (float)0.0001;  // min bound
    for( int i = 0; i < ngram; i++ ){    // for BLEU computation
        isct->deltaMatch[i] = cands[n1].nGramMatch[i];
        isct->deltaNGram[i] = (float)(cands[n1].translation->wordCount - i);
    }
    isct->deltaBestRefLen  = cands[n1].bestRefLen;
    isct->deltaShortestRefLen = cands[n1].shortestRefLen;
    isct->deltaSBPRefLen   = cands[n1].SBPRefLen;
    isct->deltaPrecision   = cands[n1].precision;
    isct->deltaRecall      = cands[n1].recall;
    isct->deltaF1          = cands[n1].f1;
    isct->deltaCorrect     = cands[n1].correct;
    isct->deltaPredicted   = cands[n1].predicted;
    isct->deltaTruth       = cands[n1].truth;
    isct->deltaAnchorLoss  = cands[n1].anchorLoss;
    isct->deltaAnchorCount = cands[n1].anchorCount;
    num++;

    int lastN = n1;

    while(true){
        bool breakFlag = true;
        int curN = -1;
        float min = para[dim].maxValue;

        for( int n = 0; n < sample->CandCount; n++ ){
            TransCand * curCand = cands + n;

            if (curCand->slope <= cands[lastN].slope || curCand->slope > a2)
                continue;

            float x = (curCand->b - cands[lastN].b) / (cands[lastN].slope - curCand->slope); // intersection of the two lines
            if( x < min || (x == min && curCand->slope > cands[lastN].slope) ){
                min  = x;
                curN = n;
                breakFlag = false;
            }
        }

        if( breakFlag )
            break;

        isct = intersections + num;
        isct->Create(ngram);
        isct->coord = min;
        for( int i = 0; i < ngram; i++ ){  // for BLEU computation
            isct->deltaMatch[i] = cands[curN].nGramMatch[i] - cands[lastN].nGramMatch[i];
            isct->deltaNGram[i] = (float)(cands[curN].translation->wordCount - cands[lastN].translation->wordCount);
        }
        isct->deltaBestRefLen = cands[curN].bestRefLen - cands[lastN].bestRefLen;
        isct->deltaShortestRefLen = cands[curN].shortestRefLen - cands[lastN].shortestRefLen;
        isct->deltaSBPRefLen = cands[curN].SBPRefLen - cands[lastN].SBPRefLen;
        isct->deltaPrecision = cands[curN].precision - cands[lastN].precision;
        isct->deltaRecall = cands[curN].recall - cands[lastN].recall;
        isct->deltaF1 = cands[curN].f1 - cands[lastN].f1;
        isct->deltaCorrect = cands[curN].correct - cands[lastN].correct;
        isct->deltaPredicted = cands[curN].predicted - cands[lastN].predicted;
        isct->deltaTruth = cands[curN].truth - cands[lastN].truth;
        isct->deltaAnchorLoss = cands[curN].anchorLoss - cands[lastN].anchorLoss;
        isct->deltaAnchorCount = cands[curN].anchorCount - cands[lastN].anchorCount;

        lastN = curN;
        num++;
        count++;
    }

    delete[] fId;

    return count;
}

int SortIntersectionWithW( const void * arg1, const void * arg2 )
 {
     Intersection * p1 = (Intersection *)arg1;
     Intersection * p2 = (Intersection *)arg2;

    if ( p1->coord > p2->coord )
         return 1;
     if ( p1->coord < p2->coord )
         return -1;
     return 0;
 };

// Minimum error rate training with line search (Och, ACL 2003)
int OurTrainer::MERTWithLineSearch(TrainingSet * ts, ParaInfo * para, 
                                   ParaInfo * finalPara, 
                                   int paraCount, int nRoundConverged, 
                                   int ngram, BLEU_TYPE BLEUType, float anchorAlpha)
{
    int ncoverged = 0;
    float lastScore = 0; 
    float maxScore, maxW, maxAnchorLoss;
    int maxDim, maxRound;
    int maxItersectionNum = 0;
    int iNum = 0;
    int n = 0, count = 0;
    char criterion[128];

    if( GlobalVar::useF1Criterion )
        strcpy(criterion, "F1");
    else
        sprintf(criterion,"BLEU%d", ngram);

    for( int i = 0; i < ts->sampleCount; i++ ){
        int sNum = ts->samples[i].CandCount;
        maxItersectionNum += sNum * sNum + 2;
    }
    if( maxItersectionNum > MILLION || maxItersectionNum <= 0)
        maxItersectionNum = MILLION;
    Intersection * intersections = new Intersection[MILLION];

    ParaBase * pb = new ParaBase(nRoundConverged + 1);

    fprintf(stderr, ">>>> optimizing with %s:", criterion);
    while(count++ < 100){
        maxScore = 0;
        maxDim  = 0;
        maxW    = 0;

        for( int d = 1; d < paraCount; d++ ){
            if( para[d].isFixed )
                continue;

            float * matchedNGram = new float[ngram];
            float * statNGram = new float[ngram];
            float bestRefLen, shortestRefLen, SBPRefLen;
            float precision, recall, f1;
            float correct, predicted, truth;
            float anchorLoss, anchorCount;
            float wRecord = para[d].weight; // record the i-th weight
            para[d].weight = 0;             // remove it
            iNum = 0;                       // clear "intersections" list

            for( int s = 0; s < ts->sampleCount; s++ )
                ComputeIntersections(ts->samples + s, d, intersections, iNum, ngram);

            // sort by the d-th weight
            qsort(intersections, iNum, sizeof(Intersection), SortIntersectionWithW);

            // max bound
            intersections[iNum].Create(ngram);
            intersections[iNum++].coord = para[d].maxValue;

            memset(matchedNGram, 0, sizeof(float) * ngram);
            memset(statNGram, 0, sizeof(float) * ngram);
            bestRefLen     = 0;
            shortestRefLen = 0;
            SBPRefLen      = 0;
            precision      = 0;
            recall         = 0;
            f1             = 0;
            correct        = 0;
            predicted      = 0;
            truth          = 0;
            anchorLoss     = 0;
            anchorCount    = 0;

            for( int i = 0; i < iNum - 1; i++ ){
                for( int j = 0; j < ngram; j++){
                    matchedNGram[j] += intersections[i].deltaMatch[j];
                    statNGram[j] += intersections[i].deltaNGram[j];
                }
                bestRefLen  += intersections[i].deltaBestRefLen;
                shortestRefLen += intersections[i].deltaShortestRefLen;
                SBPRefLen   += intersections[i].deltaSBPRefLen;
                precision   += intersections[i].deltaPrecision;
                recall      += intersections[i].deltaRecall;
                f1          += intersections[i].deltaF1;
                correct     += intersections[i].deltaCorrect;
                predicted   += intersections[i].deltaPredicted;
                truth       += intersections[i].deltaTruth;
                anchorLoss  += intersections[i].deltaAnchorLoss;
                anchorCount += intersections[i].deltaAnchorCount;

                //if( intersections[i].coord < para[d].minValue || intersections[i].coord == intersections[i + 1].coord )
                //    continue;
                if( intersections[i].coord - para[d].minValue < 0)
                    continue;

                float score = 0, loss = 0;
                if( GlobalVar::useF1Criterion ){
                    //float pAVG = precision / ts->sampleCount;
                    //float rAVG = recall / ts->sampleCount;
                    float pAVG = correct / predicted;
                    float rAVG = correct / truth;
                    score = 2 * pAVG * rAVG / (pAVG + rAVG);
                }
                else{
                    score = Evaluator::CalculateBLEU(matchedNGram, statNGram, bestRefLen, 
                                                      shortestRefLen, SBPRefLen, ngram, BLEUType);
                    loss = anchorCount > 0 ? anchorAlpha * anchorLoss / anchorCount : 0; // default: loss = 0;
					score -= loss;
                }

                if(score > maxScore){
                    maxDim = d;
                    maxScore = score;
                    maxW = (intersections[i].coord + intersections[i + 1].coord) / 2;
					maxAnchorLoss = loss;
                }
            }

            para[d].weight = wRecord;
            
            delete[] matchedNGram;
            delete[] statNGram;
        }

        para[maxDim].weight = maxW;
        //fprintf( stderr, "\rFeature %d at %.4f with %s = %.4f", maxDim, maxW, criterion, maxScore );

        if( fabs(lastScore - maxScore) < 1.0E-5 ){

			if(anchorAlpha == 0)
				fprintf( stderr, "\r>>>> %d: converging with %s = %.4f            \n", n + 1, criterion, maxScore );
			else
				fprintf( stderr, "\r>>>> %d: converging with %s = %.4f (%.4f + %.4f)            \n",
				                 n + 1, criterion, maxScore, maxScore + maxAnchorLoss, -maxAnchorLoss);

            pb->Add(para, paraCount, maxScore);    // record the weights
            n++;

            for( int d = 1; d < paraCount; d++ ){
    #ifndef    WIN32
                timeval time;
                gettimeofday (&time, NULL);
                srand(time.tv_sec);
    #else
                srand(clock());
    #endif
                int per = rand() % 21;
                //if( abs(para[d].weight) > 1 )
                //    para[d].weight *= (float)(1.0 + ((float)per - 10) / 100); // shirt +/- 10%
                //else
                    para[d].weight += ((float)per - 10) / 100; // shirt +/- 0.1

                if( para[d].weight <= para[d].minValue )
                    para[d].weight = para[d].minValue;
                if( para[d].weight >= para[d].maxValue )
                    para[d].weight = para[d].maxValue;

                maxDim = 0;
                maxScore = 0;
            }
        
            if(n >= nRoundConverged)
                break;
        }

        lastScore = maxScore;
    }

    maxScore = 0;
    maxRound = -1;
    for( int i = 0; i < pb->paraListLen; i++ ){
        /*bool equal = true;
        for( int f = 1; f < pb->paraLen[i]; f++){
            if( para[f].weight != pb->paraList[i][f].weight)
                equal = false;
        }
        if( equal )
            continue;*/

        if( maxScore < pb->bleuList[i] ){
            maxScore = pb->bleuList[i];
            maxRound = i;
        }
    }

    fprintf(stderr, "\n");

    if( maxRound >= 0){
        for( int i = 1; i < pb->paraLen[maxRound]; i++ )
            finalPara[i].weight = pb->paraList[maxRound][i].weight;    // optimized weight
    }
    else
        fprintf( stderr, "Invalid %s is generated during MER training\n", criterion );

    delete[] intersections;
    delete pb;

    return 0;
}

// Perceptron-based training
int OurTrainer::PeceptronBasedTraining(TrainingSet * ts, ParaInfo * para, ParaInfo * finalPara, 
                                       int paraCount, int nRoundConverged, int ngram, 
                                       BLEU_TYPE BLEUType)
{
    return 0;
}

}

