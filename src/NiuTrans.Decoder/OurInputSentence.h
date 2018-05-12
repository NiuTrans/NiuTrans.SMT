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
 * input for the decoder ; OurInputSentence.h
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email:xiaotong@mail.neu.edu.cn)
 *
 */


#ifndef _OURINPUTSENTENCE_H_
#define _OURINPUTSENTENCE_H_

#include "Global.h"
#include "Model.h"
#include "OurTrainer.h"

namespace smt {

////////////////////////////////////////
// input sentence

typedef struct DecodingSentence
{
    char *      string;       // input string
                              // format: source sentence ||| NE translations (optional) ||| parse tree (optional)
    List *      viterbiRules; // rules that used in viterbi derivations
    List *      matchedRules; // rules that matched onto the input string/tree (currently this feature is available for tree-parsing only)
    RefSents *  refs;         // reference translations (for "forced" decoding and loss-augumented decoding)
    int         id;

}* pDecodingSentence;


//////////////////////////////////////////////////////////////
// record the boundaries of a span

typedef struct Boundary
{
    short   left;
    short   right;
}* pSpan;

////////////////////////////////////////
// a very simple rule structure

typedef struct SimpleRule: public BasicRule
{
    int    beg;   // begining of span
    int    end;   // end of span
    List * slots;
    bool   isUnaryRule;
    float  viterbiScore;
    float  viterbiNoLMScore;
    float  viterbiLMScore;
    short  state; // 0: successfully used, > 0: rule is defeated
    char * translation; 
}* pSimpleRule;

class SimpleRuleBuilder
{
public:
    static SimpleRule * CreateRule(char * root, char * src, char * tgt, char * slots, bool forTreeParsing, MemPool * mem);
    static void ParseSymbol(char * symbol, char * &root, char * &rootSrc, char * &rootTgt, MemPool * mem);
    static char * GetSymbol(const char * symbol, MemPool * mem);
    static void ParseSourceSide(SimpleRule * rule, char * &key, bool generateKey, bool forSourceTreeParsing, MemPool * mem);
    static void PostProcessingSourceSide(char * &key, int &keyLen, MemPool * mem);
    static void LoadSlotInformation(SimpleRule * rule, char * slots, MemPool * mem);
    static bool IsUnaryRule(const char * src);
    static bool IsUnaryRuleForSourceSide(char * srcSymbol, char * RuleSymbol);
    static bool IsUnaryRuleForTargetSide(char * tgtSymbol, char * RuleSymbol);
};

////////////////////////////////////////
// pre-matched rules for each sentence
class MatchedRuleSet
{
protected:
    MemPool * mem;
    Model *     model;

public:
    int    sentCount;
    List * sentRules;

public:
    MatchedRuleSet();
    ~MatchedRuleSet();

public:
    void Load(const char * fn, Model * myModel, int maxSentNum, bool forTreeParsing, bool forViterbiRule);
    List * GetRules(int sentId);

private:
    float GetNGramLMScore(char * str);
};

////////////////////////////////////////
// viterbi rules for each sentence

class ViterbiRuleSet : public MatchedRuleSet
{
public:
    ViterbiRuleSet();
    ~ViterbiRuleSet();
};

}

#endif

