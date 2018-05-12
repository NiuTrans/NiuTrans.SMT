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
 * Export functions; Interface.cpp
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

#include "Global.h"
#include "OurLM.h"
#include "OurDecoder_Phrase_ITG.h"
#include "OurDecoder_SCFG.h"
#include "OurTrainer.h"
#include "TrainingAndDecoding.h"
#include "Interface.h"


namespace smt {

OurTrainer * trainer = new OurTrainer();
OurSystem * smtSystem = NULL;
Model * smtModel = NULL;
DECODER_TYPE dtype = PHRASE_BASED;
bool serverMode = false;
int threadNum = 1;

bool MERTraining(const char * reffn, const char * trainfn, const char * configfn, int ngram, int bleuType)
{
    TrainingSet * ts = new TrainingSet();
    if( bleuType < 3 )
        ts->LoadRefData(reffn, 4, ngram, MAX_SENT_NUM);
    ts->LoadTrainingData(trainfn, false);
    trainer->LoadPara(configfn);
    trainer->OptimzieWeightsWithMERT(ts, ngram, (BLEU_TYPE)bleuType, 0);
    delete ts;
    return true;
}

float CalculateBLEU(const char * reffn, const char * trainfn, int refNum, int ngram, int bleuType)
{
	TrainingSet * ts = new TrainingSet();
    if( bleuType < 3 )
        ts->LoadRefData(reffn, refNum, ngram, MAX_SENT_NUM);
    ts->LoadTrainingData(trainfn, false);
	float BLEU = trainer->GetScore(ts, ngram, (BLEU_TYPE)bleuType, 0);
	delete ts;
	return BLEU;
}

float CalculateBP(const char * reffn, const char * trainfn, int refNum, int ngram, int bleuType)
{
    TrainingSet * ts = new TrainingSet();
    if( bleuType < 3 )
        ts->LoadRefData(reffn, refNum, ngram, MAX_SENT_NUM);
    ts->LoadTrainingData(trainfn, false);
    float BP = trainer->GetBP(ts, ngram, (BLEU_TYPE)bleuType, 0);
    delete ts;
    return BP;
}

void UnloadMERTrainer()
{
    delete trainer;
}

void SetNormalizationLabel(bool label)
{
    GlobalVar::normalizeText = label;
}

void SetUseF1Criterion(bool use)
{
    GlobalVar::SetUseF1Criterion(use);
}

void SetInternationalTokenization(bool token)
{
    GlobalVar::SetInternationalTokenization(token);
}

void LoadSMTModel(const char * arg)
{
    char ** tmp;
    int argc = (int)StringUtil::Split(arg, " ", tmp);
    const char ** argv = NULL;

    argv = new const char*[argc];
    for(int i = 0; i < argc; i++)
        argv[i] = tmp[i];

    ConfigManager.Create(argc, argv); // load arguments

    for(int i = 0; i < argc; i++)
        delete[] tmp[i];
    delete[] tmp;
    delete[] argv;

    if(ConfigManager.GetBool("scfg", false)){
        if(ConfigManager.GetBool("syntax", false))
            dtype = SYNTAX_BASED;
        else
            dtype = HIERO;
    }
    else
        dtype = PHRASE_BASED;

    SetUseF1Criterion( false );
    SetNormalizationLabel( true );
    smtSystem = new OurSystem();

    smtModel = new Model((DECODER_TYPE)dtype);
    smtModel->Init();

	if(ConfigManager.GetBool("server", false) || ConfigManager.GetBool("servermode", false)){
		serverMode = true;
		threadNum = ConfigManager.GetInt("nthread", 1);
		CreateSMTService(threadNum);
	}
}

void UnloadSMTModel()
{
    delete smtSystem;
    smtSystem = NULL;

    delete smtModel;
    smtModel = NULL;

    ConfigManager.Destroy();
}

void LoadConfig(const char * configfn)
{
    ConfigManager.LoadArgsFromConfig(configfn);

    if(ConfigManager.GetBool("scfg", false)){
        if(ConfigManager.GetBool("syntax", false))
            dtype = SYNTAX_BASED;
        else
            dtype = HIERO;
    }
    else
        dtype = PHRASE_BASED;

    if(smtModel != NULL){
        smtModel->InitFeatNum();
        smtModel->InitForDecoding();
        smtModel->LoadPara();
        smtModel->InitNTProb();
        smtModel->LoadNESymbol();
    }

    fprintf(stderr, "Reload config file from %s\n", configfn);
    fprintf(stderr, "weights:");
    for(int i = 0; i < smtModel->featNum; i++){
        fprintf(stderr, "%.4f", smtModel->featWeight[i]);
    }
    fprintf(stderr, "\n");
}

void Decode(const char * infn, const char * outfn, const char * logfn)
{

#ifndef    WIN32
	timeval startT, endT;
    gettimeofday (&startT, NULL);
#else
	long startT, endT;
    startT = clock();
#endif

	if(serverMode)
		smtSystem->Decoding(infn, outfn, logfn, smtModel, dtype, true);
	else
		smtSystem->Decoding(infn, outfn, logfn, smtModel, dtype, false);

#ifndef    WIN32
	gettimeofday (&endT, NULL);
	double time = ((double)(endT.tv_sec - startT.tv_sec) * 1000000 + (double)(endT.tv_usec - startT.tv_usec))/1000000;
#else
	endT = clock();
	double time = (double)(endT - startT)/CLOCKS_PER_SEC;
#endif

	fprintf(stderr, "translation time: %.3fs\n", time);
}

void Decode(const char * infn, const char * outfn, const char * logfn, const char * configfn)
{
    LoadConfig(configfn);
    Decode(infn, outfn, logfn);
}

void CreateSMTService(int threadNum)
{
    smtSystem->RunThreads(smtModel, dtype, threadNum);
}

void DecodeInService(const char * infn, const char * outfn, const char * logfn)
{
#ifndef    WIN32
	timeval startT, endT;
    gettimeofday (&startT, NULL);
#else
	long startT, endT;
    startT = clock();
#endif

	smtSystem->Decoding(infn, outfn, logfn, smtModel, dtype, true);

#ifndef    WIN32
	gettimeofday (&endT, NULL);
	double time = ((double)(endT.tv_sec - startT.tv_sec) * 1000000 + (double)(endT.tv_usec - startT.tv_usec))/1000000;
#else
	endT = clock();
	double time = (double)(endT - startT)/CLOCKS_PER_SEC;
#endif

	fprintf(stderr, "translation time: %.3fs\n", time);
}

void DecodeInService(const char * infn, const char * outfn, const char * logfn, const char * configfn)
{
	LoadConfig(configfn);
    DecodeInService(infn, outfn, logfn);
}

}

