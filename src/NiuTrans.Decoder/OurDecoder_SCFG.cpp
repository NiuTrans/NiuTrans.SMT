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
 * an SCFG-based decoder; OurDecoder_SCFG.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 *
 * $Last Modified by:
 * 2014/11/26 Tong Xiao (xiaotong@mail.neu.edu.cn) bug fixes for decoding failure in t2s models
 *
 */


#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Model.h"
#include "OurDecoder_SCFG.h"

namespace smt {

FILE * tmpF = NULL;
int tmpTrigger = 0;

//////////////////////////////////////////////////////////////
// structure defined for rule matching


void MatchedPattern::Init(const char * myKey, int keyLen, MemPool * myMem)
{
    memset(this, 0, sizeof(MatchedPattern));

    this->key = (char*)myMem->Alloc(sizeof(char) * (keyLen + 1));
    strcpy(this->key, myKey);
}

MatchedPattern * MatchedPattern::GeneratePatternWithLeftEnd(MatchedPattern * prePattern, char * key, 
                                                             int keyLen, int varLeftEnd, int varRightEnd, 
                                                             MemPool * myMem)
{
    MatchedPattern * newPattern = (MatchedPattern*)myMem->Alloc(sizeof(MatchedPattern));
    newPattern->Init(key, keyLen, myMem);

    if( prePattern != NULL ){
        newPattern->matching = (NTBoundary*)myMem->Alloc(sizeof(NTBoundary) * (prePattern->matchingNum + 1));
        memcpy(newPattern->matching + 1, prePattern->matching, sizeof(NTBoundary) * prePattern->matchingNum);
        newPattern->matchingNum = prePattern->matchingNum + 1;
    }
    else{
        newPattern->matching = (NTBoundary*)myMem->Alloc(sizeof(NTBoundary));
        newPattern->matchingNum = 1;
    }

    // first variable
    newPattern->matching[0].left     = varLeftEnd;
    newPattern->matching[0].right    = varRightEnd;
    newPattern->matching[0].cellNode = NULL;

    return newPattern;
}

MatchedPattern * MatchedPattern::GeneratePatternWithRightEnd(MatchedPattern * prePattern, char * key, 
                                                             int keyLen, int varLeftEnd, int varRightEnd,
                                                             MemPool * myMem)
{
    MatchedPattern * newPattern = (MatchedPattern*)myMem->Alloc(sizeof(MatchedPattern));
    newPattern->Init(key, keyLen, myMem);

    if( prePattern != NULL ){
        newPattern->matching = (NTBoundary*)myMem->Alloc(sizeof(NTBoundary) * (prePattern->matchingNum + 1));
        memcpy(newPattern->matching, prePattern->matching, sizeof(NTBoundary) * prePattern->matchingNum);
        newPattern->matchingNum = prePattern->matchingNum + 1;
    }
    else{
        newPattern->matching = (NTBoundary*)myMem->Alloc(sizeof(NTBoundary));
        newPattern->matchingNum = 1;
    }

    // last variable
    newPattern->matching[prePattern->matchingNum].left     = varLeftEnd;
    newPattern->matching[prePattern->matchingNum].right    = varRightEnd;
    newPattern->matching[prePattern->matchingNum].cellNode = NULL;

    return newPattern;
}

//////////////////////////////////////////////////////////////
// structure defined for chart parsing (syntax-based decoding)

void Cell::SynInit(int b, int e, int maxTransNum, bool withSymbol, MemPool * myMem)
{
    matchedPatternList = new List((e - b) * 20);
    hypoNodeList = new List(MIN_HYPO_NODE_NUM);

    if(withSymbol)
        nListWithSymbol = new HashTable(maxTransNum * 5 > 100 ? 100 : maxTransNum * 5);
    else
        nListWithSymbol = NULL;

    treeNodes = new List(MIN_HYPO_NODE_NUM);
    cellNodes = new List(MIN_HYPO_NODE_NUM);

    prematchedRules = new List(1);
    viterbiRules = new List(1);
}

void Cell::SynClear()
{
    matchedPatternList->Clear();
    matchedPatternCount = 0;
    hypoNodeList->Clear();
    if(nListWithSymbol != NULL)
        nListWithSymbol->Clear();
    treeNodes->Clear();
    cellNodes->Clear();
    prematchedRules->Clear();
    viterbiRules->Clear();
}

void Cell::AddMatchedPattern(MatchedPattern * pattern)
{
    matchedPatternList->Add(pattern);
}

//////////////////////////////////////////////////////////////
// functions used in compare the searching nodes

int CompareExploredNodeKey( const void * node1, const void * node2 )
{
    char * p1 = ((ExploredNode *)node1)->ch->translation;
    char * p2 = ((ExploredNode *)node2)->ch->translation;
    return strcmp(p1, p2);
}

int CompareExploredNodeValue( const void * node1, const void * node2 )
{
    float v1 = ((ExploredNode *)node1)->ch->modelScore;
    float v2 = ((ExploredNode *)node2)->ch->modelScore;
    if(v1 > v2)
        return 1;
    else if(v1 < v2)
        return -1;
    else
        return 0;
}


//////////////////////////////////////////////////////////////
// SCFG-based decoder

Decoder_SCFG::Decoder_SCFG()
{
    transBuf      = new char[MAX_LINE_LENGTH];
    widBuf        = new int[MAX_LINE_LENGTH];
    LMScoreBuf    = new float[MAX_LINE_LENGTH];
    wordIndexBuf  = new int[MAX_LINE_LENGTH];
    sourceSideBuf = new char[MAX_LINE_LENGTH];

    
    defaultSymbol = new char[MAX_WORD_LENGTH];
    defaultSymbolFineModel = new char[MAX_WORD_LENGTH];
    defaultOneSideSymbolFineModel = new char[MAX_WORD_LENGTH];
    hieroSymbol   = new char[MAX_WORD_LENGTH];
    candPool      = NULL;
    tmpHypoPool   = NULL;
    tmpFeatValues = NULL;
    cellNodes     = NULL;
}

Decoder_SCFG::Decoder_SCFG(DECODER_TYPE type)
{
    //Decoder_SCFG();
    transBuf      = new char[MAX_LINE_LENGTH];
    widBuf        = new int[MAX_LINE_LENGTH];
    LMScoreBuf    = new float[MAX_LINE_LENGTH];
    wordIndexBuf  = new int[MAX_LINE_LENGTH];
    sourceSideBuf = new char[MAX_LINE_LENGTH];

    defaultSymbol = new char[MAX_WORD_LENGTH];
    defaultSymbolFineModel = new char[MAX_WORD_LENGTH];
    defaultOneSideSymbolFineModel = new char[MAX_WORD_LENGTH];
    hieroSymbol   = new char[MAX_WORD_LENGTH];
    candPool      = NULL;
    tmpHypoPool   = NULL;
    tmpFeatValues = NULL;
    cellNodes     = NULL;
    dtype = type;
}

Decoder_SCFG::~Decoder_SCFG()
{
    delete[] transBuf;
    delete[] widBuf;
    delete[] LMScoreBuf;
    delete[] wordIndexBuf;
    delete[] sourceSideBuf;
    delete   heapForSearch;
    delete[] defaultSymbol;
    delete[] defaultSymbolFineModel;
    delete[] defaultOneSideSymbolFineModel;
    delete[] hieroSymbol;
    delete   candPool;
    delete[] tmpHypoPool;
    delete[] tmpFeatValues;
    delete[] cellNodes;

    heapForSearch = NULL;
    candPool      = NULL;
}

void Decoder_SCFG::Init(Model * m)
{
    Decoder_Phrase_ITG::Init(m);

    maxWordNumInARule = configer->GetInt("maxrulewordNum", 4);
    maxBlockNum       = configer->GetInt("maxruleblockNum", 3);
    maxVarSpan        = configer->GetInt("maxvarSpan", 30);
    allowVarBounary   = configer->GetBool("boundaryspan", true);
    useGlueRule       = configer->GetBool("usegluerule", dtype == SYNTAX_BASED ? false : true);
    fastDecoding      = configer->GetBool("fastdecoding", true);
    allowNULLSubstitution = configer->GetBool("allownullsubstitution", true);
    allowUnaryProduction  = configer->GetBool("allowunaryproduction", dtype == SYNTAX_BASED ? true : false);
    replaceFoctoid        = configer->GetBool("replacefoctoid", true);
    dealWithDecodingFailure = configer->GetBool("dealwithdecodingfailure", true);
    dumpDefeatedViterbiRule = configer->GetBool("dumpdefeatedviterbirule", false);
    dumpUsedRule            = configer->GetBool("dumpusedrule", false) || configer->GetBool("dumprule", false);
    dumpLeftHypo          = configer->GetBool("dumplefthypo", dtype == SYNTAX_BASED ? true : false);
    removeInfeasibleRule  = configer->GetBool("rminfeasiblerule", dtype == SYNTAX_BASED ? true : false);
    fuzzyMatching         = configer->GetBool("fuzzymatching", false);
    coarseHeuristic       = configer->GetBool("coarseheuristic", false);
    useCoarseModelAlso    = configer->GetBool("usecoarsemodelalso", true);
    maxNumOfSymbolWithSameTrans     = configer->GetInt("maxnosymboltrans", MAX_HYPO_NUM_IN_CELL);
    maxNumOfFineSymbolWithSameTrans = configer->GetInt("maxnofinesymboltrans", 1);
    topNDefaultNT         = configer->GetInt("topndefaultnt", 1);
    pDefaultNT            = configer->GetFloat("pdefaultnt", log(0.001));
    treeParsing           = configer->GetBool("treeparsing", false);
    generateTreeStructure = configer->GetBool("treestructure", false) || configer->GetBool("generatetreestructure", false);
    epsilonViterbiScore   = configer->GetFloat("epsilonviterbiscore", 0.17);
    srcTree->maxWordNum   = configer->GetInt("maxwordnumintreefrag", 4);
    srcTree->maxNTNum     = configer->GetInt("maxvarnumintreefrag", 5);
    srcTree->maxDepth     = configer->GetInt("maxdepthintreefrag", 3);
    srcTree->maxEdgeNumPerNode = configer->GetInt("maxedgenumpernode", MILLION);
    srcTree->maxFragNumPerNode = configer->GetInt("maxfragnumpernode", MILLION);
    hieroMatchAll         = configer->GetBool("hieromatchall", false);

    withNTSymbol = m->IsSCFGWithNTSymbol();
    maxVarSpan   = maxDistortionDistance; // why???
    CSFMMode     = m->CSFMMode;
    withFineNTSymbol = CSFMMode; // to be "true" when CSFM mode is activated

    tmpFeatValues = new float[m->featNum];

    directionForCSFMDecoding = 0;
    if(CSFMMode){
        candPool    = new HypoCandPool(beamSize * beamSize * 5000, NULL);
        tmpHypoPool = new CellHypo*[MAX_HYPO_NUM_IN_CELL];

        if(model->treeBasedModel == 1)
            directionForCSFMDecoding = 2;
        else if(model->treeBasedModel == 2)
            directionForCSFMDecoding = 1;
        else
            directionForCSFMDecoding = 0;
    }

    for( int i = 0; i < MAX_WORD_NUM; i++ ){
        for( int j = i + 1; j < MAX_WORD_NUM; j++ ){
            cell[i][j].SynInit(i, j, beamSize, withNTSymbol, mem);
        }
    }

    int heapSize = 5 * beamSize * beamSize; // ??????????
    if( heapSize < 50 )
        heapSize = 50;
    if(heapForSearch != NULL)
        delete heapForSearch;
    heapForSearch = new NodeHeapForSearch(heapSize, MAXHEAP);

    SetDefaultSymbol();
}

void Decoder_SCFG::SetDefaultSymbol()
{
    if(configer->GetBool("tree2tree", false))
        strcpy(defaultSymbol, "NP=NP");
    else
        strcpy(defaultSymbol, "NP");

    if(CSFMMode){
        const char * fineGType = configer->GetString("finegrainedmodel", NULL);

        if(!strcmp(fineGType, "tree2tree"))
            strcpy(defaultSymbolFineModel, "NP=NP");
        else
            strcpy(defaultSymbolFineModel, "NP");

        strcpy(defaultOneSideSymbolFineModel, "NP");
    }

    if(configer->GetString("hierosymbol", NULL) != NULL)
        strcpy(hieroSymbol, configer->GetString("hierosymbol", NULL));
    else
        strcpy(hieroSymbol, "X");
}

void Decoder_SCFG::ResetTransInfo()
{
    Decoder_Phrase_ITG::ResetTransInfo();

    for( int i = 0; i <= srcLength; i++ )
    {
        for( int j = i + 1; j <= srcLength; j++ )
        {
            cell[i][j].SynClear();
        }
    }

    delete[] cellNodes;
    cellNodes   = NULL;
    cellNodeNum = 0;
}

void Decoder_SCFG::MatchSCFGRules()
{
    SCFG = model->SCFG;

    if(treeParsing){
        if(srcTree == NULL){
            fprintf(stderr, "ERROR! No source-tree input\n");
            exit(0);
        }
        MatchRulesForTreeParsing(mem);
        return;
    }

    MatchedPattern * initPattern = (MatchedPattern *)mem->Alloc(sizeof(MatchedPattern));

    initPattern->Init("", 0, mem);
    GenerateRuleKey(initPattern, 0, 0, 0, 0, 0);
}

// the following fuction generates all the possible patterns for a given source span
// using the generated patterns, the generateion can be trivially done without addtional parsing/rule-matching computation
void Decoder_SCFG::GenerateRuleKey(MatchedPattern * preKey, int keyLen, int leftEnd, int beg, int wordCount, int blockCount)
{
    if(wordCount > maxWordNumInARule || blockCount >= maxBlockNum)
        return;

    MatchedPattern * newPattern;
    MatchedPattern * newPatternOther;
    char key[MAX_PATTERN_LENGTH];
    int curLen, lexLeft;
    bool invalidRightExpansion = false;
    
    for(int i = beg; i < srcLength; i++){
        if(keyLen > 0 && i - beg > maxVarSpan) // a variable that covers a too large span
            break;

        bool expandLeft = i == 0 ? false : true;
        if(keyLen == 0 && i > beg && !allowVarBounary) // variables are not allowed to appear on the left-end
            expandLeft = false;                        // e.g. rule like: # buy it .

        //memset(key, 0, MAX_PATTERN_LENGTH);
        strcpy(key, preKey->key);
        if(keyLen == 0){
            strcpy(key + keyLen, "#");
            curLen = keyLen + 1;
        }
        else{
            strcpy(key + keyLen, " #");
            curLen = keyLen + 2;
        }
        key[curLen] = '\0';

        if(keyLen > 0 && !invalidRightExpansion && 
           i < srcLength - 1 && allowVarBounary)
        { // expend right end (variable): "... w #"

            List * ruleList = model->FindRuleList(key);

            if(ruleList != NULL){
                newPattern = 
                MatchedPattern::GeneratePatternWithRightEnd(preKey, key, curLen,
                                                                beg, i, mem);
                newPattern->ruleList= ruleList;

                AddMatchedPattern(leftEnd, i + 1, newPattern);    // new pattern

                if(key[0] == '#'){ // boundied by a variable
                    int firstVarBeg = newPattern->matching[0].left;
                    int firstVarEnd = newPattern->matching[0].right;

                    // shrink the left most variable
                    for(int newLeftEnd = firstVarBeg + 1 < firstVarEnd - maxVarSpan?  firstVarEnd - maxVarSpan : firstVarBeg + 1; 
                        newLeftEnd <= firstVarEnd;
                        newLeftEnd++)
                    { 
                        newPatternOther = 
                        MatchedPattern::GeneratePatternWithRightEnd(preKey, key, curLen, beg, i, mem);

                        newPatternOther->ruleList = newPattern->ruleList;
                        newPatternOther->matching[0].left = newLeftEnd;
                        AddMatchedPattern(newLeftEnd, i + 1, newPatternOther);
                    }
                }
            }
            else
                invalidRightExpansion = true;
        }

        key[curLen++] = ' ';
        key[curLen] = '\0';

        lexLeft = curLen; // left end

        for(int l = 1; l <= maxWordNumInARule && i + l < srcLength - 1; l++){
            if(wordCount + l > maxWordNumInARule)
                break;

            // " w"
            if(l > 1)
                key[curLen++] = ' ';
            strcpy(key + curLen, srcWords[i + l]);
            curLen += srcWordLength[i + l];
            key[curLen] = '\0';

            if(keyLen == 0){ // pattern boundied by lexicons (i.e. words) : "w # w" 
                newPattern = (MatchedPattern *)mem->Alloc(sizeof(MatchedPattern));
                newPattern->Init(key + 2, curLen - 2, mem);
                
                newPattern->ruleList = model->FindRuleList(key + 2);

                if(newPattern->ruleList != NULL)
                    AddMatchedPattern(i + 1, i + l + 1, newPattern);    // new pattern

                GenerateRuleKey(newPattern, curLen - 2, i + 1, i + l + 1, wordCount + l, blockCount + 1);
            }

            if(expandLeft) // expend left end (variable): "# w # w"
            {
                newPattern = MatchedPattern::GeneratePatternWithRightEnd(preKey, key, curLen, beg, i, mem);
                newPattern->ruleList = model->FindRuleList(key);

                if(newPattern->ruleList != NULL){

                    AddMatchedPattern(leftEnd, i + l + 1, newPattern);    // new pattern

                    int firstVarBeg = newPattern->matching[0].left;
                    int firstVarEnd = newPattern->matching[0].right;

                    if(firstVarBeg == leftEnd){ // boundied by a variable

                        // shrink the left most variable
                        for(int newLeftEnd = firstVarBeg + 1 < firstVarEnd - maxVarSpan?  firstVarEnd - maxVarSpan : firstVarBeg + 1; 
                            newLeftEnd <= firstVarEnd; 
                            newLeftEnd++)
                        {
                            newPatternOther = MatchedPattern::GeneratePatternWithRightEnd(preKey, key, curLen, beg, i, mem);
                            newPatternOther->ruleList = newPattern->ruleList;
                            newPatternOther->matching[0].left = newLeftEnd;
                            AddMatchedPattern(newLeftEnd, i + l + 1, newPatternOther);
                        }
                    }

                }

                GenerateRuleKey(newPattern, curLen, leftEnd, i + l + 1, wordCount + l, blockCount + 1);
            }
        }
    }
}

void Decoder_SCFG::AddMatchedPattern(int beg, int end, MatchedPattern * pattern)
{
    cell[beg][end].AddMatchedPattern(pattern);

    /*fprintf(tmpF, "%d, %d: %s", beg, end, pattern->key);
    fprintf(tmpF, " |||");
    for(int i = 0; i < pattern->matchingNum; i++){
        fprintf(tmpF, " %d:[%d, %d]", i, pattern->matching[i].left, pattern->matching[i].right);
    }
    fprintf(tmpF, "\n");*/
}

void Decoder_SCFG::AddBasicRules()
{
    // processing unknown words
    char phrase[MAX_LINE_LENGTH];
    int maxPhraseLen = 1; // maxWordNumInPhrase;

    for( int beg = 0; beg < srcLength; beg++ ){
        phrase[0] = '\0';
        bool haveFactoid = false;
        bool forcedTrans = false;
        for( int end = beg + 1 ; end - beg <= maxPhraseLen && end <= srcLength; end++){
            if(end == beg + 1)
                strcpy(phrase, srcWords[end - 1]);
            else
                sprintf(phrase, "%s %s", phrase, srcWords[end - 1]);

            /*if(treeParsing){
                for(int i = 0; i < cell[beg][end].cellNodes->count; i++){
                    Cell * c = (Cell *)cell[beg][end].cellNodes->GetItem(i);
                    char * label = GetDefaultRootLabel(c, "NP", mem);
                    HandleUnknownWord(c, phrase, label);
                    c->CompleteWithBeamPruning(-1, this, withNTSymbol, withFineNTSymbol, true, false, mem);
                }
            }*/

            if(cell[end-1][end].forcedTrans)
                forcedTrans = true;
            else if(forcedTrans)
                break;
            
            if(!treeParsing || IsSentEnd(&cell[beg][end])){
                HandleUnknownWord(&cell[beg][end], phrase, NULL);
                cell[beg][end].CompleteWithBeamPruning(-1, this, withNTSymbol, withFineNTSymbol, true, false, mem);
            }
        }
    }
}

void Decoder_SCFG::HandleUnknownWord(Cell * c, char * phrase, char * label)
{
    List * ruleList = model->FindRuleList(phrase);
    bool useUNKList = false;

    if( ruleList == NULL || ruleList->count == 0){
        StringUtil::ToLowercase( phrase );
        ruleList = model->FindRuleList(phrase);
    }

    if(ruleList != NULL && !model->SCFG->IsMetaSymbol(phrase))
        return;

    if( ruleList == NULL || ruleList->count == 0){
        if(c->tList->count > 0)
            return;
        ruleList = model->FindRuleList("<unk>"); // othewise, unknown words
        useUNKList = true;
    }

    for( int i = 0; i < ruleList->count; i++ ){

        UnitRule * rule = (UnitRule *)ruleList->items[i];

        AddCellTrans(c, rule, useUNKList && outputOOV, label);
    }
}

void Decoder_SCFG::AddCellTrans(Cell * c, UnitRule * rule, bool outputUnknown, char * label)
{
    CellHypo * ch = (CellHypo*)mem->Alloc(sizeof(CellHypo));
    ch->Init(c, mem, model->featNum);

    memcpy(ch->featValues, rule->feat, sizeof(float) * model->featNum); // feature values
    
    ch->translation = rule->tgt;
    ch->transLength = (int)strlen(ch->translation);

    if(outputUnknown){
        int transLength = (int)strlen(srcWords[c->beg]) + 3;
        ch->translation = (char *)mem->Alloc(sizeof(char) * transLength);
        memset(ch->translation, 0, sizeof(char) * transLength);
        if(labelOOV)
            sprintf(ch->translation, "<%s>", srcWords[c->beg]);
        else
            sprintf(ch->translation, "%s", srcWords[c->beg]);
        ch->transLength = transLength;
    }

    ch->wid = rule->wid;
    ch->wCount = rule->wCount;
    //ch->featValues[Model::WordCount] = ch->wCount;

    ch->root = label == NULL ? rule->root : label;

    if(CSFMMode){
        NTMappingNode * NTList = NULL;
        model->NTProb->GetMappingAboveP(directionForCSFMDecoding, ch->root, topNDefaultNT, pDefaultNT, NTList);

        ch->rootFineModel = directionForCSFMDecoding == 0 ?
                            NTList->nt :
                            GenerateFineGrainedSymbol(ch->root, NTList->nt, rule, mem);
    }

    ComputeModelScore(ch);   // computing the model score

    c->AddCellHypo(ch);
}


void Decoder_SCFG::DecodeInput(DecodingSentence * sentence)
{
    //fprintf(tmpF, "=================\n");
    //fprintf(tmpF, "%s\n", sentence);

    decodingFailure = false;
    decodingSent = sentence;

    // load reference sentences for loss augumented decoding
    if(lossAugumentedDecoding)
        LoadRefForLossAugumentedDecoding(sentence);

    // load input sentence
    if(!LoadInputSentence(sentence->string)){
        ResetTransInfo();
        fprintf(stderr, "\nFailure: %s\n", sentence->string);
        return;
    }

    // clear chart
    ResetTransInfo();

    // set user-defined translations
    SetUserTranslations(SYNTAX_BASED, model->NE2NT, model->NE2NTFineModel); 
    
    // rule matching
    MatchSCFGRules();

    // basic rules (to deal with OOV words)
    AddBasicRules();

    // load pre-matched rules and rules used in Viterbi derivation
    LoadPrematchedRules(sentence);
    LoadViterbiRules(sentence);

    if(treeParsing)
        TreeParsing(); // tree-parsing
    else
        CKYDecoding(sentence); // parsing (CKY)

    if(cell[0][srcLength].n == 0)
        decodingFailure = true;

    if(cell[0][srcLength].n == 0 && dealWithDecodingFailure)
        CreateTransUsingTreeSequence();

    if(cell[0][srcLength].n == 0)
        fprintf(stderr, "\nFailure: %s\n", srcSent);

    CheckMe(); // check something
}

void Decoder_SCFG::CKYDecoding(DecodingSentence * sent)
{
    for( int len = 1; len <= srcLength; len++ ){
        for( int beg = 0; beg + len <= srcLength; beg++ ){
            int end = beg + len;
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

   /* CellHypo * ch = cell[8][9].nList[1];
    for(int i = 0; i < model->featNum; i++){
        fprintf(stderr, "%d %f %f\n", i, ch->featValues[i], model->featWeight[i]);
    }*/

    //exit(0);
}

void Decoder_SCFG::GenerateTrans(Cell * c)
{
    int         beg = c->beg, end = c->end;
    List *      patternList = c->matchedPatternList;
    MemPool * tmpMem = smallMem ? new MemPool(M_BYTE * 32, 256) : mem;
    bool        lastUpdate = false;
    bool        entireSpan = (beg <= 1 && end >= srcLength - 1) ? true : false;
    int         nbestSize = (beg == 0 && end == srcLength) ? nbest : beamSize;
    bool        completedWithNTSymbol = withNTSymbol && !entireSpan;         // distinguish different Non-terminal symbols for beam search
    bool        completedWithFineNTSymbol = withFineNTSymbol && !entireSpan; // distinguish different (fine-grained) Non-terminal symbols for beam search

    defaultHypo = (CellHypo*)tmpMem->Alloc(sizeof(CellHypo));
    defaultHypo->Init(c, tmpMem, model->featNum);
    defaultHypo->Create("", model);
    defaultHypo->modelScore = (float)FLOAT_MIN;

    if(CSFMMode)
        candPool->Clear();

    if(treeParsing){ // tree-parsing
        for(int i = 0; i < patternList->count; i++){ // loop for each matched rule
            MatchedPattern * pattern = (MatchedPattern *)patternList->GetItem(i);
            List * ruleList = pattern->ruleList; // list of matched rules

            SlotInformation * slotInfo = (SlotInformation *)tmpMem->Alloc(sizeof(SlotInformation));
            bool haveFoctoid = HaveFoctoid(beg, end, pattern, slotInfo, tmpMem);

            if(ruleList->count > 0)
                ApplyMatchedPattern(c, ruleList, pattern, haveFoctoid, slotInfo, tmpMem); // apply these rules
        }

        c->CompleteWithBeamPruning(nbestSize, this, completedWithNTSymbol, 
                                   completedWithFineNTSymbol, true, false, mem);
    }
    else{ // parsing
        if(CheckForPunctPruning(beg, end)){

            // lexicalized rules!!!
            // loop for each translation pattern
            for(int i = 0; i < patternList->count; i++){
                MatchedPattern * pattern = (MatchedPattern *)patternList->GetItem(i);
                List * ruleList = pattern->ruleList; // list of matched rules

                SlotInformation * slotInfo = (SlotInformation *)tmpMem->Alloc(sizeof(SlotInformation));
                bool haveFoctoid = HaveFoctoid(beg, end, pattern, slotInfo, tmpMem);

                if(ruleList->count > 0)
                    ApplyMatchedPattern(c, ruleList, pattern, haveFoctoid, slotInfo, tmpMem); // apply these rules
            }

            // non-lexicalized rules!!!
            if(withNTSymbol && dtype == SYNTAX_BASED)
                ApplyNonlexRules(c, tmpMem);

            // glue rules
            if(useGlueRule)
                ApplyGlueRulesForSpan(c, tmpMem);
            else if(IsSentEnd(c))
                ApplyGlueRulesForEndSpan(c, tmpMem); // reach the left-end or the right-end
        }

        // beam pruning
        if(!lastUpdate)
            c->CompleteWithBeamPruning(nbestSize, this, completedWithNTSymbol, 
                                       completedWithFineNTSymbol, true, removeInfeasibleRule, mem);

        
    }

    // apply unary rules
    if(allowUnaryProduction && c->end - c->beg <= 1){
        ApplyUnaryRulesInBeam(c, tmpMem);
        c->CompleteWithBeamPruning(nbestSize, this, completedWithNTSymbol, 
                                   completedWithFineNTSymbol, true, 
                                   treeParsing ? false : removeInfeasibleRule, mem);
    }

    // record NT symbols
    if(withNTSymbol)
        CompleteSpanForNTSymbols(c, mem);

    // reranking with loss-augumented score
    if(lossAugumentedDecoding)
        RerankingWithLossAugumentedScore(c);

    if(smallMem)
        delete tmpMem;
}

void Decoder_SCFG::ApplyMatchedPattern(Cell * c, List * ruleList, MatchedPattern * pattern, bool haveFoctoid, SlotInformation * slotInfo, MemPool * tmpMem)
{
    if(withNTSymbol || treeParsing){

        // loop for matched source-side
        for(int i = 0; i < ruleList->count; i++){

            UnitRule * rule = (UnitRule *)ruleList->GetItem(i);
            char * key = rule->src; // matched source-side
            List * matchedRuleList = model->FindRuleListWithSymbol(key); // find all applicable rules

            //if(hieroMatchAll && c->end - c->beg >= 10 && strcmp(rule->root, hieroSymbol))
            //    continue;

            ApplyTranslationRule(c, matchedRuleList, pattern, haveFoctoid, slotInfo, tmpMem);
        }
    }
    else{
        // apply matched rules directly
        ApplyTranslationRule(c, ruleList, pattern, haveFoctoid, slotInfo, tmpMem);
    }
}

void Decoder_SCFG::ApplyTranslationRule(Cell * c, List * ruleList, MatchedPattern * pattern, bool haveFoctoid, SlotInformation * slotInfo, MemPool * tmpMem)
{
    if(!fastDecoding || pattern->matchingNum == 0){

        // loop for each rule
        for(int i = 0; i < ruleList->count; i++){

            UnitRule * rule = (UnitRule *)ruleList->GetItem(i);

            if(!InitSlotInfo(pattern, rule, slotInfo, tmpMem)) // initilize slot information
                continue;
            
            HeuristicSearch(c, rule, slotInfo, haveFoctoid, false, tmpMem); // generate new hypothese for the given rule and the given span
        }
    }
    else if(ruleList->count > 0){ // let's run faster

        UnitRule * firstRule = withNTSymbol ? (UnitRule *)ruleList->GetItem(0) : NULL;

        if(!InitSlotInfo(pattern, firstRule, slotInfo, tmpMem)) // initilize slot information
            return;

        slotInfo->ruleList = ruleList;

        HeuristicSearch(c, NULL, slotInfo, haveFoctoid, false, tmpMem); // generate new hypothese for the given span
    }
}

void Decoder_SCFG::ApplyNonlexRules(Cell * c, MemPool * tmpMem)
{
    if(c->end - c->beg <= 1)
        return;

    //if(hieroMatchAll && c->end - c->beg >= 10)
    //    return;

    int beg = c->beg, end = c->end;

    SlotInformation * slotInfo = (SlotInformation *)tmpMem->Alloc(sizeof(SlotInformation));
    slotInfo->Init(2, tmpMem);
    
    for(int mid = beg + 1; mid < end; mid++)
        ApplyNonlexRules(c, &cell[beg][mid], &cell[mid][end], slotInfo, tmpMem);
}

void Decoder_SCFG::ApplyNonlexRules(Cell * c, Cell * subc1, Cell * subc2, SlotInformation * slotInfo, MemPool * tmpMem)
{
    Cell ** clist = (Cell **)tmpMem->Alloc(sizeof(Cell**) * 2);

    clist[0] = subc1;
    clist[1] = subc2;

    // record the variable information
    for(int i = 0; i < 2; i++){
        Cell * childCell = clist[i];

        if(childCell->n == 0)
            return;

        slotInfo->slotHypoList[i]    = childCell->nList;    // hypothesis list
        slotInfo->slotHypoPointer[i] = childCell->nList[0]; // point to the first item
        slotInfo->hypoNum[i]         = childCell->n;        // number of hypotheses
        slotInfo->invertedNTIndex[i] = i;
        slotInfo->ruleList           = NULL;
    }

    slotInfo->ruleList = GetRuleListForSlotInfo(slotInfo);

    HeuristicSearch(c, NULL, slotInfo, false, true, tmpMem); // apply glue rules

    tmpTrigger = false;
}

// get the rule list for a given source-side
// available for BINARY non-lexicalized expension only
List * Decoder_SCFG::GetRuleListForSlotInfo(SlotInformation * slotInfo) 
{
    sprintf(sourceSideBuf, "#%s #%s\0", slotInfo->slotHypoPointer[0]->root, slotInfo->slotHypoPointer[1]->root);

    return (List *)model->FindRuleListWithSymbol(sourceSideBuf);
}

bool Decoder_SCFG::HaveValidNULLTrans(SlotInformation * slotInfo)
{
    CellHypo * ch0 = slotInfo->slotHypoPointer[0];
    CellHypo * ch1 = slotInfo->slotHypoPointer[1];

    bool leftIsNULL  = !strcmp(ch0->root, "NULL");
    bool rightIsNULL = !strcmp(ch1->root, "NULL");
    bool leftIsComplete  = SynchronousGrammar::IsComplete(ch0->root);
    bool rightIsComplete = SynchronousGrammar::IsComplete(ch1->root);


    int beg = ch0->cell->beg;
    int end = ch1->cell->end;

    if(!leftIsNULL && !rightIsNULL)
        return false;

    if(beg > 1 || end < srcLength - 1)
        return true;
    else
        return false;

    if(leftIsNULL && rightIsNULL)
        return true;
    else if(leftIsNULL && rightIsComplete)
        return true;
    else if(rightIsNULL && leftIsComplete)
        return true;
    else
        return false;
}

void ShowSearchHeap(FILE * file, NodeHeapForSearch * h)
{
    for(int i = 0; i < h->count; i++){
        ExploredNode * node = (ExploredNode *)h->items[i];
        fprintf(file, " %d[%d - %d, %d] %.4e", i, node->ruleOffset, node->offsets[0], node->offsets[1], node->ch == NULL ? 1 : node->ch->modelScore);
        //fprintf(file, " %d[%d - %d]", i, h->items[i]->ruleOffset, h->items[i]->offsets[0]);
    }
    fprintf(file, "\n");
}

void Decoder_SCFG::HeuristicSearch(Cell * c, UnitRule * rule, SlotInformation * slotInfo, bool haveFoctoid, bool forNonlex, MemPool * tmpMem)
{
    int NTCount = slotInfo->slotNum;

    // generate the first hypothesis
    ExploredNode * exploredNode = (ExploredNode *)tmpMem->Alloc(sizeof(ExploredNode));
    exploredNode->offsets = (int *)tmpMem->Alloc(sizeof(int) * NTCount);
    memset(exploredNode->offsets, 0, sizeof(int) * NTCount);
    exploredNode->ruleList = slotInfo->ruleList;

    if(rule == NULL && slotInfo->ruleList != NULL){ // search along "slot" dimensions and the "rule" dimension together
        UnitRule * curRule = (UnitRule *)slotInfo->ruleList->GetItem(0);
        exploredNode->ch = GenerateHypothesisWithTheRule(c, curRule, slotInfo, haveFoctoid, tmpMem);
        exploredNode->lastUpdated = -1;
        exploredNode->lastUpdatedSlot = 0;
        exploredNode->ruleOffset  = 0;
        
    }
    else{ // search along "slot" dimensions only
        if(forNonlex && rule == NULL && !HaveValidNULLTrans(slotInfo))
            exploredNode->ch = NULL;
        else
            exploredNode->ch = GenerateHypothesisWithTheRule(c, rule, slotInfo, haveFoctoid, tmpMem);
        exploredNode->lastUpdated = 0;
        exploredNode->lastUpdatedSlot = 0;
        exploredNode->ruleOffset  = -1;
    }

    int count = 0, computedCount = 0;
    int maxCount = (int)(beamSize * beamScale), maxComputedCount = maxCount * 10;
    heapForSearch->Clear();
    heapForSearch->Push(exploredNode); // starting point

    if(forNonlex){
        ClearExplored();
        SetExplored(exploredNode);
    }

    // heuristics-based search
    while(heapForSearch->count > 0){
        ExploredNode * node = (ExploredNode *)heapForSearch->Pop(); // pop the best hypothesis

        if(IsAcceptable(node->ch)){
            if(forcedDecoding && !CheckHypoInRef(node->ch, decodingSent->refs))
                node->ch = NULL;
            else if(!CSFMMode)
                c->AddCellHypo(node->ch);
            else
                AddCellHypoForCSFM(c, node->ch);
            count++;
        }
        computedCount++;

        if(count > maxCount + 10 || computedCount > maxComputedCount)
            break;
        
        EnQueue(c, node, rule, slotInfo, haveFoctoid, forNonlex, tmpMem); // expand from the current node
    }

    // consider the remaining hypothese in the heap (to allieviate the duplication of hypothese)
    if(dumpLeftHypo){

        while(heapForSearch->count > 0) {
            ExploredNode * node = (ExploredNode *)heapForSearch->Pop(); // pop the best hypothesis

            if(IsAcceptable(node->ch)){
                if(forcedDecoding && !CheckHypoInRef(node->ch, decodingSent->refs))
                    node->ch = NULL;
                else
                    c->AddCellHypo(node->ch);
            }
        }
    }
}

void Decoder_SCFG::EnQueue(Cell * c, ExploredNode * exploredNode,
                           UnitRule * rule, SlotInformation * slotInfo, 
                           bool haveFoctoid, bool forNonlex, MemPool * tmpMem)
{
    int NTCount = slotInfo->slotNum;
    List * ruleList = exploredNode->ruleList;

    for(int i = 0; i < NTCount; i++){
        int offset = exploredNode->offsets[i];
        slotInfo->slotHypoPointer[i] = slotInfo->slotHypoList[i][offset];
    }

    // search alone the "rule" dimension
    if(exploredNode->lastUpdated == -1){

        int newOffset = exploredNode->ruleOffset + 1;

        if(newOffset < ruleList->count){
            UnitRule * newRule = (UnitRule *)ruleList->GetItem(newOffset);
            CellHypo * newch = GenerateHypothesisWithTheRule(c, newRule, slotInfo, haveFoctoid, tmpMem); // generate a new hypothsis
            ExploredNode * newNode = CopyExploredNode(exploredNode, NTCount, tmpMem); // generate a new node for searching
            newNode->ch = newch;
            newNode->ruleOffset = newOffset;

            if(newNode->ch != NULL)
                heapForSearch->Push(newNode);
        }

        if(forNonlex && newOffset > 1) // why ???
            return;
    }

    if(!forNonlex && exploredNode->ruleOffset >= 0)
        rule = (UnitRule *)ruleList->GetItem(exploredNode->ruleOffset); // fix the "rule" dimension

    int updatedSlot = forNonlex ? exploredNode->lastUpdatedSlot : exploredNode->lastUpdated;

    // search alone the "slot" (i.e. variable in SCFG) dimension
    for(int i = updatedSlot; i < NTCount; i++){

        if(i == -1)
            continue;

        bool first = true, failure = false;
        List * ruleListBackup = exploredNode->ruleList;       // backup
        int oldOffset = exploredNode->offsets[i];             // backup
        CellHypo * hypoBackup = slotInfo->slotHypoPointer[i]; // backup

        // for the i-th slot
        for(int newOffset = oldOffset + 1; newOffset < slotInfo->hypoNum[i]; newOffset++){

            exploredNode->offsets[i] = newOffset;  // move forward
            slotInfo->slotHypoPointer[i] = slotInfo->slotHypoList[i][newOffset];
            CellHypo * newHypo = NULL;

            if(forNonlex){
                if(CheckExplored(exploredNode)){
                    first = false;
                    if(tmpTrigger){
                        fprintf(tmpF, "!!!Have explored!!!\n");
                    }
                    continue;
                }

                failure = false;
                exploredNode->ruleList = GetRuleListForSlotInfo(slotInfo);
                if(exploredNode->ruleList != NULL)    // find the matched rules
                    rule = (UnitRule *)exploredNode->ruleList->GetItem(0);
                else if(HaveValidNULLTrans(slotInfo)) // handle word-deletion
                    rule = NULL;
                else{
                    if(first){
                        rule = NULL;
                        newHypo = defaultHypo;    // "defaultHypo" means that we find nothing
                                                  // but we intend to restart from this state in next round
                    }
                    else
                        continue;

                    failure = true;
                }
            }

            ExploredNode * newNode = CopyExploredNode(exploredNode, NTCount, tmpMem); // generate a new node for searching
            if(!failure)
                newHypo = GenerateHypothesisWithTheRule(c, rule, slotInfo, haveFoctoid, tmpMem); // generate a new hypothsis
            newNode->ch = newHypo;

            if(!forNonlex)
                newNode->lastUpdated = i;     // "slot" dimension
            else{
                newNode->lastUpdated = rule == NULL ? i : -1;    // "rule" dimension
                newNode->lastUpdatedSlot = forNonlex ? 0 : i;    // then "slot" dimension
                newNode->ruleOffset  = 0;
                SetExplored(exploredNode);
            }

            if(newNode->ch != NULL){
                heapForSearch->Push(newNode);
                if(!failure)
                    break; // stop until we find a reachable state
            }

            if(!forNonlex) // ???? why do we need this ???
                break;

            first = false;
        }

        exploredNode->ruleList = ruleListBackup;   // recover
        exploredNode->offsets[i] = oldOffset;      // recover
        slotInfo->slotHypoPointer[i] = hypoBackup; // recover
    }
}

bool Decoder_SCFG::CheckExplored(ExploredNode * me)
{
    return (explored[me->offsets[0]][me->offsets[1]] != 0);
}

void Decoder_SCFG::SetExplored(ExploredNode * me)
{
    explored[me->offsets[0]][me->offsets[1]] = 1;
}


ExploredNode * Decoder_SCFG::CopyExploredNode(ExploredNode * me, int NTCount, MemPool * tmpMem)
{
    ExploredNode * newNode = (ExploredNode *)tmpMem->Alloc(sizeof(ExploredNode));

    newNode->offsets = (int *)tmpMem->Alloc(sizeof(int) * NTCount);
    memcpy(newNode->offsets, me->offsets, sizeof(int) * NTCount);

    newNode->lastUpdated     = me->lastUpdated;
    newNode->lastUpdatedSlot = me->lastUpdatedSlot;
    newNode->ruleOffset      = me->ruleOffset;
    newNode->ruleList        = me->ruleList;

    return newNode;
}

char XSymbol[2] = "X";
char HieroBaseSymbol[3] = "XX";

CellHypo * Decoder_SCFG::GenerateHypothesisWithTheRule(Cell * c, UnitRule * rule, SlotInformation * slotInfo, bool haveFoctoid, MemPool * tmpMem)
{
    CellHypo * ch = NULL;

    if(rule == NULL){ // glue rule?
        if(slotInfo->slotNum == 2){
            CellHypo * ch0 = slotInfo->slotHypoPointer[0];
            CellHypo * ch1 = slotInfo->slotHypoPointer[1];

            ch = ComposeTwoHypotheses(c, slotInfo->slotHypoPointer[0], slotInfo->slotHypoPointer[1], HIERO, tmpMem); // yes

            if(ch != NULL){
                ch->lc = slotInfo->slotHypoPointer[0];
                ch->rc = slotInfo->slotHypoPointer[1];
                ch->ruleUsed = rule;
                
                // addtional computation for model score
                AddFeatValue(ch, Model::GlueRuleCount, 1);  // glue rule penalty

                if(withNTSymbol){
                    AssignRuleAndSyntacticLabel(ch, ch->lc, ch->rc);
                    AddRuleProbForImcompleteHypo(ch, ch->lc, ch->rc); // rule score for incomplete rule production
                }
                else{
                    if(ch0->cell->end - ch0->cell->beg == 1 && ch0->cell->beg == 0)
                        ch->root = ch1->root;
                    else if(ch1->cell->end - ch1->cell->beg == 1 && ch1->cell->end == srcLength)
                        ch->root = ch0->root;
                    else
                        ch->root = XSymbol;
                }
            }

            if(dumpUsedRule){
                RecordChildHypo(ch, slotInfo, tmpMem);
                ch->ruleUsed = CreateGlueRule(c, slotInfo, mem); // NOTE: no tmp memory is used
            }

            if(CSFMMode)
                ch = FineModelHypo(ch, rule, slotInfo, tmpMem);

            return ch;
        }
        else
            return NULL;
    }

    float NGramLMScore = 0;
    int wCount = 0, NTCount = 0;

    // for n-gram language model
    for(int i = 0; i < rule->wCount; i++){
        if(rule->wid[i] < 0){ // variable
            //int varIndex = slotInfo->invertedNTIndex[NTCount]; // which source-side variable is aligned with me?
            int varIndex = -(rule->wid[i] + 1); // which source-side variable is aligned with me?
            CellHypo * currentHypo = slotInfo->slotHypoPointer[varIndex]; // OK, I found you!
            slotInfo->invertedNTIndex[NTCount] = varIndex; // record non-terminal alignment

            if(currentHypo->wCount == 0 && !allowNULLSubstitution)
                return NULL;

            //memcpy(LMScoreBuf + wCount, currentHypo->LMScore, currentHypo->wCount);
            memcpy(widBuf + wCount, currentHypo->wid, sizeof(int) * currentHypo->wCount);

            for(int k = 0; k < currentHypo->wCount; k++){
                int end = wCount + 1;
                int beg = end - ngram < 0 ? 0 : end - ngram;
                if( k + 1 >= ngram )
                    LMScoreBuf[wCount] = currentHypo->LMScore[k];
                else
                    LMScoreBuf[wCount] = model->GetNGramLMProb(beg, end, widBuf);
                NGramLMScore += LMScoreBuf[wCount];

                wCount++;
            }

            NTCount++;
        }
        else{ // words in the rule
            widBuf[wCount] = rule->wid[i];

            int end = wCount + 1;
            int beg = end - ngram < 0 ? 0 : end - ngram;
            LMScoreBuf[wCount] = model->GetNGramLMProb(beg, end, widBuf);
            NGramLMScore += LMScoreBuf[wCount];

            wCount++;
        }
    }

    ch = (CellHypo*)tmpMem->Alloc(sizeof(CellHypo));
    ch->Init(c, tmpMem, model->featNum);

    ch->wid = (int *)tmpMem->Alloc(sizeof(int) * wCount);
    memcpy(ch->wid, widBuf, sizeof(int) * wCount);
    ch->LMScore = (float *)tmpMem->Alloc(sizeof(float) * wCount);
    memcpy(ch->LMScore, LMScoreBuf, sizeof(int) * wCount);
    ch->wCount = wCount;

    // n-gram LM
    ch->featValues[Model::NGramLM] = NGramLMScore;
    ch->featValues[Model::WordCount] = (float)ch->wCount;

    // translation
    GenerateTranslationWithTheRule(rule, slotInfo, ch->translation, ch->transLength, tmpMem);

    if(haveFoctoid && replaceFoctoid){
        ch->translation = ReplaceFoctoidTrans(slotInfo->wordIndex, slotInfo->wordNum, ch->translation, tmpMem);
        ch->transLength = (int)strlen(ch->translation);
    }

    // model score
    float overallScore = 0;

    // score of substituted variables
    for(int var = 0; var < NTCount; var++){
        // for the i-th variable
        int varIndex = slotInfo->invertedNTIndex[var];
        CellHypo * currentHypo = slotInfo->slotHypoPointer[varIndex];

        for( int f = 0; f < model->featNum; f++ ){
            if( f != Model::NGramLM &&  f != Model::WordCount)
                ch->featValues[f] += currentHypo->featValues[f];
        }
    }

    // score of the rule
    for( int f = 0; f < model->featNum; f++ ){
        if(f != Model::NGramLM &&  f != Model::WordCount){
            if(rule->isComplete)
                ch->featValues[f] += rule->feat[f];
            else
                overallScore += rule->feat[f] * model->featWeight[f];
        }

        overallScore += ch->featValues[f] * model->featWeight[f];
    }

    ch->modelScore = overallScore;

    // loss of substituted variables
    if(lossAugumentedDecoding){
        ch->beamLoss = 0;
        ch->bleuLoss = 0;
        for(int var = 0; var < NTCount; var++){
            int varIndex = slotInfo->invertedNTIndex[var];
            CellHypo * currentHypo = slotInfo->slotHypoPointer[varIndex];
            ch->beamLoss += currentHypo->beamLoss;
        }
    }

    // root label
    ch->root = rule->root;

    ch->lc = NULL;
    ch->rc = NULL;
    if(NTCount > 0){
        if(NTCount <= 2)
            ch->lc = slotInfo->slotHypoPointer[0];
        if(NTCount == 2)
            ch->rc = slotInfo->slotHypoPointer[1];
    }
    ch->ruleUsed = rule;
    ch->patternUsed = slotInfo->pattern;

    // record path
    if(dumpUsedRule)
        RecordChildHypo(ch, slotInfo, tmpMem);

    // scoring using a fine-grained model
    if(CSFMMode)
        ch = FineModelHypo(ch, rule, slotInfo, tmpMem);

    /*if(!model->noFiltering && rule->isComplete && strstr(ch->translation, "$number") != NULL){
        fprintf(stderr, "$number error[%d, %d]: %s\n%s -> %s\n", c->beg, c->end, ch->translation, rule->src, rule->tgt);
        exit(0);
    }*/

    return ch;
}

void Decoder_SCFG::GenerateTranslationWithTheRule(UnitRule * rule, SlotInformation * slotInfo, char * & trans, int & transLen, MemPool * tmpMem)
{
    // generate translation
    char * tgt = rule->tgt;
    int    len = (int)strlen(tgt);
    char * beg = strchr(tgt, NT_SYMBOL);
    char * end = beg + 1;
    char * last = tgt;
    int    tgtLen = 0, NTCount = 0;

    while(beg > tgt && *(beg - 1) != ' '){
       beg = strchr(beg + 1, NT_SYMBOL);
       end = beg + 1;
    }

    while(beg != NULL){
        end = beg + 1;

        while(*end != ' ' && *end != '\0')
            end++;

        if(end != beg + 1){
            strncpy(transBuf + tgtLen, last, beg - last); // words
            tgtLen += (int)(beg - last);
            
            int varIndex = slotInfo->invertedNTIndex[NTCount]; // which source-side variable is aligned with me?
            CellHypo * currentHypo = slotInfo->slotHypoPointer[varIndex]; // OK, I found you!
            int slotTransLen = (int)strlen(currentHypo->translation);

            // the variable is replaced with the word sequence
            strncpy(transBuf + tgtLen, currentHypo->translation, slotTransLen);
            tgtLen += slotTransLen;

            if(*end != '\0' && slotTransLen > 0)
                transBuf[tgtLen++] = ' ';

            last = end + 1;
            NTCount++;
        }

        beg = strchr(end, NT_SYMBOL);

        while(beg > tgt && *(beg - 1) != ' ')
            beg = strchr(beg + 1, NT_SYMBOL);

    }

    beg = tgt + len;
    if(beg - last > 0){
        strncpy(transBuf + tgtLen, last, beg - last); // lexicons
        tgtLen += (int)(beg - last);
    }

    if(transBuf[tgtLen - 1] == ' ')
        tgtLen--;

    transBuf[tgtLen] = '\0';

    trans = (char *)tmpMem->Alloc(sizeof(char) * (tgtLen + 1));
    strcpy(trans, transBuf);

    transLen = tgtLen;
}

void Decoder_SCFG::RecordChildHypo(CellHypo * ch, SlotInformation * slotInfo, MemPool * tmpMem)
{
    ch->childHypoNum = slotInfo->slotNum;
    ch->childHypos   = (CellHypo**)tmpMem->Alloc(sizeof(CellHypo*) * slotInfo->slotNum);
    memcpy(ch->childHypos, slotInfo->slotHypoPointer, sizeof(CellHypo*) * slotInfo->slotNum);
}

// create a vitual rule to explain "glue rule"
UnitRule * Decoder_SCFG::CreateGlueRule(Cell * c, SlotInformation * slotInfo, MemPool * tmpMem)
{
    int srcLength = 0;
    int tgtLength = 0;

    SCFGRule * rule = (SCFGRule *)tmpMem->Alloc(sizeof(SCFGRule));
    UnitRule * urule = (UnitRule *)tmpMem->Alloc(sizeof(UnitRule));

    for(int i = 0; i < slotInfo->slotNum; i++){
        char * root = !strcmp(slotInfo->slotHypoPointer[i]->root, "") ?
                      GetDefaultRootLabel(slotInfo->slotHypoPointer[i]->cell, "NP", tmpMem):
                      slotInfo->slotHypoPointer[i]->root;
        srcLength += strlen(root) + 2;
        tgtLength += i >= 10 ? 4 : 3;
    }

    srcLength++;
    tgtLength++;

    rule->src = (char *)tmpMem->Alloc(sizeof(char) * srcLength);
    memset(rule->src, 0, sizeof(char) * srcLength);
    rule->tgt = (char *)tmpMem->Alloc(sizeof(char) * tgtLength);
    memset(rule->tgt, 0, sizeof(char) * tgtLength);

    for(int i = 0; i < slotInfo->slotNum; i++){
        char * root = !strcmp(slotInfo->slotHypoPointer[i]->root, "") ?
                      GetDefaultRootLabel(slotInfo->slotHypoPointer[i]->cell, "NP", tmpMem):
                      slotInfo->slotHypoPointer[i]->root;
        sprintf(rule->src, "%s#%s ", rule->src, root);
        sprintf(rule->tgt, "%s#%d ", rule->tgt, i + 1);
    }

    rule->src[srcLength - 2] = '\0';
    rule->tgt[tgtLength - 2] = '\0';

    //if(hieroMatchAll)
    //    rule->root = hieroSymbol;
    //else
        rule->root = GetDefaultRootLabel(c, "NP", tmpMem);

    char * newRoot = (char *)tmpMem->Alloc(sizeof(char) * (strlen(rule->root) + 6));
    sprintf(newRoot, "%s_GLUE", rule->root);
    rule->root = newRoot;

    rule->alignment = (char *)tmpMem->Alloc(sizeof(char) * 8);
    strcpy(rule->alignment, "0-0 1-1");

    memcpy(urule, rule, sizeof(BasicRule));

    rule->ruleList = (UnitRule **)tmpMem->Alloc(sizeof(UnitRule *));
    rule->ruleList[0] = urule;
    rule->ruleCount = 1;

    urule->parentRule = rule;

    return urule;
}

bool Decoder_SCFG::InitSlotInfo(MatchedPattern * pattern, BasicRule * rule, SlotInformation * slotInfo, MemPool * tmpMem)
{
    if(withNTSymbol && rule == NULL)
        return false;

    int slotNum = pattern->matchingNum;
    
    CellHypo *** slotHypoList = (CellHypo ***)tmpMem->Alloc(sizeof(CellHypo**) * slotNum); // n-best hypotheses for each slot (variable)
    CellHypo ** slotHypoPointer = (CellHypo **)tmpMem->Alloc(sizeof(CellHypo*) * slotNum); // a poniter to the current hypothese used
    int * hypoNum = (int *)tmpMem->Alloc(sizeof(int) * slotNum);
    int * invertedNTIndex = (int *)tmpMem->Alloc(sizeof(int) * slotNum); // point to the corresponding variable on the source-language side
    Nonterminal * NTs = rule == NULL ? NULL : rule->NT;

    for(int i = 0; i < slotNum; i++){
        int beg = pattern->matching[i].left;
        int end = pattern->matching[i].right + 1;
        Cell * childCell = treeParsing ? pattern->matching[i].cellNode : &cell[beg][end];

        if(hieroMatchAll){
            char * NTSymbol    = GetNonterminalSymbol(NTs + i);
            if(!strcmp(NTSymbol, hieroSymbol)){           // hiero symbol => match anything
                if(childCell->n == 0)
                    return false;

                slotHypoList[i]    = childCell->nList;    // hypothesis list
                slotHypoPointer[i] = childCell->nList[0]; // point to the first item
                hypoNum[i]         = childCell->n;        // number of hypotheses
                continue;
            }
        }

        if(!withNTSymbol){ // no syntactic labels are invloved
            if(childCell->n == 0)
                return false;

            slotHypoList[i]    = childCell->nList;    // hypothesis list
            slotHypoPointer[i] = childCell->nList[0]; // point to the first item
            hypoNum[i]         = childCell->n;        // number of hypotheses
        }
        else{
            char * NTSymbol    = GetNonterminalSymbol(NTs + i);
            HypoList * hypos   = (HypoList *)childCell->nListWithSymbol->GetObject(NTSymbol);

            if(hypos == NULL)
                return false;

            slotHypoList[i]    = (CellHypo**)hypos->list;          // hypothesis list
            slotHypoPointer[i] = (CellHypo*)hypos->list[0];       // point to the first item
            hypoNum[i]         = hypos->n;        // number of hypotheses ???????????????????????????
        }

        ///////////////////////////////////////////
        /// TODO: exact label matching
    }

    // record the variable information
    slotInfo->slotNum = slotNum;
    slotInfo->slotHypoPointer = slotHypoPointer;
    slotInfo->slotHypoList = slotHypoList;
    slotInfo->invertedNTIndex = invertedNTIndex;
    slotInfo->hypoNum = hypoNum;
    slotInfo->ruleList = NULL;
    slotInfo->pattern = pattern;

    return true;
}

char * Decoder_SCFG::GetNonterminalSymbol(Nonterminal * NT)
{
    return NT->symbol;

    /// TODO: return the source-side symbol or the target-side symbol
}

bool Decoder_SCFG::HaveFoctoid(int beg, int end, MatchedPattern * pattern, SlotInformation * slotInfo, MemPool * myMem)
{
    bool haveFoctoid = false;
    int nslot = 0, wordIndexCount = 0;

    for(int i = beg; i < end; i++)
    {
        if(nslot < pattern->matchingNum){
            if(i >= pattern->matching[nslot].left && i <= pattern->matching[nslot].right){
                i = pattern->matching[nslot].right;
                nslot++;
                continue;
            }
        }

        wordIndexBuf[wordIndexCount++] = i;

        if(IsFactoid(srcWords[i]))
            haveFoctoid = true;
    }

    slotInfo->wordIndex = (int *)myMem->Alloc(sizeof(int) * wordIndexCount);
    memcpy(slotInfo->wordIndex, wordIndexBuf, sizeof(int) * wordIndexCount);
    slotInfo->wordNum = wordIndexCount;

    return haveFoctoid;
}

// given $number -> 2000
// There are $number students in this school . -> There are 2000 students in this school .
char * Decoder_SCFG::ReplaceFoctoidTrans(int * wordIndex, int wordNum, char * trans, MemPool * myMem)
{
    char * result;
    char   resultLen = 0;
    int    len = (int)strlen(trans);
    int    p = 0, q = 0;
    int    fid = 0, fsrcCount = 0;
    int    totalLen = 0;

    transBuf[0] = '\0';

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

                for(int i = 0; i < wordNum; i++){
                    int wi = wordIndex[i];
                    if( IsFactoid(srcWords[wi]) ){
                        if( fid == fsrcCount ){
                            if( partialTrans[wi] != NULL )
                                sprintf(transBuf, "%s%s ", transBuf, partialTrans[wi]->translation);
                        }
                        fsrcCount ++;
                    }
                }
                fid++;
            }
            else
                sprintf(transBuf, "%s%s ", transBuf, curWord);

            trans[p++] = tmp;
            q = p;
        }
        else
            p++;
    }

    totalLen = (int)strlen(transBuf);
    totalLen--; // remove the last space character
    if( totalLen == -1 )  // modified by zhanghao, 2011 April 22th
        totalLen = 0;

    transBuf[totalLen] = '\0';

    result = (char*)myMem->Alloc(sizeof(char) * (totalLen + 1));
    strcpy(result, transBuf);
    return result;
}

// glue two hypotheses
void Decoder_SCFG::ApplyGlueRulesForSpan(Cell * c, MemPool * tmpMem)
{
    int beg = c->beg, end = c->end;

    SlotInformation * slotInfo = (SlotInformation *)tmpMem->Alloc(sizeof(SlotInformation));
    slotInfo->Init(2, tmpMem);
    
    for(int mid = beg + 1; mid < end; mid++)
        GlueTwoCells(c, &cell[beg][mid], &cell[mid][end], slotInfo, tmpMem);
}

void Decoder_SCFG::ApplyGlueRulesForEndSpan(Cell * c, MemPool * tmpMem)
{
    if(c->end - c->beg == 1)
        return;

    int beg = c->beg, end = c->end;

    SlotInformation * slotInfo = (SlotInformation *)tmpMem->Alloc(sizeof(SlotInformation));
    slotInfo->Init(2, tmpMem);

    if(beg == 0)
        GlueTwoCells(c, &cell[beg][beg+1], &cell[beg+1][end], slotInfo, tmpMem);
    else if(end == srcLength)
        GlueTwoCells(c, &cell[beg][end-1], &cell[end-1][end], slotInfo, tmpMem);

    tmpTrigger = false;
}

void Decoder_SCFG::GlueTwoCells(Cell * c, Cell * subc1, Cell * subc2, SlotInformation * slotInfo, MemPool * tmpMem)
{
    Cell ** clist = (Cell **)tmpMem->Alloc(sizeof(Cell**) * 2);

    clist[0] = subc1;
    clist[1] = subc2;

    // record the variable information
    for(int i = 0; i < 2; i++){
        Cell * childCell = clist[i];

        if(childCell->n == 0)
            return;

        slotInfo->slotHypoList[i]    = childCell->nList;    // hypothesis list
        slotInfo->slotHypoPointer[i] = childCell->nList[0]; // point to the first item
        slotInfo->hypoNum[i]         = childCell->n;        // number of hypotheses
        slotInfo->invertedNTIndex[i] = i;
        slotInfo->ruleList           = NULL;
    }

    HeuristicSearch(c, NULL, slotInfo, false, false, tmpMem); // apply glue rules
}

void Decoder_SCFG::ApplyUnaryRulesInBeam(Cell * c, MemPool * tmpMem)
{
    char root[MAX_PHRASE_CHAR_NUM];

    if(c->end - c->beg == 1 && (c->beg == 0 || c->end == srcLength))
        return;

    // create a virtual structure to record the slot information
    SlotInformation * slotInfo = (SlotInformation *)tmpMem->Alloc(sizeof(SlotInformation));
    memset(slotInfo, 0, sizeof(SlotInformation));
    slotInfo->slotNum = 1;
    slotInfo->slotHypoPointer = (CellHypo**)tmpMem->Alloc(sizeof(CellHypo*));
    slotInfo->invertedNTIndex = (int *)tmpMem->Alloc(sizeof(int));

    for(int i = 0; i < c->n; i++){
        CellHypo * ch = c->nList[i];

        slotInfo->slotHypoPointer[0] = ch;
        slotInfo->invertedNTIndex[0] = 0;

        sprintf(root, "#%s\0", ch->root);
        List * unaryRuleList = model->FindRuleListWithSymbol(root);

        if(unaryRuleList != NULL){ // 1-depth recursion only
            for(int k = 0; k < unaryRuleList->count; k++){
                UnitRule * urule = (UnitRule *)unaryRuleList->GetItem(k);
                CellHypo * newch = GenerateHypothesisWithTheRule(c, urule, slotInfo, false, tmpMem);
                if(newch != NULL)
                    c->AddCellHypo(newch);

                if(newch != NULL && !strcmp(newch->root, "")){
                    fprintf(stderr, "ERROR: newch->root[0] == '\\0'\n");
                }
            }
        }
    }
}

inline int CompareWithNTSymbolAndModeScore(CellHypo * ch1, CellHypo * ch2)
{
    int comp = strcmp(ch1->root, ch2->root);
    if(comp == 0){
        if(ch1->modelScore > ch2->modelScore)
            comp = 1;
        else if(ch1->modelScore < ch2->modelScore)
            comp = -1;
        else
            comp = 0;
    }

    return comp;
}

void QuickSortWithNTSymbolAndModeScore(void ** list, int left, int right)
{
      int i = left, j = right;
      void * tmp;
      CellHypo * mid = (CellHypo*)list[(left + right) / 2];

      /* partition */
      while (i <= j) {
          while(i <= j && CompareWithNTSymbolAndModeScore((CellHypo*)list[i], mid) > 0)
              i++;
          while(i <= j && CompareWithNTSymbolAndModeScore((CellHypo*)list[j], mid) < 0)
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
          QuickSortWithNTSymbolAndModeScore(list, left, j);
      if (i < right)
           QuickSortWithNTSymbolAndModeScore(list, i, right);
}

void Decoder_SCFG::CompleteSpanForNTSymbols(Cell * c, MemPool * mem)
{
    if(c->n == 0)
        return;

    CellHypo ** newList = (CellHypo **)mem->Alloc(sizeof(CellHypo*) * c->n);
    memcpy(newList, c->nList, sizeof(CellHypo*) * c->n);

    QuickSortWithNTSymbolAndModeScore((void**)newList, 0, c->n - 1);

    int beg = 0, end = beg;

    for(end = beg + 1; end < c->n; end++){
        CellHypo * chend0 = newList[end - 1];
        CellHypo * chend1 = newList[end];

        if(strcmp(chend0->root, chend1->root)){
            HypoList * listNode = (HypoList*)mem->Alloc(sizeof(HypoList));
            listNode->list = (void**)(newList + beg);
            listNode->n    = end - beg;
            beg            = end;

            c->nListWithSymbol->AddObject(chend0->root, listNode); // index it using the root symbol
        }
    }

    HypoList * listNode = (HypoList*)mem->Alloc(sizeof(HypoList));
    listNode->list = (void**)(newList + beg);
    listNode->n    = end - beg;
    c->nListWithSymbol->AddObject(newList[end - 1]->root, listNode);

    for(int i = 0; i < c->n; i++){
        if(!strcmp(newList[i]->root, "")){
            if(beg == 0 && end == 1)
                continue;
            if(beg == srcLength - 1 && end == srcLength)
                continue;

            fprintf(stderr, "ERROR: No root label for span[%d, %d]!\n", c->beg, c->end);
        }
    }
}

void Decoder_SCFG::AssignRuleAndSyntacticLabel(CellHypo * ch, CellHypo * subc1, CellHypo * subc2)
{
    Cell * cell1 = subc1->cell;
    Cell * cell2 = subc2->cell;

    bool leftEnd   = cell1->beg == 0 && cell1->end == 1;
    bool rightEnd  = cell2->beg == srcLength - 1 && cell2->end == srcLength;
    bool leftNULL  = !strcmp(subc1->root, "NULL");
    bool rightNULL = !strcmp(subc2->root, "NULL");

    if(!leftEnd && rightEnd){
        ch->root = subc1->root;
        ch->ruleUsed = subc1->ruleUsed;
    }
    else if(leftEnd && !rightEnd){
        ch->root = subc2->root;
        ch->ruleUsed = subc2->ruleUsed;
    }
    else if(leftNULL || rightNULL){
        if(leftNULL){
            ch->root = subc2->root;
            ch->ruleUsed = subc2->ruleUsed;
        }
        else{
            ch->root = subc1->root;
            ch->ruleUsed = subc1->ruleUsed;
        }
    }
    else if(hieroMatchAll){
        ch->root = hieroSymbol;
    }
    else{
        ch->root = defaultSymbol; // TODO: use the most likely symbol instead of "NP". How to do it?
    }
}

void Decoder_SCFG::AddRuleProbForImcompleteHypo(CellHypo * newch, CellHypo * subc1, CellHypo * subc2)
{
    if(!SynchronousGrammar::IsComplete(newch->root)){
        if(!SynchronousGrammar::IsComplete(subc1->root)){ // rule score for the first hypothesis
            float overallScore = newch->modelScore;
            UnitRule * rule = (UnitRule *)subc1->ruleUsed;
            for( int f = 0; f < model->featNum; f++ ){
                if(f != Model::NGramLM && f != Model::WordCount){
                    overallScore += rule->feat[f] * model->featWeight[f];
                }
            }
            newch->modelScore = overallScore;

        }
        if(!SynchronousGrammar::IsComplete(subc2->root)){ // rule score for the seconod hypothesis
            float overallScore = newch->modelScore;
            UnitRule * rule = (UnitRule *)subc2->ruleUsed;
            for( int f = 0; f < model->featNum; f++ ){
                if(f != Model::NGramLM && f != Model::WordCount){
                    overallScore += rule->feat[f] * model->featWeight[f];
                }
            }
            newch->modelScore = overallScore;
        }
    }
}

bool Decoder_SCFG::IsSentEnd(Cell * c)
{
    bool leftEnd  = c->beg == 0;
    bool rightEnd = c->end == srcLength;
    return leftEnd || rightEnd;
}

bool Decoder_SCFG::IsAcceptable(CellHypo * ch)
{
    if(ch == NULL)
        return false;
    else if(ch->modelScore <= MODEL_SCORE_MIN)
        return false;

    int beg = ch->cell->beg, end = ch->cell->end;

    if(beg == 0 && end == srcLength){ // entire span
        if(!SynchronousGrammar::IsComplete(ch->root))
            return false;
    }
    return true;
}

void Decoder_SCFG::CompleteCell(Cell * c)
{
    int  beg = c->beg, end = c->end;
    int  nbestSize = (beg == 0 && end == srcLength) ? nbest : beamSize;
    bool entireSpan = (beg <= 1 && end >= srcLength - 1) ? true : false;
    bool completedWithNTSymbol = withNTSymbol && !entireSpan;         // distinguish different Non-terminal symbols for beam search
    bool completedWithFineNTSymbol = withFineNTSymbol && !entireSpan; // distinguish different (fine-grained) Non-terminal symbols for beam search

    if(treeParsing){ // tree-parsing
        c->CompleteWithBeamPruning(nbestSize, this, completedWithNTSymbol, 
                                   completedWithFineNTSymbol, true, false, mem);
    }
    else{ // parsing
        c->CompleteWithBeamPruning(nbestSize, this, completedWithNTSymbol, 
                                   completedWithFineNTSymbol, true, removeInfeasibleRule, mem);
    }

    // record NT symbols
    if(withNTSymbol)
        CompleteSpanForNTSymbols(c, mem);
}

//////////////////////////////////////////////////////////////
//// to build a reachable derivation when decoding fails

void Decoder_SCFG::CreateTransUsingTreeSequence()
{
    List * bestSpanSequence = NULL;

    // search for a sequence of nodes with "most" translations (or target words)
    bestSpanSequence = treeParsing ? 
                       GetBestSpanSequenceForTreeParsing() : 
                       GetBestSpanSequenceForParsing();

    if(bestSpanSequence == NULL)
        return;

    bestSpanSequence->Insert(0, (void*)&cell[0][1]); // <s>: begining of the sentence
    bestSpanSequence->Add((void*)&cell[srcLength - 1][srcLength]); // </s>: : end of the sentence

    // glue translations for the (tree-node) sequence
    GlueMultipleNodeTrans(&cell[0][srcLength], bestSpanSequence, mem);

    cell[0][srcLength].CompleteWithBeamPruning(nbest, this, false, false, 
                                               true, false, mem);

    if(lossAugumentedDecoding)
        RerankingWithLossAugumentedScore(&cell[0][srcLength]);
}

List * Decoder_SCFG::GetBestSpanSequenceForTreeParsing()
{

    List * bestSequence = (List *)mem->Alloc(sizeof(List));
    bestSequence->Create(1, mem);

	if(srcTree != NULL && srcTree->root != NULL){

		TreeNode * root = (TreeNode *)cellNodes->treeNodes->GetItem(0); // root of the (1best) tree
																		// TODO: forest
		// get a sequence of tree-nodes yeiled by a given root node
		GetBestSpanSequenceForTreeParsing(bestSequence, root);
	}

    return bestSequence;
}


float Decoder_SCFG::GetBestSpanSequenceForTreeParsing(List * sequence, TreeNode * root)
{
    Cell * c = &cellNodes[root->id];

    // if pre-terminal has nothing to be matched
    if(c->n == 0 && (root->edges == NULL || root->edges->count == 0)){
        if(forcedDecoding){
            List * ruleList = model->FindRuleList("<unk>");
            for(int i = 0; i < ruleList->count; i++){
                UnitRule * rule = (UnitRule *)ruleList->GetItem(i);
                AddCellTrans(c, rule, outputOOV, GetDefaultRootLabel(c, "NP", mem));
            }
            CompleteCell(c);
        }
        else{
            fprintf(stderr, "ERROR! no available derivation in leaf nodes!\n");
            return 0;
        }
    }

    // if some hypotheses have been generated, use them!
    if(c->n > 0){
        sequence->Add(c);
        CellHypo * bestTrans = c->nList[0];
        return bestTrans->modelScore;
    }
    else if(root->edges == NULL || root->edges->count == 0){
        fprintf(stderr, "what's wrong!!!\n");
        return 0;
    }

    float bestScore = MODEL_SCORE_MIN;
    List * bestSequence = NULL;

    // glue partial translations recursively 
    for(int i = 0; i < root->edges->count; i++){ // for each edge
        TreeEdge * edge = (TreeEdge *)root->edges->GetItem(i);

        float score = 0;
        List * childSequence = (List *)mem->Alloc(sizeof(List));
        childSequence->Create(edge->children->count, mem);

        for(int j = 0; j < edge->children->count; j++){ // for each child node
            TreeNode * child = (TreeNode *)edge->children->GetItem(j);
            score += GetBestSpanSequenceForTreeParsing(childSequence, child); // build the sequence recursively
            score += -1000; // penatly
        }

        if(score > bestScore){
            bestScore = score;
            bestSequence = childSequence;
        }
    }

    sequence->Add(bestSequence->items, bestSequence->count);

    return bestScore;
}

List * Decoder_SCFG::GetBestSpanSequenceForParsing()
{
    List * bestSequence = (List *)mem->Alloc(sizeof(List));
    bestSequence->Create(1, mem);

    for(int beg = 1; beg < srcLength; beg++){
        for(int end = beg + 1; end < srcLength; end++){
            Cell * c = &cell[beg][end];

            if(c->n == 0){
                if(end > beg + 1){
                    bestSequence->Add(&cell[beg][end - 2]);
                    beg = end - 1;
                    break;
                }
                else{
                    bestSequence->Add(c); // oov
                    break;
                }
            }
        }
    }

    return bestSequence;
}

//////////////////////////////////////////////////////////////
//// to add rules using pre-matched patterns

void Decoder_SCFG::LoadPrematchedRules(DecodingSentence * sentence)
{
    if(sentence->matchedRules == NULL)
        return;

    List * ruleList = sentence->matchedRules;
    for(int i = 0; i < ruleList->count; i++){
        SimpleRule * rule = (SimpleRule *)ruleList->GetItem(i);
        int beg = rule->beg, end = rule->end + 1;

        if(rule->isUnaryRule)
            continue;

        if(treeParsing){
            Cell * c = &cell[beg][end];
            for(int j = 0; j < c->cellNodes->count; j++){
                Cell * cellNode = (Cell *)c->cellNodes->GetItem(j);
                AddPrematchedRules(cellNode, rule);
            }
        }
        else
            AddPrematchedRules(&cell[beg][end], rule);
    }
}

void Decoder_SCFG::AddPrematchedRules(Cell * c, SimpleRule * rule)
{
    TreeNode * rootNode = NULL;

    if(treeParsing){
        rootNode = (TreeNode *)c->treeNodes->GetItem(0);
        if(!IsMatched(rootNode, rule->rootSrc))
            return;
    }

    bool haveMatched = false;

    for(int i = 0; i < c->matchedPatternList->count && i < c->matchedPatternCount; i++){
        MatchedPattern * pattern = (MatchedPattern *)c->matchedPatternList->GetItem(i);

        if(treeParsing){
            if(IsMatched(rootNode, pattern, rule))
                haveMatched = true;
        }
        else if(IsMatched(NULL, pattern, rule))
            haveMatched = true;

        if(haveMatched) // exist
            return;
    }

    if(!haveMatched){
        MatchedPattern * newPattern = GenerateMatchedPattern(rule, mem);
        if(newPattern != NULL){
            c->matchedPatternList->Add((void *)newPattern); // add pattern
            //fprintf(stderr, "[%d, %d] Loaded matched rule \"%s -> %s\"\n", c->beg, c->end, rule->src, rule->tgt);
        }
        else{
            //fprintf(stderr, "[%d, %d] WARNING! No rules found for \"%s || %s -> %s\"\n", c->beg, c->end, rule->root, rule->src, rule->tgt);
        }
    }
}

bool Decoder_SCFG::IsMatched(TreeNode * rootNode, MatchedPattern * pattern, SimpleRule * rule)
{
    if(treeParsing && strcmp(rootNode->label, rule->rootSrc))
        return false;

    if(pattern->matchingNum != rule->NTCount)
        return false;

    for(int i = 0; i < pattern->matchingNum; i++){
        NTBoundary * slot1 = &pattern->matching[i];
        Boundary * slot2 = (Boundary *)rule->slots->GetItem(i);

        if(slot1->left != slot2->left || slot1->right != slot2->right)
            return false;

        if(treeParsing){
            TreeNode * childNode = (TreeNode *)slot1->cellNode->treeNodes->GetItem(0);
            if(!IsMatched(childNode, rule->NT[i].symbolSrc))
                return false;
        }
        
    }

    return true;
}

bool Decoder_SCFG::IsMatched(TreeNode * node, char * targetLabel)
{
    if(node == NULL || targetLabel == NULL)
        return true;

    if(treeParsing && strcmp(node->label, targetLabel))
        return false;

    return true;
}

MatchedPattern * Decoder_SCFG::GenerateMatchedPattern(SimpleRule * rule, MemPool * myMem)
{
    List * ruleList = model->FindRuleList(rule->key);

    if(ruleList == NULL)
        return NULL;

    MatchedPattern * pattern = (MatchedPattern *)myMem->Alloc(sizeof(MatchedPattern));
    memset(pattern, 0, sizeof(MatchedPattern));

    pattern->matching = new NTBoundary[rule->NTCount];
    for(int i = 0; i < rule->NTCount; i++){
        Boundary * slot = (Boundary *)rule->slots->GetItem(i);
        pattern->matching[i].left  = slot->left;
        pattern->matching[i].right = slot->right;
        pattern->matching[i].cellNode = NULL;
        
        if(treeParsing){
            Cell * base = &cell[slot->left][slot->right + 1];
            for(int j = 0; j < base->cellNodes->count; j++){
                TreeNode * treeNode = (TreeNode *)base->treeNodes->GetItem(j);

                if(IsMatched(treeNode, rule->NT[i].symbolSrc)){
                    pattern->matching[i].cellNode = (Cell *)base->cellNodes->GetItem(j);
                    break;
                }
            }
        }
    }
    pattern->matchingNum = rule->NTCount;
    pattern->ruleList = ruleList;

    return pattern;
}


//////////////////////////////////////////////////////////////
//// check various issues
void Decoder_SCFG::CheckMe()
{
    if(dumpDefeatedViterbiRule)
        CheckViterbiRules();
}


//////////////////////////////////////////////////////////////
//// to load Viterbi rules

void Decoder_SCFG::LoadViterbiRules(DecodingSentence * sentence)
{
    if(sentence->viterbiRules == NULL)
        return;

    List * ruleList = sentence->viterbiRules;
    for(int i = 0; i < ruleList->count; i++){
        SimpleRule * rule = (SimpleRule *)ruleList->GetItem(i);
        int beg = rule->beg, end = rule->end + 1;

        //if(rule->isUnaryRule)
        //    continue;

        if(treeParsing){
            Cell * c = &cell[beg][end];
            for(int j = 0; j < c->cellNodes->count; j++){
                Cell * cellNode = (Cell *)c->cellNodes->GetItem(j);
                TreeNode * treeNode = (TreeNode *)cellNode->treeNodes->GetItem(0);

                if(IsMatched(treeNode, rule->rootSrc)){
                    cellNode->viterbiRules->Add(rule);
                    break;
                }
            }
        }
        else
            cell[beg][end].viterbiRules->Add(rule);
    }
}

void Decoder_SCFG::CheckViterbiRules()
{
    if(treeParsing)
        CheckViterbiRulesForTreeParsing(srcTree->root);
    else{
        // TODO: the same procedure for chart-parsing
    }
}

int tmpCount = 0;

// check for tree node (hyper-node in source-side tree/forest)
bool Decoder_SCFG::CheckViterbiRulesForTreeParsing(TreeNode * rootNode)
{
    bool ok = false;
    List * edges = rootNode->edges;

    if(edges != NULL && edges->count > 0){
        for(int i = 0; i < edges->count; i++){  // for each hyper-edge
            TreeEdge * edge = (TreeEdge *)edges->GetItem(i);
            List * children = edge->children;

            bool done = true;
            for(int j = 0; j < children->count; j++){ // for each child-node
                TreeNode * childNode = (TreeNode *)children->GetItem(j);
                if(!CheckViterbiRulesForTreeParsing(childNode))
                    done = false;
            }

            if(done) // hey, you passed the exam
                ok = true;
        }
    }
    else
        ok = true;

    if(ok){
        Cell * c = &cellNodes[rootNode->id];
        ok = CheckViterbiRulesForTreeParsing(c);
    }

    return ok;
}

// check for a cell (hyper-node in translation forest)
bool Decoder_SCFG::CheckViterbiRulesForTreeParsing(Cell * c)
{
    bool ok = true;
    
    for(int r = 0; r < c->viterbiRules->count; r++){ // for each (Viterbi) rule
        SimpleRule * vRule = (SimpleRule *)c->viterbiRules->GetItem(r);

        if(model->FindRuleListWithSymbol(vRule->src) == NULL){
            vRule->state = -1; // no such a rule contained in the rule set
            continue;
        }

        bool found = false;
        for(int i = 0; i < c->n; i++){
            CellHypo * ch = c->nList[i];  // for each hypothesis
            if(CheckViterbiRuleForHypothesis(vRule, ch)){
                vRule->state = 1; // label the rule as "found"
                found = true;
                break;
            }
        }

        if(!found){  // the (Viterbi) rule is not used
            List * defeatedRulesInChildNodes = NULL;
            for(int i = 0; i < c->n; i++){
                CellHypo * ch = c->nList[i];  // for each hypothesis
                if(CheckViterbiRuleUsage(vRule, ch)){ // if the rule is actually used
                    defeatedRulesInChildNodes = FindDefeatedRulesForChildNodes(ch);
                    break;
                }
            }
            
            CellHypo * bestHypo = c->n > 0 ? c->nList[0] : NULL; // compared with best derivation
            //FindCausesForDefeatedViterbiRule(bestHypo, vRule, defeatedRulesInChildNodes); // figure out why the rule is defeated

            CellHypo * lastHypoInBeam = c->n > 0 ? c->nList[c->n - 1] : NULL; // compared with the last derivation in beam
            FindCausesForDefeatedViterbiRule(lastHypoInBeam, vRule, defeatedRulesInChildNodes); // figure out why the rule is defeated
            if(vRule->state != -2)
                FindTMCauseForDefeatedViterbiRule(bestHypo, vRule, defeatedRulesInChildNodes);  // problem with translation model ?

            ok = false;
        }
    }

    return ok;
}

// check whether a (Viterbi) rule is used in generating a given hypothsis
bool Decoder_SCFG::CheckViterbiRuleForHypothesis(SimpleRule * vRule, CellHypo * ch)
{
    if(ch->ruleUsed == NULL)
        return false;

    UnitRule * uRule = (UnitRule *)ch->ruleUsed;
    SCFGRule * hRule = uRule->parentRule; // the rule used in generating the hypothesis (derivation)

    //if(strcmp(vRule->src, hRule->src))    // source-language side
    //    return false;

    if(strcmp(vRule->root, hRule->root))  // root symbol
        return false;

    //if(strcmp(vRule->tgt, hRule->tgt))    // target-language side
    //    return false;

    if(strcmp(vRule->translation, ch->translation))    // translation
        return false;

    return true;
}

bool Decoder_SCFG::CheckViterbiRuleUsage(SimpleRule * vRule, CellHypo * ch)
{
    if(ch->ruleUsed == NULL)
        return false;

    UnitRule * uRule = (UnitRule *)ch->ruleUsed;
    SCFGRule * hRule = uRule->parentRule; // the rule used in generating the hypothesis (derivation)

    if(strcmp(vRule->src, hRule->src))    // source-language side
        return false;

    if(strcmp(vRule->root, hRule->root))  // root symbol
        return false;

    if(strcmp(vRule->tgt, hRule->tgt))    // target-language side
        return false;

    return true;
}

void Decoder_SCFG::FindCausesForDefeatedViterbiRule(CellHypo * baseHypo, SimpleRule * vRule, List * defeatedRulesInChildNodes)
{
    UnitRule * uRule = (UnitRule *)baseHypo->ruleUsed;
    SCFGRule * hRule = uRule->parentRule;

    if(baseHypo == NULL || baseHypo->modelScore - epsilonViterbiScore < vRule->viterbiScore){ // search error
        if(defeatedRulesInChildNodes == NULL)
            vRule->state = -2;
        else{
            for(int i = 0; i < defeatedRulesInChildNodes->count; i++){
                SimpleRule * rule = (SimpleRule *)defeatedRulesInChildNodes->GetItem(i);
                rule->state = -2;
            }
        }
    }
    else{ // othewise a "better" derivation exists, though we thought that the "Viterbi" one was the best
        float baseHypoLMScore = baseHypo->featValues[Model::NGramLM] * model->featWeight[Model::NGramLM] +
                                baseHypo->featValues[Model::WordCount] * model->featWeight[Model::WordCount];
        float baseHypoNoLMScore = baseHypo->modelScore - baseHypoLMScore;

        if(baseHypoLMScore > vRule->viterbiLMScore && 
          (baseHypoLMScore - vRule->viterbiLMScore > baseHypoNoLMScore - vRule->viterbiNoLMScore))
            vRule->state = -3;  // modeling error. 
                                // (inconsistency between the models (feature sets) used in training and decoding)
        else
            vRule->state = -4;  // modeling error.
                                // (inconsistency between derivation space used in training and decoding)
    }
}

void Decoder_SCFG::FindTMCauseForDefeatedViterbiRule(CellHypo * baseHypo, SimpleRule * vRule, List * defeatedRuleList)
{
    UnitRule * uRule = (UnitRule *)baseHypo->ruleUsed;
    SCFGRule * hRule = uRule->parentRule;

    if(baseHypo == NULL || baseHypo->modelScore - epsilonViterbiScore < vRule->viterbiScore) // search error
        return;

    float baseHypoLMScore = baseHypo->featValues[Model::NGramLM] * model->featWeight[Model::NGramLM] +
                            baseHypo->featValues[Model::WordCount] * model->featWeight[Model::WordCount];
    float baseHypoNoLMScore = baseHypo->modelScore - baseHypoLMScore;

    if(!(baseHypoLMScore > vRule->viterbiLMScore && 
      (baseHypoLMScore - vRule->viterbiLMScore > baseHypoNoLMScore - vRule->viterbiNoLMScore)))
        vRule->state = -4;  // modeling error. 
                            // (inconsistency between the models (feature sets) used in training and decoding)
}

List * Decoder_SCFG::FindDefeatedRulesForChildNodes(CellHypo * ch)
{
    Cell * c = ch->cell;
    UnitRule * uRule = (UnitRule *)ch->ruleUsed;
    SCFGRule * hRule = uRule->parentRule;
    Nonterminal * NTs = hRule->NT;
    MatchedPattern * pattern = (MatchedPattern *)ch->patternUsed;

    if(pattern == NULL)
        return NULL;

    List * defeatedRules = (List *)mem->Alloc(sizeof(List));
    defeatedRules->Create(1, mem);

    for(int i = 0; i < pattern->matchingNum; i++){
        Cell * childCell = pattern->matching[i].cellNode;
        List * viterbiRulesForchildCell = childCell->viterbiRules;
        char * NTSymbol = GetNonterminalSymbol(NTs + i);
        HypoList * hypos = (HypoList *)childCell->nListWithSymbol->GetObject(NTSymbol);

        if(viterbiRulesForchildCell == NULL || viterbiRulesForchildCell->count == 0 || hypos == NULL){
            fprintf(stderr, "ERORR! no viterbi rules are matched!\n");
            continue;
        }

        
        CellHypo ** chs = (CellHypo **)hypos->list;
        int n = hypos->n;
        CellHypo * bestHypo = n > 0 ? chs[0] : NULL;

        for(int j = 0; j < viterbiRulesForchildCell->count; j++){
            SimpleRule * vRule = (SimpleRule *)viterbiRulesForchildCell->GetItem(j);
            
            if(bestHypo != NULL && !strcmp(vRule->translation, bestHypo->translation))
                continue;

            defeatedRules->Add((void *)vRule);
        }
    }

    return defeatedRules;
}


//////////////////////////////////////////////////////////////
//// log

void Decoder_SCFG::DumpLog(DecodingLoger * log)
{
    if(log == NULL)
        return;

    if(log->defeatedViterbiLog != NULL)
        delete[] log->defeatedViterbiLog;
    log->defeatedViterbiLog = new char[1];
    log->defeatedViterbiLog[0] = '\0';

    if(log->usedRuleLog != NULL)
        delete[] log->usedRuleLog;
    log->usedRuleLog = new char[MAX_RULE_SIZE * 100];
    log->usedRuleLog[0] = '\0';

    if(treeParsing){
        // log information of Viterbi rules
        for(int i = 0; i < cellNodeNum; i++){
            Cell * c = &cellNodes[i];
            DumpViterbiLog(c, log);
        }
    }
    else{
    }

    // log information of the rule actually used
    if(cell[0][srcLength].n > 0){
        
        CellHypo * oneBestHypo = cell[0][srcLength].nList[0];
        sprintf(log->usedRuleLog, "model: %.6f, smt: %.6f lbeam: %.6f, lbleu: %.6f\n", 
                                  oneBestHypo->modelScore, GetSMTModelScore(oneBestHypo),
                                  oneBestHypo->beamLoss, oneBestHypo->bleuLoss);
        DumpUsedRuleLog(oneBestHypo, log);
    }
    else
        sprintf(log->usedRuleLog, "model: 0, smt: 0 lbeam: 0, lbleu: 0\n");

    char * tLog = StringUtil::Copy(log->usedRuleLog);
    delete[] log->usedRuleLog;
    log->usedRuleLog = tLog;
}

const char * GetErrorType(short state)
{
    if(state == -2)
        return "search error";
    else if(state == -3)
        return "n-gram error";
    else if(state == -4)
        return "TM error";
    else
        return "Unknown error";
}

void Decoder_SCFG::DumpViterbiLog(Cell * c, DecodingLoger * log)
{
    char ruleLog[MAX_RULE_SIZE] = "";

    for(int i = 0; i < c->viterbiRules->count; i++){
        SimpleRule * rule = (SimpleRule *)c->viterbiRules->GetItem(i);
        if(rule->state < 0){
            sprintf(ruleLog, "fail %d (%s) [%d, %d] ||| %s ||| %s ||| %s\n", 
                rule->state, GetErrorType(rule->state), rule->beg, rule->end, rule->src, rule->tgt, rule->root);
            int len = strlen(ruleLog) + (log->defeatedViterbiLog == NULL ? 0 : strlen(log->defeatedViterbiLog)) + 1;
            char * newLog = new char[len];
            
            if(log->defeatedViterbiLog == NULL)
                strcpy(newLog, ruleLog);
            else
                sprintf(newLog, "%s%s", log->defeatedViterbiLog, ruleLog);

            if(log->defeatedViterbiLog != NULL)
                delete[] log->defeatedViterbiLog;
            log->defeatedViterbiLog = newLog;
        }
    }
}

void Decoder_SCFG::DumpUsedRuleLog(CellHypo * ch, DecodingLoger * log)
{
    char ruleLog[MAX_RULE_SIZE] = "";

    if(ch->ruleUsed != NULL){
        Cell * c = ch->cell;
        UnitRule * urule = (UnitRule *)ch->ruleUsed;
        SCFGRule * rule = urule->parentRule;

        sprintf(ruleLog, "[%d, %d] ||| %s ||| %s ||| %s ||| %s", 
                c->beg, c->end, rule->src, rule->tgt, 
                rule->root, 
                rule->alignment == NULL ? "NULL" : rule->alignment); // rule information

        DumpUsedRuleScore(ch, ruleLog); // model score
        DumpUsedRuleSlots(ch, ruleLog); // slot information
        sprintf(ruleLog, "%s ||| %s", ruleLog, ch->translation); // translation

        strcat(ruleLog, "\n");
        strcat(log->usedRuleLog, ruleLog);
    }

    // access child-hypo recursively
    if(ch->childHypoNum > 0){
        for(int i = 0; i < ch->childHypoNum; i++){
            CellHypo * childHypo = ch->childHypos[i];
            DumpUsedRuleLog(childHypo, log);
        }
    }
}

void Decoder_SCFG::DumpUsedRuleSlots(CellHypo * ch, char * log)
{
    char curSlot[MAX_RULE_SIZE] = "";
    char slotInfo[MAX_RULE_SIZE] = "";

    for(int i = 0; i < ch->childHypoNum; i++){
        CellHypo * curHypo = ch->childHypos[i];
        sprintf(curSlot, "%d %d", curHypo->cell->beg, curHypo->cell->end - 1);
        strcat(slotInfo, curSlot);
        if(i < ch->childHypoNum - 1)
            strcat(slotInfo, " ");
    }

    sprintf(log, "%s ||| %s", log, slotInfo);
}

void Decoder_SCFG::DumpUsedRuleScore(CellHypo * ch, char * log)
{
    float SMTScore = GetSMTModelScore(ch);
    float LMScore = GetNGramLMModelScore(ch);

    sprintf(log, "%s ||| %.6f %.6f %.6f",
                 log, SMTScore, SMTScore - LMScore, LMScore);
}

// translation model score (i.e. model.score  - lm.score)
float Decoder_SCFG::GetTranslationModelScore(CellHypo * ch)
{
    return ch->modelScore - GetNGramLMModelScore(ch);
}

float Decoder_SCFG::GetNGramLMModelScore(CellHypo * ch)
{
    return ch->featValues[Model::NGramLM] * model->featWeight[Model::NGramLM] +
           ch->featValues[Model::WordCount] * model->featWeight[Model::WordCount];
}

float Decoder_SCFG::GetSMTModelScore(CellHypo * ch)
{
    float score = 0;
    for( int f = 0; f < model->featNum; f++ ){
        score += ch->featValues[f] * model->featWeight[f];
    }
    return score;
}

//////////////////////////////////////////////////////////////
//// data structure used in SCFG-based decoder
void SlotInformation::Init(int slotNum, MemPool * tmpMem)
{
    this->slotNum = slotNum;
    this->slotHypoPointer = (CellHypo **)tmpMem->Alloc(sizeof(CellHypo*) * slotNum); // a poniter to the current hypothesis
    memset(this->slotHypoPointer, 0, sizeof(CellHypo*) * slotNum);
    this->slotHypoList = (CellHypo ***)tmpMem->Alloc(sizeof(CellHypo**) * slotNum);  // n-best hypotheses for each slot (variable)
    memset(this->slotHypoList, 0, sizeof(CellHypo**) * slotNum);
    this->hypoNum = (int *)tmpMem->Alloc(sizeof(int) * slotNum);
    memset(this->hypoNum, 0, sizeof(int) * slotNum);
    this->invertedNTIndex = (int *)tmpMem->Alloc(sizeof(int) * slotNum);
    memset(this->invertedNTIndex, 0, sizeof(int) * slotNum);
    pattern = NULL;
};

}

