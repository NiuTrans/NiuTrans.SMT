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
 * training and decoding components for our SMT system; TrainingAndDecoding.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); June 22th, 2011; add function "Decoding()"
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "TrainingAndDecoding.h"
#include "OurDecoder_Phrase_ITG.h"
#include "OurDecoder_SCFG.h"
#include "DataCheck.h"

#ifndef    WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

namespace smt {


////////////////////////////////////////
// system

#ifndef    WIN32
    timeval OurSystem::startT;
#else
    long OurSystem::startT;
#endif

int OurSystem::verbose = 0;

OurSystem::OurSystem(int maxSentNum)
{
    rlists     = NULL;
    rlistLen   = NULL;
    rlistCount = 0;

    if( maxSentNum > 0 ){
        srcSent = new char*[maxSentNum];
        memset(srcSent, 0, sizeof(char*) * maxSentNum);

        prematchedRules = new List*[maxSentNum];
        memset(prematchedRules, 0, sizeof(List*) * maxSentNum);

        viterbiRules = new List*[maxSentNum];
        memset(viterbiRules, 0, sizeof(List*) * maxSentNum);
    }
    else{
        srcSent = NULL;
        prematchedRules = NULL;
        viterbiRules = NULL;
    }

    srcSentCount       = 0;
    sentThreadCount    = NULL;
    numOfThread        = 0;
    failureCount       = 0;
    failureThreadCount = NULL;
    log                = NULL;
    trainingSet        = NULL;
   listenThread       = false;
   breakThread        = false;
   checkInFootprint   = -1;
   checkOutFootprint  = -1;
   footprint          = 0;
   jobCount           = 0;
}

OurSystem::~OurSystem()
{
    Clear();
    delete[] srcSent;
    delete[] prematchedRules;
    delete[] viterbiRules;
    delete[] sentThreadCount;
    delete[] failureThreadCount;
}

void OurSystem::Training(const char * reffn, int decoderType, int trainingMethod, BLEU_TYPE bleuType)
{
    int maxSentNum = ConfigManager.GetInt("maxsentnum", MAX_SENT_NUM);
    int refNum     = ConfigManager.GetInt("nref", 4);
    int NGramBLEU  = ConfigManager.GetInt("bleungram", 4);
    int nthread    = ConfigManager.GetInt("nthread", -1);
    bool forTreeParsing = ConfigManager.GetBool("treeparsing", false);
    const char * alignmentFN = ConfigManager.GetString("anchortuning", NULL);
    const char * premachedRulesFN = ConfigManager.GetString("Prematched-Rule-File", NULL);
    const char * viterbiRulesFN = ConfigManager.GetString("Viterbi-Rule-File", NULL);
    float anchorAlpha = alignmentFN == NULL ? 0 : ConfigManager.GetFloat("anchoralpha", 1);
    OurSystem::verbose = ConfigManager.GetInt("verbose", 0);

    // check the input sentences first
    DataChecker::CheckInputSentences(reffn, true, refNum);

    numOfThread = nthread;

    Model * model = new Model((DECODER_TYPE)decoderType);
    model->Init();

    // load reference sentences
    TrainingSet * ts = new TrainingSet();
    if( bleuType < 3 ){
        ts->LoadRefData(reffn, refNum, NGramBLEU, maxSentNum);
        if(alignmentFN != NULL)
            ts->LoadWordAlignment(alignmentFN, refNum, maxSentNum);
    }
    trainingSet = ts;

    // load pre-matched rules/phrases
    MatchedRuleSet * premachedRuleSet = new MatchedRuleSet();
    if(premachedRulesFN != NULL)
        premachedRuleSet->Load(premachedRulesFN, model, maxSentNum, forTreeParsing, false);

    // load rules/phrases used in viterbi derivation
    ViterbiRuleSet * viterbiRuleSet = new ViterbiRuleSet();
    if(viterbiRulesFN != NULL)
        viterbiRuleSet->Load(viterbiRulesFN, model, maxSentNum, forTreeParsing, true);

    OurTrainer * trainer = new OurTrainer();

    int nround = ConfigManager.GetInt("nround", 5);

    fprintf(stderr, "\n");
    DisplaySetting(stderr, "MER Training", decoderType, model);
    for( int f = 0; f < model->featNum; f++ ){
        ParaInfo p = model->paras[f];
        fprintf( stderr, ">>> %2d: %20s = %6.3f  min = %6.3f  max = %6.3f\n", 
            f, model->GetFeatString(f, (DECODER_TYPE)decoderType, model->CSFMMode), p.weight, p.minValue, p.maxValue);
    }
    
    for( int r = 0; r < nround; r++ ){
        fprintf( stderr, "\n");

        InitForTraining(reffn, ts, premachedRuleSet, viterbiRuleSet, nthread);

        tmpF = fopen("pattern.log", "wb");

        // decode the input sentences to obtain the n-best list
        DecodeInput(model, decoderType, nthread);

        fclose(tmpF);

        for( int i = 0; i < ts->sampleCount; i++ ){
            ts->LoadTrainingData(rlists[i], rlistLen[i], model->featNum, i, true);
        }

        trainer->LoadParaFromModel(model->paras, model->featNum);
        trainer->OptimzieWeightsWithMERT(ts, NGramBLEU, bleuType, anchorAlpha); // optimization

        fprintf(stderr, ">>>> optimized weights:");
        for( int i = 1; i < trainer->paraCount; i++ ){
            model->paras[i - 1].weight = trainer->finalPara[i].weight;
            model->featWeight[i - 1] = model->paras[i - 1].weight;
            fprintf(stderr, " %.6f", model->featWeight[i - 1]);
        }
        fprintf(stderr, "\n");

        //if((DECODER_TYPE)decoderType == SKELETON_BASED)
        //   model->UpdateAssociatedModelFeatureWeights();
    }

    delete ts;
    delete trainer;
    delete premachedRuleSet;
    delete viterbiRuleSet;
    trainingSet = NULL;

    const char * fn = ConfigManager.GetString("output", "nbest.final.mert.txt");
    int    nbest = ConfigManager.GetInt("nbest", 20);
    FILE * file = fopen(fn, "wb");
    FILE * log = fopen( "decoding.log.txt", "w" );

    fprintf(stderr, "dumping the final results ... ");
    // dump the final results
    for( int i = 0; i < rlistCount; i++ ){
        for(int k = 0; k < rlistLen[i] && k < nbest; k++ ){
            fprintf(file, "%s ||||", rlists[i][k].translation);
            for( int f = 0; f < model->featNum; f++ )
                fprintf(file, " %d:%.7f", f + 1, rlists[i][k].featValues[f]);
            fprintf(file, " |||| %.7f", rlists[i][k].modelScore);
            fprintf(file, " |||| %.3f\n", rlists[i][k].anchorLoss);
            fprintf( log, "%s\n", rlists[i][k].log );
        }
        fprintf(file, "================\n");
        fprintf(log, "================\n");
    }
    fprintf(stderr, "[done]\n");

    fclose( log );
    fclose( file );

    
    DumpLog();

    delete model;
}

void OurSystem::Decoding(const char * infn, const char * outfn, const char * logfn, int decoderType, bool serviceMode)
{
    DataChecker::CheckInputSentences(infn, false, 0);

    Model * model = new Model((DECODER_TYPE)decoderType);
    model->Init();

    trainingSet = NULL;

    Decoding(infn, outfn, logfn, model, decoderType, serviceMode);

    delete model;
}

void OurSystem::Decoding(const char * infn, const char * outfn, const char * logfn, Model * model, int decoderType, bool serviceMode)
{
   OurSystem::verbose = ConfigManager.GetInt("verbose", 0);
   int nthread   = ConfigManager.GetInt("nthread", serviceMode ? 1 : -1);
    bool oneBestOnly = ConfigManager.GetBool("onebestonly", false) || ConfigManager.GetBool("1bestonly", false);
    numOfThread = nthread;

    // decode the input sentences to obtain the n-best list
   if(!serviceMode){
      InitForDecoding(infn, nthread);
      DecodeInput(model, decoderType, nthread);
   }
   else{
      ReadTestSentences(infn);
      rlistCount = srcSentCount;
      footprint = ++footprint % 1000000;
      DecodeInputInService(runningThreads, model, decoderType, nthread, footprint);
   }


    int    nbest = ConfigManager.GetInt("nbest", 20);
    FILE * file = stdout;
    if( outfn != NULL )
        file = fopen(outfn, "wb");
    FILE * log = fopen( logfn != NULL ? logfn : "decoding.log.txt", "w" );
    fprintf(stderr, "\n");
    fprintf(stderr, "dumping the decoding results ... ");

    // dump the final results
    for( int i = 0; i < rlistCount; i++ ){
        if(oneBestOnly){
            if( rlistLen[ i ] > 0 ){
             fprintf( file, "%s\n", rlists[ i ][ 0 ].translation );
             fprintf( log, "%s\n", rlists[ i ][ 0 ].log );
          }
          else{
             fprintf( file, "\n" );
             fprintf( log, "================\n" );
          }
        }
        else{
            for(int k = 0; k < rlistLen[i] && k < nbest; k++ ){
                fprintf(file, "%s\n", rlists[i][k].translation);
                fprintf(log, "%s\n", rlists[i][k].log );
            }

            if(rlistLen[i] == 0)
                fprintf(file, "\n");

            if( nbest > 1 ) {
                fprintf(file, "================\n");
            }
            fprintf(log, "================\n");
        }
    }
    fprintf(stderr, "[done]\n");
    fclose( log );
    fclose( file );

   if(!serviceMode)
      Clear();
   else
      ClearForServiceMode();
}

void OurSystem::InitForDecoding(const char * testfn, int threadNum)
{
    InitBasicInformation(threadNum);
    
   ReadTestSentences(testfn);

   rlistCount = srcSentCount;
}

void OurSystem::InitForTraining(const char * reffn, TrainingSet * ts, 
                                MatchedRuleSet * myPrematchedRules, MatchedRuleSet * myViterbiRules, int threadNum)
{
    int    refNum = ConfigManager.GetInt("nref", 4);
    char   line[MAX_LINE_LENGTH];
    int    count = 0;

    Clear();
    rlists = new TransResult*[ts->sampleCount];
    rlistLen = new int[ts->sampleCount];
    log = new DecodingLoger*[MAX_SENT_NUM];
    rlistCount = ts->sampleCount;
    
    for( int i = 0; i < srcSentCount; i++ ){
        if( srcSent && srcSent[i] != NULL ){
            delete[] srcSent[i];
            srcSent[i] = 0;
        }

        if( prematchedRules != NULL )
            prematchedRules[i] = NULL;

        if( viterbiRules != NULL )
            viterbiRules[i] = NULL;
    }

    if( threadNum > 0 ){
        delete[] sentThreadCount;
        delete[] failureThreadCount;
        sentThreadCount = new int[threadNum];
        memset(sentThreadCount, 0, sizeof(int) * threadNum);
        failureThreadCount = new int[threadNum];
        memset(failureThreadCount, 0, sizeof(int) * threadNum);
    }
    
    FILE * file = fopen(reffn, "rb");

    if( file == NULL ){
        fprintf(stderr, "ERROR: cannot open file \"%s\"\n", reffn);
        exit(-1);
    }

    srcSentCount = 0;
    while(fgets(line, MAX_LINE_LENGTH - 1, file)){
        if( count++ % (refNum + 2) != 0 )
            continue;

        StringUtil::TrimRight(line);
        srcSent[srcSentCount] = new char[(int)strlen(line) + 1];
        strcpy(srcSent[srcSentCount], line);

        if(myPrematchedRules != NULL)
            prematchedRules[srcSentCount] = myPrematchedRules->GetRules(srcSentCount);

        if(myViterbiRules != NULL)
            viterbiRules[srcSentCount] = myViterbiRules->GetRules(srcSentCount);

        srcSentCount++;

        if(srcSentCount >= ts->sampleCount)
            break;
    }

    fclose(file);
}

void OurSystem::InitForDecodingForServicecMode(int threadNum)
{
   InitBasicInformation(threadNum);
}

void OurSystem::ReadTestSentences(const char * testfn)
{
   char line[MAX_LINE_LENGTH/4] = "";

   /* load test sentences */
    FILE * file = fopen(testfn, "rb");
    if( file == NULL ){
        fprintf(stderr, "ERROR: cannot open file \"%s\"\n", testfn);
        exit(-1);
    }
    srcSentCount = 0;
    while(fgets(line, MAX_LINE_LENGTH - 1, file)){
        StringUtil::TrimRight(line);
      if(srcSent[srcSentCount] != NULL)
         delete[] srcSent[srcSentCount];
      srcSent[srcSentCount] = new char[(int)strlen(line) + 1];
        strcpy(srcSent[srcSentCount], line);
        srcSentCount++;
    }
    fclose(file);
}

void OurSystem::InitBasicInformation(int threadNum)
{
   if( threadNum > 0 ){
        sentThreadCount = new int[threadNum];
        memset(sentThreadCount, 0, sizeof(int) * threadNum);
        failureThreadCount = new int[threadNum];
        memset(failureThreadCount, 0, sizeof(int) * threadNum);
    }
    
    /* data structure to store translation results */
    rlists = new TransResult*[MAX_SENT_NUM];
    rlistLen = new int[MAX_SENT_NUM];
    log = new DecodingLoger*[MAX_SENT_NUM];

   OurSystem::verbose = ConfigManager.GetInt("verbose", 0);
}

void OurSystem::Clear()
{
    if( rlists != NULL ){
        for( int i = 0; i < rlistCount; i++ ){
            for(int j = 0; j < rlistLen[i]; j++ ){
                WordTranslation * wt = (WordTranslation *)rlists[i][j].wordTranslation;
                if(wt != NULL)
                    delete wt;
            }
            delete[] rlists[i];
        }
        delete[] rlists;
        delete[] rlistLen;
        rlists   = NULL;
        rlistLen = NULL;
        rlistCount = 0;
    }

    if(log != NULL){
        for( int i = 0; i < rlistCount; i++ ){
            delete log[i];
            log[i] = NULL;
        }
        delete[] log;
        log = NULL;
    }

    if( srcSent != NULL ){
        for( int i = 0; i < srcSentCount; i++ ){
            if( srcSent != NULL && srcSent[i] != NULL ){
                delete[] srcSent[i];
                srcSent[i] = NULL;
            }
        }
        memset(srcSent, 0, sizeof(char*) * MAX_SENT_NUM);
    }

    if( prematchedRules != NULL )
        memset(prematchedRules, 0, sizeof(List *) * MAX_SENT_NUM);

    if( viterbiRules != NULL )
        memset(viterbiRules, 0, sizeof(List *) * MAX_SENT_NUM);

    if( sentThreadCount != NULL )
        memset(sentThreadCount, 0, sizeof(int) * numOfThread);

    if( failureThreadCount != NULL )
        memset(failureThreadCount, 0, sizeof(int) * numOfThread);

    srcSentCount = 0;
    failureCount = 0;
}

void OurSystem::ClearForServiceMode()
{
   if( rlists != NULL ){
        for( int i = 0; i < rlistCount; i++ ){
            for(int j = 0; j < rlistLen[i]; j++ ){
                WordTranslation * wt = (WordTranslation *)rlists[i][j].wordTranslation;
                if(wt != NULL)
                    delete wt;
            }
            delete[] rlists[i];
        }
        rlistCount = 0;
    }

    if(log != NULL){
        for( int i = 0; i < rlistCount; i++ ){
            delete log[i];
            log[i] = NULL;
        }
    }

    if( srcSent != NULL ){
        for( int i = 0; i < srcSentCount; i++ ){
            if( srcSent != NULL && srcSent[i] != NULL ){
                delete[] srcSent[i];
                srcSent[i] = NULL;
            }
        }
        memset(srcSent, 0, sizeof(char*) * MAX_SENT_NUM);
    }

    if( prematchedRules != NULL )
        memset(prematchedRules, 0, sizeof(List *) * MAX_SENT_NUM);

    if( viterbiRules != NULL )
        memset(viterbiRules, 0, sizeof(List *) * MAX_SENT_NUM);

    if( sentThreadCount != NULL )
        memset(sentThreadCount, 0, sizeof(int) * numOfThread);

    if( failureThreadCount != NULL )
        memset(failureThreadCount, 0, sizeof(int) * numOfThread);

    srcSentCount = 0;
    failureCount = 0;
}

#ifdef HAVE_PTHREAD_H
    void OurSystem::DecodeInput(Model * model, int decoderType, int threadId, int threadNum, bool listen, pthread_t * hnd)
#else
#ifdef _WIN32
    void OurSystem::DecodeInput(Model * model, int decoderType, int threadId, int threadNum, bool listen, HANDLE * hnd)
#endif
#endif
{
    int  refNum = ConfigManager.GetInt("nref", 4);
    int  nbest = ConfigManager.GetInt("nbest", 256);
    int  count = 0, sentCount = 0;
    bool dumpLog = ConfigManager.GetBool("dumpdefeatedviterbirule", false) || 
                   ConfigManager.GetBool("dumpusedrule", false) || 
                   ConfigManager.GetBool("dumprule", false);
    TransResult ** resultBase = new TransResult*[MAX_SENT_NUM];
    int * resultLenBase = new int[MAX_SENT_NUM];
    DecodingLoger ** logBase = new DecodingLoger*[MAX_SENT_NUM];
    DecodingSentence * sent = new DecodingSentence();

    BaseDecoder * decoder = NULL;
    if( decoderType == PHRASE_BASED){
        decoder = new Decoder_Phrase_ITG();
        decoder->Init(model);
    }
    else if(decoderType == HIERO || decoderType == SYNTAX_BASED){
        decoder = new Decoder_SCFG((DECODER_TYPE)decoderType);
        decoder->Init(model);
    }
   else if(decoderType == SKELETON_BASED){
      decoder = new Decoder_Skeleton();
      decoder->Init(model);
   }

#ifndef    WIN32
//    timeval startT, endT;
//    gettimeofday (&startT, NULL);
    timeval endT;
#else
//    long startT, endT;
//    startT = clock();
    long endT;
#endif

   while(1){
      if(listen){
         if(verbose > 0)
            fprintf(stderr, "suspend thread %ld\n", *hnd);

#ifdef HAVE_PTHREAD_H
         pthread_mutex_lock(&mutex_);
         while(jobCount == 0){
            pthread_cond_wait(&cond_, &mutex_);// this function unlock the mutex first
                                               // and then wait
         }
#else
#ifdef _WIN32
         SuspendThread(*hnd);
#endif
#endif
         if(breakThread)
            break;
      }

      if(verbose > 0)
         fprintf(stderr, "return from thread %ld\n", *hnd);

      for(int i = 0; i < srcSentCount; i++ ){
         if( threadId >= 0 && i % threadNum != threadId)
            continue;

         memset(sent, 0, sizeof(DecodingSentence));
         sent->string = srcSent[i];
         sent->matchedRules = prematchedRules[i];
         sent->viterbiRules = viterbiRules[i];
         sent->refs = trainingSet != NULL ? trainingSet->GetRef(i) : NULL;
         sent->id = i;

         decoder->DecodeInput(sent); // decode the input sentence

         TransResult * result = new TransResult[nbest];
         DecodingLoger * curLog = dumpLog ? new DecodingLoger() : NULL;
         int resultCount = decoder->DumpTransResult(nbest, result, curLog);
         resultBase[i] = result;
         resultLenBase[i] = resultCount;
         logBase[i] = curLog;
         if(resultCount == 0 || decoder->DoesDecodingFail())
            failureCount++;

         sentCount++;
         if( threadId >= 0 ){
            sentThreadCount[threadId]++;
            if(resultCount == 0 || decoder->DoesDecodingFail())
               failureThreadCount[threadId]++;
         }

   #ifndef    WIN32
         gettimeofday (&endT, NULL);
         double time = ((double)(endT.tv_sec - startT.tv_sec) * 1000000 + (double)(endT.tv_usec - startT.tv_usec))/1000000;
   #else
         endT = clock();
         double time = (double)(endT - startT)/CLOCKS_PER_SEC;
   #endif
         int count = sentCount;
         int failure = failureCount;
         if( threadNum > 0 ){
            count = 0;
            failure = 0;
            for( int i = 0; i < threadNum; i++ ){
               count += sentThreadCount[i];
               failure += failureThreadCount[i];
            }
         }

         fprintf(stderr, "\rsentence #%d thread %d [elapsed: %.2fs, speed: %.2fsent/s, failure: %d] ", count, threadId, time, count/time, failure);
      }

      for( int i = 0; i < srcSentCount; i++ ){
         if( threadId >= 0 && i % threadNum != threadId)
            continue;
         rlists[i] = resultBase[i];
         rlistLen[i] = resultLenBase[i];
         log[i] = logBase[i];
      }

      checkOutFootprint = checkInFootprint;

#ifdef HAVE_PTHREAD_H
      if(listen){
         jobCount--;
         pthread_mutex_unlock(&mutex_);
      }
#endif

      if(!listen)
         break;
   }

    delete[] resultBase;
    delete[] resultLenBase;
    delete[] logBase;
    delete decoder;
    delete sent;

}

void OurSystem::DecodeInput(Model * model, int decoderType, int threadNum)
{
    if( threadNum > 32 )
        threadNum = 32;

#ifndef    WIN32
    gettimeofday (&startT, NULL);
#else
    startT = clock();
#endif
    
    if( threadNum <= 0 ){
        DecodeInput(model, decoderType, -1, threadNum, false, NULL);
        return;
    }

    OurSystem ** threads = new OurSystem*[threadNum];
    for( int t = 0; t < threadNum; t++ ){
        threads[t] = new OurSystem(-1);
        memcpy(threads[t], this, sizeof(OurSystem));
        threads[t]->m = model;
        threads[t]->did = decoderType;
        threads[t]->tid = t;
        threads[t]->nthread = threadNum;
    }

#ifndef    WIN32
    gettimeofday (&startT, NULL);
#else
    startT = clock();
#endif

    for( int t = 0; t < threadNum; t++ )
        threads[t]->Start();
    for( int t = 0; t < threadNum; t++ )
        threads[t]->Join();

    for( int t = 0; t < threadNum; t++ ){
        failureCount += threads[t]->failureCount;
        threads[t]->rlists = NULL;
        threads[t]->rlistLen = NULL;
        threads[t]->srcSent = NULL;
        threads[t]->prematchedRules = NULL;
        threads[t]->viterbiRules = NULL;
        threads[t]->sentThreadCount = NULL;
        threads[t]->failureThreadCount = NULL;
        threads[t]->log = NULL;
        delete threads[t];
    }
    delete[] threads;
}

void OurSystem::DecodeInputInService(OurSystem ** threads, Model * model, int decoderId, int threadNum, int CurFootprint)
{
#ifndef    WIN32
    gettimeofday (&startT, NULL);
#else
    startT = clock();
#endif

      for( int t = 0; t < threadNum; t++ ){

        // copy neccessary information
        threads[t]->rlists = this->rlists;
        threads[t]->rlistLen = this->rlistLen;
        threads[t]->srcSent = this->srcSent;
        threads[t]->srcSentCount = this->srcSentCount;
        threads[t]->prematchedRules = this->prematchedRules;
        threads[t]->viterbiRules = this->viterbiRules;
        threads[t]->sentThreadCount = this->sentThreadCount;
        threads[t]->numOfThread = this->numOfThread;
        threads[t]->failureCount = this->failureCount;
        threads[t]->failureThreadCount = this->failureThreadCount;
      threads[t]->log = this->log;
      threads[t]->verbose = this->verbose;

      threads[t]->checkInFootprint = CurFootprint;
      if(verbose > 0)
         fprintf(stderr, "resume thread %ld\n", threads[t]->hnd_);

#ifndef    WIN32

      pthread_mutex_lock(&threads[t]->mutex_);
      threads[t]->jobCount++;
      pthread_mutex_unlock(&threads[t]->mutex_);
      pthread_cond_broadcast(&threads[t]->cond_);
      //pthread_cond_signal(&threads[t]->cond_);
#else
      ResumeThread(threads[t]->hnd_);
#endif
   }

   for( int t = 0; t < threadNum; t++){
      while(threads[t]->checkInFootprint != threads[t]->checkOutFootprint){
#ifndef    WIN32
#else
         Sleep(100);
#endif
      }
   }
}

void OurSystem::RunThreads(Model * model, int decoderType, int &threadNum)
{
   if(threadNum < 1){
      fprintf(stderr, "ERROR: no threads are running (%d < 1)!!!\n", threadNum);
      return;
   }

   fprintf(stderr, "creating threads for server mode(it may take %d seconds)\n", threadNum);
   InitForDecodingForServicecMode(threadNum);

   OurSystem ** threads = CreateThreadPool(model, decoderType, threadNum);
   runningThreads = threads;

   for( int t = 0; t < threadNum; t++ ){
#ifndef WIN32
      pthread_mutex_init(&threads[t]->mutex_, NULL);
      pthread_cond_init(&threads[t]->cond_, NULL);
#endif
        threads[t]->listenThread = true;
        threads[t]->Start();
      if(verbose > 0)
         fprintf(stderr, "start thread %ld\n", threads[t]->hnd_);
    }
#ifndef    WIN32
   sleep(threadNum);
#else
   Sleep(1000 * threadNum);
#endif

   if(verbose > 0)
      fprintf(stderr, "waited for %d seconds for creating the threads\n", threadNum);
}

void OurSystem::KillThreads(int threadNum)
{
#ifndef    WIN32
   for( int t = 0; t < threadNum; t++ ){
        runningThreads[t]->breakThread = true;
      pthread_cond_signal(&runningThreads[t]->cond_);
    }
#else
   for( int t = 0; t < threadNum; t++ ){
        runningThreads[t]->breakThread = true;
      ResumeThread(runningThreads[t]->hnd_);
    }
#endif
   FreeThreadPool(runningThreads, threadNum);
}

OurSystem ** OurSystem::CreateThreadPool(Model * model, int decoderType, int &threadNum)
{
   OurSystem ** threads = new OurSystem*[threadNum];
    for( int t = 0; t < threadNum; t++ ){
        threads[t] = new OurSystem(-1);
        memcpy(threads[t], this, sizeof(OurSystem));
        threads[t]->m = model;
        threads[t]->did = decoderType;
        threads[t]->tid = t;
        threads[t]->nthread = threadNum;
    }
   return threads;
}

void OurSystem::FreeThreadPool(OurSystem ** threads, int threadNum)
{
   for( int t = 0; t < threadNum; t++ ){
        failureCount += threads[t]->failureCount;
        threads[t]->rlists = NULL;
        threads[t]->rlistLen = NULL;
        threads[t]->srcSent = NULL;
        threads[t]->prematchedRules = NULL;
        threads[t]->viterbiRules = NULL;
        threads[t]->sentThreadCount = NULL;
        threads[t]->failureThreadCount = NULL;
        threads[t]->log = NULL;
        delete threads[t];
    }
    delete[] threads;
   threads = NULL;
}

void OurSystem::Run()
{
    DecodeInput(m, did, tid, nthread, listenThread, &hnd_);
}

void OurSystem::DumpLog()
{
    FILE * file = NULL;

    if(ConfigManager.GetBool("dumpdefeatedviterbirule", false)){
        const char * vfn = ConfigManager.GetString("Defeated-Viterbi-Rule-File", "defeated.viterbi.rule.txt");
        file = fopen(vfn, "w" );

        fprintf(stderr, "dumping defeated Viterbi rules ... ");
        for( int i = 0; i < rlistCount; i++ ){
            fprintf(file, "sentence: %d\n", i + 1);
            fprintf(file, "%s", log[i]->defeatedViterbiLog);
            fprintf(file, "================\n");
        }
        fprintf(stderr, "[done]\n");

        fclose(file);
    }

    if(ConfigManager.GetBool("dumpusedrule", false) || ConfigManager.GetBool("dumprule", false)){
        const char * rfn = ConfigManager.GetString("Used-Rule-File", "used.rule.txt");
        file = fopen(rfn, "w" );

        fprintf(stderr, "dumping rules used in decoding ... ");
        for( int i = 0; i < rlistCount; i++ ){
            fprintf(file, "sentence: %d\n", i + 1);
            fprintf(file, "%s", log[i]->usedRuleLog);
            fprintf(file, "================\n");
        }
        fprintf(stderr, "[done]\n");

        fclose(file);
    }
}

void OurSystem::DisplaySetting(FILE * file, const char * head, int decoderId, Model * model)
{
    int nthread    = ConfigManager.GetInt("nthread", -1);
    int nround = ConfigManager.GetInt("nround", 5);
    char * modelType = DECODER_NAME[decoderId];

    fprintf(file, "%s (round = %d, nthread = %d, model = %s", 
            head, nround, nthread > 1 ? nthread : 1, modelType);

    if(!strcmp(model->GetCSFMInfo(), "yes"))
        fprintf(file, ", csfm = yes");

    if(decoderId == 2){
        if(ConfigManager.GetBool("tree2tree", false))
            fprintf(file, ", tree2tree = yes");
        else if(ConfigManager.GetBool("tree2string", false))
            fprintf(file, ", tree2string = yes");
        else
            fprintf(file, ", string2tree = yes");
    }

    if(ConfigManager.GetBool("forceddecoding", false) || ConfigManager.GetBool("forced", false))
        fprintf(file, ", forced = yes");

    if(ConfigManager.GetBool("lossaugumenteddecoding", false) || ConfigManager.GetBool("lossaug", false))
        fprintf(file, ", lossaug = yes");

    if(GlobalVar::internationalTokenization)
        fprintf(stderr, ", token=international");

    if(ConfigManager.GetBool("charngramlm", false)){
        fprintf(stderr, ", ngram=%d, charngram=%d", 
                           ConfigManager.GetInt("ngram", 3), 
                           ConfigManager.GetInt("ngram2", 3));
    }

    fprintf(file, ") ... \n");
}

}

