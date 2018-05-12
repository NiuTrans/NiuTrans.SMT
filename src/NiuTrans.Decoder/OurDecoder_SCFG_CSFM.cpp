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
 * course-grained search + fine-grained modeling; OurDecoder_SCFG_CSFM.cpp
 * course-grained search = create search space using a course-grained grammar
 * fine-grained modeling = score every hypothesis using a fine-grained grammar (or model)
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

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Model.h"
#include "OurDecoder_SCFG.h"

namespace smt {

int goHave = 0;
int totalHave = 0;
int doHave = 0;
int nullruleHave = 0;
int resultzeroHave = 0;

// scoring a given hypothesis using the fine-grained model
CellHypo * Decoder_SCFG::FineModelHypo(CellHypo * ch, UnitRule * rule, SlotInformation * slotInfo, MemPool * tmpMem)
{
    //ch->CSFMCandId = -1;
    //return ch;

    CellHypo * resultingHypo = ch;
    int resultNum = 0;

    goHave++;
    if(rule != NULL && rule->parentRule != NULL && rule->parentRule->CSFMRuleNum > 0){ // associated fine-grained rules are found
        CellHypo * base       = GetBaseScoreUsingFineGrainedModel(ch, rule, slotInfo, tmpMem);
        int newHypoNum        = rule->parentRule->CSFMRuleNum;
        SCFGRule ** fineRules = rule->parentRule->CSFMRules;
        CellHypo ** newHypos  = tmpHypoPool;
        float bestScore       = (float)MODEL_SCORE_MIN;
        CellHypo * bestHypo   = NULL;
        short ruleOffset      = rule->id;
        float * featForUnmatchedNT = tmpFeatValues;

        for(int i = 0; i < newHypoNum; i++){ // loop for each fine-grained rule
            if(fineRules[i] == NULL) // filtered ??
                continue;

            UnitRule * fineGrainedRule = fineRules[i]->ruleList[ruleOffset];

            memset(featForUnmatchedNT + Model::CSFMSurrogateRuleCount, 0, 
                   sizeof(float) * (Model::CSFMUnmatchedTgtNTCount - Model::CSFMSurrogateRuleCount + 1));

            if(!fuzzyMatching && CheckNTConsistency(ch, fineGrainedRule, slotInfo, NULL) > 0)
                continue;

            CellHypo * newHypo = CellHypo::Copy(base, model, tmpMem);
            ScoreHypoUsingFineGrainedModel(newHypo, fineGrainedRule, slotInfo, tmpMem); // scoring the hypothesis using the fine-grained model
            newHypo->rootFineModel = fineGrainedRule->root;

            if(fuzzyMatching){ // update feature values
                CheckNTConsistency(ch, fineGrainedRule, slotInfo, featForUnmatchedNT);
                featForUnmatchedNT[Model::CSFMSurrogateRuleCount] = 1;

                for(int i = Model::CSFMSurrogateRuleCount; i <= Model::CSFMUnmatchedTgtNTCount; i++)
                    AddFeatValue(newHypo, i, featForUnmatchedNT[i]);
            }

            if(bestScore < newHypo->modelScore){
                bestScore = newHypo->modelScore;
                bestHypo  = newHypo;
            }

            newHypos[resultNum] = newHypo;
            resultNum++;

            fprintf(tmpF, "%s ||| %s\n", rule->src, fineGrainedRule->src);
        }

        if(resultNum > 0){
            if(!coarseHeuristic)
                resultingHypo = bestHypo;

            resultingHypo->CSFMCandId = candPool->Add(resultNum, newHypos);
            doHave++;
        }
        totalHave++;
        fprintf(tmpF, "%d %d %d %f %d %d\n", doHave, totalHave, goHave, (float)doHave/totalHave, nullruleHave, resultzeroHave);
    }

    if(resultNum == 0){ // no fine-grained rules are available

        if(rule != NULL){
            fprintf(tmpF, "result = 0: %s\n", rule->src);
            resultzeroHave++;
        }
        else{
            fprintf(tmpF, "rule = NULL\n");
            nullruleHave++;
        }

        if(rule != NULL && SynchronousGrammar::IsMetaSymbol(rule->src)){ // meta rules
            resultingHypo = ch;
            resultingHypo->rootFineModel = resultingHypo->root;
            resultingHypo->CSFMCandId = -1;
        }
        else if(strcmp(ch->root, "NULL") && SynchronousGrammar::IsComplete(ch->root)){ // normal rules

            NTMappingNode * NTList = NULL;
            int NTNum              = 0;
            CellHypo ** newHypos   = tmpHypoPool;
            float bestScore        = (float)MODEL_SCORE_MIN;
            CellHypo * bestHypo    = NULL;

            NTNum = model->NTProb->GetMappingAboveP(directionForCSFMDecoding, ch->root, topNDefaultNT, pDefaultNT, NTList);

            for(int i = 0; i < NTNum; i++){
                NTMappingNode * node = NTList + i;

                CellHypo * newHypo = GetBaseScoreUsingFineGrainedModel(ch, rule, slotInfo, tmpMem);
                newHypo->rootFineModel = directionForCSFMDecoding == 0 ?
                                         node->nt :
                                         GenerateFineGrainedSymbol(newHypo->root, node->nt, rule, tmpMem);

                ScoreHypoUsingDefaultModel(newHypo, rule, slotInfo, newHypo->rootFineModel, tmpMem);
                AddFeatValue(newHypo, Model::PrCFG2, node->prob);

                if(i == 0 || bestScore < newHypo->modelScore){
                    bestScore = newHypo->modelScore;
                    bestHypo  = newHypo;
                }

                newHypos[resultNum] = newHypo;
                resultNum++;
            }

            if(!coarseHeuristic)
                resultingHypo = bestHypo;

            resultingHypo->CSFMCandId = candPool->Add(resultNum, newHypos);
        }
        else{ // "NULL-translation" rules and other rules
            CellHypo * newHypo = GetBaseScoreUsingFineGrainedModel(ch, rule, slotInfo, tmpMem);
            newHypo->rootFineModel = newHypo->root;

            ScoreHypoUsingDefaultModel(newHypo, rule, slotInfo, defaultSymbolFineModel, tmpMem);

            if(!coarseHeuristic)
                resultingHypo = newHypo;

            resultingHypo->CSFMCandId = candPool->Add(1, &newHypo);
        }
    }

    return resultingHypo;
}

// return # of unmatched NTs
int Decoder_SCFG::CheckNTConsistency(CellHypo * ch, UnitRule * rule, SlotInformation * slotInfo, float * featForUnmatchedNT)
{
    int unmatched = 0;

    if(slotInfo == NULL || rule == NULL)
        return unmatched;

    if(slotInfo->slotNum != rule->NTCount){
        fprintf(stderr, "ERROR: slotInfo->slotNum != rule->NTCount\n");
        exit(1);
    }
    
    for(int i = 0; i < slotInfo->slotNum; i++){
        CellHypo * curHypo = slotInfo->slotHypoPointer[i];

        if((strchr(rule->NT[i].symbol, VIRUTAL_NT_SYMBOL) != NULL) && (strchr(curHypo->rootFineModel, VIRUTAL_NT_SYMBOL) != NULL))
            continue;

        if(featForUnmatchedNT != NULL){
            if(GetFeatValueForUnmatchedNT(rule->NT[i].symbol, curHypo->rootFineModel, featForUnmatchedNT))
                unmatched++;
        }
        else if(strcmp(rule->NT[i].symbol, curHypo->rootFineModel)) // unmatching
            unmatched++;
    }

    return unmatched;
}

bool Decoder_SCFG::GetFeatValueForUnmatchedNT(char * symbol1, char * symbol2, float * featForUnmatchedNT)
{
    int unmatched = false;
    if(strcmp(symbol1, symbol2)){
        unmatched = true;
        featForUnmatchedNT[Model::CSFMUnmatchedNTCount] += 1;
    }
    else
        featForUnmatchedNT[Model::CSFMMatchedNTCount] += 1;

    char * s1 = strchr(symbol1, SYMBOL_SEPARATOR);
    char * s2 = strchr(symbol2, SYMBOL_SEPARATOR);

    if(s1 != NULL && s2 != NULL){
        char tmp1 = *s1;
        char tmp2 = *s2;
        *s1 = '\0';
        *s2 = '\0';

        if(strcmp(symbol1, symbol2))
            featForUnmatchedNT[Model::CSFMUnmatchedTgtNTCount] += 1;
        else
            featForUnmatchedNT[Model::CSFMMatchedTgtNTCount] += 1;

        *s1 = tmp1;
        *s2 = tmp2;

        if(strcmp(s1 + 1, s2 + 1))
            featForUnmatchedNT[Model::CSFMUnmatchedSrcNTCount] += 1;
        else
            featForUnmatchedNT[Model::CSFMMatchedSrcNTCount] += 1;
    }

    return unmatched;
}


CellHypo * Decoder_SCFG::GetBaseScoreUsingFineGrainedModel(CellHypo * coarseHypo, UnitRule * coarseRule, SlotInformation * slotInfo, MemPool * tmpMem)
{
    CellHypo * ch = CellHypo::Copy(coarseHypo, model, tmpMem);
    memset(ch->featValues, 0, sizeof(float) * model->featNum);
    ch->featValues[Model::NGramLM]   = coarseHypo->featValues[Model::NGramLM];
    ch->featValues[Model::WordCount] = coarseHypo->featValues[Model::WordCount];

    float overallScore = 0;

    // score of substituted variables
    for(int var = 0; var < slotInfo->slotNum; var++){
        // for the i-th variable
        int varIndex = slotInfo->invertedNTIndex[var];
        CellHypo * currentHypo = slotInfo->slotHypoPointer[varIndex];

        for( int f = 0; f < model->featNum; f++ ){
            if( f != Model::NGramLM &&  f != Model::WordCount)
                ch->featValues[f] += currentHypo->featValues[f];
        }
    }

    if(useCoarseModelAlso){
        if(coarseRule != NULL){
            // score of the rule
            for( int f = Model::TableFeatBegCoarse; f <= Model::TableFeatEndCoarse; f++ ){
                if(f != Model::NGramLM &&  f != Model::WordCount){
                    if(coarseRule->isComplete)
                        ch->featValues[f] += coarseRule->feat[f - Model::TableFeatBegCoarse + Model::MainTableBeg];
                    else
                        overallScore += coarseRule->feat[f - Model::TableFeatBegCoarse + Model::MainTableBeg] * model->featWeight[f];
                }
            }
        }
        else if(slotInfo->slotNum == 2){ // glue rule
            ch->featValues[Model::GlueRuleCount - Model::MainTableBeg + Model::TableFeatBegCoarse] += 1;
        }
    }

    // score of the rule
    for(int f = 0; f < model->featNum; f++)
        overallScore += ch->featValues[f] * model->featWeight[f];

    ch->modelScore = overallScore;

    return ch;
}

float Decoder_SCFG::ScoreHypoUsingFineGrainedModel(CellHypo * ch, UnitRule * fineGrainedRule, SlotInformation * slotInfo, MemPool * tmpMem)
{
    float ruleScore = 0;

    for( int f = 0; f < model->featNum; f++ ){
        if(f != Model::NGramLM &&  f != Model::WordCount){
            if(fineGrainedRule->isComplete)
                ch->featValues[f] += fineGrainedRule->feat[f];

            ruleScore += fineGrainedRule->feat[f] * model->featWeight[f];
        }
    }

    ch->modelScore += ruleScore;
    return ruleScore;
}

float Decoder_SCFG::ScoreHypoUsingDefaultModel(CellHypo * ch, UnitRule * coarseGrainedRule, SlotInformation * slotInfo, char * root, MemPool * tmpMem)
{
    float ruleScore = 0;
    memset(tmpFeatValues, 0, sizeof(float) * model->featNum);

    if(coarseGrainedRule != NULL){
        if(model->SCFG->type == HIERO){
            for(int f = Model::PrF2E; f <= Model::PrE2FLex; f++){
                tmpFeatValues[f] = coarseGrainedRule->feat[f] * model->backoffToHiero;
            }
            tmpFeatValues[Model::RuleCount]        = 1;
            tmpFeatValues[Model::BiLexCount]       = coarseGrainedRule->feat[Model::BiLexCount];
            tmpFeatValues[Model::PenaltyNull]      = coarseGrainedRule->feat[Model::PenaltyNull];
            tmpFeatValues[Model::PhrasalRuleCount] = coarseGrainedRule->feat[Model::PhrasalRuleCount];
            tmpFeatValues[Model::GlueRuleCount]    = coarseGrainedRule->feat[Model::GlueRuleCount];

            tmpFeatValues[Model::PrBiCFG] = coarseGrainedRule->feat[Model::PrF2E] + coarseGrainedRule->feat[Model::PrE2F] +
                                            (float)model->baseBiCFGProb;
            tmpFeatValues[Model::PrCFG2]  = tmpFeatValues[Model::PrBiCFG] + (float)log(0.01);
        }
        else{
            memcpy(tmpFeatValues, coarseGrainedRule->feat, sizeof(float) * model->featNum);
            float discount = log(model->backoffToSyntax);
            for(int f = Model::PrF2E; f <= Model::PrE2FLex; f++){
                tmpFeatValues[f] = coarseGrainedRule->feat[f] + discount;
            }
            tmpFeatValues[Model::PrBiCFG] = coarseGrainedRule->feat[Model::PrBiCFG]  + discount;
            tmpFeatValues[Model::PrCFG2]  = coarseGrainedRule->feat[Model::PrCFG2]  + discount;
        }
    }
    else{ // glue rule ?
        //tmpFeatValues[Model::PrF2E]     = (float)log(0.0001);
        //tmpFeatValues[Model::PrE2F]     = (float)log(0.0001);
        //tmpFeatValues[Model::RuleCount] = 1;
        tmpFeatValues[Model::GlueRuleCount] = 1;
        //tmpFeatValues[Model::PrBiCFG]   = model->baseBiCFGProb + (float)log(0.01);
        //tmpFeatValues[Model::PrCFG2]    = tmpFeatValues[Model::PrBiCFG] + (float)log(0.01);

    }

    for( int f = 0; f < model->featNum; f++ ){
        if(f != Model::NGramLM &&  f != Model::WordCount){
            if(coarseGrainedRule == NULL || coarseGrainedRule->isComplete)
                ch->featValues[f] += tmpFeatValues[f];

            ruleScore += tmpFeatValues[f] * model->featWeight[f];
        }
    }

    ruleScore += GetLabelProb(root);

    ch->modelScore += ruleScore;
    return ruleScore;
}

float Decoder_SCFG::GetLabelProb(char * symbol)
{
    return 0;
}

char * Decoder_SCFG::GenerateFineGrainedSymbol(char * coarseSymbol, char * suggestedSymbol, UnitRule * coarseRule, MemPool * tmpMem)
{
    if(model->treeBasedModel == 0 || (coarseRule != NULL && !coarseRule->isComplete))
        return coarseSymbol;

    char * fineGrainedSymbol = (char*)tmpMem->Alloc(sizeof(char) * ((int)strlen(coarseSymbol) + (int)strlen(suggestedSymbol) + 2));

    if(model->treeBasedModel == 2) // string-to-tree
        sprintf(fineGrainedSymbol, "%s=%s", suggestedSymbol, coarseSymbol);
    else if(model->treeBasedModel == 1)
        sprintf(fineGrainedSymbol, "%s=%s", coarseSymbol, suggestedSymbol);
    else
        fprintf(stderr, "anything wrong?\n");

    return fineGrainedSymbol;
}

char * Decoder_SCFG::GenerateFineGrainedSymbol(char * coarseSymbol, UnitRule * coarseRule, MemPool * tmpMem) // to be updated
{
    if(model->treeBasedModel == 0 || (coarseRule != NULL && !coarseRule->isComplete))
        return coarseSymbol;

    char * fineGrainedSymbol = (char*)tmpMem->Alloc(sizeof(char) * ((int)strlen(coarseSymbol) + (int)strlen(defaultOneSideSymbolFineModel) + 2));

    if(model->treeBasedModel == 2) // string-to-tree
        sprintf(fineGrainedSymbol, "%s=%s", defaultOneSideSymbolFineModel, coarseSymbol);
    else if(model->treeBasedModel == 1)
        sprintf(fineGrainedSymbol, "%s=%s", coarseSymbol, defaultOneSideSymbolFineModel);
    else
        fprintf(stderr, "anything wrong?\n");

    return fineGrainedSymbol;
}

void Decoder_SCFG::AddCellHypoForCSFM(Cell * c, CellHypo * ch)
{
    if(ch->CSFMCandId < 0)
        c->AddCellHypo(ch);
    else{ // from the candidate pool
        int candNum = 0;
        CellHypo ** cands = NULL;
        candPool->Get(ch->CSFMCandId, candNum, cands);

        for(int i = 0; i < candNum; i++){
            c->AddCellHypo(cands[i]);
        }
    }
}

/////////////////////////////////////////////////////
// hypopthesis candidate pool

HypoCandPool::HypoCandPool(int max, MemPool * mem)
{
    if(mem != NULL){
        p          = new List(max, mem);
        n          = new List(max, mem);
        pool       = new List(max, mem);
    }
    else{
        p          = new List(max);
        n          = new List(max);
        pool       = new List(max);
    }

    maxCandNum = max;
    cur        = 0;
}

HypoCandPool::~HypoCandPool()
{
    delete p;
    delete n;
    delete pool;
}

void HypoCandPool::Clear()
{
    p->Clear();
    n->Clear();
    pool->Clear();

    p->Add(NULL);
    n->Add(NULL);

    cur = 1;
}

int HypoCandPool::Add(long num, CellHypo ** head)
{
    if(cur + num >= maxCandNum){
        fprintf(stderr, "WARNING: hypocandpool is full!\n");
        return -1;
    }

    p->Add((void*)(pool->items + pool->count));
    n->Add((void*)num);

    for(int i = 0; i < num; i++)
        pool->Add((void*)(head[i]));

    return cur++;
}

void HypoCandPool::Get(int id, int &num, CellHypo ** &head)
{
    if(id >= cur){
        num  = 0;
        head = NULL;
        return;
    }

    head = (CellHypo**) p->GetItem(id);

    long long lnum = (long long)n->GetItem(id);
    num  = (int) lnum;
}

}

