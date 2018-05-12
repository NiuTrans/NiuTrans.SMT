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
 * phrase extraction and scorer; PE_extract_phrases.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2011/7/14 add "source language words -> null" and "dest language words -> null" by Qiang Li 
 * 
 *
 */


#include "hierarchyrule_extractor.h"
using namespace hierarchyrule_extractor;

bool PhrasesPairHiero::extractPhrasesPair(
WordAlignment  &wordAlignment               , 
ofstream       &outfileExtractPhrasesPair   ,
ofstream       &outfileExtractPhrasesPairInv,
HieroRuleNum   &hieroRuleNum                ,
OptionsOfHiero &options                       )
{
    sizeOfTgtLang = wordAlignment.destLangAndAlignedInfo.destLanguage.size();
    sizeOfSrcLang = wordAlignment.sourceLangAndAlignedInfo.sourceLanguage.size();

    SubPhraseExist subPhraseExist( sizeOfTgtLang );

    // 11/06/2011 the same with moses : extract target phrase with length becoming longer 
    for( VS_SIZE_TYPE tgtLength = 1; tgtLength <= size_t( options.maxTargetPhraseSize ) && tgtLength <= sizeOfTgtLang; ++tgtLength )
    {
        for( VS_SIZE_TYPE destStart = 0; destStart < sizeOfTgtLang - ( tgtLength - 1 ); ++destStart )
        {
            VS_SIZE_TYPE destEnd = destStart + tgtLength - 1;
/*
            if( !options.unalignedEdgeFlag )
            {
                if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( destStart ).size() == 0 )
                    continue;
                if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( destEnd ).size() == 0 )
                    continue;
            }
*/
            VS_SIZE_TYPE sourceStart = sizeOfSrcLang;
            VS_SIZE_TYPE sourceEnd = 0;
            for( VS_SIZE_TYPE i = destStart; i <= destEnd; ++i )
            {
                for( vector< size_t >::iterator alignedToDestLanguage_iter = wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage[ i ].begin();
                     alignedToDestLanguage_iter != wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage[ i ].end(); ++alignedToDestLanguage_iter )
                {
                    sourceStart = ( *alignedToDestLanguage_iter < sourceStart ? *alignedToDestLanguage_iter : sourceStart );          // find sourceStart 
                    sourceEnd = ( *alignedToDestLanguage_iter > sourceEnd ? *alignedToDestLanguage_iter : sourceEnd );                // find sourceEnd
                }
            }
            SrcAndTgtPos srcAndTgtPos( sourceStart, sourceEnd, destStart, destEnd );
            validatePhrasesPair( wordAlignment, srcAndTgtPos, outfileExtractPhrasesPair, outfileExtractPhrasesPairInv, subPhraseExist, hieroRuleNum,options );
        }
    }

    outputHieroRules( outfileExtractPhrasesPair, outfileExtractPhrasesPairInv );

    // add source->null and dest->null
    if( options.nullFuncFlag )
    {
        string rootLabel;
        string rootLabelInv;

        for( vector< vector< size_t > >::size_type pos = 0; pos != wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.size(); ++pos )
        {
            if( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( pos ).size() == 0 )
            {
                if( options.srcParseTreeFlag && options.tgtParseTreeFlag )
                {
                    rootLabel = wordAlignment.srcParseTree.validateSpanOfParseTree( pos, pos );
                    rootLabelInv = "NULL=" + rootLabel;
                    rootLabel += "=NULL";
                }
                else if( options.srcParseTreeFlag )
                {
                    rootLabel = wordAlignment.srcParseTree.validateSpanOfParseTree( pos, pos );
                    rootLabelInv = rootLabel;
                }
                else if( options.tgtParseTreeFlag )
                {
                    rootLabel = "NULL";
                    rootLabelInv = rootLabel;
                }
                else
                {
                    rootLabel = "X";
                    rootLabelInv = rootLabel;
                }

                ++hieroRuleNum.nullRuleNum;
                outfileExtractPhrasesPair   <<wordAlignment.sourceLangAndAlignedInfo.sourceLanguage.at( pos )
                                            <<" ||| NULL ||| 0-0 ||| "
                                            <<rootLabel
                                            <<"\n";
                outfileExtractPhrasesPairInv<<"NULL ||| "
                                            <<wordAlignment.sourceLangAndAlignedInfo.sourceLanguage.at( pos )
                                            <<" ||| 0-0 ||| "
                                            <<rootLabelInv
                                            <<"\n";
            }
        }
        for( vector< vector< size_t > >::size_type pos = 0; pos != wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.size(); ++pos )
        {
            if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( pos ).size() == 0 )
            {
                if( options.srcParseTreeFlag && options.tgtParseTreeFlag )
                {
                    rootLabel = wordAlignment.tgtParseTree.validateSpanOfParseTree( pos, pos );
                    if( rootLabel == "," || rootLabel == "." || rootLabel == ":" )
                        rootLabel = "PU";
                    rootLabelInv = rootLabel + "=NULL";
                    rootLabel = "NULL=" + rootLabel;
                }
                else if( options.tgtParseTreeFlag )
                {
                    rootLabel = wordAlignment.tgtParseTree.validateSpanOfParseTree( pos, pos );
                    if( rootLabel == "," || rootLabel == "." || rootLabel == ":" )
                        rootLabel = "PU";
                    rootLabelInv = rootLabel;
                }
                else if( options.srcParseTreeFlag )
                {
                    rootLabel = "NULL";
                    rootLabelInv = rootLabel;
                }
                else
                {
                    rootLabel = "X";
                    rootLabelInv = rootLabel;
                }
                
                ++hieroRuleNum.nullRuleNum;
                outfileExtractPhrasesPair   <<"NULL ||| "
                                            <<wordAlignment.destLangAndAlignedInfo.destLanguage.at( pos )
                                            <<" ||| 0-0 ||| "
                                            <<rootLabel
                                            <<"\n";
                outfileExtractPhrasesPairInv<<wordAlignment.destLangAndAlignedInfo.destLanguage.at( pos )
                                            <<" ||| NULL ||| 0-0 ||| "
                                            <<rootLabelInv
                                            <<"\n";
            }
        }
    }

    return true;
}

bool PhrasesPairHiero::validatePhrasesPair(    
WordAlignment      &wordAlignment               , 
const SrcAndTgtPos &srcAndTgtPos                ,
ofstream           &outfileExtractPhrasesPair   ,
ofstream           &outfileExtractPhrasesPairInv,
SubPhraseExist     &subPhraseExist              ,
HieroRuleNum       &hieroRuleNum                ,
OptionsOfHiero     &options                       )
{
    if( srcAndTgtPos.srcEndPos < srcAndTgtPos.srcStartPos )
        return false;

    if( srcAndTgtPos.srcEndPos - srcAndTgtPos.srcStartPos >= size_t( options.maxSourcePhraseSize ) )
        return false;
    
    // validate legal
    for( size_t i = srcAndTgtPos.srcStartPos; i <= srcAndTgtPos.srcEndPos; ++i )
    {
        for( vector< size_t >::iterator alignToSourceLang_iter = wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( i ).begin(); 
             alignToSourceLang_iter != wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( i ).end(); ++ alignToSourceLang_iter )
        {
            if( *alignToSourceLang_iter < srcAndTgtPos.tgtStartPos || *alignToSourceLang_iter > srcAndTgtPos.tgtEndPos )
            {
                return false;
            }
        }
    }

    for( VS_SIZE_TYPE sourceS = srcAndTgtPos.srcStartPos; ( ( sourceS == srcAndTgtPos.srcStartPos ) ||        // expand the phrases pairs being consistent table
       ( wordAlignment.sourceLangAndAlignedInfo.alignedCountSourceLang.at( sourceS ) == 0 ) ) 
         && sourceS >= 0; --sourceS )                                                                         // head direction
    {
        for( VS_SIZE_TYPE sourceE = srcAndTgtPos.srcEndPos; ( sourceE < sizeOfSrcLang ) && ( ( sourceE == srcAndTgtPos.srcEndPos ) || 
           ( wordAlignment.sourceLangAndAlignedInfo.alignedCountSourceLang.at( sourceE ) == 0 ) ) 
             && ( ( sourceE - sourceS ) < size_t( options.maxSourcePhraseSize ) )  ; ++ sourceE )             // tail direction
        {

            if( !options.unalignedEdgeInitFlag )
            {
                if( ( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtStartPos ).size() == 0 ) ||
                    ( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtEndPos ).size() == 0 ) ||
                    ( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( sourceS ).size() == 0 ) ||
                    ( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( sourceE ).size() == 0 ) )
                    continue;
            }
            else
            {
                // target phrase head part validate the null aligned 
                if( srcAndTgtPos.tgtStartPos + options.maxNulExtTgtInitNum < srcAndTgtPos.tgtEndPos )
                {
                    bool tgtHeadNullFlag = true;
                    for( int i = 0; i < options.maxNulExtTgtInitNum; ++i )
                    {
                        if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtStartPos + i ).size() != 0 )
                        {
                            tgtHeadNullFlag = false;
                            break;
                        }
                    }
                     if( tgtHeadNullFlag == true )
                    {
                        if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtStartPos + options.maxNulExtTgtInitNum ).size() == 0 )
                            continue;
                    }
                }

                // target phrase tail part validate the null aligned
                if( ( int( int ( srcAndTgtPos.tgtEndPos ) - options.maxNulExtTgtInitNum ) ) > int( srcAndTgtPos.tgtStartPos ) )
                {
                    bool tgtTailNullFlag = true;
                    for( int i = 0; i < options.maxNulExtTgtInitNum; ++i )
                    {
                        if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtEndPos - i ).size() != 0 )
                        {
                            tgtTailNullFlag = false;
                            break;
                        }
                    }
                    if( tgtTailNullFlag == true )
                    {
                        if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtEndPos - options.maxNulExtTgtInitNum ).size() == 0 )
                            continue;
                    }
                }

                // source phrase head part validate the null aligned
                if( srcAndTgtPos.srcStartPos + options.maxNulExtSrcInitNum < srcAndTgtPos.srcEndPos )
                {
                    bool srcHeadNullFlag = true;
                    for( int i = 0; i < options.maxNulExtSrcInitNum; ++i )
                    {
                        if( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( srcAndTgtPos.srcStartPos + i ).size() != 0 )
                        {
                            srcHeadNullFlag = false;
                            break;
                        }
                    }
                    if( srcHeadNullFlag == true )
                    {
                        if( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( srcAndTgtPos.srcStartPos + options.maxNulExtSrcInitNum ).size() == 0 )
                            continue;
                    }
                }

                // source phrase tail part validate the null aligned
                if( ( int( int( srcAndTgtPos.srcEndPos ) - options.maxNulExtSrcInitNum) ) > int( srcAndTgtPos.srcStartPos ) )
                {
                    bool srcTailNullFlag = true;
                    for( int i = 0; i < options.maxNulExtSrcInitNum; ++i )
                    {
                        if( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( srcAndTgtPos.srcEndPos - i ).size() != 0 )
                        {
                            srcTailNullFlag = false;
                            break;
                        }
                    }
                    if( srcTailNullFlag == true )
                    {
                        if( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( srcAndTgtPos.srcEndPos - options.maxNulExtSrcInitNum ).size() == 0 )
                            continue;
                    }
                }
            }
            

//          if( ( sourceE - sourceS < size_t( options.maxSourceHieroSize ) ) && ( srcAndTgtPos.tgtEndPos - srcAndTgtPos.tgtStartPos < size_t( options.maxTargetHieroSize ) ) )
            if( ( sourceE - sourceS < size_t( options.maxSourceInitSize ) ) && ( srcAndTgtPos.tgtEndPos - srcAndTgtPos.tgtStartPos < size_t( options.maxTargetInitSize ) ) )
            {
                //generate initial hiero rules : just fully-lexical phrase pairs.
                addInitialRule( wordAlignment, SrcAndTgtPos( sourceS, sourceE, srcAndTgtPos.tgtStartPos, srcAndTgtPos.tgtEndPos ), hieroRuleNum.initialRuleNum, options );
            }

            if( !options.unalignedEdgeHieroFlag )
            {
                if( ( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtStartPos ).size() == 0 ) ||
                    ( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtEndPos ).size() == 0 ) ||
                    ( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( sourceS ).size() == 0 ) ||
                    ( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( sourceE ).size() == 0 ) )
                    continue;
            }
            else
            {
                // target phrase head part validate the null aligned 
                if( srcAndTgtPos.tgtStartPos + options.maxNulExtTgtHieroNum < srcAndTgtPos.tgtEndPos )
                {
                    bool tgtHeadNullFlag = true;
                    for( int i = 0; i < options.maxNulExtTgtHieroNum; ++i )
                    {
                        if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtStartPos + i ).size() != 0 )
                        {
                            tgtHeadNullFlag = false;
                            break;
                        }
                    }
                     if( tgtHeadNullFlag == true )
                    {
                        if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtStartPos + options.maxNulExtTgtHieroNum ).size() == 0 )
                            continue;
                    }
                }

                // target phrase tail part validate the null aligned
                if( ( int( int ( srcAndTgtPos.tgtEndPos ) - options.maxNulExtTgtHieroNum ) ) > int( srcAndTgtPos.tgtStartPos ) )
                {
                    bool tgtTailNullFlag = true;
                    for( int i = 0; i < options.maxNulExtTgtHieroNum; ++i )
                    {
                        if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtEndPos - i ).size() != 0 )
                        {
                            tgtTailNullFlag = false;
                            break;
                        }
                    }
                    if( tgtTailNullFlag == true )
                    {
                        if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( srcAndTgtPos.tgtEndPos - options.maxNulExtTgtHieroNum ).size() == 0 )
                            continue;
                    }
                }

                // source phrase head part validate the null aligned
                if( srcAndTgtPos.srcStartPos + options.maxNulExtSrcHieroNum < srcAndTgtPos.srcEndPos )
                {
                    bool srcHeadNullFlag = true;
                    for( int i = 0; i < options.maxNulExtSrcHieroNum; ++i )
                    {
                        if( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( srcAndTgtPos.srcStartPos + i ).size() != 0 )
                        {
                            srcHeadNullFlag = false;
                            break;
                        }
                    }
                    if( srcHeadNullFlag == true )
                    {
                        if( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( srcAndTgtPos.srcStartPos + options.maxNulExtSrcHieroNum ).size() == 0 )
                            continue;
                    }
                }

                // source phrase tail part validate the null aligned
                if( ( int( int( srcAndTgtPos.srcEndPos ) - options.maxNulExtSrcHieroNum) ) > int( srcAndTgtPos.srcStartPos ) )
                {
                    bool srcTailNullFlag = true;
                    for( int i = 0; i < options.maxNulExtSrcHieroNum; ++i )
                    {
                        if( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( srcAndTgtPos.srcEndPos - i ).size() != 0 )
                        {
                            srcTailNullFlag = false;
                            break;
                        }
                    }
                    if( srcTailNullFlag == true )
                    {
                        if( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( srcAndTgtPos.srcEndPos - options.maxNulExtSrcHieroNum ).size() == 0 )
                            continue;
                    }
                }
            }

            subPhraseExist.AddSubPhrase( SrcAndTgtPos( sourceS, sourceE, srcAndTgtPos.tgtStartPos, srcAndTgtPos.tgtEndPos ) );

            size_t tgtNonTermStartPos = ( ( options.headNonTermFlag == true ) ? srcAndTgtPos.tgtStartPos : srcAndTgtPos.tgtStartPos + 1 ); 

            // aligned lexical words count
            VS_SIZE_TYPE srcLexiCount = sourceE - sourceS + 1;
            VS_SIZE_TYPE tgtLexiCount = srcAndTgtPos.tgtEndPos - srcAndTgtPos.tgtStartPos + 1;

            SubPhraseCollection subPhraseCollection;

            SrcAndTgtPos newSrcAndTgtPos( sourceS, sourceE, srcAndTgtPos.tgtStartPos, srcAndTgtPos.tgtEndPos );

            addHieroRule( wordAlignment, newSrcAndTgtPos, subPhraseExist, subPhraseCollection, 0, tgtNonTermStartPos, srcLexiCount, tgtLexiCount, hieroRuleNum.hieroRuleNum, options );
        }
        if( sourceS == 0 )
            break;
    }
//  }
    return true;
}


bool PhrasesPairHiero::addInitialRule(
WordAlignment      &wordAlignment ,
const SrcAndTgtPos &srcAndTgtPos  ,
size_t             &initialRuleNum,
OptionsOfHiero     &options         )
{
    ExtractedHieroRule extractedInitRule( srcAndTgtPos );

    // source phrase
    extractedInitRule.srcHieroRule = "";
    for( size_t i = srcAndTgtPos.srcStartPos; i <= srcAndTgtPos.srcEndPos; ++i )
    {
        if( options.srcParseTreeFlag && options.tgtParseTreeFlag )
        {
            string rootLabelofSrc, rootLabelofTgt;        
            size_t beginPos = srcAndTgtPos.srcStartPos;
            size_t endPos = srcAndTgtPos.srcEndPos;
        
            rootLabelofSrc = wordAlignment.srcParseTree.validateSpanOfParseTree( beginPos, endPos );

            beginPos = srcAndTgtPos.tgtStartPos;
            endPos = srcAndTgtPos.tgtEndPos;
            rootLabelofTgt = wordAlignment.tgtParseTree.validateSpanOfParseTree( beginPos, endPos );
            if( rootLabelofTgt == "," || rootLabelofTgt == "." || rootLabelofTgt == ":" )
                rootLabelofTgt = "PU";
            extractedInitRule.rootLabel = rootLabelofSrc + "=" + rootLabelofTgt;
        
        }
        else if( options.tgtParseTreeFlag )
        {
            size_t beginPos = srcAndTgtPos.tgtStartPos;
            size_t endPos = srcAndTgtPos.tgtEndPos;
            extractedInitRule.rootLabel = wordAlignment.tgtParseTree.validateSpanOfParseTree( beginPos, endPos );
            if( extractedInitRule.rootLabel == "," || extractedInitRule.rootLabel == "." || extractedInitRule.rootLabel == ":" )
                extractedInitRule.rootLabel = "PU";
        }
        else if( options.srcParseTreeFlag )
        {
            size_t beginPos = srcAndTgtPos.srcStartPos;
            size_t endPos = srcAndTgtPos.srcEndPos;
            extractedInitRule.rootLabel = wordAlignment.srcParseTree.validateSpanOfParseTree( beginPos, endPos );
        }

        extractedInitRule.srcHieroRule += ( wordAlignment.sourceLangAndAlignedInfo.sourceLanguage.at( i ) + " " );
    }
    extractedInitRule.srcHieroRule = extractedInitRule.srcHieroRule.substr( 0, ( extractedInitRule.srcHieroRule.length() - 1 ) );

    // target phrase and alignment
    extractedInitRule.tgtHieroRule = "";
    for( size_t i = srcAndTgtPos.tgtStartPos; i <= srcAndTgtPos.tgtEndPos; ++i )
    {
        for( vector< size_t >::iterator iter = wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( i ).begin();
            iter != wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( i ).end(); ++iter )
        {
            size_t srcCurPos = *iter - srcAndTgtPos.srcStartPos;
            size_t tgtCurPos = i - srcAndTgtPos.tgtStartPos;
            extractedInitRule.alnHieroRule += size_tToString( srcCurPos ) + "-" + size_tToString( tgtCurPos ) + " ";
            extractedInitRule.alnHieroRuleInv += size_tToString( tgtCurPos ) + "-" + size_tToString( srcCurPos ) + " ";
        }
        extractedInitRule.tgtHieroRule += ( wordAlignment.destLangAndAlignedInfo.destLanguage.at( i ) + " " );
    }
    extractedInitRule.tgtHieroRule = extractedInitRule.tgtHieroRule.substr( 0, ( extractedInitRule.tgtHieroRule.length() - 1 ) );
    extractedInitRule.alnHieroRule = extractedInitRule.alnHieroRule.substr( 0, ( extractedInitRule.alnHieroRule.length() - 1 ) );
    extractedInitRule.alnHieroRuleInv = extractedInitRule.alnHieroRuleInv.substr( 0, ( extractedInitRule.alnHieroRuleInv.length() - 1 ) );

    addRuleToHieroVec( extractedInitRule, initialRuleNum, options );
    return true;
}

bool PhrasesPairHiero::addRuleToHieroVec( ExtractedHieroRule &extractedHieroRule, size_t &ruleNum, OptionsOfHiero &options )
{
    if( !options.duplicateHieroRuleFlag )
    {    
        multimap< string, ExtractedHieroRule >::iterator iterUpper = extractedHieroRules.upper_bound( extractedHieroRule.srcHieroRule );
        multimap< string, ExtractedHieroRule >::iterator iterLower = extractedHieroRules.lower_bound( extractedHieroRule.srcHieroRule );
        for( multimap< string, ExtractedHieroRule >::iterator iter = iterLower; iter != iterUpper; ++iter )
        {
            if( iter->second.tgtHieroRule == extractedHieroRule.tgtHieroRule &&
                iter->second.alnHieroRule == extractedHieroRule.alnHieroRule )
                return false;

            if( iter->second.tgtHieroRule == extractedHieroRule.tgtHieroRule &&
                !( iter->second.srcAndTgtPos.tgtEndPos < extractedHieroRule.srcAndTgtPos.tgtStartPos || 
                iter->second.srcAndTgtPos.tgtStartPos > extractedHieroRule.srcAndTgtPos.tgtEndPos ) )
                return false;
        }
    }
    
    ++ruleNum;
    extractedHieroRules.insert( make_pair( extractedHieroRule.srcHieroRule, extractedHieroRule ) );
    return true;
}

string PhrasesPairHiero::writeTgtHieroRule(
WordAlignment         &wordAlignment      ,
const SrcAndTgtPos    &srcAndTgtPos       ,
map< size_t, size_t > &tgtLabelIndex      ,
SubPhraseCollection   &subPhraseCollection  )
{
    list< SubPhrase >::iterator iterSubPhraseList = subPhraseCollection.subPhraseList.begin();
    string tgtHieroRule( "" );

    int outPos = 0;
    for( size_t currPos = srcAndTgtPos.tgtStartPos; currPos <= srcAndTgtPos.tgtEndPos; ++currPos )
    {
        bool isSubPhrase = false;
        if( iterSubPhraseList != subPhraseCollection.subPhraseList.end() )
        {
            const SubPhrase &subPhrase = *iterSubPhraseList;
            isSubPhrase = subPhrase.srcAndTgtPos.tgtStartPos == currPos;
        }

        if( isSubPhrase )
        {
            SubPhrase &subPhrase = *iterSubPhraseList;

            tgtHieroRule += ( " " + subPhrase.tgtLabel );

            currPos = subPhrase.srcAndTgtPos.tgtEndPos;
            subPhrase.tgtPos = outPos;
            ++iterSubPhraseList;
        }
        else
        {
            tgtLabelIndex[ currPos ] = outPos;
            tgtHieroRule += " " + wordAlignment.destLangAndAlignedInfo.destLanguage[ currPos ];
        }

        ++outPos;
    }

    return tgtHieroRule.substr( 1 );
}

string PhrasesPairHiero::writeSrcHieroRule(
WordAlignment         &wordAlignment      , 
const SrcAndTgtPos    &srcAndTgtPos       , 
map< size_t, size_t > &srcLabelIndex      ,
SubPhraseCollection   &subPhraseCollection,
OptionsOfHiero        &options              ) 
{
    vector< SubPhrase* >::iterator iterSrcSortedSubPhrases = subPhraseCollection.srcSortedSubPhrases.begin();

    string srcHieroRule( "" );
    int outPos = 0;
    for( size_t currPos = srcAndTgtPos.srcStartPos; currPos <= srcAndTgtPos.srcEndPos; ++currPos )
    {
        bool isSubPhrase = false;
        if( iterSrcSortedSubPhrases != subPhraseCollection.srcSortedSubPhrases.end() )
        {
            const SubPhrase &subPhrase = **iterSrcSortedSubPhrases;
            isSubPhrase = subPhrase.srcAndTgtPos.srcStartPos == currPos;
        }
        if( isSubPhrase )
        {
            SubPhrase &subPhrase = **iterSrcSortedSubPhrases;
            
            if( options.srcParseTreeFlag && options.tgtParseTreeFlag )
            {
                string srcLabel, tgtLabel;

                size_t beginPos = subPhrase.srcAndTgtPos.srcStartPos;
                size_t endPos = subPhrase.srcAndTgtPos.srcEndPos;

                srcLabel = wordAlignment.srcParseTree.validateSpanOfParseTree( beginPos, endPos );
                srcLabel = "#" + srcLabel;

                beginPos = subPhrase.srcAndTgtPos.tgtStartPos;
                endPos = subPhrase.srcAndTgtPos.tgtEndPos;

                tgtLabel = wordAlignment.tgtParseTree.validateSpanOfParseTree( beginPos, endPos );
                if( tgtLabel == "," || tgtLabel == "." || tgtLabel == ":" )
                    tgtLabel = "PU";
                tgtLabel = "#" + tgtLabel;

                subPhrase.srcLabel = srcLabel + "=" + tgtLabel;
            }
            else if( options.tgtParseTreeFlag )
            {
                size_t beginPos = subPhrase.srcAndTgtPos.tgtStartPos;
                size_t endPos = subPhrase.srcAndTgtPos.tgtEndPos;

                subPhrase.srcLabel = wordAlignment.tgtParseTree.validateSpanOfParseTree( beginPos, endPos );
                if( subPhrase.srcLabel == "," || subPhrase.srcLabel == "." || subPhrase.srcLabel == ":" )
                    subPhrase.srcLabel = "PU";
                subPhrase.srcLabel = "#" + subPhrase.srcLabel;
            }
            else if( options.srcParseTreeFlag )
            {
                size_t beginPos = subPhrase.srcAndTgtPos.srcStartPos;
                size_t endPos = subPhrase.srcAndTgtPos.srcEndPos;

                subPhrase.srcLabel = wordAlignment.srcParseTree.validateSpanOfParseTree( beginPos, endPos );
                subPhrase.srcLabel = "#" + subPhrase.srcLabel;
            }

            srcHieroRule += ( " " + subPhrase.srcLabel );

            currPos = subPhrase.srcAndTgtPos.srcEndPos;
            subPhrase.srcPos = outPos;
            ++iterSrcSortedSubPhrases;
        }
        else
        {
            srcLabelIndex[ currPos ] = outPos;
            srcHieroRule += " " + wordAlignment.sourceLangAndAlignedInfo.sourceLanguage[ currPos ];
        }
        ++outPos;
    }
    return srcHieroRule.substr( 1 ); 
}

void PhrasesPairHiero::writeAlnHieroRule(
WordAlignment               &wordAlignment      , 
const SrcAndTgtPos          &srcAndTgtPos       , 
const map< size_t, size_t > &srcLabelIndex      , 
const map< size_t, size_t > &tgtLabelIndex      , 
SubPhraseCollection         &subPhraseCollection, 
ExtractedHieroRule          &extractedHieroRule   )
{
    for( size_t iTgt = srcAndTgtPos.tgtStartPos; iTgt <= srcAndTgtPos.tgtEndPos; ++iTgt )
    {
        if( tgtLabelIndex.find( iTgt ) != tgtLabelIndex.end() )
        {
            for( size_t j = 0; j < wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage[ iTgt ].size(); ++j )
            {
                size_t iSrc = wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage[ iTgt ][ j ];
                size_t tempSrcPos = srcLabelIndex.find( iSrc )->second;
                size_t tempTgtPos = tgtLabelIndex.find( iTgt )->second;
                extractedHieroRule.alnHieroRule += " " + size_tToString( tempSrcPos ) + "-" + size_tToString( tempTgtPos );
                extractedHieroRule.alnHieroRuleInv += " " + size_tToString( tempTgtPos ) + "-" + size_tToString( tempSrcPos );
            }
        }
    }

    list< SubPhrase >::const_iterator iterSubPhraseList;
    for( iterSubPhraseList = subPhraseCollection.subPhraseList.begin(); iterSubPhraseList != subPhraseCollection.subPhraseList.end(); ++iterSubPhraseList )
    {
        size_t tempSrcPos = iterSubPhraseList->srcPos;
        size_t tempTgtPos = iterSubPhraseList->tgtPos;
        extractedHieroRule.alnHieroRule += " " + size_tToString( tempSrcPos ) + "-" + size_tToString( tempTgtPos );
        extractedHieroRule.alnHieroRuleInv += " " + size_tToString( tempTgtPos ) + "-" + size_tToString( tempSrcPos );
    }
    extractedHieroRule.alnHieroRule = extractedHieroRule.alnHieroRule.substr( 1 );
    extractedHieroRule.alnHieroRuleInv = extractedHieroRule.alnHieroRuleInv.substr( 1 );
}

void PhrasesPairHiero::setSrcAndTgtNonTermLabel( SubPhraseCollection &subPhraseCollection )
{
    subPhraseCollection.SortSrcSubPhrase();
    size_t srcSubPhrasePos = 0;
    for( vector< SubPhrase* >::iterator iter = subPhraseCollection.srcSortedSubPhrases.begin(); iter != subPhraseCollection.srcSortedSubPhrases.end(); ++iter )
    {
        ++srcSubPhrasePos;
        SubPhrase &subPhrase = **iter;
        subPhrase.srcLabel = "#X";
        subPhrase.tgtLabel = "#" + size_tToString( srcSubPhrasePos );    
    }
}

void PhrasesPairHiero::writeHieroRule( 
WordAlignment       &wordAlignment      , 
const SrcAndTgtPos  &srcAndTgtPos       , 
SubPhraseCollection &subPhraseCollection, 
size_t              &hieroRuleNum       ,
OptionsOfHiero      &options              )
{
    setSrcAndTgtNonTermLabel( subPhraseCollection );

    map< size_t, size_t > srcLabelIndex, tgtLabelIndex;

    ExtractedHieroRule extractedHieroRule( srcAndTgtPos );

    if( options.srcParseTreeFlag && options.tgtParseTreeFlag )
    {
        string rootLabelofSrc, rootLabelofTgt;

        size_t beginPos = srcAndTgtPos.srcStartPos;
        size_t endPos = srcAndTgtPos.srcEndPos;
        rootLabelofSrc = wordAlignment.srcParseTree.validateSpanOfParseTree( beginPos, endPos );
    
        beginPos = srcAndTgtPos.tgtStartPos;
        endPos = srcAndTgtPos.tgtEndPos;
        rootLabelofTgt = wordAlignment.tgtParseTree.validateSpanOfParseTree( beginPos, endPos );
        if( rootLabelofTgt == "," || rootLabelofTgt == "." || rootLabelofTgt == ":" )
            rootLabelofTgt = "PU";

        extractedHieroRule.rootLabel = rootLabelofSrc + "=" + rootLabelofTgt;
    }
    else if( options.tgtParseTreeFlag )
    {
        size_t beginPos = srcAndTgtPos.tgtStartPos;
        size_t endPos = srcAndTgtPos.tgtEndPos;
        extractedHieroRule.rootLabel = wordAlignment.tgtParseTree.validateSpanOfParseTree( beginPos, endPos );
        if( extractedHieroRule.rootLabel == "," || extractedHieroRule.rootLabel == "." || extractedHieroRule.rootLabel == ":" )
            extractedHieroRule.rootLabel = "PU";
    }
    else if( options.srcParseTreeFlag )
    {
        size_t beginPos = srcAndTgtPos.srcStartPos;
        size_t endPos = srcAndTgtPos.srcEndPos;
        extractedHieroRule.rootLabel = wordAlignment.srcParseTree.validateSpanOfParseTree( beginPos, endPos );
    }

    extractedHieroRule.tgtHieroRule = writeTgtHieroRule( wordAlignment, srcAndTgtPos, tgtLabelIndex, subPhraseCollection );

    extractedHieroRule.srcHieroRule = writeSrcHieroRule( wordAlignment, srcAndTgtPos, srcLabelIndex, subPhraseCollection, options );    
    
    writeAlnHieroRule( wordAlignment, srcAndTgtPos, srcLabelIndex, tgtLabelIndex, subPhraseCollection, extractedHieroRule );

    addRuleToHieroVec( extractedHieroRule, hieroRuleNum, options );
}

bool PhrasesPairHiero::addHieroRule(
WordAlignment             &wordAlignment      ,
const SrcAndTgtPos        &srcAndTgtPos       ,
SubPhraseExist            &subPhraseExist     ,
const SubPhraseCollection &subPhraseCollection,
const VS_SIZE_TYPE         numOfSubPhrase     ,
const VS_SIZE_TYPE        &tgtNonTermStartPos ,
const VS_SIZE_TYPE        &srcLexiCount       ,
const VS_SIZE_TYPE        &tgtLexiCount       ,
size_t                    &hieroRuleNum       ,
OptionsOfHiero            &options              )
{
    
    // maxinmum number of nonterminal in hiero rule
    if( numOfSubPhrase >= size_t( options.maxNonTermNum ) )
        return true;
    
    // find a sub-phrase
    for( VS_SIZE_TYPE tgtNonTermStartP = tgtNonTermStartPos; tgtNonTermStartP <= srcAndTgtPos.tgtEndPos; ++tgtNonTermStartP )
    {
        for( VS_SIZE_TYPE tgtNonTermEndP = tgtNonTermStartP + ( options.minTargetSubPhrase - 1 ); tgtNonTermEndP <= srcAndTgtPos.tgtEndPos; ++tgtNonTermEndP )
        {
            // !!!!!!!!!!!!!!!!!! problem
//          int a = int(tgtLexiCount) - ( int(tgtNonTermEndP) - int(tgtStartPos) + 1 ) + ( int(numOfSubPhrase) + 1 );
//          cerr<<a<<endl;
            // allow the last subPhrase to add into Hiero Rule
            if( ( numOfSubPhrase == options.maxNonTermNum - 1 ) && ( int(tgtLexiCount) - ( int(tgtNonTermEndP) - int(srcAndTgtPos.tgtStartPos) + 1 ) + ( int(numOfSubPhrase) + 1 ) > options.maxTargetHieroSize ) )
                continue;

            const int newTgtLexiCount = int( tgtLexiCount ) - ( int( tgtNonTermEndP ) - int( tgtNonTermStartP ) + 1 );

            if( newTgtLexiCount < options.minLexiNumOfHieroRule )
                continue;

            // not allow the whole span to be the sub phrase
            if( tgtNonTermStartP == srcAndTgtPos.tgtStartPos && tgtNonTermEndP == srcAndTgtPos.tgtEndPos )
                continue;
            
            const list< SubPhrase > &sourceSubPhraseList = subPhraseExist.GetSourceSubPhraseList( tgtNonTermStartP, tgtNonTermEndP );

            for( list< SubPhrase >::const_iterator iterSubPhrase = sourceSubPhraseList.begin(); iterSubPhrase != sourceSubPhraseList.end(); ++iterSubPhrase )
            {
                const SubPhrase &sourceSubPhrase = *iterSubPhrase;
                const int sourceSubPhraseSize = int( sourceSubPhrase.srcAndTgtPos.srcEndPos ) - int( sourceSubPhrase.srcAndTgtPos.srcStartPos ) + 1;

                if( sourceSubPhraseSize < options.minSourceSubPhrase )
                    continue;

                const int newSrcLexiCount = int( srcLexiCount ) - sourceSubPhraseSize;

                if( ( numOfSubPhrase == options.maxNonTermNum - 1 ) && ( newSrcLexiCount + ( int( numOfSubPhrase ) + 1 ) > options.maxSourceHieroSize ) )
                    continue;

                if( newSrcLexiCount < options.minLexiNumOfHieroRule )
                    continue;

                if( srcAndTgtPos.srcStartPos > sourceSubPhrase.srcAndTgtPos.srcStartPos || srcAndTgtPos.srcEndPos < sourceSubPhrase.srcAndTgtPos.srcEndPos )
                    continue;

                if( subPhraseCollection.OverlapSource( sourceSubPhrase ) )
                    continue;

                // if allow adjacent non-term in source hiero rule
                if( !options.srcNonTermAdjacentFlag && subPhraseCollection.AdjacentSource( sourceSubPhrase ) )
                    continue;

                // why not !!!!!!!( newSrcLexiCount > 0 && newTgtLexiCount > 0 )
                if( options.alignedLexiRequiredFlag && ( newSrcLexiCount > 0 || newTgtLexiCount > 0 ) )
                {
                    list< SubPhrase >::const_iterator iterSubPhraseCollection = subPhraseCollection.subPhraseList.begin();
                    bool foundAlignedWordFlag( false );
                    for( VS_SIZE_TYPE pos = srcAndTgtPos.tgtStartPos; pos <= srcAndTgtPos.tgtEndPos && !foundAlignedWordFlag; pos ++ )
                    {
                        if( pos == tgtNonTermStartP )
                        {
                            pos = tgtNonTermEndP;
                        }
                        else if( iterSubPhraseCollection != subPhraseCollection.subPhraseList.end() && iterSubPhraseCollection->srcAndTgtPos.tgtStartPos == pos )
                        {
                            pos = iterSubPhraseCollection->srcAndTgtPos.tgtEndPos;
                        }
                        else
                        {
                            if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage[ pos ].size() > 0 )
                            {
                                foundAlignedWordFlag = true;
                            }    
                        }
                    }
                    if( !foundAlignedWordFlag )
                        continue;
                }

                SubPhraseCollection copySubPhraseCollection( subPhraseCollection );
                //string srcLabel( "#X" );
                string srcLabel( "" );
                string tgtLabel( "" );

                SrcAndTgtPos newSrcAndTgtPos( sourceSubPhrase.srcAndTgtPos.srcStartPos, sourceSubPhrase.srcAndTgtPos.srcEndPos, tgtNonTermStartP, tgtNonTermEndP );
                SubPhrase newSubPhrase( newSrcAndTgtPos, srcLabel, tgtLabel );
//              if( options.srcParseTreeFlag )
//              {
//                  newSubPhrase.srcLabel = wordAlignment.srcParseTree.validateSpanOfParseTree( newSrcAndTgtPos.srcStartPos, newSrcAndTgtPos.tgtEndPos );
//              }                
                copySubPhraseCollection.AddSubPhrase( newSubPhrase );

                bool newHieroRuleFlag = true;

                if( newSrcLexiCount + ( int( numOfSubPhrase ) + 1 ) > options.maxSourceHieroSize )
                    newHieroRuleFlag = false;

                if( newTgtLexiCount + ( int( numOfSubPhrase ) + 1 ) > options.maxTargetHieroSize )
                    newHieroRuleFlag = false;

                if( newHieroRuleFlag )
                {
                    writeHieroRule( wordAlignment, srcAndTgtPos, copySubPhraseCollection, hieroRuleNum, options );
                }
                VS_SIZE_TYPE nextTgtNonTermStartPos = options.tgtNonTermAdjacentFlag ? tgtNonTermEndP + 1 : tgtNonTermEndP + 2;

                addHieroRule( wordAlignment, srcAndTgtPos, subPhraseExist, copySubPhraseCollection, numOfSubPhrase + 1, nextTgtNonTermStartPos, newSrcLexiCount, newTgtLexiCount, hieroRuleNum, options );
            }

        }
    }
    return true;
}

void PhrasesPairHiero::outputHieroRules(
ofstream &outfileExtractedHieroRules   ,
ofstream &outfileExtractedHieroRulesInv  )
{
    for( multimap< string, ExtractedHieroRule >::iterator iter = extractedHieroRules.begin(); iter != extractedHieroRules.end(); ++iter )
    {
        outfileExtractedHieroRules   <<iter->second.srcHieroRule
                                     <<" ||| "
                                     <<iter->second.tgtHieroRule
                                     <<" ||| "
                                     <<iter->second.alnHieroRule
                                     <<" ||| "
                                     <<iter->second.rootLabel
                                     <<"\n";
        outfileExtractedHieroRulesInv<<iter->second.tgtHieroRule
                                     <<" ||| "
                                     <<iter->second.srcHieroRule
                                     <<" ||| "
                                     <<iter->second.alnHieroRuleInv
                                     <<" ||| "
                                     <<iter->second.rootLabel
                                     <<"\n";
    }
}

bool PhrasesPairHiero::addPhrasePair(        
WordAlignment      &wordAlignment               , 
const SrcAndTgtPos &srcAndTgtPos                ,
ofstream           &outfileExtractPhrasesPair   ,
ofstream           &outfileExtractPhrasesPairInv  )
{
    if( !alignedInfoInv.empty() )
        alignedInfoInv.clear();
    
    string tempDestLang;
    string tempSourceLang;

    for( VS_SIZE_TYPE i = srcAndTgtPos.tgtStartPos; i <= srcAndTgtPos.tgtEndPos; ++i )
    {
        for( vector< size_t >::iterator alignedToDestLang_iter = wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( i ).begin(); 
            alignedToDestLang_iter != wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( i ).end(); ++ alignedToDestLang_iter )
            alignedInfoInv.push_back( make_pair( i- srcAndTgtPos.tgtStartPos, *alignedToDestLang_iter - srcAndTgtPos.srcStartPos ) );   
        {
            tempDestLang += ( wordAlignment.destLangAndAlignedInfo.destLanguage.at( i ) + " " );
        }
    }

    for( VS_SIZE_TYPE i = srcAndTgtPos.srcStartPos; i <= srcAndTgtPos.srcEndPos; ++i )
    {
        tempSourceLang += ( wordAlignment.sourceLangAndAlignedInfo.sourceLanguage.at( i ) + " " );
    }

    outfileExtractPhrasesPair   <<tempSourceLang
                                <<"||| "
                                <<tempDestLang
                                <<"|||";
    outfileExtractPhrasesPairInv<<tempDestLang
                                <<"||| "
                                <<tempSourceLang
                                <<"|||";

    for( vector< pair< VS_SIZE_TYPE, VS_SIZE_TYPE > >::iterator iter = alignedInfoInv.begin(); iter != alignedInfoInv.end(); ++iter )
    {
        outfileExtractPhrasesPair   <<" "
                                    <<iter->second
                                    <<"-"
                                    <<iter->first;
        outfileExtractPhrasesPairInv<<" "
                                    <<iter->first
                                    <<"-"
                                    <<iter->second;            
    }

    outfileExtractPhrasesPair   <<"\n";
    outfileExtractPhrasesPairInv<<"\n";

    return true;
}
