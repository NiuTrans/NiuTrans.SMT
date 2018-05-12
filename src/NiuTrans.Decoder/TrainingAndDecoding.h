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
 * training and decoding components for our SMT system; TrainingAndDecoding.h
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


#ifndef _TRAININGANDDECODING_H_
#define _TRAININGANDDECODING_H_

#include <stdio.h>
#include <stdlib.h>
#include "OurInputSentence.h"
#include "OurDecoder_Phrase_ITG.h"
#include "OurDecoder_SCFG.h"
#include "OurDecoder_Skeleton.h"
#include "OurTrainer.h"

#ifndef WIN32
#define HAVE_PTHREAD_H // for linux
#endif

//////////////////////////////////////////////////
// parts of the below code are copied from CRF++
#ifdef HAVE_PTHREAD_H
#include <pthread.h> // use "-lpthread" when compiling on linux systems
#else
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif
#endif

#if defined HAVE_PTHREAD_H
#define CRFPP_USE_THREAD 1
#endif

#if(defined(_WIN32) && !defined (__CYGWIN__))
#define CRFPP_USE_THREAD 1
#define BEGINTHREAD(src, stack, func, arg, flag, id) \
     (HANDLE)_beginthreadex((void *)(src), (unsigned)(stack), \
                       (unsigned(_stdcall *)(void *))(func), (void *)(arg), \
                       (unsigned)(flag), (unsigned *)(id))
#endif

namespace smt {


class DecodingThread {

protected:
#ifdef HAVE_PTHREAD_H
    pthread_t hnd_;
	pthread_mutex_t mutex_;
	pthread_cond_t cond_;
#else
#ifdef _WIN32
    HANDLE  hnd_;
#endif
#endif

public:
    Model * m;
    int     did;
    int     tid;
    int     nthread;

public:
    DecodingThread(){};

public:
    static void* Wrapper(void *ptr) 
    {
        DecodingThread *p = (DecodingThread*)ptr;
        p->Run();
        return 0;
    }

    virtual void Run() {}

    void Start() 
    {
#ifdef HAVE_PTHREAD_H
      pthread_create(&hnd_, 0, &DecodingThread::Wrapper,
                     static_cast<void *>(this));

#else
#ifdef _WIN32
      DWORD id;
      hnd_ = BEGINTHREAD(0, 0, &DecodingThread::Wrapper, this, 0, &id);
#else
      Run();
#endif
#endif
    }

    void Join() 
    {
#ifdef HAVE_PTHREAD_H
      pthread_join(hnd_, 0);
#else
#ifdef _WIN32
      WaitForSingleObject(hnd_, INFINITE);
      CloseHandle(hnd_);
#endif
#endif
    }

    virtual ~DecodingThread() {}
};


////////////////////////////////////////
// system
class OurSystem : public DecodingThread
{
protected:
    TransResult **   rlists;
    int *            rlistLen;
    int              rlistCount;
    char **          srcSent;
    int              srcSentCount;
    List **          prematchedRules;
    List **          viterbiRules;
    int *            sentThreadCount;
    int              numOfThread;
    int              failureCount;
    int *            failureThreadCount;
    DecodingLoger ** log;
    TrainingSet *    trainingSet;
	bool             listenThread;
	bool             breakThread;
	int              checkInFootprint;
	int              checkOutFootprint;
	int              footprint;
	OurSystem **     runningThreads;
	int              jobCount;

public:
	static int verbose;

#ifndef    WIN32
    static timeval startT;
#else
    static long startT;
#endif

public:
    OurSystem(int maxSentNum = MAX_SENT_NUM);
    ~OurSystem();

    void Training(const char * reffn, int decoderId, int trainingMethod, BLEU_TYPE bleuType);
    void Decoding(const char * infn, const char * outfn, const char * logfn, int decoderType, bool serviceMode);
    void Decoding(const char * infn, const char * outfn, const char * logfn, Model * model, int decoderType, bool serviceMode);
    void InitForTraining(const char * reffn, TrainingSet * ts, MatchedRuleSet * myPrematchedRules, MatchedRuleSet * myViterbiRules, int threadNum);
    void InitForDecoding(const char * testfn, int threadNum);
	void InitForDecodingForServicecMode(int threadNum);
	void ReadTestSentences(const char * testfn);
	void InitBasicInformation(int threadNum);
    void Run();
    void DumpLog();
    void DisplaySetting(FILE * file, const char * head, int decoderId, Model * model);

protected:
    void Clear();
	void ClearForServiceMode();

#ifdef HAVE_PTHREAD_H
    void DecodeInput(Model * model, int decoderType, int threadId, int threadNum, bool listen, pthread_t * hnd);
#else
#ifdef _WIN32
    void DecodeInput(Model * model, int decoderType, int threadId, int threadNum, bool listen, HANDLE * hnd);
#endif
#endif

    //void DecodeInput(Model * model, int decoderId, int threadId, int threadNum);
    void DecodeInput(Model * model, int decoderId, int threadNum);
	void DecodeInputInService(OurSystem ** threads, Model * model, int decoderId, int threadNum, int CurFootprint);

public:
    void RunThreads(Model * model, int decoderType, int &threadNum);
	void KillThreads(int threadNum);
	OurSystem ** CreateThreadPool(Model * model, int decoderType, int &threadNum);
	void FreeThreadPool(OurSystem ** threads, int threadNum);
    
};


}

#endif

