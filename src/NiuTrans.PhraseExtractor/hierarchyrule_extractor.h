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
 * phrase extraction and scorer; PE_extract_phrase_hiero.h
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2011/11/5
 *
 */


#ifndef _HIERARCHYRULE_EXTRACTOR_H_
#define _HIERARCHYRULE_EXTRACTOR_H_

#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <map>
#include <algorithm>
#include "basic_method.h"
#include "word_alignment.h"
#include "n_ary_tree.h"

using namespace std;
using namespace word_alignment;
using namespace n_ary_tree;
using namespace basic_method;

namespace hierarchyrule_extractor
{

    // add in 11/07/2011
    class OptionsOfHiero
    {
    public:
        int maxSourcePhraseSize  ;
        int maxTargetPhraseSize  ;
        int maxSourceInitSize    ;
        int maxTargetInitSize    ;
        int maxSourceHieroSize   ;
        int maxTargetHieroSize   ;
        int minSourceSubPhrase   ;
        int minTargetSubPhrase   ;
        int maxNonTermNum        ;
        int minLexiNumOfHieroRule;
        int maxNulExtSrcInitNum  ;
        int maxNulExtTgtInitNum  ;
        int maxNulExtSrcHieroNum ;
        int maxNulExtTgtHieroNum ;

        bool headNonTermFlag        ;
        bool nullFuncFlag           ;
        bool srcNonTermAdjacentFlag ;
        bool tgtNonTermAdjacentFlag ;
        bool alignedLexiRequiredFlag;
        bool duplicateHieroRuleFlag ;
        bool unalignedEdgeHieroFlag ;
        bool unalignedEdgeInitFlag  ;

        //add in 2012/03/29
        bool srcParseTreeFlag;
        bool tgtParseTreeFlag;

        string srcParseTreePath;
        string tgtParseTreePath;

        Tree tree;
        //add in 2012/03/29

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
            : srcStartPos( srcStartP ), srcEndPos( srcEndP ), 
              tgtStartPos( tgtStartP ), tgtEndPos( tgtEndP ){}

        SrcAndTgtPos( const SrcAndTgtPos &newSrcAndTgtPos )
            :srcStartPos( newSrcAndTgtPos.srcStartPos ), srcEndPos( newSrcAndTgtPos.srcEndPos ),
             tgtStartPos( newSrcAndTgtPos.tgtStartPos ), tgtEndPos( newSrcAndTgtPos.tgtEndPos ){}
    };

    class HieroRuleNum
    {
    public:
        size_t initialRuleNum;
        size_t hieroRuleNum  ;
        size_t nullRuleNum   ;
        size_t totalRuleNum  ;

        HieroRuleNum()
            : initialRuleNum( 0 ),
              hieroRuleNum(   0 ),
              nullRuleNum(    0 ),
              totalRuleNum(   0 ){}
    };


    // add in 11/06/2011
    class ExtractedHieroRule
    {
    public:
        string srcHieroRule   ;     // source part Hiero Rule
        string tgtHieroRule   ;     // target part Hiero Rule
        string alnHieroRule   ;     // Hiero Rule's alignment
        string alnHieroRuleInv;     // Hiero Rule's inv alignment
        string rootLabel      ;     // root label

        SrcAndTgtPos srcAndTgtPos;    

        float count;

        ExtractedHieroRule()
            : srcHieroRule(), tgtHieroRule(   ),
              alnHieroRule(), alnHieroRuleInv(),
              srcAndTgtPos(), rootLabel( "X"  ),
              count( 1 ){}

        ExtractedHieroRule( const SrcAndTgtPos &newSrcAndTgtPos, const string &rootLable )
            : srcHieroRule(), tgtHieroRule(   ), 
              alnHieroRule(), alnHieroRuleInv(), 
              srcAndTgtPos( newSrcAndTgtPos   ), 
              rootLabel( rootLable            ),
              count( 1 ){}


        ExtractedHieroRule( const SrcAndTgtPos &newSrcAndTgtPos )
            : srcHieroRule(), tgtHieroRule(   ), 
              alnHieroRule(), alnHieroRuleInv(), 
              srcAndTgtPos( newSrcAndTgtPos   ), 
              rootLabel( "X"                  ),
              count( 1 ){}

    };

    // add in 11/06/2011
    class SubPhrase
    {
    public:
        SrcAndTgtPos srcAndTgtPos;

        string srcLabel;
        string tgtLabel;

        size_t srcPos;
        size_t tgtPos;

        SubPhrase(){}
        SubPhrase( const SubPhrase &copy )
            : srcAndTgtPos( copy.srcAndTgtPos ), srcLabel( copy.srcLabel ), tgtLabel( copy.tgtLabel ){}
        SubPhrase( const SrcAndTgtPos &newSrcAndTgtPos )
            : srcAndTgtPos( newSrcAndTgtPos ){}
        SubPhrase( const SrcAndTgtPos &newSrcAndTgtPos, const string &newSrcLabel, const string &newTgtLabel )
            : srcAndTgtPos( newSrcAndTgtPos ), srcLabel( newSrcLabel ), tgtLabel( newTgtLabel )    {}

        bool Overlap( const SubPhrase &newSubPhrase, bool &directFlag ) const
        {
            if( directFlag )
            {
                return !( newSubPhrase.srcAndTgtPos.srcEndPos < srcAndTgtPos.srcStartPos || newSubPhrase.srcAndTgtPos.srcStartPos > srcAndTgtPos.srcEndPos );
            }
            else
            {
                return !( newSubPhrase.srcAndTgtPos.tgtEndPos < srcAndTgtPos.tgtStartPos || newSubPhrase.srcAndTgtPos.tgtStartPos > srcAndTgtPos.tgtEndPos );
            }
        }

        bool Adjacent( const SubPhrase &newSubPhrase, bool &directFlag ) const
        {
            if( directFlag )
            {
                return ( ( newSubPhrase.srcAndTgtPos.srcEndPos + 1 ) == srcAndTgtPos.srcStartPos || ( newSubPhrase.srcAndTgtPos.srcStartPos == srcAndTgtPos.srcEndPos + 1 ) );
            }
            else
            {
                return ( ( newSubPhrase.srcAndTgtPos.tgtEndPos + 1 ) == srcAndTgtPos.tgtStartPos || ( newSubPhrase.srcAndTgtPos.tgtStartPos == srcAndTgtPos.tgtEndPos + 1 ) );
            }
        }
    };

    // add in 11/06/2011
    class SubPhraseExist
    {
    public:
        typedef list< SubPhrase > SubPhraseList;
        vector< vector< SubPhraseList > > subPhraseExist;

        SubPhraseExist( size_t &sizeOfTgtLang )
            : subPhraseExist( sizeOfTgtLang )
        {
            for( size_t pos = 0; pos < sizeOfTgtLang; ++pos )
                subPhraseExist[ pos ].resize( sizeOfTgtLang - pos );
        }

        void AddSubPhrase( const SrcAndTgtPos &srcAndTgtPos )
        {
            subPhraseExist[ srcAndTgtPos.tgtStartPos ][ srcAndTgtPos.tgtEndPos - srcAndTgtPos.tgtStartPos ].push_back( SubPhrase( srcAndTgtPos ) );
        }

        const SubPhraseList &GetSourceSubPhraseList( size_t &tgtStartPos, size_t &tgtEndPos ) const
        {
            const SubPhraseList &sourceSubPhraseList = subPhraseExist[ tgtStartPos ][ tgtEndPos - tgtStartPos ];
            return sourceSubPhraseList;
        }
    };


    class SrcSubPhraseOrder
    {
    public:
        bool operator()( const SubPhrase *subPhraseA, const SubPhrase *subPhraseB ) const
        {
            return subPhraseA->srcAndTgtPos.srcStartPos < subPhraseB->srcAndTgtPos.srcStartPos;
        }
    };


    class SubPhraseCollection
    {
    public:
        list< SubPhrase > subPhraseList;
        vector< SubPhrase* > srcSortedSubPhrases;

        SubPhraseCollection(){}
        SubPhraseCollection( const SubPhraseCollection &copy )
            : subPhraseList( copy.subPhraseList ){}

        void AddSubPhrase( SubPhrase &newSubPhrase )
        {
            subPhraseList.push_back( newSubPhrase );
        }

        void AddSubPhrase( size_t &srcStartPos, size_t &srcEndPos, size_t &tgtStartPos, size_t &tgtEndPos, string &srcLabel, string &tgtLabel )
        {
            SubPhrase newSubPhrase(  SrcAndTgtPos( srcStartPos, srcEndPos, tgtStartPos, tgtEndPos ), srcLabel, tgtLabel );
            subPhraseList.push_back( newSubPhrase );
        }

        bool OverlapSource( const SubPhrase &sourceSubPhrase ) const
        {
            bool directFlag = true;
            for( list< SubPhrase >::const_iterator iter = subPhraseList.begin(); iter != subPhraseList.end(); ++iter )
            {
                const SubPhrase &currSubPhrase = *iter;
                if( currSubPhrase.Overlap( sourceSubPhrase, directFlag ) )
                    return true;
            }
            return false;
        }

        bool AdjacentSource( const SubPhrase &sourceSubPhrase ) const
        {
            bool directFlag = true;
            for( list< SubPhrase >::const_iterator iter = subPhraseList.begin(); iter != subPhraseList.end(); ++iter )
            {
                if( iter->Adjacent( sourceSubPhrase, directFlag ) )
                    return true;
            }
            return false;
        }

        void SortSrcSubPhrase()
        {
            for( list< SubPhrase >::iterator iter = subPhraseList.begin(); iter != subPhraseList.end(); ++iter )
            {
                SubPhrase &currSubPhrase = *iter;
                srcSortedSubPhrases.push_back( &currSubPhrase );
            }
            
            sort( srcSortedSubPhrases.begin(), srcSortedSubPhrases.end(), SrcSubPhraseOrder() );
        }
    };

    class PhrasesPairHiero: public BasicMethod
    {
    public:
        typedef vector< string >::size_type        VS_SIZE_TYPE;

        PhrasesPairHiero(){}
        bool extractPhrasesPair(
                WordAlignment  &wordAlignment                , 
                ofstream       &outfileExtractedHieroRules   ,
                ofstream       &outfileExtractedHieroRulesInv,
                HieroRuleNum   &hieroRuleNum                 ,    
                OptionsOfHiero &options                        );

        bool addInitialRule( 
                WordAlignment      &wordAlignment , 
                const SrcAndTgtPos &srcAndTgtPos  ,
                size_t             &initialRuleNum,
                OptionsOfHiero     &options         );

        bool addHieroRule(
                WordAlignment             &wordAlignment      ,
                const SrcAndTgtPos        &srcAndTgtPos       ,
                SubPhraseExist            &subPhraseExist     ,
                const SubPhraseCollection &subPhraseCollection,
                const VS_SIZE_TYPE        numOfSubPhrase      ,
                const VS_SIZE_TYPE        &tgtNonTermStartPos ,
                const VS_SIZE_TYPE        &srcLexiCount       ,
                const VS_SIZE_TYPE        &tgtLexiCount       ,
                size_t                    &hieroRuleNum       ,
                OptionsOfHiero            &options              );

        void writeHieroRule( 
                WordAlignment       &wordAlignment      , 
                const SrcAndTgtPos  &srcAndTgtPos       , 
                SubPhraseCollection &subPhraseCollection, 
                size_t              &hieroRuleNum       ,
                OptionsOfHiero      &options              );

        void setSrcAndTgtNonTermLabel( SubPhraseCollection &subPhraseCollection );

        string writeTgtHieroRule( 
                WordAlignment         &wordAlignment      ,
                const SrcAndTgtPos    &srcAndTgtPos       ,
                map< size_t, size_t > &tgtLabelIndex      ,
                SubPhraseCollection   &subPhraseCollection  );

        string writeSrcHieroRule(
                WordAlignment         &wordAlignment      ,
                const SrcAndTgtPos    &srcAndTgtPos       ,
                map< size_t, size_t > &srcLabelIndex      ,
                SubPhraseCollection   &subPhraseCollection,
                OptionsOfHiero        &options              );

        void writeAlnHieroRule(
                WordAlignment               &wordAlignment      ,
                const SrcAndTgtPos          &srcAndTgtPos       ,
                const map< size_t, size_t > &srcLabelIndex      ,
                const map< size_t, size_t > &tgtLabelIndex      ,
                SubPhraseCollection         &subPhraseCollection,
                ExtractedHieroRule          &extractedHieroRule   );

        void outputHieroRules( 
                ofstream &outfileExtractedHieroRules   ,
                ofstream &outfileExtractedHieroRulesInv  );

        // Hiero Rule contains Initial Rules and normal Hiero Rule 
        bool addRuleToHieroVec( 
                ExtractedHieroRule &extractedHieroRule,
                size_t             &ruleNum           ,
                OptionsOfHiero     &options             );

    private:
        vector< pair< VS_SIZE_TYPE, VS_SIZE_TYPE > > alignedInfoInv;
        VS_SIZE_TYPE sizeOfSrcLang;
        VS_SIZE_TYPE sizeOfTgtLang;

        multimap< string, ExtractedHieroRule > extractedHieroRules;

        bool validatePhrasesPair(    
                WordAlignment      &wordAlignment               , 
                const SrcAndTgtPos &srcAndTgtPos                ,
                ofstream           &outfileExtractPhrasesPair   ,
                ofstream           &outfileExtractPhrasesPairInv,
                SubPhraseExist     &subPhraseExist              ,
                HieroRuleNum       &hieroRuleNum                ,
                OptionsOfHiero     &options                       );
        
        bool addPhrasePair(
                WordAlignment      &wordAlignment               , 
                const SrcAndTgtPos &srcAndTgtPos                ,
                ofstream           &outfileExtractPhrasesPair   ,
                ofstream           &outfileExtractPhrasesPairInv  );
    };

}

#endif
