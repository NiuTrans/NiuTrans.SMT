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
 * phrase extraction and scorer; PE_extract_phrase.h
 *
 * $Version:
 * 1.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2012/09/26 add composing-based phrase table extractor
 * 2012/09/24 bug fixes
 *
 */


#ifndef _PHRASETABLE_EXTRACTOR_H_
#define _PHRASETABLE_EXTRACTOR_H_

#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include "word_alignment.h"

using namespace std;
using namespace word_alignment;

namespace phrasetable_extractor
{

    class OptionsOfPhraseTable 
    {
    public:
       string srcFileName;
       string tgtFileName;
       string alnFileName;
       string outFileName;

       int    srclen;
       int    tgtlen;

	   int    maxSrcNulExtNum;
	   int    maxTgtNulExtNum;
       
       bool nullFlag   ;
       bool allSentFlag;

       string method ;      // "heuristic" or "compose", Default value is "heuristic".

	   int compose;        // composing-based phrase table extractor, default is 2.
    };

    class SrcAndTgtPos
    {
    public:
        size_t srcStartPos;
        size_t srcEndPos  ;
        size_t tgtStartPos;
        size_t tgtEndPos  ;

        SrcAndTgtPos(){}

        SrcAndTgtPos( size_t srcStartP, size_t srcEndP, size_t tgtStartP, size_t tgtEndP )
            : srcStartPos( srcStartP ), 
              srcEndPos(   srcEndP   ), 
              tgtStartPos( tgtStartP ), 
              tgtEndPos(   tgtEndP   )  {}

        SrcAndTgtPos( const SrcAndTgtPos &newSrcAndTgtPos )
            : srcStartPos( newSrcAndTgtPos.srcStartPos ), 
              srcEndPos(   newSrcAndTgtPos.srcEndPos   ),
              tgtStartPos( newSrcAndTgtPos.tgtStartPos ), 
              tgtEndPos(   newSrcAndTgtPos.tgtEndPos   )  {}
    };

    class RuleNum
    {
    public:
        size_t initialRuleNum;
        size_t nullRuleNum   ;
        size_t totalRuleNum  ;

        RuleNum()
            : initialRuleNum( 0 ),
              nullRuleNum(    0 ),
              totalRuleNum(   0 ){}
    };

    class PhrasesPair
    {
    public:
        typedef vector< string >::size_type        VS_SIZE_TYPE;


        map< size_t, size_t > targetLangStartAndEndPos;

        PhrasesPair(){}

        bool extractPhrasesPair(
                WordAlignment        &wordAlignment               , 
                ofstream             &outfileExtractPhrasesPair   ,
                ofstream             &outfileExtractPhrasesPairInv,
                OptionsOfPhraseTable &options                     ,
                RuleNum              &ruleNum                       );


    private:
        vector< pair< VS_SIZE_TYPE, VS_SIZE_TYPE > > alignedInfoInv;

        VS_SIZE_TYPE lengthSourceLanguage;
        VS_SIZE_TYPE lengthTargetLanguage;

        bool validatePhrasesPair(    
                WordAlignment              &wordAlignment               , 
                const SrcAndTgtPos         &srcAndTgtPos                , 
                ofstream                   &outfileExtractPhrasesPair   ,
                ofstream                   &outfileExtractPhrasesPairInv,
                const OptionsOfPhraseTable &options                     ,
                RuleNum                    &ruleNum                       );

        bool addPhrasePair(
                WordAlignment              &wordAlignment               , 
                const SrcAndTgtPos         &srcAndTgtPos                , 
                ofstream                   &outfileExtractPhrasesPair   ,
                ofstream                   &outfileExtractPhrasesPairInv,
                RuleNum                    &ruleNum                     ,
				const OptionsOfPhraseTable &options                       );
    };

}

#endif
