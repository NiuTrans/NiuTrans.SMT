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


#include "phrasetable_extractor.h"
using namespace phrasetable_extractor;


//map< size_t, size_t > targetLangStartAndEndPos;  // for composing-based extract


bool PhrasesPair::extractPhrasesPair(
WordAlignment        &wordAlignment               , 
ofstream             &outfileExtractPhrasesPair   ,
ofstream             &outfileExtractPhrasesPairInv,
OptionsOfPhraseTable &options                     , 
RuleNum              &ruleNum                       )
{
    lengthTargetLanguage = wordAlignment.destLangAndAlignedInfo.destLanguage.size();
    lengthSourceLanguage = wordAlignment.sourceLangAndAlignedInfo.sourceLanguage.size();

    // for composing-based extract    
    if( options.method == "compose" )
	{
		if( !targetLangStartAndEndPos.empty() )
		{
			targetLangStartAndEndPos.clear();
		}
	}
	// for composing-based extract


    for( VS_SIZE_TYPE tgtLength = 1; tgtLength <= size_t( options.tgtlen ) && tgtLength <= lengthTargetLanguage; ++tgtLength )
	{
        for( VS_SIZE_TYPE targetStart = 0; targetStart < lengthTargetLanguage - ( tgtLength - 1 ); ++targetStart )
		{
            VS_SIZE_TYPE targetEnd = targetStart + tgtLength - 1;

            if( targetStart + options.maxTgtNulExtNum < targetEnd )
			{
				bool tgtNulFlag = true;
				for( int i = 0; i < options.maxTgtNulExtNum; ++i )
				{
					if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( targetStart + i ).size() != 0 )
					{
					    tgtNulFlag = false;
						break;
					}
				}

				if( tgtNulFlag == true )
				{
				    if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( targetStart + options.maxTgtNulExtNum ).size() == 0 )
					{
					    continue;
					}
				}
			}

			if( ( int( int( targetEnd ) - options.maxTgtNulExtNum ) ) > int( targetStart )  )
			{
			    bool tgtNulFlag = true;
				for( int i = 0; i < options.maxTgtNulExtNum; ++i )
				{
                    if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( targetEnd - i ).size() != 0 )
					{
						tgtNulFlag = false;
						break;
					}
				}

				if( tgtNulFlag == true )
				{
					if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( targetEnd - options.maxTgtNulExtNum ).size() == 0 )
					{
					    continue;
					}
				}
			}

            VS_SIZE_TYPE sourceStart = lengthSourceLanguage;
            VS_SIZE_TYPE sourceEnd   = 0;

            for( VS_SIZE_TYPE i = targetStart; i <= targetEnd; ++i )
            {
                for( vector< size_t >::iterator alignedToDestLanguage_iter = wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage[ i ].begin();
                     alignedToDestLanguage_iter != wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage[ i ].end(); ++alignedToDestLanguage_iter )
                {
                    sourceStart = ( *alignedToDestLanguage_iter < sourceStart ? *alignedToDestLanguage_iter : sourceStart );          // find sourceStart 
                    sourceEnd   = ( *alignedToDestLanguage_iter > sourceEnd   ? *alignedToDestLanguage_iter : sourceEnd   );          // find sourceEnd
                }
            }
            SrcAndTgtPos srcAndTgtPos( sourceStart, sourceEnd, targetStart, targetEnd );


            validatePhrasesPair( wordAlignment, srcAndTgtPos, outfileExtractPhrasesPair, outfileExtractPhrasesPairInv, options, ruleNum );
        }
    }

    // add source->null and dest->null
    if( options.nullFlag == true )
    {
        for( vector< vector< size_t > >::size_type pos = 0; pos != wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.size(); ++pos )
        {
            if( wordAlignment.sourceLangAndAlignedInfo.alignedToSourceLanguage.at( pos ).size() == 0 )
            {
                ++ruleNum.nullRuleNum;
                outfileExtractPhrasesPair   <<wordAlignment.sourceLangAndAlignedInfo.sourceLanguage.at( pos )
                                            <<" ||| NULL ||| 0-0\n";

                outfileExtractPhrasesPairInv<<"NULL ||| "
                                            <<wordAlignment.sourceLangAndAlignedInfo.sourceLanguage.at( pos )
                                            <<" ||| 0-0\n";
            }
        }
        for( vector< vector< size_t > >::size_type pos = 0; pos != wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.size(); ++pos )
        {
            if( wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( pos ).size() == 0 )
            {
                ++ruleNum.nullRuleNum;
                outfileExtractPhrasesPair   <<"NULL ||| "
                                            <<wordAlignment.destLangAndAlignedInfo.destLanguage.at( pos )
                                            <<" ||| 0-0\n";

                outfileExtractPhrasesPairInv<<wordAlignment.destLangAndAlignedInfo.destLanguage.at( pos )
                                            <<" ||| NULL ||| 0-0\n";
            }
        }
    }
    return true;
}

bool PhrasesPair::validatePhrasesPair(    
WordAlignment              &wordAlignment               , 
const SrcAndTgtPos         &srcAndTgtPos                ,
ofstream                   &outfileExtractPhrasesPair   ,
ofstream                   &outfileExtractPhrasesPairInv,
const OptionsOfPhraseTable &options                     ,
RuleNum                    &ruleNum                       )
{
    if( srcAndTgtPos.srcEndPos < srcAndTgtPos.srcStartPos )
        return false;

    if( srcAndTgtPos.srcEndPos - srcAndTgtPos.srcStartPos >= size_t( options.srclen ) )
        return false;

    for( VS_SIZE_TYPE i = srcAndTgtPos.srcStartPos; i <= srcAndTgtPos.srcEndPos; i++ )                                        // validate legal
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

    for( VS_SIZE_TYPE sourceS = srcAndTgtPos.srcStartPos; 
		( ( sourceS == srcAndTgtPos.srcStartPos ) || ( wordAlignment.sourceLangAndAlignedInfo.alignedCountSourceLang.at( sourceS ) == 0 ) ) &&            
        ( sourceS >= 0                                                                                                                    ) && 
		( ( srcAndTgtPos.srcStartPos - sourceS ) <= size_t( options.maxSrcNulExtNum )                                                     ) ; 
	    -- sourceS )      
	     // expand the phrases pairs being consistent table 
		 // head direction
    {
        for( VS_SIZE_TYPE sourceE = srcAndTgtPos.srcEndPos; 
            ( sourceE < lengthSourceLanguage                                                                                                ) && 
            ( ( sourceE == srcAndTgtPos.srcEndPos ) || ( wordAlignment.sourceLangAndAlignedInfo.alignedCountSourceLang.at( sourceE ) == 0 ) ) && 
            ( ( sourceE - sourceS ) < size_t( options.srclen )                                                                              ) && 
			( ( sourceE - srcAndTgtPos.srcEndPos ) <= size_t( options.maxSrcNulExtNum )                                                     ) ; 
            ++ sourceE )                                             // tail direction
        {
            SrcAndTgtPos srcAndTgtPosNew( sourceS, sourceE, srcAndTgtPos.tgtStartPos, srcAndTgtPos.tgtEndPos );
            addPhrasePair( wordAlignment, srcAndTgtPosNew, outfileExtractPhrasesPair, outfileExtractPhrasesPairInv, ruleNum, options );
        }
        if( sourceS == 0 )
            break;
    }
    return true;
}

bool PhrasesPair::addPhrasePair(        
WordAlignment              &wordAlignment               , 
const SrcAndTgtPos         &srcAndTgtPos                ,
ofstream                   &outfileExtractPhrasesPair   ,
ofstream                   &outfileExtractPhrasesPairInv,
RuleNum                    &ruleNum                     ,
const OptionsOfPhraseTable &options                       )
{
    if( !alignedInfoInv.empty() )
        alignedInfoInv.clear();
    
	// for composing-based extract
    if( options.method == "compose" )
	{
		int composeTimes = 1;
        
		// first time, find the start position of the target phrase
		map< size_t, size_t >::iterator iter = targetLangStartAndEndPos.find( srcAndTgtPos.tgtStartPos );
		if( iter != targetLangStartAndEndPos.end() )
		{
			if( srcAndTgtPos.tgtEndPos > iter->second ) // compose times larger than 1
			{
				size_t curStartPos = iter->second + 1;  // first end pos of the unit phrase
                
				while( srcAndTgtPos.tgtEndPos >= curStartPos )
				{
					while( ( iter = targetLangStartAndEndPos.find( curStartPos ) ) == targetLangStartAndEndPos.end() )
					{
						if( curStartPos < srcAndTgtPos.tgtEndPos )
							++curStartPos;
						else
							break;
					}
                 
                    iter = targetLangStartAndEndPos.find( curStartPos );
					if( iter == targetLangStartAndEndPos.end() )
						break;
					else if( iter->second > srcAndTgtPos.tgtEndPos )
						break;

					size_t curEndPos = iter->second;
					if( srcAndTgtPos.tgtEndPos == curEndPos )
					{
						++composeTimes;
						break;
					}
					else if( srcAndTgtPos.tgtEndPos > curEndPos )
					{
						++composeTimes;
						curStartPos = curEndPos + 1;
					}
					else
					{
						cerr<<"Error!\n";
					}
				}		
			}
		}
		else
		{
			targetLangStartAndEndPos.insert( make_pair( srcAndTgtPos.tgtStartPos, srcAndTgtPos.tgtEndPos ) );
		}

		if( composeTimes > options.compose )
		{
			return true;
		}
	}
	// for composing-based extract

    string tempDestLang;
    string tempSourceLang;

    for( VS_SIZE_TYPE i = srcAndTgtPos.tgtStartPos; i <= srcAndTgtPos.tgtEndPos; ++i )
    {
        for( vector< size_t >::iterator alignedToDestLang_iter = wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( i ).begin(); 
            alignedToDestLang_iter != wordAlignment.destLangAndAlignedInfo.alignedToDestLanguage.at( i ).end(); ++ alignedToDestLang_iter )
            alignedInfoInv.push_back( make_pair( i- srcAndTgtPos.tgtStartPos, *alignedToDestLang_iter - srcAndTgtPos.srcStartPos ) );
        
        tempDestLang += ( wordAlignment.destLangAndAlignedInfo.destLanguage.at( i ) + " " );
    }

    for( VS_SIZE_TYPE i = srcAndTgtPos.srcStartPos; i <= srcAndTgtPos.srcEndPos; ++i )
    {
        tempSourceLang += ( wordAlignment.sourceLangAndAlignedInfo.sourceLanguage.at( i ) + " " );
    }

    ++ruleNum.initialRuleNum;
    outfileExtractPhrasesPair<<tempSourceLang<<"||| "<<tempDestLang<<"|||";
    outfileExtractPhrasesPairInv<<tempDestLang<<"||| "<<tempSourceLang<<"|||";

    for( vector< pair< VS_SIZE_TYPE, VS_SIZE_TYPE > >::iterator iter = alignedInfoInv.begin(); iter != alignedInfoInv.end(); ++iter )
    {
        outfileExtractPhrasesPair<<" "<<iter->second<<"-"<<iter->first;
        outfileExtractPhrasesPairInv<<" "<<iter->first<<"-"<<iter->second;            
    }

    outfileExtractPhrasesPair   <<"\n";
    outfileExtractPhrasesPairInv<<"\n";

    return true;
}
