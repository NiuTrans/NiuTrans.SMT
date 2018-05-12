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
 * input for the decoder ; OurInputSentence.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email:xiaotong@mail.neu.edu.cn)
 *
 */

#include <stdlib.h>
#include "OurInputSentence.h"

namespace smt {

////////////////////////////////////////
// a very simple rule structure

SimpleRule * SimpleRuleBuilder::CreateRule(char * root, char * src, char * tgt, char * slots, bool forTreeParsing, MemPool * mem)
{
    SimpleRule * rule = (SimpleRule *)mem->Alloc(sizeof(SimpleRule));
    memset(rule, 0, sizeof(SimpleRule));

    ParseSymbol(root, rule->root, rule->rootSrc, rule->rootTgt, mem);

    bool isUnary = IsUnaryRule(src) && IsUnaryRuleForSourceSide(rule->rootSrc, src + 1);
    //isUnary = false;

    rule->isUnaryRule = isUnary;

    if(forTreeParsing && !isUnary){
        int len = (int)(strlen(src) + strlen(root) + 7);
        rule->src = (char*)mem->Alloc(len);
        memset(rule->src, 0, sizeof(char) * len);
        sprintf(rule->src, "#%s ( %s )", root, src); // root (frontier node sequence)
    }
    else{
        rule->src = (char*)mem->Alloc(strlen(src) + 1);
        strcpy(rule->src, src);
    }

    rule->tgt = (char*)mem->Alloc(strlen(tgt) + 1);
    strcpy(rule->tgt, tgt);

    // generate key for indexing
    if(forTreeParsing)
        ParseSourceSide(rule, rule->key, true, !isUnary, mem);  // source-side => key
    else
        ParseSourceSide(rule, rule->key, true, false, mem);     // source-side => key

    if(slots != NULL){
        rule->slots = (List *)mem->Alloc(sizeof(List));
        rule->slots->Create(rule->NTCount, mem);
        LoadSlotInformation(rule, slots, mem);
    }

    return rule;
}

void SimpleRuleBuilder::ParseSymbol(char * symbol, char * &root, char * &rootSrc, char * &rootTgt, MemPool * mem)
{
    root    = NULL;
    rootSrc = NULL;
    rootTgt = NULL;

    if(symbol == NULL)
        return;

    int slen = (int)strlen(symbol);
    if(slen <= 0)
        return; // default symbol #

    root = GetSymbol(symbol, mem);
    char * separator = strchr(symbol, SYMBOL_SEPARATOR);

    if(separator != NULL){
        *separator = '\0';
        rootTgt = GetSymbol(symbol, mem);
        rootSrc = GetSymbol(separator + 1, mem);
        *separator = SYMBOL_SEPARATOR;
    }
    else{
        rootSrc = root;
        rootTgt = root;
    }
}

char * SimpleRuleBuilder::GetSymbol(const char * symbol, MemPool * mem)
{
    char * s = (char*)mem->Alloc(sizeof(char) * ((int)strlen(symbol) + 1));
    strcpy(s, symbol);
    return s;
}

void SimpleRuleBuilder::ParseSourceSide(SimpleRule * rule, char * &key, bool generateKey, bool forSourceTreeParsing, MemPool * mem)
{
    char * src = rule->src;
    int    len = (int)strlen(src);
    char * beg = strchr(src, NT_SYMBOL);
    char * end = beg + 1;
    char * last = src;
    int    keyLen = 0;
    bool   needPostProcesing = false;

    key = generateKey ? (char*)mem->Alloc(sizeof(char) * len + 1) : NULL;

    Nonterminal * tmpNTs = new Nonterminal[len];
    int   tmpNTCount = 0;

    while(beg != NULL){
        end = beg + 1;

        while(*end != ' ' && *end != '\0')
            end++;

        if(end != beg + 1){
            char tmpc = *end;
            *end = '\0';
            ParseSymbol(beg + 1, tmpNTs[tmpNTCount].symbol, tmpNTs[tmpNTCount].symbolSrc, tmpNTs[tmpNTCount].symbolTgt, mem);
            *end = tmpc;
            tmpNTCount++;

            if( generateKey ){
                strncpy(key + keyLen, last, beg - last); // terminals
                keyLen += (int)(beg - last);

                key[keyLen++] = NT_SYMBOL; // non-terminals

                if(forSourceTreeParsing){
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
    if(generateKey && beg - last > 0){
        strncpy(key + keyLen, last, beg - last); // terminals
        keyLen += (int)(beg - last);
    }

    key[keyLen] = '\0';
    
    if(needPostProcesing)
        PostProcessingSourceSide(key, keyLen, mem);

    rule->NTCount = tmpNTCount;
    if(tmpNTCount == 0)
        rule->NTCount = 0;
    else if(forSourceTreeParsing){
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

void SimpleRuleBuilder::PostProcessingSourceSide(char * &key, int &keyLen, MemPool * mem)
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

void SimpleRuleBuilder::LoadSlotInformation(SimpleRule * rule, char * slots, MemPool * mem)
{
    if(strlen(slots) < 3)
        return;

    int pos = 0, count = 0;
    char * head = slots;

    while(head != NULL){
        
        Boundary * slot = (Boundary *)mem->Alloc(sizeof(Boundary));

        sscanf(head, "%d", &pos);
        slot->left = pos;

        head = strchr(head + 1, ' ');
        sscanf(head, "%d", &pos);
        slot->right = pos;

        rule->slots->Add((void *)slot);

        count++;
        head = strchr(head + 1, ' ');
    }
}

bool SimpleRuleBuilder::IsUnaryRule(const char * src)
{
    if(strlen(src) < 2)
        return false;

    if(src[0] != NT_SYMBOL || strchr(src, ' ') != NULL)
        return false;

    return true;
}

bool SimpleRuleBuilder::IsUnaryRuleForSourceSide(char * srcSymbol, char * ruleSymbol)
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

bool SimpleRuleBuilder::IsUnaryRuleForTargetSide(char * tgtSymbol, char * ruleSymbol)
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

////////////////////////////////////////
// pre-matched rules for each sentence

MatchedRuleSet::MatchedRuleSet()
{
    model = NULL;
    mem = new MemPool(M_BYTE * 4, 1024 * 8);
    sentRules = new List(10);
}

MatchedRuleSet::~MatchedRuleSet()
{
    delete mem;
    delete sentRules;
}

void MatchedRuleSet::Load(const char * fn, Model * myModel, int maxSentNum, bool forTreeParsing, bool forViterbiRule)
{
    char * line = new char[MAX_LINE_LENGTH];
    int    sentCount = 0, sentId = 0;

    model = myModel;

    fprintf( stderr, "Loading %s\n", "pre-matched rules" );
    fprintf( stderr, "  >> From File: %s ...\n", fn );
    fprintf( stderr, "  >> " );

    FILE * file = fopen(fn, "r");

    if(file == NULL){
        fprintf( stderr, "cannot open file \"%s\"!\n", fn);
        delete[] line;
        return;
    }

    while(fgets(line, MAX_LINE_LENGTH, file )){
        sscanf(line, "sentence: %d", &sentId);
        if(sentId != ++sentCount)
            break;
            //sentId = sentCount;

        fgets(line, MAX_LINE_LENGTH, file ); // skip a line

        List * ruleList = (List *)mem->Alloc(sizeof(List));
        ruleList->Create(10, mem);

        while(1){
            fgets(line, MAX_LINE_LENGTH, file );
            StringUtil::TrimRight(line);
            if(!strncmp(line, "======", 6))  // the "while" breaks when "======" appears
                break;

            char ** terms;
            int termCount = (int)StringUtil::Split(line, " ||| ", terms);

            SimpleRule * rule = SimpleRuleBuilder::CreateRule(terms[1], terms[2], terms[3], terms[4], forTreeParsing, mem);

            if(forViterbiRule){
                if(termCount > 5)
                    rule->viterbiScore = atof(terms[5]);
                if(termCount > 6)
                    rule->viterbiNoLMScore = atof(terms[6]);
                if(termCount > 7)
                    rule->viterbiLMScore = atof(terms[7]);
                if(termCount > 11){
                    rule->translation = new char[(int)strlen(terms[11]) + 1];
                    strcpy(rule->translation, terms[11]);
                    rule->viterbiLMScore = GetNGramLMScore(rule->translation);
                    rule->viterbiScore = rule->viterbiLMScore + rule->viterbiNoLMScore;
                }
                else{
                    rule->translation = new char[1];
                    rule->translation[0] = '\0';
                }
            }

            sscanf(terms[0], "[%d, %d]", &rule->beg, &rule->end);
            ruleList->Add((void *)rule);

            for( int i = 0; i < termCount; i++ )
                delete[] terms[i];
            delete[] terms;
        }

        sentRules->Add((void *)ruleList);

        if(sentCount >= maxSentNum)
            break;
    }

    fclose(file);
    delete[] line;
    fprintf( stderr, "\nDone [%d sentence(s)]\n", sentCount );
}

List * MatchedRuleSet::GetRules(int sentId)
{
    return (List *)sentRules->GetItem(sentId);
}

float MatchedRuleSet::GetNGramLMScore(char * str)
{
    float LMScore = 0, overallScore = 0;
    int wCount = 0;
    int * wId = NULL;
    int ngram = model->ngram;

    model->GetWId(str, wId, wCount, NULL);

    for( int i = 0; i < wCount; i++){
        int end = i + 1;
        int beg = end - ngram > 0 ? end - ngram : 0;
        float prob = model->GetNGramLMProb(beg, end, wId);
        LMScore += prob;
    }

    delete[] wId;

    overallScore = LMScore * model->featWeight[Model::NGramLM] + wCount * model->featWeight[Model::WordCount];

    return overallScore;
}

////////////////////////////////////////
// viterbi rules for each sentence

ViterbiRuleSet::ViterbiRuleSet() : MatchedRuleSet()
{
}

ViterbiRuleSet::~ViterbiRuleSet()
{
}

}

