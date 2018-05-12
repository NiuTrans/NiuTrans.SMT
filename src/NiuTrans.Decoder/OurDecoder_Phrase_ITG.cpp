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
 * ITG-Based decoder for phrase-based SMT; OurDecoder_Phrase_ITG.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (xiaotong@mail.neu.edu.cn); December 20th, 2010
 * Hao Zhang (email: zhanghao1216@gmail.com); January 21th, 2011
 *
 * $Last Modified by:
 * 2014/11/26 Tong Xiao (xiaotong@mail.neu.edu.cn) - bug fixes for decoding failure in t2s models
 * 2012/09/13 Tong Xiao (xiaotong@mail.neu.edu.cn) - add a method to dump word translations
 * 2011/06/19 Tong Xiao (xiaotong@mail.neu.edu.cn) - bug fixes
 * 2011/05/09 Hao Zhang (zhanghao1216@gmail.com) - bug fixes
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "OurDecoder_Phrase_ITG.h"

namespace smt {

////////////////////////////////////////
// phrase-based decoder
// Created by Xiao Tong and Zhang Zhao

Decoder_Phrase_ITG::Decoder_Phrase_ITG()
{
    cell               = NULL;
    mem                = NULL;
    maxWordNumInPhrase = 0;
    nbest              = 0;
    beamSize           = 0;
    featNum            = 0;
    partialTrans       = NULL;
    nullString[0]      = '\0';
    beamScale          = 1;
    useMEReordering    = true;
    usePuncPruning     = false;
    boundaryWordNumForPunctPruning = 2;
    useCubePruning     = false;
	useCubePruningFaster = false;
	maxItemNumInCubePruning = 0;

    explored           = NULL;
	explored2          = NULL;
    hypoQueue          = NULL;
	heapForSearch      = NULL;
    ignoreComposeNum = 0;
    totalComposeNum = 0;
    
    factoidCount = 0;
    factoids = new char*[128];
    MEReFeats = new char*[128];
    for( int i = 0 ; i < 128; i++ ){
        factoids[i] = new char[128];
        MEReFeats[i] = new char[128];
        memset(factoids[i], 0, sizeof(char) * 128);
        memset(MEReFeats[i], 0, sizeof(char) * 128);
    }

    strcpy(factoids[factoidCount++], "$number");
    strcpy(factoids[factoidCount++], "$time");
    strcpy(factoids[factoidCount++], "$date");

    serverLog = new char[M_BYTE];
}

Decoder_Phrase_ITG::~Decoder_Phrase_ITG()
{
    Unload();
}

void Decoder_Phrase_ITG::Unload()
{
	int cc = 0, mc = 0;
    if( cell != NULL ){
        for( int i = 0; i < MAX_WORD_NUM; i++ ){
            for( int j = i + 1; j < MAX_WORD_NUM; j++ ){
            	cc += cell[i][j].tList->count;
            	mc += cell[i][j].tList->maxNum;
            }
            delete[] cell[i];
        }
        delete[] cell;
    }

    delete mem;
	mem = NULL;


    delete[] partialTrans;

    for( int i = 0; i < 128; i++ ){
        delete[] factoids[i];
        delete[] MEReFeats[i];
    }
    delete[] factoids;
    delete[] MEReFeats;
    delete[] MEReFeatsForSrc;


    for( int k=0; k < beamSize+20; k++ )
        delete[] explored[k];
	delete[] explored2;
    delete[] explored;
    delete   hypoQueue;
	delete heapForSearch;
    delete[] serverLog;

    if(lossAugumentedDecoding){
        for(int i = 0; i < refNum; i++)
            delete refNGram[i];
        delete[] refNGram;
        refNGram = NULL;
    }
}

void Decoder_Phrase_ITG::Init(Model * m)
{
    CreatBase(m);

    LoadSettings();
    featNum = m->featNum;

	mem = new MemPool(M_BYTE * memBlockSize, memBlockNum);

    MEReFeatsForSrc = new MEReFeatInfo[MAX_WORD_NUM];

    explored = new int*[beamSize+20];
    for( int k=0; k < beamSize+20; k++ ) {
        explored[k] = new int[beamSize+20];
    }

	if(useCubePruningFaster){
		maxItemNumInCubePruning = maxBundleNum * (beamSize + 20) * (beamSize + 20);
		explored2 = new bool[maxItemNumInCubePruning];
	}

    int hypoQueueSize = 2 * beamSize * beamSize;
    if( hypoQueueSize < 50 )
        hypoQueueSize = 50;
    hypoQueue = new HypoHeap(hypoQueueSize, MAXHEAP);
    heapForSearch = new NodeHeapForSearch(hypoQueueSize, MAXHEAP);

    srcTree = new Tree();

    InitCell();
    InitRefNGram();
}

void Decoder_Phrase_ITG::InitCell()
{
    cell = new Cell*[MAX_WORD_NUM];
    for( int i = 0; i < MAX_WORD_NUM; i++ ){
        cell[i] = new Cell[MAX_WORD_NUM];
        for( int j = i + 1; j < MAX_WORD_NUM; j++ ){
            cell[i][j].Init(i, j, this, beamSize, mem);
        }
    }
}

void Decoder_Phrase_ITG::InitRefNGram()
{
    if(lossAugumentedDecoding){
        refNGram = new RefNGram*[refNum];
        for(int i = 0; i < refNum; i++)
            refNGram[i] = new RefNGram(MAX_SENT_LENGTH_SHORT, bleuLossNGram);
    }
    else
        refNGram = NULL;

}

void Decoder_Phrase_ITG::ResetTransInfo()
{
    mem->Clear(M_BYTE * minMemSize);

    for( int i = 0; i <= srcLength; i++ )
    {
        bool havePunct = false;
        bool haveFactoid = false;
        for( int j = i + 1; j <= srcLength; j++ )
        {
            cell[i][j].Clear();

            //>>>if( Util::IsPunc(srcWords[j - 1]) )
            if( StringUtil::IsPunc( srcWords[j-1] ) )
                havePunct = true;
            if( IsFactoid(srcWords[j - 1]) )
                haveFactoid = true;

            cell[i][j].havePunct = havePunct;
            cell[i][j].haveFactoid = haveFactoid;
            cell[i][j].forcedTrans = false;
            cell[i][j].valid = true;
        }
    }
    delete[] partialTrans;
    partialTrans = NULL;

    memset(MEReFeatsForSrc, 0, sizeof(MEReFeatInfo) * srcLength);

    serverLog[0] = '\0';
}

void Decoder_Phrase_ITG::LoadSettings()
{
    maxWordNumInPhrase = configer->GetInt("maxphraselength", 7);
    ngram              = configer->GetInt("ngram", 3);
    ngram2             = configer->GetInt("ngram2", 3);
    nbest              = configer->GetInt("nbest", 256);
    beamSize           = configer->GetInt("beamsize", 20);
    maxDistortionDistance = configer->GetInt("maxdd", 10);
    useMEReordering    = configer->GetBool("use-me-reorder", true);
    usePuncPruning     = configer->GetBool("usepuncpruning", false);
    boundaryWordNumForPunctPruning = configer->GetInt("boundwordnum", 2);
    useCubePruning     = configer->GetBool("usecubepruning", false);
	useCubePruningFaster = configer->GetBool("usecubepruning2", false) || configer->GetBool("usecubepruningfaster", false);
	useCubePruningFaster = (useCubePruningFaster || configer->GetInt("usecubepruning", 0) == 2);
	maxBundleNum       = configer->GetInt("nboundle", 20);
    useMSDReordering   = configer->GetBool( "use-msd-reorder", true );
    beamScale          = configer->GetFloat("beamscale", 1.0);
    outputOOV          = configer->GetBool("outputoov", false);
    labelOOV           = configer->GetBool("labeloov", false);
    toEnglish          = configer->GetBool("toenglish", false);
    allowSequentialNULLTrans = configer->GetBool("snulltrans", false);
    dumpLeftHypo       = configer->GetBool("dumplefthypo", false);
    normalizeOutput    = configer->GetBool("normalizeoutput", false);
    maxNumOfSymbolWithSameTrans     = configer->GetInt("maxnosymboltrans", MAX_HYPO_NUM_IN_CELL);
    maxNumOfFineSymbolWithSameTrans = configer->GetInt("maxnofinesymboltrans", MAX_HYPO_NUM_IN_CELL);
    maxNumOfHPhraseHypo             = configer->GetInt("maxphrasehyponum", 0x7FFFFFFF);
    serverDisplay      = configer->GetBool("serverdisplay", false);
    posteriorAlpha     = configer->GetFloat("posterioralpha", 0.5);
    forcedDecoding     = configer->GetBool("forceddecoding", false) || configer->GetBool("forced", false);
    lossAugumentedDecoding = configer->GetBool("lossaugumenteddecoding", false) || 
                             configer->GetBool("lossaug", false);
    beamLoss           = configer->GetBool("beamloss", true); // loss for beam search
    bleuLoss           = configer->GetBool("bleuloss", true); // goodness loss
    bleuLossNGram      = configer->GetInt("bleulossngram", 4);// ngram order for goodness loss
    beamLossK          = configer->GetInt("beamlossk", beamSize);  // a parameter in beam loss
    beamLossWeight     = configer->GetFloat("beamlossweight", 0.1); // weight of beam loss
    bleuLossWeight     = configer->GetFloat("bleulossweight", 1); // weight of bleu loss
    refNum             = configer->GetInt("nref", 4);         // number of reference translations
    dumpCell           = configer->GetBool("dumpcell", false);
    doRecasing         = configer->GetBool("dorecasing", false) || configer->GetBool("recasing", false);
    dumpWordTrans      = configer->GetBool("anchortuning", false);
    useUppercase       = configer->GetBool("useuppercase", false);
	smallMem           = configer->GetBool("smallmem", false);
	memBlockSize       = configer->GetInt("memblocksize", 16);
	memBlockNum        = configer->GetInt("memblocknum", 1024);
	minMemSize         = configer->GetInt("minmemsize", 64);
    multiLMs           = configer->GetBool("charngramlm", false);

    if(doRecasing){
        useMEReordering  = false;
        useMSDReordering = false;
        usePuncPruning   = false;
    }
}

void Decoder_Phrase_ITG::DecodeInput(DecodingSentence * sentence)
{
    decodingFailure = false;
    decodingSent = sentence;

    if(forcedDecoding)
        LoadRef(sentence);

    LoadInputSentence(sentence->string);
    ResetTransInfo();
    SetUserTranslations(PHRASE_BASED);
    if(useMEReordering)
        MatchSrcReorderFeat(model);
    MatchPhrases();

    CKYDecoding(sentence);

    if(cell[0][srcLength].n == 0)
        decodingFailure = true;
}

void Decoder_Phrase_ITG::CKYDecoding(DecodingSentence * sent)
{
	//fprintf(stderr, "%d mem used=%d\n", sent->id, mem->GetUsedMemSize());
    
    for( int len = 1; len <= srcLength; len++ ){
        for( int beg = 0; beg + len <= srcLength; beg++ ){
            int end = beg + len;

            if(len > 1 && cell[beg][end].valid)
                GenerateTrans(&cell[beg][end]);

            if(dumpCell){
                char fn[1024], rule[1024];
                sprintf(fn, "./cell/[%d]-%d-%d.txt", sent->id, beg, end);
                FILE * f = fopen(fn, "w");
                for(int i = 0; i < cell[beg][end].n; i++){
                    CellHypo * ch = cell[beg][end].nList[i];
                    if(ch->ruleUsed != NULL){
                        UnitRule * urule = (UnitRule *)ch->ruleUsed;
                        SCFGRule * srule = urule->parentRule;
                        sprintf(rule, "%s ||| %s -> %s", urule->src, srule->src, srule->tgt);
                    }
                    else{
                        UnitRule * rule0 = (UnitRule*)ch->lc;
                        UnitRule * rule1 = (UnitRule*)ch->rc;
                        sprintf(rule, "<glue rule> : %s %s", ch->lc == NULL ? "<NULL>" : ch->lc->root, ch->rc == NULL ? "<NULL>" : ch->rc->root);
                        sprintf(rule, "%s [%d, %d] [%d, %d] %d %d", rule, ch->lc == NULL ? -1 : ch->lc->cell->beg, ch->lc == NULL ? -1 : ch->lc->cell->end, 
                                                            ch->rc == NULL ? -1 : ch->rc->cell->beg, ch->rc == NULL ? -1 : ch->rc->cell->end,
                                                            ch->lc == NULL ? -1 : SynchronousGrammar::IsComplete(ch->lc->root), ch->rc == NULL ? -1 : SynchronousGrammar::IsComplete(ch->rc->root));
                    }
                    fprintf(f, "%2d[%.4f]: %s %s: %s ||| %s ||| %.4f |||", i, ch->modelScore, ch->root, ch->rootFineModel, ch->translation, rule, ch->modelScore);

                    if(ch->LMScore != NULL){
                        for(int i = 0; i < ch->wCount; i++){
                            fprintf(f, " %.3f", ch->LMScore[i]);
                        }

                        fprintf(f, " |||");

                        for(int i = 0; i < ch->wCount; i++){
                            fprintf(f, " %d", ch->wid[i]);
                        }
                    }
                    fprintf(f, "\n");
                }
                fclose(f);
            }
        }
    }
}

char* Decoder_Phrase_ITG::GenDecodingLog( CellHypo* ch ) {

    char* ret;
    int len;

    if( !ch )
        return GlobalVar::nullString;

    if( !ch->lc && !ch->rc ) {
        char *src = GetSrcString(ch->cell->beg, ch->cell->end - 1, mem);
        len = 40 + (int)strlen( src ) + (int)strlen( ch->translation ) + 1;
        ret = new char[len];
        if(ch->cell->beg == 0 && ch->cell->end == 1)
            sprintf( ret, "[%d, %d]: %s => %s\n", ch->cell->beg, ch->cell->end, src, "<s>" );
        else if(ch->cell->beg == srcLength - 1 && ch->cell->end == srcLength)
            sprintf( ret, "[%d, %d]: %s => %s\n", ch->cell->beg, ch->cell->end, src, "</s>" );
        else
            sprintf( ret, "[%d, %d]: %s => %s\n", ch->cell->beg, ch->cell->end, src, ch->translation );
    }
    else {
        char* ls = GenDecodingLog( ch->lc );
        char* rs = GenDecodingLog( ch->rc );
        len = (int)strlen( ls ) + (int)strlen( rs ) + 1;
        ret = new char[len];
        sprintf( ret, "%s%s", ls, rs );

        if(ch->lc != NULL)
            delete[] ls;
        if(ch->rc != NULL)
            delete[] rs;
    }

    return ret;

}

void Decoder_Phrase_ITG::DumpSpanTransForServerDisplay(char * log, Cell * c)
{
    float probSum = 0;
    sprintf(log, "%s%d || %d", log, c->beg, c->end);

    for(int i = 0; i < c->n; i++){
        probSum += exp(c->nList[i]->modelScore * posteriorAlpha);
    }

    for(int i = 0; i < c->n; i++){
        char buf[MAX_LINE_LENGTH] = "";
        float prob = exp(c->nList[i]->modelScore * posteriorAlpha) / probSum;
        if(prob < 0 || prob > 1)
            prob = 1.0;

        CellHypo * ch = c->nList[i];

        if(c->beg == 0 && c->end == 1)
            sprintf(buf, " ||| <s> || 1.0");
        else if(c->beg == srcLength - 1 && c->end == srcLength)
            sprintf(buf, " ||| </s> || 1.0");
        else if(!strcmp(ch->translation, "") && c->end - c->beg == 1)
            sprintf(buf, " ||| <%s> || 0.0", srcWords[c->beg]);
        else if(strcmp(ch->translation, ""))
            sprintf(buf, " ||| %s || %.2f", ch->translation, prob);

        strcat(log, buf);
    }
    strcat(log, "\n");
}

int Decoder_Phrase_ITG::DumpTransResult(int nbest, TransResult * result, DecodingLoger * log)
{
    CellHypo ** nlist = cell[0][srcLength].nList;
    int n = cell[0][srcLength].n;

    if( nbest > n )
        nbest = n;

    for( int i = 0; i < nbest; i++ ){
        CellHypo * ch = nlist[i];
        int transLength = (int)strlen(ch->translation);

        result[i].modelScore = ch->modelScore;            // model score
        if(normalizeOutput)
            result[i].translation = StringUtil::NormalizeText(ch->translation);
        else{
            result[i].translation = new char[transLength * 3 + 1];
            memset(result[i].translation, 0, sizeof(char)*(transLength + 1));
            strcpy(result[i].translation, ch->translation);   // translation
        }
        result[i].featValues = new float[model->featNum];
        memcpy(result[i].featValues, ch->featValues, sizeof(float)*model->featNum); // features

        if(dumpWordTrans){  // dump word translation
            WordTranslation * wordTrans = new WordTranslation();
            wordTrans->Create(srcLength - 2);
            result[i].wordTranslation = (void*)wordTrans;
            DumpWordAlignment(result + i, ch);
        }

        if(serverDisplay){
            if(srcSent == NULL){
                result[i].log = new char[30];
                sprintf(result[i].log, "\n================");
            }
            else{
                char * viterbiDerivationLog = GenDecodingLog(ch);
                result[i].log = new char[strlen(srcSent) + strlen(serverLog) + strlen(viterbiDerivationLog) + 50];
                sprintf(result[i].log, "<s> %s </s>\n%s****************\n%s================", srcSent, serverLog, viterbiDerivationLog);
            }
        }
        else
            result[i].log = GenDecodingLog( ch );
    }

    DumpLog(log);

    return nbest;
}

int Decoder_Phrase_ITG::DumpWordAlignment(TransResult * result, CellHypo * ch)
{
	WordTranslation * wordt = (WordTranslation *)result->wordTranslation;
	int wordc = 0;

	if( !ch )
		return 0;

	if( !ch->lc && !ch->rc ) {
		if(ch->cell->beg == 0 && ch->cell->end == 1)
			return 1;
		else if(ch->cell->beg == srcLength - 1 && ch->cell->end == srcLength)
			return 1;
		else{
			for(int i = ch->cell->beg; i < ch->cell->end; i++)
				wordt->Add(i - 1, ch->translation);
			return ch->cell->end - ch->cell->beg;
		}
	}
	else {
		wordc += DumpWordAlignment(result, ch->lc);
		wordc += DumpWordAlignment(result, ch->rc);
	}

    return wordc;
}

// fix user-input translations into decoding
void Decoder_Phrase_ITG::SetUserTranslations(DECODER_TYPE decoderType, NEToNTSymbol * NESymbol, NEToNTSymbol * NESymbolFineModel)
{
    partialTrans = new UserTranslation*[srcLength];
    memset(partialTrans, 0, sizeof(UserTranslation*) * srcLength);

    for( int i = 0; i < userTransCount; i++ ){
        UserTranslation * utrans = userTrans + i;
        Cell * c = &cell[utrans->beg][utrans->end];

        if( IsFactoid(utrans->type) && utrans->end - utrans->beg == 1 ){  // for $number, $time, $date etc.
            if(partialTrans[utrans->beg] == NULL || 
               strcmp(utrans->type, "$number") || strpbrk(utrans->translation, "0123456789"))
                partialTrans[utrans->beg] = utrans;
        }
        else if( IsByline(utrans->type) ){ // for $byline
            CellHypo * ch = (CellHypo*)mem->Alloc(sizeof(CellHypo));
            ch->Init(c, mem, model->featNum);
            ch->Create(utrans->translation, model);

            if(multiLMs)
                AddCharNGramLM(c, ch, utrans->translation);

            if(decoderType == PHRASE_BASED)
                SetBoundaryTgtWords(ch, false, mem);

            ch->featValues[Model::BiLexCount]  = (float)ch->wCount;

            ch->cell = c;

            ch->tLeftPhrase->Init(utrans->translation, utrans->beg, utrans->end - 1);
            ch->tRightPhrase->Init(utrans->translation, utrans->beg, utrans->end - 1);

            if(NESymbol != NULL)
                ch->root = NESymbol->GetSymbolByDefault(utrans->type);
            if(NESymbolFineModel != NULL)
                ch->rootFineModel = NESymbolFineModel->GetSymbolByDefault(utrans->type);

            c->AddCellHypo(ch);
        }
        else if( IsNE(utrans->type) ){    // for $loc, $person etc

            CellHypo * ch = (CellHypo*)mem->Alloc(sizeof(CellHypo));
            ch->Init(c, mem, model->featNum);
            ch->Create(utrans->translation, model);

            if(multiLMs)
                AddCharNGramLM(c, ch, utrans->translation);

            if(decoderType == PHRASE_BASED)
                SetBoundaryTgtWords(ch, false, mem);

            // set feature values
            if(decoderType == PHRASE_BASED)
                ch->featValues[Model::PhraseCount] = (float)1;
            else if(decoderType == HIERO || decoderType == SYNTAX_BASED)
                ch->featValues[Model::RuleCount] = (float)1;
            ch->featValues[Model::WordCount]   = (float)ch->wCount;
            if( utrans->beg > 0 && utrans->end < srcLength - 1){
                ch->featValues[Model::PrF2E]    = (float)log(0.0001);
                ch->featValues[Model::PrF2ELex] = (float)log(0.0001);
                ch->featValues[Model::PrE2F]    = (float)log(0.0001);
                ch->featValues[Model::PrE2FLex] = (float)log(0.0001);
            }

            //ch->cell = c;
            ComputeModelScore(ch);

            ch->tLeftPhrase->Init(utrans->translation, utrans->beg, utrans->end - 1);
            ch->tRightPhrase->Init(utrans->translation, utrans->beg, utrans->end - 1);

            if(NESymbol != NULL)
                ch->root = NESymbol->GetSymbolByDefault(utrans->type);
            if(NESymbolFineModel != NULL)
                ch->rootFineModel = NESymbolFineModel->GetSymbolByDefault(utrans->type);

            c->AddCellHypo(ch);

            if(!strcmp(utrans->type, "$forced") || !strcmp(utrans->type, "$literal"))
                SetForcedTrans(utrans->beg, utrans->end);
        }
    }
}

void Decoder_Phrase_ITG::ComputeModelScore(CellHypo * ch)
{
    if( ch->LMScore == NULL )
        ch->LMScore = (float*)mem->Alloc(sizeof(float) * ch->wCount);

    // n-gram language model
    float overallLMScore = 0;
    for( int i = 0; i < ch->wCount; i++){
        int end = i + 1;
        int beg = end - ngram > 0 ? end - ngram : 0;
        float prob = model->GetNGramLMProb(beg, end, ch->wid);
        ch->LMScore[i] = prob;
        overallLMScore += prob;
    }
    ch->featValues[Model::NGramLM] = overallLMScore;

    if(multiLMs)
        Compute2ndLMScore(ch);

    // (overall) model score
    float overallScore = 0;
    for( int f = 0; f < model->featNum; f++){
        overallScore += ch->featValues[f] * model->featWeight[f];
    }
    ch->modelScore = overallScore;
}

void Decoder_Phrase_ITG::Compute2ndLMScore(CellHypo * ch)
{
    if( ch->LMScore2 == NULL )
        ch->LMScore2 = (float*)mem->Alloc(sizeof(float) * ch->wCount2);

    // n-gram language model
    float overallLMScore = 0;
    for( int i = 0; i < ch->wCount2; i++){
        int end = i + 1;
        int beg = end - ngram2 > 0 ? end - ngram2 : 0;
        float prob = model->GetNGramLMProb(beg, end, ch->wid2);
        ch->LMScore2[i] = prob;
        overallLMScore += prob;
    }
    ch->featValues[Model::NGramLM2]   = overallLMScore;
    ch->featValues[Model::WordCount2] = ch->wCount2;
}

void Decoder_Phrase_ITG::SetForcedTrans(int beg, int end)
{
    cell[beg][end].forcedTrans = true;

    for(int l = 1; l < end - beg; l++){
        for(int b = beg; b + l < end - beg; b++){
            int e = beg + l;
            cell[b][e].valid = false;
        }
    }
}

void Decoder_Phrase_ITG::SetBoundaryTgtWords(CellHypo * ch, bool LM2, MemPool * curMem)
{
	if(LM2 && ch->wCount2 > 0){
		ch->ltgtword = model->GetWord(ch->wid2[0]);
        ch->rtgtword = model->GetWord(ch->wid2[ch->wCount2 - 1]);
	}
    else if( ch->wCount > 0 ){
        ch->ltgtword = model->GetWord(ch->wid[0]);
        ch->rtgtword = model->GetWord(ch->wid[ch->wCount - 1]);
    }
    else{
        ch->ltgtword = nullString;
        ch->rtgtword = nullString;
    }

    char * word1 = ch->ltgtword;
    char * word2 = ch->rtgtword;
    MEReFeatInfo * info = (MEReFeatInfo*)curMem->Alloc(sizeof(MEReFeatInfo));
    float * feat = info->feats;
    MEReorderModel * MEModel = model->MEReModel;

    // reordering features used in ME-based reordering model
    feat[0] = MEModel->GetFeatWeight(0, 4, word1);
    feat[1] = MEModel->GetFeatWeight(0, 6, word1);
    feat[2] = MEModel->GetFeatWeight(1, 4, word1);
    feat[3] = MEModel->GetFeatWeight(1, 6, word1);
    feat[4] = MEModel->GetFeatWeight(0, 5, word2);
    feat[5] = MEModel->GetFeatWeight(0, 7, word2);
    feat[6] = MEModel->GetFeatWeight(1, 5, word2);
    feat[7] = MEModel->GetFeatWeight(1, 7, word2);

    ch->MERe = info;
}

void Decoder_Phrase_ITG::SetBoundaryTgtWords(CellHypo * rch, CellHypo * ch1, CellHypo * ch2, bool moreLMs, MemPool * curMem)
{
    MEReFeatInfo * info = (MEReFeatInfo*)curMem->Alloc(sizeof(MEReFeatInfo));
    float * feats = info->feats;
    rch->MERe = info;
	bool ch1nothing = ch1->wCount == 0;
	bool ch2nothing = ch2->wCount == 0;

	if(moreLMs){
		ch1nothing = ch1nothing && ch1->wCount2 == 0;
		ch2nothing = ch2nothing && ch2->wCount2 == 0;
	}

    // target boundary words
    if(ch1nothing && ch2nothing == 0){
        rch->ltgtword = nullString;
        rch->rtgtword = nullString;
        memset(feats, 0, sizeof(float) * 8);
    }
    else if(ch1nothing){
        rch->ltgtword = ch2->ltgtword;
        rch->rtgtword = ch2->rtgtword;
        memcpy(feats, ch2->MERe->feats, sizeof(float) * 8);
    }
    else if(ch2nothing){
        rch->ltgtword = ch1->ltgtword;
        rch->rtgtword = ch1->rtgtword;
        memcpy(feats, ch1->MERe->feats, sizeof(float) * 8);
    }
    else{
        rch->ltgtword = ch1->ltgtword;
        rch->rtgtword = ch2->rtgtword;
        /*for( int f = 0; f < 8; f += 2 ){
            feats[f] = ch1->MERe->feats[f];
            feats[f + 1] = ch2->MERe->feats[f + 1];
        }*/
        memcpy(feats, ch1->MERe->feats, sizeof(float) * 4);
        memcpy(feats + 4, ch2->MERe->feats + 4, sizeof(float) * 4);

        // 0: class-0 left-1
        // 1: class-0 right-1
        // 2: class-1 left-1
        // 3: class-1 right-1
        // 4: class-0 left-2
        // 5: class-0 right-2
        // 6: class-1 left-2
        // 7: class-1 right-2
    }
}

void Decoder_Phrase_ITG::MatchSrcReorderFeat(Model * m)
{
    MEReorderModel * MEModel = m->MEReModel;

    for( int i = 0; i < srcLength; i++ ){
        char * word = srcWords[i];
        float * feat = MEReFeatsForSrc[i].feats;

        feat[0] = MEModel->GetFeatWeight(0, 0, word);
        feat[1] = MEModel->GetFeatWeight(0, 2, word);
        feat[2] = MEModel->GetFeatWeight(1, 0, word);
        feat[3] = MEModel->GetFeatWeight(1, 2, word);
        feat[4] = MEModel->GetFeatWeight(0, 1, word);
        feat[5] = MEModel->GetFeatWeight(0, 3, word);
        feat[6] = MEModel->GetFeatWeight(1, 1, word);
        feat[7] = MEModel->GetFeatWeight(1, 3, word);
    }
}

void Decoder_Phrase_ITG::MatchPhrases()
{
    char phrase[MAX_LINE_LENGTH];

    for( int beg = 0; beg < srcLength; beg++ ){
        phrase[0] = '\0';
        bool haveFactoid = false;
        bool forcedTrans = false;
        for( int end = beg + 1 ; end - beg <= maxWordNumInPhrase && end <= srcLength; end++){
            if(end == beg + 1)
                strcpy(phrase, srcWords[end - 1]);
            else
                sprintf(phrase, "%s %s", phrase, srcWords[end - 1]);

            if(cell[end-1][end].forcedTrans)
                forcedTrans = true;
            else if(forcedTrans)
                break;

            if( IsFactoid(srcWords[end - 1]) )
                haveFactoid = true;

            if(!cell[beg][end].forcedTrans && cell[beg][end].valid)
                AddPhrase(&cell[beg][end], phrase, haveFactoid);

			cell[beg][end].CompleteWithBeamPruning(-1, this, false, false, true, false, mem);

            if(serverDisplay)
                DumpSpanTransForServerDisplay(serverLog, &cell[beg][end]);
        }

    }
}

void Decoder_Phrase_ITG::AddPhrase(Cell * c, char * srcPhrase, bool haveFactoid)
{
    List * optionList = model->FindOptionList(srcPhrase);
    bool useUNKList = false;
    int inListSize = c->tList->count;

    if( optionList == NULL || optionList->count == 0){
        //>>>Util::ToLowerCase(srcPhrase); // lower case
        StringUtil::ToLowercase( srcPhrase );
        optionList = model->FindOptionList(srcPhrase);
    }

    if( optionList == NULL || optionList->count == 0){
        if(c->end - c->beg > 1)
            return;
        if(c->tList->count > 0)
            return;
        optionList = model->FindOptionList("<unk>"); // othewise, unknown words
        useUNKList = true;
    }

    for( int i = 0; i < optionList->count; i++ ){
        PhraseTransOption * option = (PhraseTransOption *)optionList->items[i];

        if(option == NULL)
            continue;
        if(i >= maxNumOfHPhraseHypo)
            break;

        CellHypo * ch = (CellHypo*)mem->Alloc(sizeof(CellHypo));
        ch->Init(c, mem, model->featNum);

        if(multiLMs)
            AddCharNGramLM(c, ch, option->tgt);
        
        ch->translation = haveFactoid ? ReplaceFoctoidTrans(c, option->tgt, mem) : option->tgt;

        // new code, recase literal terms (only for English translation)
        if(toEnglish){
            for(int i = c->beg; i < c->end; i++){
                if(srcWordInfo[i].isIiteral)
                    ch->translation = RecaseLiteral(i, ch->translation, mem);
            }
        }

        if(useUNKList && outputOOV){
            ch->translation = (char *)mem->Alloc(sizeof(char) * (strlen(srcWords[c->beg]) + 3));
            memset(ch->translation, 0, sizeof(char) * (strlen(srcWords[c->beg]) + 3));

            if(labelOOV)
                sprintf(ch->translation, "<%s>", srcWords[c->beg]);
            else
                sprintf(ch->translation, "%s", srcWords[c->beg]);
        }

        ch->transLength = (int)strlen(ch->translation);

        ch->wid = option->wid;
        ch->wCount = option->wCount;
        memcpy(ch->featValues, option->feat, sizeof(float) * model->featNum); // feature values

        SetBoundaryTgtWords(ch, false, mem); // bundary words
        ComputeModelScore(ch);   // computing the model score

        ch->tLeftPhrase->Init(option->tgt, c->beg, c->end - 1);
        ch->tRightPhrase->Init(option->tgt, c->beg, c->end - 1);

        if(forcedDecoding && !useUNKList){
            if(!FindTransInRef(ch->wid, ch->wCount, ch->matchedRefPOSCount, ch->matchedRefPOS, mem))
                continue;
        }

        if(useUppercase)
            RefineOptions(c, inListSize, ch);

        c->AddCellHypo(ch);
        //fprintf(GlobalVar::lmlogFile, "%s ||| %s ||| %d ||| %d ||| %.4f ||| %.4f\r\n", srcPhrase, ch->translation, c->beg, c->end, ch->featValues[Model::NGramLM], ch->modelScore);
    }
}

// the function scans the translation options that are already in the candidate list
// if the input hypothesis (i.e., ch) has the same translation as some of the existing candidates,
// the model score of those candidates will be updated according to the input hypothesis
void Decoder_Phrase_ITG::RefineOptions(Cell * c, int inListSize, CellHypo * ch)
{
    List * list = c->tList;

    for(int i = 0; i < list->count && i < inListSize; i++){
        CellHypo * cur = (CellHypo *)list->GetItem(i);
        if(cur->wCount != ch->wCount)
            continue;
        if(memcmp(cur->wid, ch->wid, sizeof(int) * ch->wCount) == 0){
            cur->modelScore = ch->modelScore;
            memcpy(cur->featValues, ch->featValues, sizeof(float) * model->featNum);

            AddFeatValue(cur, model->PrF2E, 0.2);
            AddFeatValue(cur, model->PrE2F, 0.2);
        }
    }
}

/*
Add another (character-based) language model (for English-Chinese translation)
*/
void Decoder_Phrase_ITG::AddCharNGramLM(Cell * c, CellHypo * ch, char * trans)
{
    /* for <s> and </s> */
    if(c->end - c->beg == 1 && (c->beg == 0 || c->end == srcLength - 1)){
        ch->LMScore2 = (float*)mem->Alloc(sizeof(float));
        ch->wid2 = (int*)mem->Alloc(sizeof(int));
        ch->wCount2 = 1;
        if(c->beg == 0)
            ch->wid2[0] = model->GetWId2ndVocab("<s>");
        else
            ch->wid2[0] = model->GetWId2ndVocab("</s>");
    }
    else if(ch->wCount == 1 && ch->wid[0] == model->vocabManager->unkWId){
        ch->LMScore2 = (float*)mem->Alloc(sizeof(float));
        ch->wid2 = (int*)mem->Alloc(sizeof(int));
        ch->wCount2 = 1;
        ch->wid2[0] = model->vocabManager->unkWId2;
    }
    else{
        char * charSeq = StringUtil::InternationalTokenization(trans);
        model->GetWId2ndVocab(charSeq, ch->wid2, ch->wCount2, mem);
        //fprintf(tmpF, "%s -> %s\n", ch->translation, charSeq);
        delete[] charSeq;
    }
}

// given $number -> 2000
// There are $number students in this school . -> There are 2000 students in this school .
char * Decoder_Phrase_ITG::ReplaceFoctoidTrans(Cell * c, char * trans, MemPool * myMem)
{
    char * result;
    char   resultTmp[MAX_TMP_TRANS_LENGTH] = "";
    char   resultLen = 0;
    int    beg = c->beg, end = c->end;
    int    len = (int)strlen(trans);
    int    p = 0, q = 0;
    int    fid = 0, fsrcCount = 0;
    int    totalLen = 0;

    while(p < len + 1){
        if( trans[p] == ' ' || p == len ){
            if(q >= p){
                p++;
                continue;
            }

            char tmp = trans[p];
            trans[p] = '\0';
            char * curWord = trans + q;

            int match = -1;
            for(int i = 0; i < factoidCount; i++){
                if( strcmp(curWord, factoids[i]) == 0 ){
                    match = i;
                    break;
                }
            }

            if( match >= 0){
                fsrcCount = 0;
                for( int i = beg; i < end; i++ ){
                    if( IsFactoid(srcWords[i]) ){
                        if( fid == fsrcCount ){
                            if( partialTrans[i] != NULL )
                                sprintf(resultTmp, "%s%s ", resultTmp, partialTrans[i]->translation);
                        }
                        fsrcCount ++;
                    }
                }
                fid++;
            }
            else
                sprintf(resultTmp, "%s%s ", resultTmp, curWord);

            trans[p++] = tmp;
            q = p;
        }
        else
            p++;
    }

    totalLen = (int)strlen(resultTmp);
    totalLen--; // remove the last space character
    if( totalLen == -1 )  // modified by zhanghao, 2011 April 22th
        totalLen = 0;

    resultTmp[totalLen] = '\0';

    result = (char*)myMem->Alloc(sizeof(char) * (totalLen + 1));
    strcpy(result, resultTmp);
    return result;
}

// nba-> NBA
char * Decoder_Phrase_ITG::RecaseLiteral(int iword, char * trans, MemPool * myMem)
{
    char * result;
    char * word = srcWords[iword];
    char   resultTmp[MAX_TMP_TRANS_LENGTH] = "";
    char   resultLen = 0;
    int    beg = 0, end = -1;
    int    wlen = srcWordLength[iword];
    int    tlen = (int)strlen(trans);

    for(int i = 0; i < tlen - wlen + 1; i++){
        int w, t;
        for(w = 0, t = 0; w < wlen && i + t < tlen; w++, t++){
            if(trans[i + t] == ' ' || trans[i + t] == '\t'){
                w--;
                continue;
            }

            if(word[w] != trans[i + t] && word[w] != trans[i + t] + 'A' - 'a')
                break;
        }

        if( w == wlen){ // matched
            result = (char*)myMem->Alloc(sizeof(char) * (tlen + 1));
            strcpy(result, trans);
            for(int j = 0; j < t; j++){
                if(result[i + j] >= 'a' && result[i + j] <= 'z')
                    result[i + j] += 'A' - 'a';
            }
            return result;
        }
    }

    return trans;
}

void Decoder_Phrase_ITG::GenerateTrans(Cell * c)
{
    if(doRecasing && c->beg > 0)
        return;

    int beg = c->beg, end = c->end;
	MemPool * tmpMem = smallMem ? new MemPool(M_BYTE * 32, 256) : mem;
    int nbestSize = (beg == 0 && end == srcLength) ? nbest : beamSize;
    bool lastUpdate = false;

    if( CheckForPunctPruning(beg, end)){

		if(!doRecasing && useCubePruningFaster)
			ComposeSpans_CubePruning_ForAllIncomingEdges(c, tmpMem); // cube pruing for all incoming edges
		else{
			/* search along every incoming edge */
			for( int mid = beg + 1; mid < end; mid++ ){  // partition of the span

				Cell * c1 = GetCell(beg, mid);
				Cell * c2 = GetCell(mid, end);

				if(c1->n == 0 || c2->n == 0)
					continue;

				if(doRecasing && end - mid > maxWordNumInPhrase)
					continue;

				lastUpdate = false;
				ComposeTwoSpans(c, c1, c2, tmpMem);
			}
		}
    }

    if( !lastUpdate )
		c->CompleteWithBeamPruning(nbestSize, this, false, false, true, false, mem);

	if(smallMem)
		delete tmpMem;
}

Cell * Decoder_Phrase_ITG::GetCell(int beg, int end)
{
	return &cell[beg][end];
}

void Decoder_Phrase_ITG::ComposeTwoSpans(Cell * c, Cell * sub1, Cell * sub2, MemPool * tmpMem)
{
    if(forcedDecoding)
        ComposeTwoSpansWithForcedDecoding(c, sub1, sub2, tmpMem);
    else if( !useCubePruning )
        ComposeTwoSpans_Naive(c, sub1, sub2, tmpMem);
    else
        ComposeTwoSpans_CubePruning_AlongOneEdge(c, sub1, sub2, tmpMem);

    int index = sub1->end - sub1->beg - 1;
}

void Decoder_Phrase_ITG::ComposeTwoSpans_Naive(Cell * c, Cell * sub1, Cell * sub2, MemPool * tmpMem)
{
    CellHypo ** tlist1 = sub1->nList;
    CellHypo ** tlist2 = sub2->nList;
    int tCount1 = sub1->n;
    int tCount2 = sub2->n;
    CellHypo * t1;
    CellHypo * t2;

    bool canBeReversed = CanBeReversed(c, sub1, sub2);

    for( int i1 = 0; i1 < tCount1; i1++ ){
        t1 = tlist1[i1];
        for( int i2 = 0; i2 < tCount2; i2++ ){
            t2 = tlist2[i2];

            bool localReorderFlag = !doRecasing && t1->wCount > 0 && t2->wCount > 0;

            CellHypo * ch1 = NULL;
            CellHypo * ch2 = NULL;

            ch1 = ComposeTwoHypotheses(c, t1, t2, PHRASE_BASED, tmpMem);
            if( ch1 ) {
                if( useMSDReordering ) {   // MSD re-ordering model
                    AddMSDReorderingScore( c, t1, t2, ch1 );
                }
                if( useMEReordering ) {    // ME re-ordering model
                }
                c->AddCellHypo(ch1);
            }
            if( canBeReversed && localReorderFlag ) {
                ch2 = ComposeTwoHypotheses( c, t2, t1, PHRASE_BASED, tmpMem );
                if( ch2 ) {
                    if( useMSDReordering ) {
                        AddMSDReorderingScore( c, t2, t1, ch2 );
                    }
                    if( useMEReordering ) {
                        CalculateReordering(t1, t2, ch1, ch2);
                    }
                    c->AddCellHypo(ch2);
                }
			}
        }
    }
}

void Decoder_Phrase_ITG::ComposeTwoSpans_CubePruning_AlongOneEdge(Cell * c, Cell * sub1, Cell * sub2, MemPool * tmpMem)
{
    CellHypo ** tlist1 = sub1->nList;
    CellHypo ** tlist2 = sub2->nList;
    int tCount1 = sub1->n;
    int tCount2 = sub2->n;
    int count = 0;
    int maxCount = (int)(beamSize * beamScale);

    ClearCubePruningData();
    bool canBeReversed = CanBeReversed(c, sub1, sub2);
    
    EnQueue(c, tlist1, tlist2, 0, 0, canBeReversed, tmpMem );
    explored[0][0] = 1;
    count = 0;
    while( hypoQueue->count > 0 ) {
        CellHypo* ch = hypoQueue->Pop();
        int r1 = ch->r1;
        int r2 = ch->r2;
        if( ch->lc ) {
            c->AddCellHypo( ch );
            ++count;
        }
        //if( count > beamSize + 10 )
        if( count > maxCount + 10) // modified by xiaotong 2011/01/07
        {
            break;
        }    
        if( r1 + 1 < tCount1 && !explored[r1+1][r2] )
        {
            EnQueue(c, tlist1, tlist2, r1+1, r2, canBeReversed, tmpMem );
            explored[r1+1][r2] = 1;
        }
        if( r2 + 1 < tCount2 && !explored[r1][r2+1] )
        {
            EnQueue(c, tlist1, tlist2, r1, r2+1, canBeReversed, tmpMem );
            explored[r1][r2+1] = 1;
        }
    }

}

void Decoder_Phrase_ITG::ComposeSpans_CubePruning_ForAllIncomingEdges(Cell * c, MemPool * tmpMem)
{
	int beg = c->beg, end = c->end, mid = beg;

	for(int k = c->beg; k < c->end; k += maxBundleNum){
		int bundleNum = k + maxBundleNum >= c->end ? c->end - k : maxBundleNum;
		ClearCubePruningData();

		/* Initializing the heap with the first hypothesis for each edge.
		   Here the set of hypotheses derived from a edge is called a hypothesis bundle
		*/
		for(mid = k + 1; mid < k + bundleNum; mid++){
			Cell * sub1 = GetCell(beg, mid);
			Cell * sub2 = GetCell(mid, end);
			
			if(sub1->n == 0 || sub2->n == 0)
				continue;

			if(doRecasing && end - mid > maxWordNumInPhrase)
                continue;

			bool localReorderFlag = sub1->nList[0]->wCount > 0 && sub2->nList[0]->wCount > 0;
			bool canBeReversed = CanBeReversed(c, sub1, sub2);

			EnQueue2(c, sub1->nList, sub2->nList, mid, mid - k, 0, 0, canBeReversed && localReorderFlag, tmpMem);
		}

		/* heuristics-based search (cube pruning) */
		int count = 0, b, r1, r2;
		int maxCount = (int)(beamSize * (beamScale > 1.2 && bundleNum > 2 ? beamScale : 1.2));
		while(heapForSearch->count > 0){
			ExploredSimpleNode * best = (ExploredSimpleNode *)heapForSearch->Pop(); // pop the top hypothsis

			/* put the best ones into the buffer */
			if(best->ch != NULL){
				c->AddCellHypo(best->ch);
				count++;
				if(best->ch2 != NULL){
					c->AddCellHypo(best->ch2);
					count++;
				}
			}

			if(count >= maxCount)
				break;

			short b = best->bundle - k;
			short r1 = best->offsets[0];
			short r2 = best->offsets[1];

			Cell * sub1 = GetCell(beg, best->bundle);
			Cell * sub2 = GetCell(best->bundle, end);

			/* search alone the left hypothsis */
			if(!CheckExplored2(b, r1 + 1, r2) && r1 + 1 < sub1->n)
				EnQueue2(c, sub1->nList, sub2->nList, best->bundle, b, r1 + 1, r2, CanBeReversed(c, sub1, sub2), tmpMem);

			/* search alone the right hypothsis */
			if(!CheckExplored2(b, r1, r2 + 1) && r2 + 1 < sub2->n)
				EnQueue2(c, sub1->nList, sub2->nList, best->bundle, b, r1, r2 + 1, CanBeReversed(c, sub1, sub2), tmpMem);
		}
	}
}

void Decoder_Phrase_ITG::ClearCubePruningData()
{
	/* clear matrix and hypo queue */
	if(!useCubePruningFaster){
		for( int i=0; i<beamSize; i++) {
			memset( explored[i], 0, sizeof(int)*beamSize );
		}
		hypoQueue->Clear();
	}
	else{
		memset(explored2, 0, sizeof(bool) * maxItemNumInCubePruning);
		heapForSearch->Clear();
	}
    
}

void Decoder_Phrase_ITG::ClearExplored()
{
    for( int i=0; i<beamSize; i++) {
        memset( explored[i], 0, sizeof(int)*beamSize );
    }
}

bool Decoder_Phrase_ITG::CheckExplored(int i, int j)
{
    return (explored[i][j] != 0);
}
void Decoder_Phrase_ITG::SetExplored(int i, int j)
{
    explored[i][j] = 1;
}

bool Decoder_Phrase_ITG::CheckExplored2(int bundle, int i, int j)
{
    return explored2[bundle * beamSize * beamSize + i * beamSize + j];
}
void Decoder_Phrase_ITG::SetExplored2(int bundle, int i, int j)
{
    explored2[bundle * beamSize * beamSize + i * beamSize + j] = true;
}


void Decoder_Phrase_ITG::EnQueue( Cell* c, CellHypo** tlist1, CellHypo** tlist2, int r1, int r2, bool canBeReversed, MemPool *tmpMem )
{
    CellHypo *t1, *t2, *ch1 = NULL, *ch2 = NULL;
    bool localReorderFlag;

    t1 = tlist1[r1];
    t2 = tlist2[r2];

	if(model->type == SKELETON_BASED)
		localReorderFlag = (t1->wCount > 0 || t1->wCount2 > 0) && (t2->wCount > 0 || t2->wCount2 > 0);
	else
		localReorderFlag = t1->wCount > 0 && t2->wCount > 0;

	/*ch1 = ComposeTwoHypotheses(c, t1, t2, PHRASE_BASED, tmpMem);
    if( ch1 ) {
        ch1->r1 = r1; ch1->r2 = r2;
        if( useMSDReordering ) {        // MSD re-ordering model
            AddMSDReorderingScore( c, t1, t2, ch1 );
        }
        if( useMEReordering ) {    // ME re-ordering model
        }
        hypoQueue->Push( ch1 );
    }
    if( canBeReversed && localReorderFlag ) {
        ch2 = ComposeTwoHypotheses( c, t2, t1, PHRASE_BASED, tmpMem );
        if( ch2 ) {
            ch2->r1 = r1; ch2->r2 = r2;
            if( useMSDReordering ) {
                AddMSDReorderingScore( c, t2, t1, ch2 );
            }
            if( useMEReordering ) {
                CalculateReordering(t1, t2, ch1, ch2);
            }
            hypoQueue->Push( ch2 );
        }
    }*/

	ComposeTwoHypothesesWithTheFullModel(c, t1, t2, PHRASE_BASED, ch1, ch2, canBeReversed && localReorderFlag, tmpMem);

	if(ch1 != NULL){
		ch1->r1 = r1;
		ch1->r2 = r2;
		hypoQueue->Push(ch1);
	}
	if(ch2 != NULL){
		ch2->r1 = r1;
		ch2->r2 = r2;
		hypoQueue->Push(ch2);
	}
}

void Decoder_Phrase_ITG::EnQueue2( Cell* c, CellHypo ** tlist1, CellHypo** tlist2, int mid, short bundle, short r1, short r2, bool canBeReversed, MemPool *tmpMem )
{
	CellHypo * ch1 = NULL;
	CellHypo * ch2 = NULL;

	/* generate the hypotheses */
	ComposeTwoHypothesesWithTheFullModel(c, tlist1[r1], tlist2[r2], PHRASE_BASED, ch1, ch2, canBeReversed, tmpMem);

	/* put them into the heap */
	if(ch1 != NULL || ch2 != NULL){
		ExploredSimpleNode * exploredNode = (ExploredSimpleNode *)tmpMem->Alloc(sizeof(ExploredSimpleNode));
		exploredNode->ch  = (ch2 != NULL && ch2->modelScore > ch1->modelScore) ? ch2 : ch1;
		exploredNode->ch2 = exploredNode->ch != ch2 ? ch2 : ch1;
		exploredNode->bundle = mid;
		exploredNode->offsets = (short *)tmpMem->Alloc(sizeof(short) * 2);
		exploredNode->offsets[0] = r1;
		exploredNode->offsets[1] = r2;
		SetExplored2(bundle, r1, r2);
		heapForSearch->Push(exploredNode);
	}
}


CellHypo * Decoder_Phrase_ITG::ComposeTwoHypotheses(Cell * c, CellHypo * ch1, CellHypo * ch2, DECODER_TYPE d, MemPool * tmpMem)
{
	if(skeleton)
		return ComposeTwoHypothesesForSkeleton(c, ch1, ch2, d, tmpMem);

    ++totalComposeNum;
    if( !allowSequentialNULLTrans && ch1->wCount == 0 && ch2->wCount == 0)  // ??? TODO: remove it
        return NULL;

    int index = ch1->cell->end - ch1->cell->beg - 1;

    CellHypo * rch = (CellHypo*)tmpMem->Alloc(sizeof(CellHypo));
    rch->Init(c, tmpMem, model->featNum);

    // translation
    if( ch1->transLength == 0 || ch2->transLength == 0){
        rch->transLength = ch1->transLength + ch2->transLength;
        rch->translation = (char*)tmpMem->Alloc(sizeof(char) * (rch->transLength + 1));
        sprintf(rch->translation, "%s%s", ch1->translation, ch2->translation);
    }
    else{
        rch->transLength = ch1->transLength + ch2->transLength + 1;
        rch->translation = (char*)tmpMem->Alloc(sizeof(char) * (rch->transLength + 1));
        sprintf(rch->translation, "%s %s", ch1->translation, ch2->translation);
    }
    rch->translation[rch->transLength] = '\0';

    // left and right children
    rch->lc = ch1;
    rch->rc = ch2;

    // n-gram language model
    rch->wCount = ch1->wCount + ch2->wCount;
    float NGramLMScore = 0;
    int wc = rch->wCount;
    int * wid = (int*)tmpMem->Alloc(sizeof(int) * wc);
    float * LM = (float*)tmpMem->Alloc(sizeof(float) * wc);
    for( int i = 0; i < wc; i++ ){
        if( i < ch1->wCount ){
            wid[i] = ch1->wid[i];
            LM[i] = ch1->LMScore[i];
        }
        else{
            wid[i] = ch2->wid[i - ch1->wCount];
            int e = i + 1;
            int b = e - ngram > 0 ? e - ngram : 0;
            LM[i] = b < ch1->wCount ? model->GetNGramLMProb(b, e, wid) : ch2->LMScore[i - ch1->wCount];
        }
        NGramLMScore += LM[i];
    }

    rch->wid = wid;
    rch->LMScore = LM;
    rch->wCount = wc;
    rch->featValues[Model::NGramLM] = NGramLMScore;

    // another language model
    if(multiLMs)
        Compute2ndLMScoreWhenComposeTwoHypotheses(rch, ch1, ch2, tmpMem);

    // tgt boundary words
    if(d == PHRASE_BASED)
        SetBoundaryTgtWords(rch, ch1, ch2, false, tmpMem);
    
    // model score
    float overallScore = 0;
    for( int f = 0; f < model->featNum; f++ ){
        if( f != Model::NGramLM && (!multiLMs || f != Model::NGramLM2))
            rch->featValues[f] = ch1->featValues[f] + ch2->featValues[f];
        overallScore += rch->featValues[f] * model->featWeight[f];
    }
    rch->modelScore = overallScore;

    if(lossAugumentedDecoding){
        rch->beamLoss = ch1->beamLoss + ch2->beamLoss;
        rch->bleuLoss = 0;
    }

    return rch;
}

void Decoder_Phrase_ITG::Compute2ndLMScoreWhenComposeTwoHypotheses(CellHypo * rch, CellHypo * ch1, CellHypo * ch2, MemPool * tmpMem)
{
    // n-gram language model
    rch->wCount2 = ch1->wCount2 + ch2->wCount2;
    float NGramLMScore2 = 0;
    int wc2 = rch->wCount2;
    int * wid2 = (int*)tmpMem->Alloc(sizeof(int) * wc2);
    float * LM2 = (float*)tmpMem->Alloc(sizeof(float) * wc2);
    for( int i = 0; i < wc2; i++ ){
        if( i < ch1->wCount2 ){
            wid2[i] = ch1->wid2[i];
            LM2[i] = ch1->LMScore2[i];
        }
        else{
            wid2[i] = ch2->wid2[i - ch1->wCount2];
            int e = i + 1;
            int b = e - ngram2 > 0 ? e - ngram2 : 0;
            LM2[i] = b < ch1->wCount2 ? model->GetCharNGramLMProb(b, e, wid2) : ch2->LMScore2[i - ch1->wCount2];
        }
        NGramLMScore2 += LM2[i];
    }

    rch->wid2 = wid2;
    rch->LMScore2 = LM2;
    rch->featValues[Model::NGramLM2] = NGramLMScore2;
}

bool Decoder_Phrase_ITG::ComposeTwoHypothesesWithTheFullModel(Cell * c, CellHypo * ch1, CellHypo * ch2, DECODER_TYPE d, 
															  CellHypo * &result1, CellHypo * &result2, bool canBeReversed, MemPool * tmpMem)
{
	result1 = NULL;
	result2 = NULL;

	result1 = ComposeTwoHypotheses(c, ch1, ch2, PHRASE_BASED, tmpMem);

    if(result1) {
        if( useMSDReordering )        // MSD re-ordering model
            AddMSDReorderingScore(c, ch1, ch2, result1);
    }

    if(canBeReversed) {
        result2 = ComposeTwoHypotheses(c, ch2, ch1, PHRASE_BASED, tmpMem);
        if(result2) {
            if( useMSDReordering )
                AddMSDReorderingScore(c, ch2, ch1, result2);
            if( useMEReordering )
                CalculateReordering(ch1, ch2, result1, result2);
        }
    }

	return result1 || result2;
}


bool Decoder_Phrase_ITG::CanBeReversed(Cell *c, Cell *sub1, Cell *sub2)
{
    /* any one of sub-cells exceeds maximum distortion limit */
    if (sub1->end - sub1->beg > maxDistortionDistance || sub2->end - sub2->beg > maxDistortionDistance)
        return false;
    /* the root cell spans over mark <s> and/or mark </s> */
    if(c->beg == 0 || c->end == srcLength) 
        return false;
    /* any one of the sub-cells has no hypothsis */
    if( sub1->n == 0 || sub2->n == 0 )
        return false;
    /* for Chinese-to-English SMT task, punctuations are useful marks that strongly recommend not to reorder sub-hypothses */
    if( c->havePunct )
        return false;
    
    return true;
}


void Decoder_Phrase_ITG::CalculateReordering(CellHypo * ch1, CellHypo * ch2, CellHypo * chMono, CellHypo * chRever)
{
    if( useMEReordering ){
        float scoreMonotonic = GetMEReorderingScore(ch1, ch2);
        float scoreReversed = log(1 - exp(scoreMonotonic));

        // monotonic transaltion
        if( chMono != NULL ){
            chMono->featValues[Model::MEReorder] += scoreMonotonic;
            chMono->modelScore += scoreMonotonic * model->featWeight[Model::MEReorder];
        }

        // reversed translation
        if( chRever != NULL ){
            chRever->featValues[Model::MEReorder] += scoreReversed;
            chRever->modelScore += scoreReversed * model->featWeight[Model::MEReorder];
        }
    }
}

float Decoder_Phrase_ITG::GetMEReorderingScore(CellHypo * ch1, CellHypo * ch2)
{
    // feat-id
    // 0: class-0 left-1
    // 1: class-0 right-1
    // 2: class-1 left-1
    // 3: class-1 right-1
    // 4: class-0 left-2
    // 5: class-0 right-2
    // 6: class-1 left-2
    // 7: class-1 right-2

	BaseDecoder * d1 = ch1->cell->decoder;
	BaseDecoder * d2 = ch2->cell->decoder;

	MEReFeatInfo * M1 = d1 == NULL ? MEReFeatsForSrc : d1->MEReFeatsForSrc;
	MEReFeatInfo * M2 = d2 == NULL ? MEReFeatsForSrc : d2->MEReFeatsForSrc;

    double w0 = 0, w1 = 0;

    // source words
    w0 += M1[ch1->cell->beg].feats[0]; // class 0, left phrase, left-end (i.e. left-1)
    w1 += M1[ch1->cell->beg].feats[2]; // class 1, left phrase, left-end (i.e. left-1)
    w0 += M1[ch1->cell->end - 1].feats[4]; // class 0, left phrase, right-end (i.e. left-2)
    w1 += M1[ch1->cell->end - 1].feats[6]; // class 1, left phrase, right-end (i.e. left-2)
    w0 += M2[ch2->cell->beg].feats[1]; // class 0, right phrase, left-end (i.e. right-1)
    w1 += M2[ch2->cell->beg].feats[3]; // class 1, right phrase, left-end (i.e. right-1)
    w0 += M2[ch2->cell->end - 1].feats[5]; // class 0, right phrase, right-end (i.e. right-2)
    w1 += M2[ch2->cell->end - 1].feats[7]; // class 1, right phrase, right-end (i.e. right-2)

    float * tgtCH1Feat = ch1->MERe->feats;
    float * tgtCH2Feat = ch2->MERe->feats;

    // target words
    w0 += tgtCH1Feat[0]; // class 0, left phrase, left-end (i.e. left-1)
    w1 += tgtCH1Feat[2]; // class 1, left phrase, left-end (i.e. left-1)
    w0 += tgtCH1Feat[4]; // class 0, left phrase, right-end (i.e. left-2)
    w1 += tgtCH1Feat[6]; // class 1, left phrase, right-end (i.e. left-2)
    w0 += tgtCH2Feat[1]; // class 0, right phrase, left-end (i.e. right-1)
    w1 += tgtCH2Feat[3]; // class 1, right phrase, left-end (i.e. right-1)
    w0 += tgtCH2Feat[5]; // class 0, right phrase, right-end (i.e. right-2)
    w1 += tgtCH2Feat[7]; // class 1, right phrase, right-end (i.e. right-2)
    
    double probMono = exp(w0 * FLETTEN_DIS_FACTOR) 
                      / (exp(w0 * FLETTEN_DIS_FACTOR) + exp(w1 * FLETTEN_DIS_FACTOR)); // monotonic translation
                      // FLETTEN_DIS_FACTOR is to control the sharpness of the distribution
    return (float)log(probMono);
}

bool Decoder_Phrase_ITG::CheckForPunctPruning(int beg, int end)
{
    //if (cell[beg][beg + 1].havePunct && cell[end - 1][end].havePunct)
    //    return false;

    if( !usePuncPruning || end - beg == 1 )
        return true;

    int boundaryWordNum = boundaryWordNumForPunctPruning;
    if (end - 1 > beg + 1 && cell[beg + 1][end - 1].havePunct)
    {
        bool matchLeft = false;
        bool matchRight = false;

        for (int i = 0; i < boundaryWordNum && beg - i >= 0; i++)
        {
            if (cell[beg - i][beg - i + 1].havePunct)
                matchLeft = true;
        }
        
        for (int i = 0; i < boundaryWordNum && end + i - 1 <= srcLength; i++)
        {
            if (cell[end + i - 1][end + i].havePunct)
                matchRight = true;
        }

        matchLeft = (matchLeft || beg <= 1);
        matchRight = (matchRight || end >= srcLength - 1);

        return matchLeft && matchRight;
    }
    return true;

}

void Decoder_Phrase_ITG::AddMSDReorderingScore( Cell* c, CellHypo* lt, CellHypo* rt, CellHypo* ch )
{
	Phrase *ll, *lr, *rl, *rr;
    ll = lt->tLeftPhrase;
    lr = lt->tRightPhrase;
    rl = rt->tLeftPhrase;
    rr = rt->tRightPhrase;
    char * lSrcStr = NULL;
    char * rSrcStr = NULL; 
    float val1, val2;

	int llBeg = ll->alignBeg;
	int llEnd = ll->alignEnd;
	int lrBeg = lr->alignBeg;
	int lrEnd = lr->alignEnd;
	int rlBeg = rl->alignBeg;
	int rlEnd = rl->alignEnd;
	int rrBeg = rr->alignBeg;
	int rrEnd = rr->alignEnd;

	BaseDecoder * d1 = lt->cell->decoder;
	BaseDecoder * d2 = rt->cell->decoder;

	if(skeleton){
		if(d1->decoderId == DECODER_ID_SKELETON){
			llBeg = realWordOffsetForSkeleton[llBeg];
			llEnd = realWordOffsetForSkeleton[llEnd];
			lrBeg = realWordOffsetForSkeleton[lrBeg];
			lrEnd = realWordOffsetForSkeleton[lrEnd];
		}
		if(d2->decoderId == DECODER_ID_SKELETON){
			rlBeg = realWordOffsetForSkeleton[rlBeg];
			rlEnd = realWordOffsetForSkeleton[rlEnd];
			rrBeg = realWordOffsetForSkeleton[rrBeg];
			rrEnd = realWordOffsetForSkeleton[rrEnd];
		}
		lSrcStr = d1->GetSrcString(lr->alignBeg, lr->alignEnd, mem);
		rSrcStr = d2->GetSrcString(rl->alignBeg, rl->alignEnd, mem);
	}
	else{
		lSrcStr = GetSrcString(lr->alignBeg, lr->alignEnd, mem);
		rSrcStr = GetSrcString(rl->alignBeg, rl->alignEnd, mem);
	}

    if( lrEnd == rlBeg - 1 ) {    // monotone
        val1 = model->GetMSDReoderingFeatVal( rSrcStr, rl->str, "L2R_M" );
        val2 = model->GetMSDReoderingFeatVal( lSrcStr, lr->str, "R2L_M" );
        ch->featValues[model->MosRe_L2R_M] += val1;
        ch->featValues[model->MosRe_R2L_M] += val2;
        ch->modelScore += val1 * model->featWeight[model->MosRe_L2R_M];
        ch->modelScore += val2 * model->featWeight[model->MosRe_R2L_M];
    }
    else if( lrBeg - 1 == rlEnd ) {    // swap
        val1 = model->GetMSDReoderingFeatVal( rSrcStr, rl->str, "L2R_S" );
        val2 = model->GetMSDReoderingFeatVal( lSrcStr, lr->str, "R2L_S" );
        ch->featValues[model->MosRe_L2R_S] += val1;
        ch->featValues[model->MosRe_R2L_S] += val2;
        ch->modelScore += val1 * model->featWeight[model->MosRe_L2R_S];
        ch->modelScore += val2 * model->featWeight[model->MosRe_R2L_S];
    }
    else {    // discontinous
        val1 = model->GetMSDReoderingFeatVal( rSrcStr, rl->str, "L2R_D" );
        val2 = model->GetMSDReoderingFeatVal( lSrcStr, lr->str, "R2L_D" );
        ch->featValues[model->MosRe_L2R_D] += val1;
        ch->featValues[model->MosRe_R2L_D] += val2;
        ch->modelScore += val1 * model->featWeight[model->MosRe_L2R_D];
        ch->modelScore += val2 * model->featWeight[model->MosRe_R2L_D];
    }

	if(skeleton && (d1->decoderId == DECODER_ID_SKELETON || d2->decoderId == DECODER_ID_SKELETON)){
		ch->tLeftPhrase = new Phrase(mem);
		ch->tLeftPhrase->Init(ll->str, llBeg, llEnd);
		ch->tRightPhrase = new Phrase(mem);
		ch->tRightPhrase->Init(rr->str, rrBeg, rrEnd);
	}
	else{
		ch->tLeftPhrase = ll;
		ch->tRightPhrase = rr;
	}
}

void Decoder_Phrase_ITG::AddFeatValue(CellHypo * ch, int featDim, float featValue)
{
    ch->featValues[featDim] += featValue;
    ch->modelScore += featValue * model->featWeight[featDim];
}

bool Decoder_Phrase_ITG::CheckHypoInRef(CellHypo * ch, RefSents * refs)
{
    if(decodingSent == NULL) // no reference translation is provided
        return false;
    if(ch == NULL)
        return false;

    for(int i = 0; i < refs->refCount; i++){
        if(strstr(refs->refs[i].str, ch->translation) != NULL)
            return true;
    }

    return false;
}

///////////////////////////////////
// for "forced" decoding

void Decoder_Phrase_ITG::ComposeTwoSpansWithForcedDecoding(Cell * c, Cell * sub1, Cell * sub2, MemPool * tmpMem)
{
    CellHypo ** tlist1 = sub1->nList;
    CellHypo ** tlist2 = sub2->nList;
    int tCount1 = sub1->n;
    int tCount2 = sub2->n;
    CellHypo * t1;
    CellHypo * t2;
    List * localNBest = new List(tCount1 * tCount2);
    bool canBeReversed = CanBeReversed(c, sub1, sub2);

    for( int i1 = 0; i1 < tCount1; i1++ ){
        t1 = tlist1[i1];
        for( int i2 = 0; i2 < tCount2; i2++ ){
            t2 = tlist2[i2];

            CellHypo * ch1 = TryToComopseTwoHyposWithForcedDecoding(t1, t2, tmpMem);
            CellHypo * ch2 = TryToComopseTwoHyposWithForcedDecoding(t2, t1, tmpMem);

            if(ch1 != NULL)
                localNBest->Add((void*)ch1);
            if(ch2 != NULL)
                localNBest->Add((void*)ch2);
        }
    }

    for(int i = 0; i < localNBest->count; i++){
        CellHypo * ch = (CellHypo *)localNBest->GetItem(i);
        CellHypo * newch = ComposeTwoHypotheses(c, ch->lc, ch->rc, PHRASE_BASED, tmpMem);
        //CellHypo * newch = ComposeTwoHypothesesForForcedDecoding(c, ch->lc, ch->rc, PHRASE_BASED, tmpMem);
        //c->AddCellHypo(newch);
        //continue;

        newch->matchedRefPOS = ch->matchedRefPOS;
        newch->matchedRefPOSCount = ch->matchedRefPOSCount;

        if(useMSDReordering)
            AddMSDReorderingScore(c, newch->lc, newch->rc, newch);

        if(useMEReordering){
            bool localReorderFlag = t1->wCount > 0 && t2->wCount > 0;
            if(newch->lc->cell->beg > newch->rc->cell->beg)
                CalculateReordering(newch->rc, newch->lc, NULL, newch);
            else if(localReorderFlag && canBeReversed)
                CalculateReordering(newch->lc, newch->rc, newch, NULL);
        }

        c->AddCellHypo(newch);
    }

    delete localNBest;
}

CellHypo * Decoder_Phrase_ITG::TryToComopseTwoHyposWithForcedDecoding(CellHypo * ch1, CellHypo * ch2, MemPool * tmpMem)
{
    if(ch1->matchedRefPOSCount == 0 || ch2->matchedRefPOSCount == 0)
        return NULL;

    int matched[1024];
    int matchedCount = 0;

    for(int i = 0; i < ch1->matchedRefPOSCount; i++){
        for(int j = 0; j < ch2->matchedRefPOSCount; j++){
            int beg1 = ch1->matchedRefPOS[i];
            int beg2 = ch2->matchedRefPOS[j];

            if(beg1 + ch1->wCount == beg2)
                matched[matchedCount++] = beg1;
        }
    }

    if(matchedCount > 0){
        CellHypo * ch = (CellHypo *)tmpMem->Alloc(sizeof(CellHypo));
        ch->lc = ch1;
        ch->rc = ch2;
        ch->matchedRefPOS = (int*)tmpMem->Alloc(sizeof(int) * matchedCount);
        memcpy(ch->matchedRefPOS, matched, matchedCount);
        ch->matchedRefPOSCount = matchedCount;
        return ch;
    }
    else
        return NULL;
}

CellHypo * Decoder_Phrase_ITG::ComposeTwoHypothesesForForcedDecoding(Cell * c, CellHypo * ch1, CellHypo * ch2, DECODER_TYPE d, MemPool * tmpMem)
{
    if( !allowSequentialNULLTrans && ch1->wCount == 0 && ch2->wCount == 0)  // ??? TODO: remove it
        return NULL;

    CellHypo * rch = (CellHypo*)tmpMem->Alloc(sizeof(CellHypo));
    rch->Init(c, tmpMem, model->featNum);

    // translation
    if( ch1->transLength == 0 || ch2->transLength == 0){
        rch->transLength = ch1->transLength + ch2->transLength;
        rch->translation = (char*)tmpMem->Alloc(sizeof(char) * (rch->transLength + 1));
        sprintf(rch->translation, "%s%s", ch1->translation, ch2->translation);
    }
    else{
        rch->transLength = ch1->transLength + ch2->transLength + 1;
        rch->translation = (char*)tmpMem->Alloc(sizeof(char) * (rch->transLength + 1));
        sprintf(rch->translation, "%s %s", ch1->translation, ch2->translation);
    }
    rch->translation[rch->transLength] = '\0';

    // left and right children
    rch->lc = ch1;
    rch->rc = ch2;

    // n-gram language model
    rch->wCount = ch1->wCount + ch2->wCount;
    rch->modelScore = rch->wCount;

    int wc = rch->wCount;
    int * wid = (int*)tmpMem->Alloc(sizeof(int) * wc);
    float * LM = (float*)tmpMem->Alloc(sizeof(float) * wc);

    memset(wid, 0, sizeof(int) * wc);
    memset(LM, 0, sizeof(float) * wc);

    rch->wid = wid;
    rch->LMScore = LM;

    return rch;
}

///////////////////////////////////
// for loss augumented decoding

void Decoder_Phrase_ITG::LoadRefForLossAugumentedDecoding(DecodingSentence * sentence)
{
    for(int r = 0; r < refNum; r++){
        refNGram[r]->Clear();
        refNGram[r]->Load(sentence->refs->refs[r].str, model);
    }
}

void Decoder_Phrase_ITG::RerankingWithLossAugumentedScore(Cell * c)
{
    for(int i = 0; i < c->n; i++)
        ComputeLoss(c->nList[i], i + 1); // loss calculation for each hypothesis

    c->SortHyposInBeam(); // re-sort the nbest candidates with their new scores
}

void Decoder_Phrase_ITG::ComputeLoss(CellHypo * ch, int rank)
{
    float lbeam = 0;
    float lbleu = 0;

    if(beamLoss){
        if(rank <= beamLossK)
            lbeam = 0; //log(float(beamLossK - rank + 1));
        else
            lbeam = -(rank - beamLossK);

        lbeam *= beamLossWeight;
    }

    if(bleuLoss){
        int * matchedNGram = new int[bleuLossNGram];
        memset(matchedNGram, 0, sizeof(int) * bleuLossNGram);

        for(int i = 0; i < ch->wCount; i++){
            for(int k = 0; k < bleuLossNGram && i + k < ch->wCount; k++){
                matchedNGram[k] += InRef(ch->wid + i, k + 1) ? 1 : 0;
            }
        }

        // n-gram precision
        for(int n = 0; n < bleuLossNGram && n < ch->wCount; n++){
            float ngramPrecision = float(matchedNGram[n] + 0.1) / (ch->wCount - n + 0.1); // add-alpha smoothing
            lbleu += log(ngramPrecision) / 4; // n-gram-precison ^ 1/4
        }

        delete[] matchedNGram;

        // translation length
        lbleu += log((float)ch->wCount);

        lbleu *= bleuLossWeight;
    }
    
    ch->beamLoss += lbeam;
    ch->bleuLoss = lbleu;

    ch->modelScore += ch->beamLoss + ch->bleuLoss;
}

bool Decoder_Phrase_ITG::InRef(int * wid, int ngram)
{
    for(int r = 0; r < refNum; r++){
        if(refNGram[r]->Find(wid, ngram) > 0)
            return true;
    }

    return false;
}

/////////////////////////////////////////////////////
// too many times!!!

NodeHeapForSearch::NodeHeapForSearch(int mySize, HEAPTYPE myType)
{
    type = myType;
    size = mySize;
    items = new BaseSearchNode *[mySize];
    count = 0;
    memset(items, 0, sizeof(BaseSearchNode *) * mySize);
}

NodeHeapForSearch::~NodeHeapForSearch()
{
    delete[] items;
}

void NodeHeapForSearch::Clear()
{
    count = 0;
    memset(items, 0, sizeof(BaseSearchNode*) * size);
}

void NodeHeapForSearch::Update(BaseSearchNode * node)
{
    if (type == MAXHEAP){
        fprintf(stderr, "ERROR: udpate operation is unavailable for max-heap\n");
        return;
    }

    CellHypo * ch = node->ch;

    if( count == size && items[0]->ch->modelScore >= ch->modelScore)
        return;

    // find and update the translation
    for( int i = 0; i < count; i++ ){
        if(strcmp(items[i]->ch->translation, ch->translation))
            continue;
        if(items[i]->ch->modelScore < ch->modelScore){
            items[i] = node;
            Down(i);
        }
        return;
    }

    if( count < size ){
        Push(node);
        return;
    }

    items[0] = node;
    Down(0);
}

bool NodeHeapForSearch::Compare(int i, int j)
{
    if (type == MINHEAP)
        return items[i]->ch->modelScore < items[j]->ch->modelScore;
    else
        return items[j]->ch->modelScore < items[i]->ch->modelScore;
}

void NodeHeapForSearch::Push(BaseSearchNode * node)
{
    if (count >= size){
        fprintf(stderr, "ERROR: Heap is full!\n");
        exit(-1);
    }

    items[count] = node;
    Up(count);
    count++;
}


BaseSearchNode * NodeHeapForSearch::Pop()
{
    if (size == 0){
        fprintf(stderr, "ERROR: empty heap!\n");
        exit(-1);
    }

    BaseSearchNode * node = items[0];
    items[0] = items[count - 1];
    count--;
    items[count] = NULL;
    Down(0);
    return node;
}

void NodeHeapForSearch::Down(int k)
{
    int i = k;

    while (2 * i + 1 < count)
    {
        int l = 2 * i + 1, r = 2 * i + 2;
        int m = (r >= count || Compare(l, r)) ? l : r;
        if (Compare(i, m))
            break;
        BaseSearchNode * tmp = items[i];
        items[i] = items[m];
        items[m] = tmp;
        i = m;
    }
}
void NodeHeapForSearch::Up(int k)
{
    int i = k;
    int parent = (i - 1) / 2;
    while (i > 0 && !Compare(parent, i))
    {
        BaseSearchNode * tmp = items[i];
        items[i] = items[parent];
        items[parent] = tmp;
        i = parent;
        parent = (i - 1) / 2;
    }
}

}

