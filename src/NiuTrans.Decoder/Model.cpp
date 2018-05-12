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
 * SMT System's Model; Model.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn) May 27, 2013, bug fixing
 * Hao Zhang (email: zhanghao1216@gmail.com); June 24th, 2011; fix a bug for "word deletion" in function "PhraseTable::ParseTransOption"
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn); June 19th, 2011
 * Hao Zhang (email: zhanghao1216@gmail.com); April 13th, 2011;
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "Model.h"

#ifndef    WIN32
#include <sys/time.h>
#endif

namespace smt {

////////////////////////////////////////
// resources required by our decoder 

int Model::TableFeatBeg = 2;
int Model::MaxTableFeatNum = 7;
int Model::TableFeatBegCoarse = 0;
int Model::TableFeatEndCoarse = 0;

int Model::MinFeatNum = 17;
int Model::MainTableBeg = 2;
int Model::MainTableEnd = 7;

int Model::RuleSoringDimen = 2;

int Model::CSFMSurrogateRuleCount  = 0;
int Model::CSFMMatchedNTCount      = 0;
int Model::CSFMMatchedSrcNTCount   = 0;
int Model::CSFMMatchedTgtNTCount   = 0;
int Model::CSFMUnmatchedNTCount    = 0;
int Model::CSFMUnmatchedSrcNTCount = 0;
int Model::CSFMUnmatchedTgtNTCount = 0;

int Model::associatedModelFeatBeg  = 17;

int MAX_WORD_NUM = 160;

Model::Model(DECODER_TYPE type, Configer * c)
{
    this->type = type;
    configer   = c;
    featWeight = NULL;
    paras      = NULL;
    CSFMMode   = configer->GetBool("coarsesearchfinemodeling", false);
    useCharLM  = configer->GetBool("charngramlm", false);
    skeletonModel     = NULL;
    assistantModel    = NULL;
    skeletonConfiger  = NULL;
    assistantConfiger = NULL;

    InitFeatNum();

    pTable       = NULL;
    MEReModel    = NULL;
    msdReModel   = NULL;
    SCFG         = NULL;

    if(!configer->GetBool("skeleton", false)){ // when the model is not used in skeleton translatoin 
                                               // (more details can be found in OurDecoder_Skeletion.h/cpp)
        if(type == PHRASE_BASED){
            pTable       = new PhraseTable(MathUtil::NextPrime(MILLION * 10), featNum, tableFeatNum, type, NULL, configer);
            MEReModel    = new MEReorderModel(configer);
            msdReModel   = new MSDReorderModel(false, configer);
        }
        else{
            if(!CSFMMode)
                SCFG     = new SynchronousGrammar(MathUtil::NextPrime(MILLION * 50), featNum, tableFeatNum, this, type, NULL, configer);
            else
                SCFG     = NULL;
        }
    }

    ngramLM        = NULL;
    ngramLM2       = NULL;
    vocabManager   = NULL; // just a pointer
    NE2NT          = NULL;
    NE2NTFineModel = NULL;
    fineSCFG       = NULL;
    NTProb         = NULL;
    xtsearch       = false;
    associatedModels  = NULL;
    associatedConfigs = NULL;
}

Model::~Model()
{
    if(configer->GetBool("skeleton", false)){
        pTable       = NULL;
        MEReModel    = NULL;
        msdReModel   = NULL;
        SCFG         = NULL;
    }

    delete pTable;
    delete MEReModel;
    delete ngramLM;
    delete ngramLM2;
    delete[] featWeight;
    delete[] paras;
    delete msdReModel;
    delete SCFG;
    delete NE2NT;
    delete NTProb;

    if(configer->GetString("Punct-Vocab-File", NULL) != NULL)
        StringUtil::UnloadPunctDict();

    if(associatedModels != NULL){
        for(int i = 0; i < associatedModels->count; i++){
            Model * m = (Model*)associatedModels->GetItem(i);
            //delete m;
        }
        delete associatedModels;
    }
    if(associatedConfigs != NULL){
        for(int i = 0; i < associatedConfigs->count; i++){
            Configer * c = (Configer*)associatedConfigs->GetItem(i);
            c->Destroy();
            delete c;
        }
        delete associatedConfigs;
    }
}

void Model::InitFeatNum()
{
    int freeFeatNum;
    
    featNum = 0;
    TableFeatBegCoarse = 0;
    TableFeatEndCoarse = 0;
    tableFeatNumPhraseBased = 6;
    tableFeatNumSyntaxBased = 11;

    if(type == PHRASE_BASED || type == HIERO){
        TableFeatBeg = 2;
        tableFeatNum = tableFeatNumPhraseBased;
        freeFeatNum  = tableFeatNumPhraseBased;
        MinFeatNum = 17;
        MainTableBeg = 2;
        MainTableEnd = MainTableBeg + tableFeatNum - 1;

        xtsearch = configer->GetBool("XTSearch", false);

        if(useCharLM)
            featNum = MinFeatNum + 2;
        else if(xtsearch)
            featNum = 2 + 15 + 15;
    }
    else if(type == SYNTAX_BASED){
        TableFeatBeg = 2;
        tableFeatNum = tableFeatNumSyntaxBased;
        freeFeatNum  = tableFeatNumSyntaxBased;
        MinFeatNum = 17;
        MainTableBeg = 2;
        MainTableEnd = MainTableBeg + tableFeatNum - 1;
    }
    else if(type == SKELETON_BASED){
        TableFeatBeg = 2;
        tableFeatNum = 17 + 17 - 2;
        freeFeatNum  = 17 + 17 - 2;
        MinFeatNum = 17;
        MainTableBeg = 2;
        MainTableEnd = MainTableBeg + tableFeatNum - 1;
    }

    if(CSFMMode){
        TableFeatBegCoarse = 17;
        TableFeatEndCoarse = 30;
        CSFMSurrogateRuleCount  = 31;
        CSFMMatchedNTCount      = 32;
        CSFMMatchedSrcNTCount   = 33;
        CSFMMatchedTgtNTCount   = 34;
        CSFMUnmatchedNTCount    = 35;
        CSFMUnmatchedSrcNTCount = 36;
        CSFMUnmatchedTgtNTCount = 37;
        featNum = 38;
        MinFeatNum = 38;
        MainTableBeg = 2;
        tableFeatNum = 36;
        MainTableEnd = MainTableBeg + tableFeatNum - 1;
    }
    
    if( configer->GetBool("freefeature", false) ){
        tableFeatNum = configer->GetInt("tablefeatnum", freeFeatNum);
        if(tableFeatNum >freeFeatNum)
            featNum = Model::MinFeatNum + tableFeatNum - freeFeatNum;
    }

    featNum = configer->GetInt("MaxFeatNum", featNum > Model::MinFeatNum ? featNum : Model::MinFeatNum);
}

void Model::InitForDecoding()
{
    MAX_WORD_NUM       = configer->GetInt("maxsentlength", 160);
    incompleteHypoRate = configer->GetFloat("incompletehyporate", 1.0F);
    backoffToHiero     = configer->GetFloat("backoffothiero", 0.8F);
    backoffToSyntax    = configer->GetFloat("backoffotsyntax", 0.8F);
    baseBiCFGProb      = configer->GetFloat("basebicfgprob", (float)log(0.00001));
    noFiltering        = configer->GetBool("notablefiltering", false);
    ngram              = configer->GetInt("ngram", 3);
    useForest          = configer->GetBool("forest", false);
    firstspan          = configer->GetInt("firstspan", 100000);
    

    if(type == HIERO)
        treeBasedModel = 0;
    else if(configer->GetBool("tree2tree", false))
        treeBasedModel = 3;
    else if(configer->GetBool("tree2string", false))
        treeBasedModel = 2;
    else
        treeBasedModel = 1;
}

void Model::InitNTProb()
{
    NTProb = new NTMappingProb();

    const char * fn_s2t = configer->GetString("Src-To-Tgt-NT-Prob", NULL);
    const char * fn_t2s = configer->GetString("Tgt-To-Src-NT-Prob", NULL);

    if(fn_s2t == NULL || fn_t2s == NULL)
        return;

    NTProb->Load(fn_s2t, fn_t2s);
}

void Model::Init()
{
    if(configer->GetBool("skeleton", false)){ // in skeleton translation mode
        InitForSkeletonTranslationModel();
        return;
    }

    InitFeatNum();
    InitForDecoding();
    LoadPara();
    InitNTProb();

    RuleSoringDimen = configer->GetInt("rulesortingdimen", Model::PrBiCFG);

    if(configer->GetString("Punct-Vocab-File", NULL) != NULL)
        StringUtil::LoadPunctDict(configer->GetString("Punct-Vocab-File", NULL));

    LoadNESymbol();

    // load n-gram languag models
    LoadLMs();

    if(configer->GetBool("scfg", false)){
        if(!CSFMMode){
            SCFG->LoadSCFG();
        }
        else{
            const char * coarseGrammarFN = configer->GetString("SCFG-Rule-Set", NULL);
            const char * fineGrammarFN = configer->GetString("Fine-SCFG-Rule-Set", NULL);

            if(coarseGrammarFN == NULL){
                fprintf(stderr, "ERROR: \"SCFG-Rule-Sete\" is not specified!\n");
                exit(-1);
            }

            if(fineGrammarFN == NULL){
                fprintf(stderr, "ERROR: \"Fine-SCFG-Rule-Set\" is not specified!\n");
                exit(-1);
            }

            SCFG     = new SynchronousGrammar(MathUtil::NextPrime(MILLION * 50), featNum, 
                                              tableFeatNum, this, type, NULL, configer);

            fineSCFG = new SynchronousGrammar(MathUtil::NextPrime(MILLION * 50), 
                                              featNum, tableFeatNumSyntaxBased, this, SYNTAX_BASED, SCFG, configer);

            SynchronousGrammar::LoadCSFM(coarseGrammarFN, fineGrammarFN, SCFG, fineSCFG);
        }
        vocabManager = SCFG;
    }
    else{
        pTable->LoadTable();
        MEReModel->LoadModel();
        msdReModel->LoadMSDReorderModel();
        vocabManager = pTable;
    }
}

void Model::InitForSkeletonTranslationModel()
{
    int f = 0;

    InitFeatNum();
    InitForDecoding();
    LoadPara();
    InitNTProb();

    const char * configFile1 = configer->GetString("skeletonconfig", "NiuTrans.skeleton.config");
    const char * configFile2 = configer->GetString("fullconfig", "NiuTrans.full.config");

    Configer * c1 = new Configer();
    Configer * c2 = new Configer();
    c1->Create(configFile1);
    c2->Create(configFile2);

    if(c1->GetBool("Target-Vocab-File", false) && c2->GetBool("Target-Vocab-File", false) && 
        strcmp(c1->GetString("Target-Vocab-File", NULL), c2->GetString("Target-Vocab-File", NULL)) != 0)
    {
        fprintf(stderr, "ERROR! the skeleton and full translaton models should use the same vocabulary!\n");
        fprintf(stderr, "Skeleton-Config:   %s\n", c1->GetString("Target-Vocab-File", NULL));
        fprintf(stderr, "Full-Model-Config: %s\n", c2->GetString("Target-Vocab-File", NULL));
        exit(0);
    }

    const char * modelType1 = c1->GetString("modeltype", "phrasebased");
    const char * modelType2 = c2->GetString("modeltype", "phrasebased");

    Model * m1 = new Model(GetModelType(modelType1), c1);
    Model * m2 = new Model(GetModelType(modelType2), c2);
    m1->Init();
    m2->Init();

    for(f = 0; f < m1->featNum; f++){
        m1->paras[f] = this->paras[f];
        //m1->featWeight[f] = this->featWeight[f];
    }
    for(f = 0; f < m2->featNum; f++){
        m2->paras[f] = this->paras[f+associatedModelFeatBeg];
        //m2->featWeight[f] = this->featWeight[f+associatedModelFeatBeg];
    }

    skeletonConfiger   = c1;
    assistantConfiger = c2;
    skeletonModel     = m1;
    assistantModel    = m2;

    associatedModels = new List(2);
    associatedConfigs = new List(2);

    associatedModels->Add((void*)m1);
    associatedModels->Add((void*)m2);
    associatedConfigs->Add((void*)c1);
    associatedConfigs->Add((void*)c2);

    pTable       = m1->pTable;
    MEReModel    = m1->MEReModel;
    msdReModel   = m1->msdReModel;
    SCFG         = m1->SCFG;
}

void Model::LoadPara()
{
    const char * ws = configer->GetString("weights", NULL);
    const char * rs = configer->GetString("ranges", NULL);
    const char * fs = configer->GetString("fixedfs", NULL);

    if( ws == NULL ){
        fprintf(stderr, "ERROR: \"weights\" is not specified!\n");
        exit(-1);
    }

    if( rs == NULL ){
        fprintf(stderr, "ERROR: \"ranges\" is not specified!\n");
        exit(-1);
    }

    if( fs == NULL ){
        fprintf(stderr, "ERROR: \"fixedfs\" is not specified!\n");
        exit(-1);
    }

    char ** w;
    char ** r;
    char ** f;
    int wc, rc, fc;

    wc = (int)StringUtil::Split( ws, " ", w );
    rc = (int)StringUtil::Split( rs, " ", r );
    fc = (int)StringUtil::Split( fs, " ", f );

    if( wc != rc || wc != fc ){
        fprintf(stderr, "ERROR: number of features is not specified appropriatly!\n");
        exit(-1);
    }
    if( wc < MinFeatNum ){
        fprintf(stderr, "ERROR: number of features < \"MinFeatNum\"(%d)!\n", MinFeatNum);
        exit(-1);
    }

    //if( wc < featNum && configer->GetBool("freefeature", false)){
    if(wc < featNum){
        fprintf(stderr, "ERROR: number of features < specified \"FeatNum\"(%d)!\n", featNum);
        exit(-1);
    }

    featNum = wc;
    if( paras != NULL )
        delete[] paras;
    paras = new ParaInfo[featNum];
    featWeight = new float[featNum];
    memset(featWeight, 0, sizeof(float) * featNum);

    int fixed;
    for( int i = 0; i < wc; i++){
        sscanf(w[i], "%f", &paras[i].weight);
        sscanf(r[i], "%f:%f", &paras[i].minValue, &paras[i].maxValue);
        sscanf(f[i], "%d", &fixed);
        paras[i].isFixed = fixed != 0 ? true : false;
        delete[] w[i];
        delete[] r[i];
        delete[] f[i];
        featWeight[i] = paras[i].weight;
    }

    delete[] w;
    delete[] r;
    delete[] f;
}

void Model::LoadNESymbol()
{
    NE2NT = new NEToNTSymbol();

    if(configer->GetString("NE-Symbol-File", NULL) != NULL)
        NE2NT->Load(configer->GetString("NE-Symbol-File", NULL));

    if(NE2NT->GetSymbol("$default") == NULL){
        if(configer->GetBool("tree2tree", false))
            NE2NT->Add("$default", "NP=NP");
        else if(configer->GetBool("tree2string", false))
            NE2NT->Add("$default", "NP");
        else
            NE2NT->Add("$default", "NNP");
    }

    if(CSFMMode){
        NE2NTFineModel = new NEToNTSymbol();

        if(configer->GetString("NE-Symbol-Fine-Model-File", NULL) != NULL)
            NE2NTFineModel->Load(configer->GetString("NE-Symbol-Fine-Model-File", NULL));

        const char * fineGType = configer->GetString("finegrainedmodel", NULL);

        if(NE2NTFineModel->GetSymbol("$default") == NULL){
            if(!strcmp(fineGType, "tree2tree"))
                NE2NTFineModel->Add("$default", "NP=NP");
            else if(!strcmp(fineGType, "tree2string"))
                NE2NTFineModel->Add("$default", "NP");
            else
                NE2NTFineModel->Add("$default", "NNP");
        }
    }
}

void Model::LoadLMs()
{
    const char * lmfn = configer->GetString("Ngram-LanguageModel-File", NULL);
    const char * vocabfn = configer->GetString("Target-Vocab-File", NULL);

    if( lmfn == NULL ){
        fprintf(stderr, "ERROR: \"Ngram-LanguageModel-File\" is not specified!\n");
        exit(-1);
    }

    if( vocabfn == NULL ){
        fprintf(stderr, "ERROR: \"Target-Vocab-File\" is not specified!\n");
        exit(-1);
    }

    ngramLM = new NiuTransLM();
    ngramLM->LoadModel(lmfn, vocabfn, -1, -1, 0);

    if(configer->GetBool("charngramlm", false)){ 
        lmfn = configer->GetString("Char-Ngram-LanguageModel-File", NULL);
        vocabfn = configer->GetString("Char-Target-Vocab-File", NULL);

        if( lmfn == NULL ){
            fprintf(stderr, "ERROR: \"Char-Ngram-LanguageModel-File\" is not specified!\n");
            exit(-1);
        }

        if( vocabfn == NULL ){
            fprintf(stderr, "ERROR: \"Char-Target-Vocab-File\" is not specified!\n");
            exit(-1);
        }

        ngramLM2 = new NiuTransLM();
        ngramLM2->LoadModel(lmfn, vocabfn, -1, -1, 0);
    }

}

int Model::GetWId(const char * trans)
{
    return vocabManager->GetWId(trans);
}

int Model::GetWId2ndVocab(const char * trans)
{
    return vocabManager->GetWId2ndVocab(trans);
}

void Model::GetWId(char * trans, int * &wId, int &wCount, MemPool * myMem)
{
    vocabManager->GetWId(trans, wId, wCount, 0, myMem);
}

void Model::GetWId2ndVocab(char * trans, int * &wId, int &wCount, MemPool * myMem)
{
    vocabManager->GetWId(trans, wId, wCount, 1, myMem);
}

char * Model::GetWord(int wId)
{
    return vocabManager->GetWord(wId);
}

char * Model::GetWord2ndVocab(int wId)
{
    return vocabManager->GetWord2ndVocab(wId);
}

float Model::GetNGramLMProb(int beg, int end, int * wid)
{
    float prob = ngramLM->GetProb(wid, beg, end);

#ifdef WIN32
    if(LOWEST_LM_SCORE > prob)
#else
    if(LOWEST_LM_SCORE > prob || isnan(prob))
#endif
        prob = LOWEST_LM_SCORE;

    return prob;
}

float Model::GetCharNGramLMProb(int beg, int end, int * wid)
{
    float prob = ngramLM2->GetProb(wid, beg, end);

#ifdef WIN32
    if(LOWEST_LM_SCORE > prob)
#else
    if(LOWEST_LM_SCORE > prob || isnan(prob))
#endif
        prob = LOWEST_LM_SCORE;

    return prob;
}

List * Model::FindRuleList(const char * src)
{
    return SCFG->FindOptionList(src);
}

List * Model::FindRuleListWithSymbol(const char * src)
{
    return SCFG->FindOptionListWithSymbol(src);
}

List * Model::FindOptionList(const char * src)
{
    return pTable->FindOptionList(src);
}

float Model::GetMEReorderScore(char ** feats)
{
    return MEReModel->Calculate(feats);
}


float Model::GetMSDReoderingFeatVal( char* fStr, char* tStr, const char* featType ) {

    int featId = -1;

    if( !strcmp( featType, "L2R_M" ) ) {
        featId = 0;
    }
    else if( !strcmp( featType, "L2R_S" ) ) {
        featId = 1;
    }
    else if( !strcmp( featType, "L2R_D" ) ) {
        featId = 2;
    }
    else if( !strcmp( featType, "R2L_M" ) ) {
        featId = 3;
    }
    else if( !strcmp( featType, "R2L_S" ) ) {
        featId = 4;
    }
    else if( !strcmp( featType, "R2L_D" ) ) {
        featId = 5;
    }

    if(featId == -1)
        return -1.0f;
    else{
        pMSDReFeatInfo info = msdReModel->GetFeatList( fStr,tStr );
        if(info == msdReModel->defaultFeatInfo){
            StringUtil::ToLowercase(fStr);
            StringUtil::ToLowercase(tStr);
            info = msdReModel->GetFeatList( fStr,tStr );
        }
        return info->feats[featId];
    }
}

const char * Model::GetFeatString(int featDim, DECODER_TYPE dtype, bool CSFM)
{
    if(CSFM)
        dtype = SYNTAX_BASED;

    if(dtype == SKELETON_BASED){
        if(featDim < associatedModelFeatBeg)
            dtype = skeletonModel->type;
        else{
            featDim -= associatedModelFeatBeg;
            dtype = assistantModel->type;
        }
    }

    if(featDim == 0)
        return "n-gram LM";
    else if(featDim == 1)
        return "tgt word count";
    else if(featDim == 2)
        return "prob-phr-f2e";
    else if(featDim == 3)
        return "prob-lex-f2e";
    else if(featDim == 4)
        return "prob-phr-e2f";
    else if(featDim == 5)
        return "prob-lex-e2f";
    else if(featDim == 6){
        if(dtype == PHRASE_BASED)
            return "phrase count";
        else
            return "rule count";
    }
    else if(featDim == 7)
        return "bi-lex count";
    else if(featDim == 8)
        return "null-trans count";
    else if(featDim == 9){
        if(dtype == PHRASE_BASED)
            return "ME reorder";
        else
            return "phr-rule count";
    }
    else if(featDim == 10){
        if(dtype == PHRASE_BASED)
            return "<NULL>";
        else
            return "glue-rule count";
    }
    else if(dtype == SYNTAX_BASED && featDim >= 11 && featDim <= 15){
        if(featDim == 11)
            return "prob-scfg";
        else if(featDim == 12)
            return "prob-cfg-tgt";
        else if(featDim == 13)
            return "lex-rule count";
        else if(featDim == 14)
            return "composed-rule count";
        else if(featDim == 15)
            return "fow-freq-rule count";
    }
    else if(CSFMMode){
        if(featDim == 16)
            return "<NULL>";

        const char * coarseGrammar = configer->GetString("coarsegrainedmodel", NULL);
        if(!strcmp(coarseGrammar, "hiero") && featDim >= 17 && featDim <= 25)
            return GetFeatString(featDim - 15, HIERO, false);
        else if(strcmp(coarseGrammar, "hiero") && featDim >= 17 && featDim <= 30)
            return GetFeatString(featDim - 15, SYNTAX_BASED, false);
        else if(featDim >= 17 && featDim <= 30)
            return "<NULL>";
        else if(featDim == 31)
            return "Surrogate-rule";
        else if(featDim == 32)
            return "Matched NT";
        else if(featDim == 33)
            return "Matched Src NT";
        else if(featDim == 34)
            return "Matched Tgt NT";
        else if(featDim == 35)
            return "Unmatched NT";
        else if(featDim == 36)
            return "Unmatched Src NT";
        else if(featDim == 37)
            return "Unmatched Tgt NT";
    }
    else if(featDim >= 11 && featDim <= 16){
        if(dtype != PHRASE_BASED)
            return "<NULL>";
        else if(featDim == 11)
            return "MSD reorder l2r-M";
        else if(featDim == 12)
            return "MSD reorder l2r-S";
        else if(featDim == 13)
            return "MSD reorder l2r-D";
        else if(featDim == 14)
            return "MSD reorder r2l-M";
        else if(featDim == 15)
            return "MSD reorder r2l-S";
        else if(featDim == 16)
            return "MSD reorder r2l-D";
    }

    return "User defined";
}

void GetAbbr(const char * src, char * tgt)
{
    if(!strcmp(src, "string2tree"))
        strcpy(tgt, "s2t");
    else if(!strcmp(src, "tree2string"))
        strcpy(tgt, "t2s");
    else if(!strcmp(src, "tree2tree"))
        strcpy(tgt, "t2t");
    else
        strcpy(tgt, "hiero");
}

char info[1024];
const char * Model::GetCSFMInfo()
{
    char cs[1024], ts[1024];

    if(!CSFMMode)
        return "no";

    GetAbbr(configer->GetString("coarsegrainedmodel", NULL), cs);
    GetAbbr(configer->GetString("finegrainedmodel", NULL), ts);
    sprintf(info, "%s-to-%s", cs, ts);
    return info;
}

bool Model::IsSCFGWithNTSymbol()
{
    if(SCFG != NULL)
        return SCFG->withSymbolEntry;

    return false;
}

DECODER_TYPE Model::GetModelType(const char * type)
{
    if(strcmp(type, "hiero") == 0 || strcmp(type, "hierarchy") == 0)
        return HIERO;
    if(strcmp(type, "syntax") == 0)
        return SYNTAX_BASED;
    if(strcmp(type, "string2tree") == 0 || strcmp(type, "tree2string") == 0 || strcmp(type, "tree2tree") == 0)
        return SYNTAX_BASED;
    return PHRASE_BASED;
}

void Model::UpdateAssociatedModelFeatureWeights()
{
    if(skeletonModel != NULL){
        for(int f = 0; f < skeletonModel->featNum && f < Model::associatedModelFeatBeg; f++){
            skeletonModel->featWeight[f] = featWeight[f];
            skeletonModel->paras[f].weight = featWeight[f];
        }
    }

    if(assistantModel != NULL){
        for(int f = Model::associatedModelFeatBeg; f - Model::associatedModelFeatBeg < assistantModel->featNum && f < featNum; f++){
            assistantModel->featWeight[f - Model::associatedModelFeatBeg] = featWeight[f];
            assistantModel->paras[f - Model::associatedModelFeatBeg].weight = featWeight[f];
        }
    }
}

////////////////////////////////////////
// phrase-table needed in deocding

PhraseTable::PhraseTable(int entryNum, int featNum, int tableFeatNum, DECODER_TYPE type, PhraseTable * baseTable, Configer * c)
{
    this->type = type;
    configer = c;

    unkWId = 2;
    useNULLTrans = configer->GetBool("usenulltrans", true);
    useLowercase = configer->GetBool("lowertext", true);
    outputNULL   = configer->GetBool("outputnull", false);
    patentMT     = configer->GetBool("patentmt", false);
    noFiltering  = configer->GetBool("notablefiltering", false);
    freeFeature  = configer->GetBool("freefeature", false);
    use2ndVocab  = configer->GetBool("charngramlm", false);

    mem = new MemPool(M_BYTE * 16, 1024 * 8);
    entry = new HashTable( entryNum, 0.75 );
    this->featNum = featNum;
    if(this->featNum < 17)
        this->featNum = 17;
    this->tableFeatNum = tableFeatNum;
    featList  = new float[M_BYTE];
    srcDict   = new HashTable( MILLION );
    tgtDict   = new HashTable( MILLION );
    tgtDict2  = use2ndVocab ? new HashTable( MILLION ) : NULL;
    srcVocab  = new char*[MILLION*10];
    tgtVocab  = new char*[MILLION*10];
    tgtVocab2 = use2ndVocab ? new char*[MILLION*10] : NULL;
    srcVocabSize  = 0;
    tgtVocabSize  = 0;
    tgtVocabSize2 = 0;
    NULLTransDict = new HashTable(10000);
    
    this->baseTable = baseTable;
}

PhraseTable::~PhraseTable()
{
    delete mem;
    delete entry;
    delete[] featList;
    delete srcDict;
    delete tgtDict;
    delete tgtDict2;
    delete[] srcVocab;
    delete[] tgtVocab;
    delete[] tgtVocab2;
    delete NULLTransDict;
}

void PhraseTable::LoadTable()
{
    LoadPara();

    // load phrase translations
    LoadPhraseTable();

    // meta translations
    AddMetaPhrases();
}

void PhraseTable::LoadPara()
{
    int tmp;
    useNULLTrans = configer->GetBool("usenulltrans", true);

    if(baseTable != NULL){
        tgtDict       = baseTable->tgtDict;
        unkWId        = baseTable->unkWId;
        NULLTransDict = baseTable->NULLTransDict;
    }
    else{
        // load dictionary
        const char * dfn = configer->GetString("Target-Vocab-File", NULL);
        LoadDict(tgtDict, dfn, "Target-Vocab-File", tgtVocab, tgtVocabSize);
        unkWId = tgtDict->GetInt("<unk>");

        // load words that are frequently deleted
        const char * nfn = configer->GetString("Freq-Deleted-Vocab-File", NULL);
        LoadDict(NULLTransDict, nfn, "Freq-Deleted-Vocab-File", NULL, tmp);

        if(configer->GetBool("charngramlm", false)){
            const char * dfn2 = configer->GetString("Char-Target-Vocab-File", NULL);
            LoadDict(tgtDict2, dfn2, "Char-Target-Vocab-File", tgtVocab2, tgtVocabSize2);
            unkWId2 = tgtDict2->GetInt("<unk>");
        }
    }
}

int PhraseTable::LoadPhraseTable()
{
    const char * tfn = configer->GetString("Phrase-Table", NULL);
    char line[MAX_LINE_LENGTH];
    int  totalCount = 0;

    if( tfn == NULL ){
        fprintf(stderr, "ERROR: Phrase-Table is not specified\n");
        exit(1);
    }

    char lastSrc[1024] = "";
    PhraseTransOption ** optionList = new PhraseTransOption*[MILLION];
    int optionCount = 0;

    char ** terms = new char*[128];
    for( int i = 0; i < 128; i++ )
        terms[i] = new char[1024];
    int termCount;

    FILE * file = fopen(tfn, "rb");

    if( tfn == NULL ){
        fprintf(stderr, "ERROR: cannot open translation table \"%s\"", tfn);
        exit(1);
    }

    TimeUtil timer;
    timer.StartTimer();
    fprintf( stderr, "Loading %s\n", "Phrase-Table" );
    fprintf( stderr, "  >> From File: %s ...\n", tfn );
    fprintf( stderr, "  >> ." );
    while(fgets(line, MAX_LINE_LENGTH - 1, file) != NULL){
        StringUtil::TrimRight( line );
        GetTerms(line, terms, termCount);

        if(!strcmp(terms[0], "AdsS")){
            int nnn = 0;
        }

        if(!noFiltering && !IsValid(terms[0], terms[1]))
            continue;

        PhraseTransOption * to = ParseTransOption(terms[0], terms[1], terms[2]); // get a new translation option

        if( to == NULL )
            continue;

        if( strcmp(lastSrc, terms[0]) != 0){
            AddOptionList(lastSrc, (void**)optionList, optionCount, 1);
            optionCount = 0;
        }
        else{
            if( optionCount + 1 == MILLION ){
                // too many options
                continue;
            }
        }
        optionList[optionCount++] = to;
        strcpy(lastSrc, terms[0]);

        totalCount++;

#ifdef SLOW_WINDOWS_PRINTF
        if(totalCount % 500000 == 0)
            fprintf( stderr, "." );
#else
        if(totalCount % 500000 == 0)
            fprintf( stderr, "\b.." );
        
        int tmpC = totalCount / 5000;
        if(tmpC % 4 == 0)
            fprintf(stderr, "\b/");
        else if(tmpC % 4 ==1)
            fprintf(stderr, "\b|");
        else if(tmpC % 4 == 2)
            fprintf(stderr, "\b\\");
        else
            fprintf(stderr, "\b-");
#endif

        //if(totalCount > 500000)
        //    break;
    }
    fclose(file);
    fprintf(stderr, "\b.");

    if( optionCount > 0 )
        AddOptionList(lastSrc, (void**)optionList, optionCount, 1);

    timer.EndTimer();
    double time = timer.GetTimerDiff();
    fprintf( stderr, "\nDone [%dK entries, %dMB memory, %.3f sec(s)]\n", totalCount/1000, (int)(mem->GetUsedMemSize()/ MILLION), time );

    delete[] optionList;
    for( int i = 0; i < 128; i++ )
        delete[] terms[i];
    delete[] terms;

    return totalCount;
}

int PhraseTable::LoadDict(HashTable *  dict, const char * dictfn, const char * name, char ** vocab, int &vocabSize)
{
    char line[MAX_LINE_LENGTH];
    char word[MAX_LINE_LENGTH];
    long  wid = 0;

    if( dictfn == NULL ){
        /*fprintf(stderr, "ERROR: %s is not specified\n", name);
        exit(1);*/
        return -1;
    }

    FILE * dfn = fopen(dictfn, "rb");
    if( dfn == NULL ){
        fprintf(stderr, "ERROR: cannot open %s \"%s\"\n", name, dictfn);
        exit(1);
    }

    fprintf( stderr, "Loading %s\n", name );
    fprintf( stderr, "  >> From File: %s ...\n", dictfn );
    fprintf( stderr, "  >> ." );
    TimeUtil timer;
    timer.StartTimer();
    while(fgets(line, MAX_LINE_LENGTH - 1, dfn) != NULL){
        StringUtil::TrimRight( line );
        *word = '\0';
        sscanf(line, "%s", word);
        dict->AddInt( word, wid );
        if(vocab != NULL){
            char * w = (char*)mem->Alloc(sizeof(char) * ((int)strlen(word) + 1));
            memset(w, 0, sizeof(char) * ((int)strlen(word) + 1));
            strcpy(w, word);
            vocab[wid] = w;
        }
        wid++;

#ifdef SLOW_WINDOWS_PRINTF
        if( wid % 100000 == 0 )
            fprintf(stderr, ".");
#else
        if( wid % 100000 == 0 )
            fprintf(stderr, "\b..");

        int tmpC = wid / 20000;
        if(tmpC % 4 == 0)
            fprintf(stderr, "\b/");
        else if(tmpC % 4 ==1)
            fprintf(stderr, "\b|");
        else if(tmpC % 4 == 2)
            fprintf(stderr, "\b\\");
        else
            fprintf(stderr, "\b-");
#endif
    }
    fclose(dfn);
    fprintf(stderr, "\b.");

    if (vocab != NULL)
        vocabSize = wid;

    timer.EndTimer();
    fprintf( stderr, "\nDone [%d entries, %.3f sec(s)]\n", (int)wid, timer.GetTimerDiff() );

    return wid;
}

void PhraseTable::GetTerms(char * line, char ** terms, int &termCount)
{
    int len = (int)strlen(line);
    int beg = 0, end = 0;

    termCount = 0;

    for(end = 0; end < len - 4; end++ ){
        if( strncmp(line + end, " ||| ", 5) != 0)
            continue;

        strncpy(terms[termCount], line + beg, end - beg);
        terms[termCount][end - beg] = '\0';
        termCount++;
        beg = end + 5;
        end = beg - 1;
    }

    if( len > beg ){
        strncpy(terms[termCount], line + beg, len - beg);
        terms[termCount][len - beg] = '\0';
        termCount++;
    }
}

// generate a translation option
PhraseTransOption * PhraseTable::ParseTransOption(char * src, char * tgt, char * feat)
{
    if( !useNULLTrans && strcmp(tgt, "<NULL>") == 0 )
        return NULL;

    PhraseTransOption * to = (PhraseTransOption *)mem->Alloc(sizeof(PhraseTransOption));

    to->src = NULL;
    to->tgt = (char *)mem->Alloc((int)strlen(tgt) + 1);
    strcpy(to->tgt, tgt);

    to->feat = (float *)mem->Alloc(sizeof(float) * featNum);
    memset(to->feat, 0, sizeof(float) * featNum);

    // feature values
    ParseFeatValues(to->feat, feat, tableFeatNum);

    // word-ids
    GetWId(tgt, to->wid, to->wCount);

    // word deletion
    if( strcmp(tgt, "<NULL>") == 0){
        if(NULLTransDict->GetInt( src ) == INVALID_ID)
            to->feat[Model::PenaltyNull] = 1;
        to->feat[Model::TableFeatBeg+1] = to->feat[Model::TableFeatBeg+0]; /// ???
        to->feat[Model::TableFeatBeg+3] = to->feat[Model::TableFeatBeg+2]; /// ???

        if(outputNULL){
            to->tgt = (char *)mem->Alloc(sizeof(char) * (strlen(src) + 5));
            memset(to->tgt, 0, sizeof(char) * (strlen(src) + 5));
            sprintf(to->tgt, "<-%s->", src);
        }
        else
            to->tgt[0] = '\0';

        GetWId(to->tgt, to->wid, to->wCount);
    }

    to->feat[Model::WordCount] = (float)to->wCount;

    if( useLowercase )
        StringUtil::ToLowercase( to->tgt );

    return to;
}

// translation options for a given "src"
void PhraseTable::AddOptionList(char * src, void ** optionList, int optionCount, int initListSize)
{
    List* toList = (List*) entry->GetObject( src );
    
    if(toList != NULL){
        toList->Add(optionList, optionCount);
    }
    else{
        List * toList = (List*)mem->Alloc(sizeof(List));
        toList->Create(optionCount > initListSize ? optionCount : initListSize, mem);
        toList->Add(optionList, optionCount);
        entry->AddObject( src, toList );
    }
}

// translation options for a given "src"
void PhraseTable::AddOption(const char * src, void * option, int initListSize)
{
    List* toList = (List*) entry->GetObject( src );
    
    if(toList != NULL){
        toList->Add(option);
    }
    else{
        List * toList = (List*)mem->Alloc(sizeof(List));
        toList->Create(1, mem);
        toList->Add(option);
        entry->AddObject(src, toList );
    }
}

void PhraseTable::ParseFeatValues(float * featP, char * feat, int maxFeatNum)
{
    int len = (int)strlen(feat);
    int beg = 0, end = 0;
    int featCount = 0;
    float value;

    for(end = 0; end < len + 1; end++ ){
        if(feat[end] != ' ' && feat[end] != '\0')
            continue;
        feat[end] = '\0';
        value = (float)atof(feat + beg); // the i-th feature
        feat[end] = ' ';
        beg = end + 1;

        if(!freeFeature){
            if(type != SYNTAX_BASED){
                if( featCount < 5 )
                    featP[Model::TableFeatBeg + featCount] = value;  // five feaures: Pr(e|f), Pr_lex(e|f), Pr(f|e), Pr_lex(f|e), PhraseCount
                else if(maxFeatNum <= 6 && featCount == 5)
                    featP[Model::BiLexCount] = value;       // bi-lex feature
                else if(featCount < maxFeatNum)
                    featP[Model::TableFeatBeg + featCount] = value;
            }
            else{
                if( featCount < 5 )
                    featP[Model::TableFeatBeg + featCount] = value;
                else if(featCount == 5)
                    featP[Model::PrBiCFG] = value;
                else if(featCount == 6)
                    featP[Model::PrCFG2] = value;
                else if(featCount == 7)
                    featP[Model::IsLexRule] = value;
                else if(featCount == 8)
                    featP[Model::IsComposedRule] = value;
                else if(featCount == 9)
                    featP[Model::IsLowFreqRule] = value;
                else if(featCount == 10)
                    featP[Model::BiLexCount] = value;
            }
        }
        else{
            float feat = value;//log(value);

            if(featCount < maxFeatNum){
                if(featCount <= Model::MainTableEnd - Model::MainTableBeg)
                    featP[Model::TableFeatBeg + featCount] = feat;
                else
                    featP[Model::MinFeatNum + featCount - (Model::MainTableEnd - Model::MainTableBeg) - 1] = feat;
            }
        }

        featCount++;
    }
}

int PhraseTable::GetWId(const char * trans)
{
    int wid = tgtDict->GetInt(trans);
    if( wid == -1 )
        return tgtDict->GetInt("<unk>");  // unknown word
    else
        return wid;
}

int PhraseTable::GetWId2ndVocab(const char * trans)
{
    int wid = tgtDict2->GetInt(trans);
    if( wid == -1 )
        return tgtDict2->GetInt("<unk>");  // unknown word
    else
        return wid;
}

void PhraseTable::GetWId(char * trans, int * &wId, int &wCount, int vocabId, MemPool * myMem)
{
    int widTmp[MAX_PHRASE_CHAR_NUM];
    int len = (int)strlen(trans);
    int beg = 0, end = 0;
    HashTable * tgtDict = this->tgtDict;
    if(vocabId >= 1)
        tgtDict = this->tgtDict2;

    wCount = 0;

    // skip head spaces
    while(end < len && trans[end] == ' ')
        end++;
    beg = end;

    for(; end < len; end++ ){
        if( trans[end] != ' ')
            continue;
        trans[end] = '\0';

        if( trans[beg] == NT_SYMBOL && end > beg + 1 ){
            widTmp[wCount++] = - atoi(trans + beg + 1); // variable
        }
        else{
            int wid;
            wid = tgtDict->GetInt( trans+beg );
            if( wid == -1 )
                widTmp[wCount++] = tgtDict->GetInt("<unk>");  // unknown word
            else
                widTmp[wCount++] = wid;
        }

        trans[end] = ' ';
        while(trans[end + 1] == ' ') // skip multiple spaces
            end++;
        beg = end + 1;
    }

    if( end > beg ){
        if( trans[beg] == NT_SYMBOL && end > beg + 1 ){
            widTmp[wCount++] = - atoi(trans + beg + 1); // variable
        }
        else{
            int wid = tgtDict->GetInt( trans+beg );
            if( wid == -1 )
                widTmp[wCount++] = tgtDict->GetInt("<unk>");  // unknown word
            else
                widTmp[wCount++] = wid;
        }
    }

    if(myMem != NULL)
        wId = (int *)myMem->Alloc(sizeof(int) * wCount);
    else
        wId = new int[wCount];

    memcpy(wId, widTmp, sizeof(int) * wCount);
}

void PhraseTable::GetWId(char * trans, int * &wId, int &wCount)
{
    GetWId(trans, wId, wCount, 0, mem);
}

char * PhraseTable::GetWord(int wId)
{
    if( wId >= 0 && wId < tgtVocabSize )
        return tgtVocab[wId];
    else
        return NULL;
}

char * PhraseTable::GetWord2ndVocab(int wId)
{
    if( wId >= 0 && wId < tgtVocabSize2 )
        return tgtVocab2[wId];
    else
        return NULL;
}

void PhraseTable::AddMetaPhrases()
{
    AddMetaEntry("<s>");
    AddMetaEntry("</s>");
    AddMetaEntry("<unk>");
}

// for beginning word <s>, ending word </s>, and unknown word <unk>
void PhraseTable::AddMetaEntry(const char * src)
{
    char * tmpSrc = (char*)mem->Alloc(sizeof(char) * ((int)strlen(src) + 1));
    memset(tmpSrc, 0, sizeof(char) * ((int)strlen(src) + 1));
    strcpy(tmpSrc, src);

    PhraseTransOption * to = (PhraseTransOption *)mem->Alloc(sizeof(PhraseTransOption));
    to->src = NULL;
    to->tgt = (char *)mem->Alloc(sizeof(char));
    to->tgt[0] = '\0';

#ifndef USE_OLD_TRICK
    if( !strcmp(src, "<unk>") )
        GetWId("", to->wid, to->wCount);
    else
        GetWId(tmpSrc, to->wid, to->wCount);
#else
    GetWId(tmpSrc, to->wid, to->wCount);
#endif

    to->feat = (float *)mem->Alloc(sizeof(float) * featNum);
    memset(to->feat, 0, sizeof(float) * featNum);

    //if(!strcmp(tmpSrc, "<unk>"))
    //    to->feat[Model::WordCount] = 1;
    to->feat[Model::WordCount] = (float)to->wCount;

    PhraseTable::AddOption(src, to, 1);
}

bool PhraseTable::IsValid(const char * src, const char * tgt)
{
    if(!IsValidForFoctoid(src, tgt))
        return false;
    /*if(!IsValid(src, tgt, "("))
        return false;
    if(!IsValid(src, tgt, ")"))
        return false;
    if(!IsValid(src, tgt, "\""))
        return false;*/

    if( strcmp(tgt, "<NULL>") == 0){
        if( !useNULLTrans )
            return false;
        //if( StringUtil::IsPunc(src) )
        //    return false;
    }

    if(patentMT && !IsValidForPatentMT(src, tgt)) // for 
        return false;

    return true;
}

bool PhraseTable::IsValidForFoctoid(const char * src, const char * tgt)
{
    // bacis items
    if(!IsValid(src, tgt, "$number"))
        return false;
    if(!IsValid(src, tgt, "$time"))
        return false;
    if(!IsValid(src, tgt, "$date"))
        return false;

    // some puntuations
    if(!IsValid(src, tgt, "("))
        return false;
    if(!IsValid(src, tgt, ")"))
        return false;
    return true;
}

bool PhraseTable::IsValid(const char * src, const char * tgt, const char * type)
{
    const char * s = strstr(src, type);
    const char * t = strstr(tgt, type);

    while(s != NULL && t != NULL){
        s = strstr(s + 1, type);
        t = strstr(t + 1, type);
    }

    return !((s == NULL) ^ (t == NULL));
}

bool PhraseTable::IsValidForPatentMT(const char * src, const char * tgt)
{
    int nums[10];
    memset(nums, 0, sizeof(int) * 10);
    int srcLen = (int)strlen(src);
    int tgtLen = (int)strlen(tgt);

    for(int i = 0; i < srcLen; i++){
        if(src[i] >= '0' && src[i] <= '9')
            nums[src[i] - '0']++;
    }

    for(int i = 0; i < tgtLen; i++){
        if(tgt[i] >= '0' && tgt[i] <= '9')
            nums[tgt[i] - '0']--;
    }

    for(int i = 0; i < 10; i++)
        if(nums[i] != 0)
            return false;

    if((StringUtil::IsPunc(src) || StringUtil::IsPunc(tgt))  && strcmp(src, tgt))
        return false;

    if(!strcmp(tgt, "-") && strcmp(src, tgt))
        return false;

    if(StringUtil::IsLiteral(src)){
        if(!CheckLiteralMatching(src, tgt))
            return false;
    }

    return true;
}

bool PhraseTable::CheckLiteralMatching(const char * src, const char * tgt)
{
    int    wlen = (int)strlen(src);
    int    tlen = (int)strlen(tgt);

    for(int i = 0; i < tlen - wlen + 1; i++){
        int w, t;
        for(w = 0, t = 0; w < wlen && i + t < tlen; w++, t++){
            if(tgt[i + t] == ' ' || tgt[i + t] == '\t'){
                w--;
                continue;
            }

            if(src[w] != tgt[i + t] && src[w] != tgt[i + t] + 'A' - 'a')
                break;
        }

        if( w == wlen){ // matched
            return true;
        }
    }

    return false;
}

List * PhraseTable::FindOptionList(const char * src)
{
    void * addr = entry->GetObject( src );
    if( addr == NULL )
        return NULL;
    else
        return (List*)addr;
}

//////////////////////////////////////////////////////////////
// Maximum Entropy-based reordering model (Xiong et al., 2006)
MEReorderModel::MEReorderModel(Configer * c)
{
    configer = c;
    mem      = new MemPool(M_BYTE * 5, 1024);
    entry    = new HashTable(MILLION * 5);
    entryFeat = new HashTable*[16]; // first 8: for the reversed-translation features; last 8: for the monotonic-translation features
    for( int i = 0; i <  16; i++ )
        entryFeat[i] = new HashTable(MILLION);
}

MEReorderModel::~MEReorderModel()
{
    delete mem;
    delete entry;
    for( int i = 0; i < 16; i++ )
        delete entryFeat[i];
    delete[] entryFeat;
}

void MEReorderModel::LoadModel()
{
    const char * tfn = configer->GetString("ME-Reordering-Table", NULL);
    char line[MAX_LINE_LENGTH], key[MAX_LINE_LENGTH];
    float value;
    int  totalCount = 0;

    if( tfn == NULL ){
        fprintf(stderr, "ERROR: ME-Reordering-Table is not specified\n");
        exit(1);
    }

    FILE * file = fopen(tfn, "rb");

    if( file == NULL ){
        fprintf(stderr, "ERROR: cannot open translation table \"%s\"", tfn);
        exit(1);
    }

    TimeUtil timer;
    timer.StartTimer();
    fprintf( stderr, "Loading %s\n", "ME-Reordering-Table" );
    fprintf( stderr, "  >> From File: %s ...\n", tfn );
    fprintf( stderr, "  >> ." );

    while(fgets(line, MAX_LINE_LENGTH - 1, file) != NULL){
        // sample: 0:SLL=at    -0.89466
        // where 0 denotes "reversed translation", 1 denotes ""
        StringUtil::TrimRight( line );
        sscanf(line, "%s\t%f", key, &value);

        float * v = (float*)mem->Alloc(sizeof(float));
        *v = value;
        entry->AddObject( key, v );
        totalCount++;

        key[1] = '\0';
        key[5] = '\0';

        int label = key[0] - '0', featId = 0;
        char * feat = key + 2;
        char * featValue = key + 6;

        if( strcmp(feat, "SLL") == 0 ) featId = 0;
        else if( strcmp(feat, "SLR") == 0 ) featId = 1;
        else if( strcmp(feat, "SRL") == 0 ) featId = 2;
        else if( strcmp(feat, "SRR") == 0 ) featId = 3;
        else if( strcmp(feat, "TLL") == 0 ) featId = 4;
        else if( strcmp(feat, "TLR") == 0 ) featId = 5;
        else if( strcmp(feat, "TRL") == 0 ) featId = 6;
        else if( strcmp(feat, "TRR") == 0 ) featId = 7;

        entryFeat[label * 8 + featId]->AddObject( featValue, v );

#ifdef SLOW_WINDOWS_PRINTF
        if(totalCount % 50000 == 0)
            fprintf(stderr, ".");
#else
        if(totalCount % 50000 == 0)
            fprintf(stderr, "\b..");

        int tmpC = totalCount / 2000;
        if(tmpC % 4 == 0)
            fprintf(stderr, "\b/");
        else if(tmpC % 4 ==1)
            fprintf(stderr, "\b|");
        else if(tmpC % 4 == 2)
            fprintf(stderr, "\b\\");
        else
            fprintf(stderr, "\b-");
#endif
    }
    fprintf(stderr, "\b.");

    timer.EndTimer();
    double time = timer.GetTimerDiff();
    fprintf( stderr, "\nDone [%dK entries, %.3f sec(s)]\n", totalCount/1000, time );

    fclose( file );

}

float MEReorderModel::Calculate(char ** reorderFeat)
{
    float w0 = 0, w1 = 0;
    char rf0[128], rf1[128];

    for( int i = 0; i < 8; i++ ){
        sprintf(rf0, "0:%s", reorderFeat[i]);
        sprintf(rf1, "1:%s", reorderFeat[i]);

        w0 += GetFeatWeight(rf0);
        w1 += GetFeatWeight(rf1);
    }

    double probMono = (exp(w0) * FLETTEN_DIS_FACTOR) 
                      / (exp(w0) * FLETTEN_DIS_FACTOR + exp(w1) * FLETTEN_DIS_FACTOR); // monotonic translation
                      // FLETTEN_DIS_FACTOR is to control the sharpness of the distribution
    return (float)log(probMono);
}

float MEReorderModel::GetFeatWeight(int classId, int featId, char * feat)
{
	if(feat == NULL)
		return 0;

    void * v = entryFeat[classId * 8 + featId]->GetObject( feat );
    if( v == NULL )
        return 0;
    else
        return *((float*)v);
}

float MEReorderModel::GetFeatWeight(char * feat)
{
	if(feat == NULL)
		return 0;

    void * v = entry->GetObject( feat );
    if( v == NULL )
        return 0;
    else
        return *((float*)v);
}


//////////////////////////////////////////////////////////////
// MSD reordering model
MSDReorderModel::MSDReorderModel(bool onSourceSide, Configer * c) 
{
    configer  = c;
    pMem      = new MemPool( M_BYTE, 1024 );
    entryList = new HashTable( MILLION );
    onSrcSide = onSourceSide;
    defaultFeatInfo = ( pMSDReFeatInfo )pMem->Alloc( sizeof(MSDReFeatInfo));
    for( int i=0; i < 6; i++ ) {
        defaultFeatInfo->feats[i] = (float)log( 0.0001 );
    }
}


MSDReorderModel::MSDReorderModel(Configer * c) {

    MSDReorderModel(false, c);

}


MSDReorderModel::~MSDReorderModel() {

    delete    pMem;
    delete    entryList;

}


void MSDReorderModel::LoadMSDReorderModel() {

    const char *fn;
    char line[MAX_LINE_LENGTH], key[MAX_LINE_LENGTH];
    int totalCount = 0;

    if( this->onSrcSide ) {
        fn = configer->GetString( "MSD-Reordering-Model-S", NULL );
    }
    else {
        fn = configer->GetString( "MSD-Reordering-Model", NULL );
    }

    if( fn == NULL ){
        if( this->onSrcSide ) {
            fprintf(stderr, "ERROR: MSD-Reordering-Model-S is not specified\n");
        }
        else {
            fprintf(stderr, "ERROR: MSD-Reordering-Model is not specified\n");
        }
        exit(1);
    }

    FILE * file = fopen( fn, "rb" );
    if( file == NULL ){
        fprintf(stderr, "ERROR: cannot open MSD reordering table \"%s\"", fn);
        exit(1);
    }

    TimeUtil timer;
    timer.StartTimer();
    if( this->onSrcSide ) {
        fprintf( stderr, "Loading %s\n", "MSD-Reordering-Model-S" );
        fprintf( stderr, "  >> From File: %s ...\n", fn );
    }
    else {
        fprintf( stderr, "Loading %s\n", "MSD-Reordering-Model" );
        fprintf( stderr, "  >> From File: %s ...\n", fn );
    }
    fprintf( stderr, "  >> ." );

    char ** terms = new char*[128];
    for( int i = 0; i < 128; i++ )
        terms[i] = new char[1024];
    int termCount;

    while(fgets(line, MAX_LINE_LENGTH - 1, file) != NULL){
        // sample: �� ||| pakistan ||| 0.560976 0.219512 0.219512 0.463415 0.0731707 0.463415
        StringUtil::TrimRight( line );
        GetTerms(line, terms, termCount);
        sprintf( key, "%s ||| %s", terms[0], terms[1] );
        MSDReFeatInfo *opt = ( MSDReFeatInfo* )pMem->Alloc( sizeof(MSDReFeatInfo) );
        ParseFeatValues( terms[2], opt );
        entryList->AddObject( key, opt );

        ++totalCount;

#ifdef SLOW_WINDOWS_PRINTF
        if( totalCount % 200000 == 0 )
            fprintf( stderr, "." );
#else
        if( totalCount % 200000 == 0 )
            fprintf( stderr, "\b.." );

        int tmpC = totalCount / 5000;
        if(tmpC % 4 == 0)
            fprintf(stderr, "\b/");
        else if(tmpC % 4 ==1)
            fprintf(stderr, "\b|");
        else if(tmpC % 4 == 2)
            fprintf(stderr, "\b\\");
        else
            fprintf(stderr, "\b-");
#endif

        //if(totalCount > 500000)
        //    break;
    }
    fprintf(stderr, "\b.");

    timer.EndTimer();
    double time = timer.GetTimerDiff();
    fprintf( stderr, "\nDone [%dK entries, %.3f sec(s)]\n", totalCount/1000, time );

    for( int i = 0; i < 128; i++ )
        delete[] terms[i];
    delete[] terms;

    fclose( file );

}

void MSDReorderModel::GetTerms(char * line, char ** terms, int &termCount) {
    int len = (int)strlen(line);
    int beg = 0, end = 0;

    termCount = 0;

    for(end = 0; end < len - 4; end++ ){
        if( strncmp(line + end, " ||| ", 5) != 0)
            continue;
        strncpy(terms[termCount], line + beg, end - beg);
        terms[termCount][end - beg] = '\0';
        termCount++;
        beg = end + 5;
        end = beg - 1;
    }

    if( len > beg ){
        strncpy(terms[termCount], line + beg, len - beg);
        terms[termCount][len - beg] = '\0';
        termCount++;
    }
}


void MSDReorderModel::ParseFeatValues( char* feats, pMSDReFeatInfo opt ) {

    float *featList = opt->feats;
    int len = (int)strlen( feats );
    int beg = 0, end = 0;
    int featCount = 0;
    float value;

    for( end=0; end < len; end++ ) {
        if( feats[end] != ' ' )
            continue;
        feats[end] = '\0';
        value = (float)atof( feats + beg );
        featList[featCount++] = value;
        feats[end] = ' ';
        beg = end + 1;
    }
    featList[featCount] = (float)atof( feats + beg );
}

pMSDReFeatInfo MSDReorderModel::GetFeatList( char* fStr, char* tStr ) {

    char key[MAX_LINE_LENGTH];
    void *value = NULL;

    sprintf( key, "%s ||| %s", fStr, tStr );
    value = entryList->GetObject( key );
    if( value == NULL )
        return defaultFeatInfo;
    else
        return (pMSDReFeatInfo)value;

}

////////////////////////////////////////////
// NE's syntacitic label

NEToNTSymbol::NEToNTSymbol()
{
    maxSymbolNum = 1000;
    NTDict       = new HashTable(maxSymbolNum);
    symbolVocab  = new char*[maxSymbolNum];
    symbolCount  = 0;

    memset(symbolVocab, 0, sizeof(char) * maxSymbolNum);
}

NEToNTSymbol::~NEToNTSymbol()
{
    delete NTDict;
    for(int i = 0; i < symbolCount; i++){
        delete[] symbolVocab[i];
    }
    delete[] symbolVocab;
}

void NEToNTSymbol::Add(const char * NE, const char * symbol)
{
    int id = (int)(NTDict->GetInt(NE));

    if(id == -1){
        char * s = new char[(int)strlen(symbol) + 1];
        strcpy(s, symbol);
        symbolVocab[symbolCount] = s;
        NTDict->AddInt(NE, symbolCount++);
    }
}

char * NEToNTSymbol::GetSymbol(const char * NE)
{
    int id = (int)(NTDict->GetInt(NE));

    if(id == -1)
        return NULL;
    else
        return symbolVocab[id];
}

char * NEToNTSymbol::GetSymbolByDefault(const char * NE)
{
    int id = (int)(NTDict->GetInt(NE));

    if(id == -1)
        id = (int)(NTDict->GetInt("$default"));
    
    if(id == -1)
        return NULL;
    else
        return symbolVocab[id];
}

void NEToNTSymbol::Load(const char * fn)
{
    int maxLen = 1024;
    int count = 0;
    char * line   = new char[maxLen];
    char * NE     = new char[maxLen];
    char * symbol = new char[maxLen];


    FILE * f = fopen(fn, "rb");
    if(f == NULL){
        fprintf(stderr, "ERROR: cannot open file \"%s\"!", fn);
        return;
    }

    TimeUtil timer;
    timer.StartTimer();
    fprintf( stderr, "Loading NE's syntactic labels\n");
    fprintf( stderr, "  >> From File: %s ...\n", fn );
    fprintf( stderr, "  >> ." );

    while(fgets(line, maxLen - 1, f)){
        for(int i = (int)strlen(line) - 1; i >= 0; i--){
            if(line[i] == '\r' || line[i] == '\n')
                line[i] = '\0';
            else
                break;
        }

        if(sscanf(line, "%s %s", NE, symbol) == 2)
            Add(NE, symbol);

#ifdef SLOW_WINDOWS_PRINTF
        if(count++ % 10 == 0)
            fprintf( stderr, "." );
#else
        if(count++ % 10 == 0)
            fprintf( stderr, "\b.." );
        if(count % 2 == 0)
            fprintf(stderr, "\b/");
        else if(count % 1 == 0)
            fprintf(stderr, "\b\\");
#endif
    }
    fprintf(stderr, "\b.");

    timer.EndTimer();
    double time = timer.GetTimerDiff();
    fprintf( stderr, "\nDone [%d entries, %.3f sec(s)]\n", count, time );

    delete[] line;
    delete[] NE;
    delete[] symbol;
}

////////////////////////////////////
// Non-terminal to non-terminal prob

NTMappingProb::NTMappingProb()
{
    srcNTDict     = new HashTable(MILLION);
    tgtNTDict     = new HashTable(MILLION);
    srcToTgt      = new NTMappingNode *[MAX_NUM_OF_NT_SYMBOL];
    tgtToSrc      = new NTMappingNode *[MAX_NUM_OF_NT_SYMBOL];
    srcMappingNum = new int[MAX_NUM_OF_NT_SYMBOL];
    tgtMappingNum = new int[MAX_NUM_OF_NT_SYMBOL];
    memset(srcToTgt, 0, sizeof(NTMappingNode *) * MAX_NUM_OF_NT_SYMBOL);
    memset(tgtToSrc, 0, sizeof(NTMappingNode *) * MAX_NUM_OF_NT_SYMBOL);
    memset(srcMappingNum, 0, sizeof(int) * MAX_NUM_OF_NT_SYMBOL);
    memset(tgtMappingNum, 0, sizeof(int) * MAX_NUM_OF_NT_SYMBOL);
    srcEntryCount = 0;
    tgtEntryCount = 0;
    srcToTgtDefaultNum = 0;
    tgtToSrcDefaultNum = 0;
    jointDefaultNum    = 0;

    srcToTgtDefault = new NTMappingNode[MAX_NUM_OF_NT_SYMBOL];
    memset(srcToTgtDefault, 0, sizeof(NTMappingNode) * MAX_NUM_OF_NT_SYMBOL);
    CreateMappingNode(&srcToTgtDefault[srcToTgtDefaultNum++], "NP", log(0.3));
    CreateMappingNode(&srcToTgtDefault[srcToTgtDefaultNum++], "VP", log(0.2));

    tgtToSrcDefault = new NTMappingNode[MAX_NUM_OF_NT_SYMBOL];
    memset(tgtToSrcDefault, 0, sizeof(NTMappingNode) * MAX_NUM_OF_NT_SYMBOL);
    CreateMappingNode(&tgtToSrcDefault[tgtToSrcDefaultNum++], "NP", log(0.3));
    CreateMappingNode(&tgtToSrcDefault[tgtToSrcDefaultNum++], "VP", log(0.2));

    jointDefault   = new NTMappingNode[MAX_NUM_OF_NT_SYMBOL];
    memset(jointDefault, 0, sizeof(NTMappingNode) * MAX_NUM_OF_NT_SYMBOL);
    CreateMappingNode(&jointDefault[jointDefaultNum++], "NP=NP", log(0.2));
    CreateMappingNode(&jointDefault[jointDefaultNum++], "VP=VP", log(0.1));
}

NTMappingProb::~NTMappingProb()
{
    delete srcNTDict;
    delete tgtNTDict;
    for(int i = 0; i < MAX_NUM_OF_NT_SYMBOL; i++){
        if(srcToTgt[i] != NULL){
            for(int j = 0; j < srcMappingNum[i]; j++)
                delete[] srcToTgt[i][j].nt;
        }
        if(tgtToSrc[i] != NULL){
            for(int j = 0; j < tgtMappingNum[i]; j++)
                delete[] tgtToSrc[i][j].nt;
        }
        delete[] srcToTgt[i];
        delete[] tgtToSrc[i];
    }
    delete[] srcToTgt;
    delete[] tgtToSrc;
    delete[] srcMappingNum;
    delete[] tgtMappingNum;

    for(int i = 0; i < MAX_NUM_OF_NT_SYMBOL; i++){
        if(srcToTgtDefault[i].nt != NULL)
            delete[] srcToTgtDefault[i].nt;

        if(tgtToSrcDefault[i].nt != NULL)
            delete[] tgtToSrcDefault[i].nt;

        if(jointDefault[i].nt != NULL)
            delete[] jointDefault[i].nt;
    }
    
    delete[] srcToTgtDefault;
    delete[] tgtToSrcDefault;
    delete[] jointDefault;
}

void NTMappingProb::Load(const char * fn_s2t, const char * fn_t2s)
{
    Load(fn_s2t, srcNTDict, srcToTgt, srcMappingNum, srcEntryCount);
    Load(fn_t2s, tgtNTDict, tgtToSrc, tgtMappingNum, tgtEntryCount);
}

void NTMappingProb::Load(const char * fn, HashTable * dict, NTMappingNode ** mappingProb, int * mappingNum, int &entryCount)
{
    char line[MAX_LINE_LENGTH];
    NTMappingNode * mappingBuf = new NTMappingNode[MAX_NUM_OF_NT_SYMBOL];
    memset(mappingBuf, 0, sizeof(NTMappingNode *) * MAX_NUM_OF_NT_SYMBOL);

    char ** terms = new char*[128];
    for( int i = 0; i < 128; i++ )
        terms[i] = new char[MAX_LINE_LENGTH];
    int termCount;


    FILE * file = fopen(fn, "rb");

    if( file == NULL ){
        fprintf(stderr, "ERROR: cannot open file \"%s\"", fn);
        exit(1);
    }

    char lastSrcNT[MAX_LINE_LENGTH] = "";
    int srcNTCount = 0, mappingCount = 0, totalCount = 0;

    TimeUtil timer;
    timer.StartTimer();
    fprintf( stderr, "Loading %s\n", "NT symbol mapping prob" );
    fprintf( stderr, "  >> From File: %s ...\n", fn );
    fprintf( stderr, "  >> ." );
    while(fgets(line, MAX_LINE_LENGTH - 1, file) != NULL){

        SCFGRule * rule = NULL;
        StringUtil::TrimRight( line );

        if(sscanf(line, "%s ||| %s ||| %s", terms[0], terms[1], terms[2]) == 3){// format: src-NT-symbol ||| tgt-NT-symbol ||| prob
            if(strcmp(lastSrcNT, terms[0]) && mappingCount > 0){
                AddMappingNode(lastSrcNT, mappingBuf, mappingCount, dict, mappingProb, mappingNum, entryCount);
                mappingCount = 0;
            }

            CreateMappingNode(&mappingBuf[mappingCount++], terms[1], log(atof(terms[2])));
        }

        strcpy(lastSrcNT, terms[0]);

#ifdef SLOW_WINDOWS_PRINTF
        if( totalCount++ % 1000 == 0 )
            fprintf(stderr, ".");
#else
        if( totalCount++ % 1000 == 0 )
            fprintf(stderr, "\b..");

        int tmpC = totalCount / 200;
        if(tmpC % 4 == 0)
            fprintf(stderr, "\b/");
        else if(tmpC % 4 ==1)
            fprintf(stderr, "\b|");
        else if(tmpC % 4 == 2)
            fprintf(stderr, "\b\\");
        else
            fprintf(stderr, "\b-");
#endif

    }

    if(mappingCount > 0)
        AddMappingNode(lastSrcNT, mappingBuf, mappingCount, dict, mappingProb, mappingNum, entryCount);

    fclose(file);
    fprintf(stderr, "\b.");

    timer.EndTimer();
    double time = timer.GetTimerDiff();
    fprintf( stderr, "\nDone [%d (src) NT symbols, %d mapping nodes, %.3f sec(s)]\n", entryCount, totalCount, time );

    delete[] mappingBuf;
    for( int i = 0; i < 128; i++ )
        delete[] terms[i];
    delete[] terms;
}

void NTMappingProb::CreateMappingNode(NTMappingNode * node, const char * NT, float prob)
{
    node->nt = new char[(int)strlen(NT) + 1];
    strcpy(node->nt, NT);
    node->prob = prob;
}

void NTMappingProb::AddMappingNode(char * srcNT, NTMappingNode * mappingBufProb, int mappingBufCount, 
                                   HashTable * dict, NTMappingNode ** mappingProb, int * mappingNum, int &entryCount)
{
    dict->AddInt(srcNT, entryCount);

    mappingProb[entryCount] = new NTMappingNode[mappingBufCount];
    memcpy(mappingProb[entryCount], mappingBufProb, sizeof(NTMappingNode) * mappingBufCount);
    mappingNum[entryCount] = mappingBufCount;

    entryCount++;
}

NTMappingNode * NTMappingProb::GetMappingList(int direction, char * srcNT, int &mappingNum)
// direction = 0 means joint src-tgt, 1 means src-2-tgt, 2 means tgt-2-src
{
    int ind = -1;
    NTMappingNode * result = NULL;


    if(direction == 0){
        result = jointDefault;
        mappingNum = jointDefaultNum;
    }
    else if(direction == 1){

        if(srcEntryCount > 0){
            ind = srcNTDict->GetInt(srcNT);
            if(ind >= 0){
                result = srcToTgt[ind];
                mappingNum = srcMappingNum[ind];
            }
        }

        if(srcEntryCount == 0 || ind < 0){
            result = srcToTgtDefault;
            mappingNum = srcToTgtDefaultNum;
        }
    }
    else{
        
        if(tgtEntryCount > 0){
            ind = tgtNTDict->GetInt(srcNT);
            if(ind >= 0){
                result = tgtToSrc[ind];
                mappingNum = tgtMappingNum[ind];
            }
        }

        if( tgtEntryCount == 0 || ind < 0){
            result = tgtToSrcDefault;
            mappingNum = tgtToSrcDefaultNum;
        }
    }

    return result;
}

int NTMappingProb::GetTopNMapping(int direction, char * srcNT, int n, NTMappingNode * &result) // direction = 1 means src-2-tgt, 2 means tgt-2-src
{
    int mappingNum = -1;
    result = GetMappingList(direction, srcNT, mappingNum);

    if(n > mappingNum)
        return mappingNum;
    else
        return n;
}

int NTMappingProb::GetMappingAboveP(int direction, char * srcNT, int n, float p, NTMappingNode * &result)
{
    int mappingNum = -1;
    result = GetMappingList(direction, srcNT, mappingNum);

    if(n < mappingNum)
        mappingNum = n;

    int validCount = 0;
    for(int i = 0; i < mappingNum; i++){
        if(result[i].prob >= p){
            validCount++;
            continue;
        }
        break;
    }

    return validCount;
}


////////////////////////////////////
// SCFG

SynchronousGrammar::SynchronousGrammar(int entryNum, int featNum, int tableFeatNum, Model * model, DECODER_TYPE type, PhraseTable * baseTable, Configer * c) : 
                    PhraseTable(entryNum, featNum, tableFeatNum, type, baseTable, c)
{
    this->configer = c;
    this->model = model;
    this->type  = type;

    int sizeInPrime = MathUtil::NextPrime(MILLION);

    symbolDict     = new HashTable( sizeInPrime );
    srcSymbolDict  = new HashTable( sizeInPrime );
    tgtSymbolDict  = new HashTable( sizeInPrime );
    symbolVocab    = new char*[sizeInPrime];
    srcSymbolVocab = new char*[sizeInPrime];
    tgtSymbolVocab = new char*[sizeInPrime];
    symbolCount    = 0;
    srcSymbolCount = 0;
    tgtSymbolCount = 0;
    tmpNTs         = new Nonterminal[MAX_NT_NUM];
    tmpNTCount     = 0;
    tmpList        = new UnitRule*[MAX_RULE_SIZE];
    tmpCount       = 0;
    ruleBase       = new List(entryNum);
    ruleCount      = 0;
    sourceSideBase = new List(entryNum);

    entryWithSymbol = new HashTable(entryNum, 0.75);

    AddMetaSymbol();
    LoadGrammarPara(type);
}

SynchronousGrammar::~SynchronousGrammar()
{
    delete symbolDict;
    delete srcSymbolDict;
    delete tgtSymbolDict;
    delete[] symbolVocab;
    delete[] srcSymbolVocab;
    delete[] tgtSymbolVocab;
    delete[] tmpNTs;
    delete[] tmpList;
    delete ruleBase;
    delete sourceSideBase;
    delete entryWithSymbol;
}

void SynchronousGrammar::LoadGrammarPara(DECODER_TYPE type)
{
    if(type == SYNTAX_BASED)
        withoutGrammarEncoding = configer->GetBool("withoutgrammarencoding", false) || 
                                 configer->GetBool("nogrammarencoding", false);
    else
        withoutGrammarEncoding = configer->GetBool("withoutgrammarencoding", true) ||
                                 configer->GetBool("nogrammarencoding", true);

    withSymbolEntry       = configer->GetBool("withsymbolentry", type == HIERO ? false : true);
    strictNTLabeling      = configer->GetBool("strictntlabeling", false);
    allowNonlexReordering = configer->GetBool("nonlexreordering", true);
    forTreeParsing        = configer->GetBool("treeparsing", false);
    generateTreeStructure = configer->GetBool("treestructure", false);
    dumpRule              = configer->GetBool("dumpusedrule", false) || configer->GetBool("dumprule", false);
}

void SynchronousGrammar::AddMetaSymbol()
{
    AddSymbol("", symbolDict, symbolVocab, symbolCount);
    AddSymbol("NULL", symbolDict, symbolVocab, symbolCount);
    AddSymbol("", srcSymbolDict, srcSymbolVocab, srcSymbolCount);
    AddSymbol("NULL", srcSymbolDict, srcSymbolVocab, srcSymbolCount);
    AddSymbol("", tgtSymbolDict, tgtSymbolVocab, tgtSymbolCount);
    AddSymbol("NULL", tgtSymbolDict, tgtSymbolVocab, tgtSymbolCount);
}

int SynchronousGrammar::LoadSCFG(const char * fn, bool CSFMMode)
{
    const char * tfn = fn != NULL ? fn : configer->GetString("SCFG-Rule-Set", NULL);
    char line[MAX_LINE_LENGTH];
    int  totalCount = 0, invalidCount = 0;

    if( tfn == NULL ){
        fprintf(stderr, "ERROR: \"SCFG-Rule-Set\" is not specified\n");
        exit(1);
    }

    LoadPara();

    UnitRule ** uruleBuf = new UnitRule *[MAX_UNIT_RULE_NUM_PER_SCFG_RULE];
    char ** terms = new char*[128];
    for( int i = 0; i < 128; i++ )
        terms[i] = new char[MAX_LINE_LENGTH];
    int termCount;


    FILE * file = fopen(tfn, "rb");

    if( file == NULL ){
        fprintf(stderr, "ERROR: cannot open rule table \"%s\"", tfn);
        exit(1);
    }

    int c = 0;

    TimeUtil timer;
    timer.StartTimer();
    fprintf( stderr, "Loading %s\n", "SCFG-Rule-Set" );
    fprintf( stderr, "  >> From File: %s ...\n", tfn );
    fprintf( stderr, "  >> ." );
    while(fgets(line, MAX_LINE_LENGTH - 1, file) != NULL){
        SCFGRule * rule = NULL;

        StringUtil::TrimRight( line );
        GetTerms(line, terms, termCount);

        // invalid rule
        if(!noFiltering && 
            (!IsValid(terms[0], terms[1]) || !IsValidForSCFG(terms[0], terms[1]) || !IsValidSymbol(terms[2])))
        { 
            if(!withoutGrammarEncoding){
                while(fgets(line, MAX_LINE_LENGTH - 1, file) != NULL){  // jump to next rule
                    StringUtil::TrimRight(line);
                    if(!strcmp(line,""))
                        break;
                }
            }
            ruleBase->Add(rule);
            invalidCount++;
            continue;
        }

        //rule = ParseSCFGRule(terms[2], terms[0], terms[1], terms[3], withSymbolEntry); // get a new translation rule
        rule = ParseSCFGRule(terms, termCount, withSymbolEntry); // get a new translation rule

        // add it into the list
        ruleBase->Add(rule);

        if( rule == NULL ){
            if(!withoutGrammarEncoding){
                while(fgets(line, MAX_LINE_LENGTH - 1, file) != NULL){ // jump to next rule
                    StringUtil::TrimRight(line);
                    if(!strcmp(line,""))
                        break;
                }
            }
            invalidCount++;
            continue;
        }

        // load unit rules
        if(!withoutGrammarEncoding){
            if(LoadUnitRules(file, rule, uruleBuf, line, terms, withSymbolEntry) == 0){
                invalidCount++;
                continue;
            }
        }
        else
            GenerateUnitRules(rule, withSymbolEntry);

        // record the source-side of unit rules
        if(withSymbolEntry)
            AddSourceSideForUnitRules(rule);

        // rule indexer for "coarse-search + fine-modeling"
        if(CSFMMode)
            LoadCSFMRulePointers(terms[termCount - 1], rule);

        totalCount++;

#ifdef SLOW_WINDOWS_PRINTF
        if( totalCount % 500000 == 0 )
            fprintf(stderr, ".");
#else
        if( totalCount % 500000 == 0 )
            fprintf(stderr, "\b..");

        int tmpC = totalCount / 5000;
        if(tmpC % 4 == 0)
            fprintf(stderr, "\b/");
        else if(tmpC % 4 ==1)
            fprintf(stderr, "\b|");
        else if(tmpC % 4 == 2)
            fprintf(stderr, "\b\\");
        else
            fprintf(stderr, "\b-");
#endif

        //if(totalCount > 500000)
        //    break;
    }
    fclose(file);
    fprintf(stderr, "\b.");

    timer.EndTimer();
    double time = timer.GetTimerDiff();

    if(totalCount > 1000)
        fprintf( stderr, "\nDone [%dK entries, %dK invalid rules, %dMB memory, %.3f sec(s)]\n", 
                 totalCount/1000, invalidCount/1000, (int)(mem->GetUsedMemSize() / MILLION), time );
    else
        fprintf( stderr, "\nDone [%d entries, %d invalid rules, %dMB memory, %.3f sec(s)]\n", 
                 totalCount, invalidCount, (int)(mem->GetUsedMemSize() / MILLION), time );


    delete[] uruleBuf;
    for( int i = 0; i < 128; i++ )
        delete[] terms[i];
    delete[] terms;

    AddMetaRules();
    SortRules();

    return totalCount;
}

bool SynchronousGrammar::IsMetaSymbol(char * symbol)
{
    if(strcmp(symbol, "<s>") == 0)
        return true;
    if(strcmp(symbol, "</s>") == 0)
        return true;
    if(strcmp(symbol, "<unk>") == 0)
        return true;
    return false;
}

int css = 0;

//SCFGRule * SynchronousGrammar::ParseSCFGRule(char * root, char * src, char * tgt, char * feat, bool keyWithSymbol)
SCFGRule * SynchronousGrammar::ParseSCFGRule(char ** terms, int termCount, bool keyWithSymbol)
{
    char * src  = terms[0];
    char * tgt  = terms[1];
    char * root = terms[2];
    char * feat = terms[3];
    char * alignment = termCount > 4 ? terms[4] : NULL;

    if( !useNULLTrans && strcmp(tgt, "<NULL>") == 0 )
        return NULL;

    SCFGRule * rule = (SCFGRule *)mem->Alloc(sizeof(SCFGRule));
    
    ParseSymbol(root, rule->root, rule->rootSrc, rule->rootTgt);

    bool isUnaryProduction = IsUnaryProduction(src);
    bool isUnary = isUnaryProduction && IsUnaryProductionForSourceSide(rule->rootSrc, src + 1);
    //isUnary = false;

    if(forTreeParsing && !isUnary){
    //if(forTreeParsing){
        char * realRoot = generateTreeStructure ? root : rule->root;

        int len = (int)(strlen(src) + strlen(realRoot) + 7);
        rule->src = (char*)mem->Alloc(len);
        memset(rule->src, 0, sizeof(char) * len);
        sprintf(rule->src, "#%s ( %s )", realRoot, src); // root (frontier node sequence)
    }
    else{
        rule->src = (char*)mem->Alloc(strlen(src) + 1);
        strcpy(rule->src, src);
    }

    rule->tgt = (char*)mem->Alloc(strlen(tgt) + 1);
    strcpy(rule->tgt, tgt);

    rule->feat = (float *)mem->Alloc(sizeof(float) * featNum);
    memset(rule->feat, 0, sizeof(float) * featNum);
    ParseFeatValues(rule->feat, feat, tableFeatNum); // feature values

    rule->alignment = dumpRule && alignment != NULL ? StringUtil::Copy(alignment) : NULL;

    if(!strcmp(tgt, "<NULL>")){
        if(outputNULL){
            rule->tgt = (char *)mem->Alloc(sizeof(char) * (strlen(src) + 5));
            memset(rule->tgt, 0, sizeof(char) * (strlen(src) + 5));
            sprintf(rule->tgt, "<-%s->", src);
        }
        else{
            rule->tgt = (char*)mem->Alloc(1);
            rule->tgt[0] = '\0';
        }

        rule->feat[Model::PenaltyNull] = 1;
        GetWId(GlobalVar::nullString, rule->wid, rule->wCount);
    }
    else
        GetWId(rule->tgt, rule->wid, rule->wCount);

    // generate key for indexing
    if(forTreeParsing && generateTreeStructure){
        ParseSourceSide(src, rule, rule->key, false, false);

        if(!isUnary){
            rule->key = (char *)mem->Alloc(sizeof(char) * ((int)strlen(root) + 3));
            sprintf(rule->key, "(%s)", root);
        }
        else{
            rule->key = (char *)mem->Alloc(2);
            strcpy(rule->key, NT_SYMBOL_STRING);
        }
    }
    else if(forTreeParsing && !generateTreeStructure)
        ParseSourceSide(rule->src, rule, rule->key, true, !isUnary);  // source-side => key
    else if(!keyWithSymbol && withoutGrammarEncoding)
        ParseSourceSide(rule->src, rule, rule->key, true, false);     // source-side => key
    else
        GetNTNum(rule);

    // !!!!!!!!!!!!! important
    rule->feat[Model::WordCount] = (float)(rule->wCount - rule->NTCount);
        
    if(rule->NTCount == 0) // rules with non-terminal invloved
        rule->feat[Model::PhrasalRuleCount] = 1;

    if(IsValidWidForSCFG(rule) && InformalCheck(src, tgt, rule))
        return rule;
    else
        return NULL;
}

void SynchronousGrammar::AddSCFGRule(SCFGRule * rule)
{
    if( rule == NULL )
        return;

    for( int i = 0; i < rule->ruleCount; i++ ){

        UnitRule * ur = rule->ruleList[i];
        char * src = ur->src;
        List * unitRuleList = (List*) entry->GetObject(src);
        
        if(unitRuleList != NULL){
            unitRuleList->Add((void*)ur);
        }
        else{
            List * newList = (List*)mem->Alloc(sizeof(List));
            newList->Create(5, mem); // mininum number of unit rules for an entry
            newList->Add((void*)ur);
            entry->AddObject(src, newList);
        }
    }
}

void SynchronousGrammar::ParseSymbol(char * symbol, char * &root, char * &rootSrc, char * &rootTgt)
{
    root    = NULL;
    rootSrc = NULL;
    rootTgt = NULL;

    if(symbol == NULL)
        return;

    int slen = (int)strlen(symbol);
    if(slen <= 0)
        return; // default symbol #

    char * rootSeg = strchr(symbol, ' ');

    if(rootSeg != NULL)
        *rootSeg = '\0';

    root = AddSymbol(symbol, symbolDict, symbolVocab, symbolCount);
    char * separator = strchr(symbol, SYMBOL_SEPARATOR);

    if(separator != NULL){
        *separator = '\0';
        rootTgt = AddSymbol(symbol, srcSymbolDict, srcSymbolVocab, srcSymbolCount);
        rootSrc = AddSymbol(separator + 1, tgtSymbolDict, tgtSymbolVocab, tgtSymbolCount);
        *separator = SYMBOL_SEPARATOR;
    }
    else{
        rootSrc = root;
        rootTgt = root;
    }

    if(rootSeg != NULL)
        *rootSeg = ' ';
}

// add a given non-terminal symbol into the dictionary
char * SynchronousGrammar::AddSymbol(const char * symbol, HashTable * sdict, char** symbolVocab, int &symbolCount)
{
    char * s = (char*)mem->Alloc(sizeof(char) * ((int)strlen(symbol) + 1));
    strcpy(s, symbol);
    return s;

    int symIdx = sdict->GetInt( symbol );
    if( symIdx == -1 ) {
        char * s = (char*)mem->Alloc(sizeof(char) * ((int)strlen(symbol) + 1));
        strcpy(s, symbol);
        symbolVocab[symbolCount] = s;
        sdict->AddInt(symbol, symbolCount);
        return symbolVocab[symbolCount++];
    }
    else{
        return symbolVocab[symIdx];
    }
}

// parse non-terminals on the source-language side
void SynchronousGrammar::ParseSourceSide(char * src, BasicRule * rule, char * &key, bool generateKey, bool indexForSourceTreeParsing)
{
    int    len = (int)strlen(src);
    char * beg = strchr(src, NT_SYMBOL);
    char * end = beg + 1;
    char * last = src;
    int    keyLen = 0;
    bool   needPostProcesing = false;

    key = generateKey ? (char*)mem->Alloc(sizeof(char) * len + 1) : NULL;

    tmpNTCount = 0;

    while(beg != NULL){
        end = beg + 1;

        while(*end != ' ' && *end != '\0')
            end++;

        if(end != beg + 1){
            char tmpc = *end;
            *end = '\0';
            ParseSymbol(beg + 1, tmpNTs[tmpNTCount].symbol, tmpNTs[tmpNTCount].symbolSrc, tmpNTs[tmpNTCount].symbolTgt);
            *end = tmpc;
            tmpNTCount++;

            if(generateKey){
                strncpy(key + keyLen, last, beg - last); // terminals
                keyLen += (int)(beg - last);

                key[keyLen++] = NT_SYMBOL; // non-terminals

                if(indexForSourceTreeParsing){
                    char * NTSymbol       = tmpNTs[tmpNTCount - 1].symbolSrc;
                    int    NTSymbolLength = (int)strlen(tmpNTs[tmpNTCount - 1].symbolSrc);

                    strcpy(key + keyLen, tmpNTs[tmpNTCount - 1].symbolSrc);
                    keyLen += NTSymbolLength;
                }

                if(*end != '\0')
                    key[keyLen++] = ' ';

                last = end + 1;
            }
        }
        else
            needPostProcesing = true;

        beg = strchr(end, NT_SYMBOL);
    }

    beg = src + len;

    if(generateKey){
        if(beg - last > 0){
            strncpy(key + keyLen, last, beg - last); // terminals
            keyLen += (int)(beg - last);
        }

        key[keyLen] = '\0';

        if(needPostProcesing) // if the source-side contains the default var-symbol "#"
            PostProcessingSourceSide(key, keyLen);
    }

    rule->NTCount = tmpNTCount;
    if(tmpNTCount == 0)
        rule->NTCount = 0;
    else if(indexForSourceTreeParsing){
        rule->NT = (Nonterminal*)mem->Alloc(sizeof(Nonterminal) * tmpNTCount - 1);
        memcpy(rule->NT, tmpNTs + 1, sizeof(Nonterminal) * (tmpNTCount - 1));
        rule->NTCount = tmpNTCount - 1;
    }
    else{
        rule->NT = (Nonterminal*)mem->Alloc(sizeof(Nonterminal) * tmpNTCount);
        memcpy(rule->NT, tmpNTs, sizeof(Nonterminal) * tmpNTCount);
        rule->NTCount = tmpNTCount;
    }
}

// how many non-terminals defined in a given rule
void SynchronousGrammar::GetNTNum(BasicRule * rule)
{
    char * src = rule->src;
    int    len = (int)strlen(src);
    char * beg = strchr(src, NT_SYMBOL);
    char * end = beg + 1;

    tmpNTCount = 0;

    while(beg != NULL){
        end = beg + 1;

        while(*end != ' ' && *end != '\0')
            end++;

        if(end != beg + 1){
            char tmpc = *end;
            tmpNTCount++;
        }
        beg = strchr(end, NT_SYMBOL);
    }

    rule->NTCount = tmpNTCount;
}

void SynchronousGrammar::PostProcessingSourceSide(char * &key, int &keyLen)
{
    if(!strcmp(key, "#"))
        return;

    int newKeyLen = 0;
    char * newKey = (char *)mem->Alloc(sizeof(char) * keyLen * 2);
    memset(newKey, 0, sizeof(char) * keyLen * 2);

    for(int i = 0; i < keyLen; i++){
        if(key[i] == '#')
            newKey[newKeyLen++] = '#';
        newKey[newKeyLen++] = key[i];
    }

    key = newKey;
    keyLen = newKeyLen;
}

void SynchronousGrammar::GenerateUnitRules(SCFGRule * rule, bool keyWithSymbol)
{
    UnitRule * urule  = (UnitRule *)mem->Alloc(sizeof(UnitRule));
    urule->parentRule = rule;
    urule->isLexRule  = keyWithSymbol ? !IsNonLexial(rule->src) : true; // assuming that all rules follow the Lexical Normal Form
    urule->isComplete = IsComplete(rule->root);

    urule->key = rule->key;

    memcpy(urule, rule, sizeof(BasicRule)); // basic information
    urule->id = 0;

    // unit rules
    rule->ruleCount = 0;
    rule->ruleList = (UnitRule **)mem->Alloc(sizeof(UnitRule*));
    rule->ruleList[rule->ruleCount++] = urule;

    if(!forTreeParsing){
        if(urule->isLexRule){
            if(keyWithSymbol)
                AddOptionWithRuleSourceSide(urule->key, urule, MIN_OPTION_NUM);
            else
                AddOption(urule->key, urule, MIN_OPTION_NUM);
        }

        if(keyWithSymbol)
            AddOptionWithSymbol(urule->src, urule, MIN_OPTION_NUM);
    }
    else{
        if(strcmp(urule->key, NT_SYMBOL_STRING))  // if it is not a single symbol "#"
            AddOptionWithRuleSourceSide(urule->key, urule, MIN_OPTION_NUM);

        AddOptionWithSymbol(urule->src, urule, MIN_OPTION_NUM);
    }
}

int SynchronousGrammar::LoadUnitRules(FILE * file, SCFGRule * rule, UnitRule ** uruleBuf, 
                                      char * lineBuf, char ** termsBuf, bool keyWithSymbol)
{
    int uruleCount = 0;
    int termCount = 0;
    bool validRules = true;

    while(fgets(lineBuf, MAX_LINE_LENGTH - 1, file) != NULL){
        StringUtil::TrimRight(lineBuf);

        if(!strcmp(lineBuf,""))
            break;

        if(!strcmp(lineBuf,"<null>"))
            break;

        if(forTreeParsing) // unit rules are useless for tree-parsing
            continue;

        GetTerms(lineBuf, termsBuf, termCount);
        if(!noFiltering && !IsValidForFoctoid(termsBuf[0], termsBuf[1]))
            validRules = false;

        UnitRule * urule = (UnitRule *)mem->Alloc(sizeof(UnitRule));
        urule->parentRule = rule;
        urule->feat = rule->feat;

        ParseSymbol(termsBuf[2], urule->root, urule->rootSrc, urule->rootTgt); // root

        urule->src = (char*)mem->Alloc(strlen(termsBuf[0]) + 1); // source-language side
        strcpy(urule->src, termsBuf[0]);

        urule->tgt = (char*)mem->Alloc(strlen(termsBuf[1]) + 1); // target-language side
        strcpy(urule->tgt, termsBuf[1]);
        
        GetWId(urule->tgt, urule->wid, urule->wCount);
        
        urule->isLexRule  = !IsNonLexial(urule->src); // non-lexicalized or lexicalzied ?
        urule->isComplete = IsComplete(urule->root);

        urule->id = uruleCount;

        uruleBuf[uruleCount++] = urule;
    }

    if(forTreeParsing){
        if(!strcmp(lineBuf,"<null>"))
            fgets(lineBuf, MAX_LINE_LENGTH - 1, file); // skip a line

        GenerateUnitRules(rule, keyWithSymbol);
        return 1;
    }

    if(!strcmp(lineBuf,"<null>")){ // no grammar encoding
        ParseSourceSide(rule->src, rule, rule->key, true, false); // source-side => key
        GenerateUnitRules(rule, keyWithSymbol);
        uruleCount = 1;
        fgets(lineBuf, MAX_LINE_LENGTH - 1, file); // skip a line
    }
    else if(validRules){
        rule->ruleCount = uruleCount;
        rule->ruleList = (UnitRule **)mem->Alloc(sizeof(UnitRule*) * uruleCount);
        memcpy(rule->ruleList, uruleBuf, sizeof(UnitRule*) * uruleCount);

        for(int i = 0; i < uruleCount; i++){
            UnitRule * urule = uruleBuf[i];
            ParseSourceSide(urule->src, urule, urule->key, true, false);

            if(urule->isLexRule){
                if(keyWithSymbol)
                    AddOptionWithRuleSourceSide(urule->key, urule, MIN_OPTION_NUM);
                else
                    AddOption(urule->key, urule, MIN_OPTION_NUM);
            }

            if(keyWithSymbol)
                AddOptionWithSymbol(urule->src, urule, MIN_OPTION_NUM);
        }
    }

    return uruleCount;
}

void SynchronousGrammar::AddMetaRules()
{
    SCFGRule * rule = NULL;

    rule = AddMetaRuleEntry("<s>");
    rule = AddMetaRuleEntry("</s>");
    rule = AddMetaRuleEntry("<unk>");

    if(withSymbolEntry){
        rule->root = model->NE2NT->GetSymbol("$default"); // syntactic label for unknow words
        rule->ruleList[0]->root = rule->root;
    }
}

SCFGRule * SynchronousGrammar::AddMetaRuleEntry(const char * symbol)
{
    char * tmpSymbol = (char*)mem->Alloc(sizeof(char) * ((int)strlen(symbol) + 1));
    memset(tmpSymbol, 0, sizeof(char) * ((int)strlen(symbol) + 1));
    strcpy(tmpSymbol, symbol);

    SCFGRule * rule = (SCFGRule *)mem->Alloc(sizeof(SCFGRule));
    memset(rule, 0, sizeof(SCFGRule));

    ParseSymbol(tmpSymbol, rule->root, rule->rootSrc, rule->rootTgt);

    rule->src = (char*)mem->Alloc((int)strlen(symbol) + 1);
    strcpy(rule->src, symbol);
    rule->tgt = (char*)mem->Alloc(1);
    rule->tgt[0] = '\0';

#ifndef USE_OLD_TRICK
    if( !strcmp(symbol, "<unk>") )
        GetWId(GlobalVar::nullString, rule->wid, rule->wCount);
    else
        GetWId(symbol, rule->wid, rule->wCount);
#else
    GetWId(tmpSymbol, rule->wid, rule->wCount);
#endif

    rule->feat = (float *)mem->Alloc(sizeof(float) * featNum);
    memset(rule->feat, 0, sizeof(float) * featNum);

    //if(!strcmp(symbol, "<unk>"))
    //    rule->feat[Model::WordCount] = 1;
    rule->feat[Model::WordCount] = (float)rule->wCount;

    UnitRule * urule = (UnitRule *)mem->Alloc(sizeof(UnitRule));
    memcpy(urule, rule, sizeof(BasicRule)); // basic information

    urule->key = rule->src;
    urule->parentRule = rule;
    urule->id = 0;
    urule->isComplete = true;

    // unit rules
    rule->ruleCount = 0;
    rule->ruleList = (UnitRule **)mem->Alloc(sizeof(UnitRule*));
    rule->ruleList[rule->ruleCount++] = urule;

    // index the rule
    AddOption(urule->key, urule, MIN_OPTION_NUM);
    
    if(withSymbolEntry){
        AddOptionWithSymbol(urule->key, urule, MIN_OPTION_NUM);
        urule->root = StringUtil::GeneateString(GlobalVar::nullString, mem);
    }

    // the SCFG rule
    ruleBase->Add(rule);

    return rule;
}

void SynchronousGrammar::AddOptionWithSymbol(char * src, void * option, int initListSize)
{
    List* toList = (List*) entryWithSymbol->GetObject( src );
    
    if(toList != NULL){
        toList->Add(option);
    }
    else{
        List * toList = (List*)mem->Alloc(sizeof(List));
        toList->Create(1, mem);
        toList->Add(option);
        entryWithSymbol->AddObject(src, toList);
    }
}

void SynchronousGrammar::AddOptionWithRuleSourceSide(char * key, UnitRule * urule, int initListSize)
{
    List * toList = (List*) entry->GetObject(key);
    
    if(toList != NULL){
        List * testList = (List*) entryWithSymbol->GetObject(urule->src); // use "entryWithSymbol" to check wether the source-side has been seen
        if(testList == NULL)
            toList->Add(urule);
    }
    else{
        List * toList = (List*)mem->Alloc(sizeof(List));
        toList->Create(1, mem);
        toList->Add(urule);
        entry->AddObject(key, toList);
    }
}

bool SynchronousGrammar::IsNonLexial(char * src)
{
    int len = (int)strlen(src);

    if(len == 0 || src[0] != NT_SYMBOL)
        return false;

    char * word = strchr(src, ' ');

    while(word != NULL){

        if(word[1] != NT_SYMBOL)
            return false;

        if(word - src < len - 2 && word[2] == ' ')
            return false;

        word = strchr(word + 1, ' ');
    }

    return true;
}

bool SynchronousGrammar::IsComplete(char * root)
{
    if(root == NULL)
        return true;

    char * findit = strchr(root, VIRUTAL_NT_SYMBOL);

    if(findit != NULL)
        return false;
    else
        return true;
}

// to process rules containing words with NT_SYMBOL (i.e. '#')
bool SynchronousGrammar::IsValidForSCFG(const char * src, const char * tgt)
{
    int len = (int)strlen(src);

    if(len > 2){
        if(src[0] == NT_SYMBOL && src[1] == ' ')
            return false;
        else if(src[len - 1] == NT_SYMBOL && src[len - 2] == ' ')
            return false;
        else if(strstr(src, NT_SYMBOL_STRING3) != NULL)
            return false;
    }

    return true;
}

bool SynchronousGrammar::IsValidSymbol(const char * symbol)
{
    if(strictNTLabeling && !generateTreeStructure){
        char fc = symbol[0];
        if(fc != '\0' && fc != '-' && fc != '*' && fc != ':' && (fc < 'a' || fc > 'z') && (fc < 'A' || fc > 'Z'))
            return false;
    }
    return true;
}

bool SynchronousGrammar::IsValidWidForSCFG(BasicRule * rule)
{
    bool lex = false;
    for(int i = 0; i < rule->wCount; i++){
        if(rule->wid[i] < 0 && (-rule->wid[i]) > rule->NTCount){
            return false;
        }

        if(rule->wid[i] >= 0)
            lex = true;
    }

    if(!allowNonlexReordering && !lex && IsNonLexial(rule->src)){
        for(int i = 0; i < rule->wCount; i++){
            if((-rule->wid[i]) != i + 1) // nonlexicalized reordering
                return false;
        }
    }

    return true;
}

bool SynchronousGrammar::IsUnaryProduction(const char * src)
{
    if(strlen(src) < 2)
        return false;

    if(src[0] != NT_SYMBOL || strchr(src, ' ') != NULL)
        return false;

    return true;
}

bool SynchronousGrammar::IsUnaryProductionForSourceSide(char * srcSymbol, char * ruleSymbol)
{
    char * separator = strchr(ruleSymbol, SYMBOL_SEPARATOR);

    if(separator != NULL){
        if(!strcmp(srcSymbol, separator + 1))
            return true;
        else
            return false;
    }
    else{
        if(!strcmp(srcSymbol, ruleSymbol))
            return true;
        else
            return false;
    }
}

bool SynchronousGrammar::IsUnaryProductionForTargetSide(char * tgtSymbol, char * ruleSymbol)
{
    char * separator = strchr(ruleSymbol, SYMBOL_SEPARATOR);

    if(separator != NULL){
        *separator = '\0';
        bool comp = !strcmp(tgtSymbol, ruleSymbol);
        *separator = SYMBOL_SEPARATOR;

        return comp;
    }
    else{
        if(!strcmp(tgtSymbol, ruleSymbol))
            return true;
        else
            return false;
    }
}

bool SynchronousGrammar::InformalCheck(const char * src, const char * tgt, SCFGRule * rule)
{
    if(strstr(tgt, "$number - $number , $number - $number , $number") != NULL){ // a unary rule with too much word insertion
        return false;
    }

    if(strstr(src, " ") == NULL && rule->wCount > 8){
        return false;
    }

    return true;
}

void SynchronousGrammar::PreprocessRootLabel(char * root)
{
    if(!generateTreeStructure){
        char * seg = strchr(root, ' ');
        if(seg != NULL)
            *seg = '\0';
    }
}

List * SynchronousGrammar::FindOptionListWithSymbol(const char * src)
{
    void * addr = entryWithSymbol->GetObject( src );
    if( addr == NULL )
        return NULL;
    else
        return (List*)addr;
}

void SynchronousGrammar::AddSourceSideForUnitRules(SCFGRule * rule)
{
    for(int i = 0; i < rule->ruleCount; i++){
        UnitRule * urule = rule->ruleList[i];
        List * uruleList = FindOptionListWithSymbol(urule->src);

        if(uruleList->count <= 1)
            sourceSideBase->Add(urule->src);
    }
}

void SynchronousGrammar::SortRules()
{
    fprintf(stderr, "Sorting rules ....");
    for(int i = 0; i < sourceSideBase->count; i++){
        char * src = (char *)sourceSideBase->GetItem(i);
        List * ruleList = (List *)entryWithSymbol->GetObject(src);
        ruleList->Sort(sizeof(UnitRule *), CompareUnitRules);

        int tmpC = i / 1500;
#ifdef SLOW_WINDOWS_PRINTF
        if(tmpC % 2 ==1)
            fprintf(stderr, "\b|");
        else
            fprintf(stderr, "\b-");
#else
        if(tmpC % 4 == 0)
            fprintf(stderr, "\b/");
        else if(tmpC % 4 ==1)
            fprintf(stderr, "\b|");
        else if(tmpC % 4 == 2)
            fprintf(stderr, "\b\\");
        else
            fprintf(stderr, "\b-");
#endif
    }
    
    fprintf(stderr, "\b [%dK entries, done]\n", sourceSideBase->count/1000);
}

void SynchronousGrammar::LoadCSFM(const char * cfn, const char * ffn, SynchronousGrammar * coarseGrammar, SynchronousGrammar * fineGrammar)
{
    const char * coarseGType = ConfigManager.GetString("coarsegrainedmodel", NULL);
    const char * fineGType = ConfigManager.GetString("finegrainedmodel", NULL);

    if(coarseGType == NULL){
        fprintf(stderr, "ERROR: \"coarsegrainedmodel\" is not specified\n");
        exit(1);
    }

    if(fineGType == NULL){
        fprintf(stderr, "ERROR: \"finegrainedmodel\" is not specified\n");
        exit(1);
    }

    if(!strcmp(coarseGType, "hiero"))
        coarseGrammar->LoadGrammarPara(HIERO);
    else
        coarseGrammar->LoadGrammarPara(SYNTAX_BASED);

    fineGrammar->LoadGrammarPara(SYNTAX_BASED);

    coarseGrammar->LoadSCFG(cfn, true);
    fineGrammar->LoadSCFG(ffn, true);

    SCFGRule ** coarseRuleBase = (SCFGRule **)coarseGrammar->ruleBase->items;
    SCFGRule ** fineRuleBase = (SCFGRule **)fineGrammar->ruleBase->items;
    int coarseRuleNum = coarseGrammar->ruleBase->count;
    int fineRuleNum = fineGrammar->ruleBase->count;

    // coarse-rule -> fine-rule (1:m mapping)
    for(int i = 0; i < coarseRuleNum; i++){
        SCFGRule * rule = coarseRuleBase[i];
        if(rule == NULL)
            continue;
        for(int r = 0; r < rule->CSFMRuleNum; r++){
            long p = (long)rule->CSFMRules[r];
            rule->CSFMRules[r] = fineRuleBase[p];
        }
    }

    // fine-rule -> coarse-rule (m:1 mapping)
    for(int i = 0; i < fineRuleNum; i++){
        SCFGRule * rule = fineRuleBase[i];
        if(rule == NULL)
            continue;
        for(int r = 0; r < rule->CSFMRuleNum; r++){
            long p = (long)rule->CSFMRules[r];
            rule->CSFMRules[r] = coarseRuleBase[p];
        }

        //if(rule->CSFMRuleNum > 1)
        //    fprintf(stderr, "ERROR: too many (coarse-grained) rules are mapped to rule-%d in \"%s\"!\n", i, ffn);
    }
}

// load the pointers for "coarse-search + fine-modeling"
void SynchronousGrammar::LoadCSFMRulePointers(char * pline, SCFGRule * rule)
{
    rule->CSFMRules = NULL;
    rule->CSFMRuleNum = 0;

    if(!strcmp(pline, "0"))
        return;

    int len = (int)strlen(pline);
    int beg = 0, end = 0;
    int count = 0, pCount = 0;
    long p;

    for(end = 0; end < len + 1; end++ ){
        if(pline[end] != ' ' && pline[end] != '\0')
            continue;
        char copy  = pline[end];
        pline[end] = '\0';
        p = atoi(pline + beg); // the i-th feature
        pline[end] = ' ';
        beg = end + 1;

        if(count == 0){
            mem->Alloc(1);
            rule->CSFMRuleNum = p;
            rule->CSFMRules = (SCFGRule **)mem->Alloc(sizeof(SCFGRule *) * p);
            memset(rule->CSFMRules, 0, sizeof(SCFGRule *) * p);
        }
        else
            rule->CSFMRules[count - 1] = (SCFGRule *)p;

        pline[end] = copy;

        count++;
    }

    if(rule->CSFMRuleNum != count - 1)
        fprintf(stderr, "ERROR: pointer number doesn't match in \"%s\"\n", pline);
}


////////////////////////////////////
// other functions

int CompareUnitRules(const void * rule1, const void * rule2)
{
    UnitRule ** u1 = (UnitRule **)rule1;
    UnitRule ** u2 = (UnitRule **)rule2;
    float comp = (*u2)->feat[Model::RuleSoringDimen] - (*u1)->feat[Model::RuleSoringDimen];
    
    if(comp > 0)
        return 1;
    else if(comp < 0)
        return -1;
    else
        return 0;
}

}

