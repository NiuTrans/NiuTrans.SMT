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
 * phrase extraction and scorer; SC_phrase_table.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 
 *
 */


#include "ruletable_scorer.h"
using namespace ruletable_scorer;

bool LexicalTable::loadLexicalTable( const string &fileName )
{
    ifstream infileLexicalTable( fileName.c_str() );
    if( !infileLexicalTable )
    {
        cerr<<"\nError: Can not open the lexical table\n";
        exit( 1 );
    }
    string lineOfLexicalTable;
    size_t lineNo = 0;
    while( getline( infileLexicalTable, lineOfLexicalTable ) )    // read lexical table
    {
        ++lineNo;
        vector< string > tempSplitItom;
        split( lineOfLexicalTable, ' ', tempSplitItom );
        if( tempSplitItom.size() != 3 )
        {
            cerr<<"Warning: line "<<lineNo<<" in file "<<fileName
                <<" is bad data, skipping..."<<"\n\t"
                <<lineOfLexicalTable<<endl;
            continue;
        }
        double probablity = atof( tempSplitItom.at( 2 ).c_str() );
        lexicalTable[ tempSplitItom.at( 1 ) ][ tempSplitItom.at( 0 ) ] = probablity;
        if( lineNo % 100000 == 0 )
            cout<<"\r\tprocessed "<<lineNo<<" lines."<<flush;
    }
    cout<<"\r\tprocessed "<<lineNo<<" lines."<<flush;
    return true;
}

bool PhraseAlignment::create( const string &line, const size_t &sentenceLineNo )
{
    vector< string > tempPhraseTable;
    split( line, ' ', tempPhraseTable );

    size_t splitDomain = 0;
    bool assignFlag = true;
    for( vector< string >::iterator iter_tempPhraseTable = tempPhraseTable.begin();
        iter_tempPhraseTable != tempPhraseTable.end(); ++ iter_tempPhraseTable )
    {
        if( *iter_tempPhraseTable == "|||" )
        {
            ++ splitDomain;
            continue;
        }

        switch ( splitDomain )
        {
        case 0:
            sourceLanguage.push_back( *iter_tempPhraseTable );
            break;
        case 1:
            destLanguage.push_back( *iter_tempPhraseTable );
            break;
        case 2:
            if( assignFlag )
            {
                alignedToSourceLang.assign( sourceLanguage.size(), vector< size_t >() );
                alignedToDestLang.assign( destLanguage.size(), vector< size_t >() );
                assignFlag = false;
            }
            unsigned int sourceNo, destNo;
#ifdef WIN32
            if( !sscanf_s( iter_tempPhraseTable->c_str(), "%u-%u", &sourceNo, &destNo ) )
#else
            if( !sscanf( iter_tempPhraseTable->c_str(), "%u-%u", &sourceNo, &destNo ) )
#endif
            {
                cerr<<"WARNING:"<<*iter_tempPhraseTable<<" is not num-num in line:"<<sentenceLineNo<<endl;
                return false;
            }
            alignedToSourceLang.at( sourceNo ).push_back( destNo );
            alignedToDestLang.at( destNo ).push_back( sourceNo );
            break;
        case 3:
            rootLabel += *iter_tempPhraseTable;
            break;
        default:
            break;
        }
    }
    return true;
}

bool LexicalWeight::caculateLexicalWeight( PhraseAlignment &pa, LexicalTable &lt, string &method )
{
    double lexicalWeighting = 1;
    vector< string >::size_type destPos = 0;
    string sourceLang;
    string destLang;

    if( method == "lexw" )
    {
        for( vector< vector< size_t > >::iterator iter_alignedToDest = pa.alignedToDestLang.begin(); 
            iter_alignedToDest != pa.alignedToDestLang.end(); ++iter_alignedToDest )
        {
            double weightDGivenS = 0;
            vector< size_t >::size_type alignedToDestSize = 0;

            if( iter_alignedToDest->size() != 0 )
            {
                alignedToDestSize = iter_alignedToDest->size();
                for( vector< size_t >::size_type i = 0; i < alignedToDestSize; ++i )
                {
                    sourceLang = pa.sourceLanguage.at( iter_alignedToDest->at( i ) );
                    destLang = pa.destLanguage.at( destPos );
                    weightDGivenS += lt.lexicalTable[ sourceLang ][ destLang ];
                }
            }
            if( iter_alignedToDest->size() == 0 )
            {
                alignedToDestSize = 1;
                sourceLang = "NULL";
                destLang = pa.destLanguage.at( destPos );
                weightDGivenS += lt.lexicalTable[ sourceLang ][ destLang ];

            }

            lexicalWeighting /= double ( alignedToDestSize );

            ////////// prevent from hiero rule "[X][X]" lexical score is 0, add in 2011-11-12
            if( weightDGivenS == 0 )
                weightDGivenS = 1;
            //////////

            lexicalWeighting *= weightDGivenS;

            ++destPos;
        }
    }

    if( method == "noisyor" )
    {
        for( vector< vector< size_t > >::iterator iter_alignedToDest = pa.alignedToDestLang.begin(); 
            iter_alignedToDest != pa.alignedToDestLang.end(); ++iter_alignedToDest )
        {
            double weightDGivenS = 1;
            vector< size_t >::size_type alignedToDestSize = 0;

            if( iter_alignedToDest->size() != 0 )
            {
                alignedToDestSize = iter_alignedToDest->size();
                for( vector< size_t >::size_type i = 0; i < alignedToDestSize; ++i )
                {
                    sourceLang = pa.sourceLanguage.at( iter_alignedToDest->at( i ) );
                    destLang = pa.destLanguage.at( destPos );
                    weightDGivenS *= ( 1 - lt.lexicalTable[ sourceLang ][ destLang ] );
                }
            }
            if( iter_alignedToDest->size() == 0 )
            {
                alignedToDestSize = 1;
                sourceLang = "NULL";
                destLang = pa.destLanguage.at( destPos );
                weightDGivenS *= ( 1 - lt.lexicalTable[ sourceLang ][ destLang ] );
            }

            lexicalWeighting *= ( 1 - weightDGivenS );            
            ++destPos;
        }
    }

    pa.lexicalWeighting = lexicalWeighting;
    return true;
}

//bool PhraseTable::generatePhraseTable( PhraseAlignment &pa, bool &lastline, ofstream &outfile, bool &inverseFlag, int &cutoff )
bool PhraseTable::generatePhraseTable( PhraseAlignment &pa, bool &lastline, ofstream &outfile, bool &inverseFlag, OptionsOfScore &options, ScoreClassifyNum &scoreClassifyNum )
{
    if( lastline )
    {
        // for cut off
        for( vector< PhraseTableAtom>::iterator iter = phraseTable.begin(); iter != phraseTable.end(); ++iter )
        {
            if( !inverseFlag )
            {
                if( iter->sourceLang.find( "#X") != string::npos )
                {
                    if( iter->rateOfFrequency > options.cutoffHiero )
                    {
                        phraseTableCutOff.push_back( *iter );
                    }
                    else
                    {
                        totalFrequency -= iter->rateOfFrequency;
                    }
                }
                else
                {
                    if( iter->rateOfFrequency > options.cutoffInit )
                    {
                        phraseTableCutOff.push_back( *iter );
                    }
                    else
                    {
                        totalFrequency -= iter->rateOfFrequency;
                    }
                }
            }
            else
            {
                if( iter->destLang.find( "#X") != string::npos )
                {
                    if( iter->rateOfFrequency > options.cutoffHiero )
                    {
                        phraseTableCutOff.push_back( *iter );
                    }
                    else
                    {
                        totalFrequency -= iter->rateOfFrequency;
                    }
                }
                else
                {
                    if( iter->rateOfFrequency > options.cutoffInit )
                    {
                        phraseTableCutOff.push_back( *iter );
                    }
                    else
                    {
                        totalFrequency -= iter->rateOfFrequency;
                    }
                }
            }
/*
            if( iter->frequency > cutoff )
            {
                phraseTableCutOff.push_back( *iter );            
            }
            else
            {
                totalFrequency -= iter->frequency;
            }
*/
        }

        for( vector< PhraseTableAtom >::iterator iter = phraseTableCutOff.begin(); iter != phraseTableCutOff.end(); ++iter )
        {
            iter->frequency = iter->rateOfFrequency;
            iter->maxFrequency = iter->rateOfFrequency = iter->rateOfFrequency / totalFrequency;
        }
        output( outfile, inverseFlag, options, scoreClassifyNum ,totalFrequency);
        return true;
    }
    string tempSourceLang;
    string tempDestLang;
    string tempAlignedToSourceLang;
    string tempAlignedToDestLang;
    for( vector< string >::iterator iter = pa.sourceLanguage.begin(); iter != pa.sourceLanguage.end(); ++iter )
        tempSourceLang += ( *iter + " " );
    for( vector< string >::iterator iter = pa.destLanguage.begin(); iter != pa.destLanguage.end(); ++iter )
        tempDestLang += ( *iter + " " );
    for( vector< vector< size_t > >::iterator iter = pa.alignedToSourceLang.begin(); iter != pa.alignedToSourceLang.end(); ++iter )
    {
        tempAlignedToSourceLang += "(";
        for( vector< size_t >::iterator iter_inner = iter->begin(); iter_inner != iter->end(); ++iter_inner )
        {
            if( iter_inner == iter->begin() )
                tempAlignedToSourceLang += size_tToString( *iter_inner );            
            else
                tempAlignedToSourceLang += ( "," + size_tToString( *iter_inner ) );            
        }
        tempAlignedToSourceLang += ") ";
    }
    for( vector< vector< size_t > >::iterator iter = pa.alignedToDestLang.begin(); iter != pa.alignedToDestLang.end(); ++iter )
    {
        tempAlignedToDestLang += "(";
        for( vector< size_t >::iterator iter_inner = iter->begin(); iter_inner != iter->end(); ++iter_inner )
        {
            if( iter_inner == iter->begin() )
                tempAlignedToDestLang += size_tToString( *iter_inner );
            else
                tempAlignedToDestLang += ( "," + size_tToString( *iter_inner ) );
        }
        tempAlignedToDestLang += ") ";
    }

    // caculate frequency
    if( !phraseTable.empty() )
    {    
        if( ( phraseTable.rbegin()->sourceLang == tempSourceLang ) && ( phraseTable.rbegin()->destLang == tempDestLang ) 
             && ( phraseTable.rbegin()->alignedToSourceLang == tempAlignedToSourceLang ) 
             && ( phraseTable.rbegin()->alignedToDestLang == tempAlignedToDestLang ) )
        {    
            ++totalFrequency;
			++phraseTable.rbegin()->rateOfFrequency;
        }
        else if( ( phraseTable.rbegin()->sourceLang == tempSourceLang ) && ( phraseTable.rbegin()->destLang == tempDestLang ) )
        {
            ++totalFrequency;
            ++pa.frequency;
            phraseTable.push_back( PhraseTableAtom( tempSourceLang, tempDestLang,tempAlignedToSourceLang, tempAlignedToDestLang, pa.rootLabel, pa.frequency, pa.frequency, pa.frequency, pa.lexicalWeighting ) );
        }
        else if( ( phraseTable.rbegin()->sourceLang == tempSourceLang ) )
        {    
            ++totalFrequency;
            ++pa.frequency;
            phraseTable.push_back( PhraseTableAtom( tempSourceLang, tempDestLang,tempAlignedToSourceLang, tempAlignedToDestLang, pa.rootLabel, pa.frequency, pa.frequency, pa.frequency, pa.lexicalWeighting ) );
        }
        else
        {
            // for cut off
            for( vector< PhraseTableAtom>::iterator iter = phraseTable.begin(); iter != phraseTable.end(); ++iter )
            {
                if( !inverseFlag )
                {
                    if( iter->sourceLang.find( "#X") != string::npos )
                    {
                        if( iter->rateOfFrequency > options.cutoffHiero )
                        {
                            phraseTableCutOff.push_back( *iter );
                        }
                        else
                        {
                            totalFrequency -= iter->rateOfFrequency;
                        }
                    }
                    else
                    {
                        if( iter->rateOfFrequency > options.cutoffInit )
                        {
                            phraseTableCutOff.push_back( *iter );
                        }
                        else
                        {
                            totalFrequency -= iter->rateOfFrequency;
                        }
                    }
                }
                else
                {
                    if( iter->destLang.find( "#X") != string::npos )
                    {
                        if( iter->rateOfFrequency > options.cutoffHiero )
                        {
                            phraseTableCutOff.push_back( *iter );
                        }
                        else
                        {
                            totalFrequency -= iter->rateOfFrequency;
                        }
                    }
                    else
                    {
                        if( iter->rateOfFrequency > options.cutoffInit )
                        {
                            phraseTableCutOff.push_back( *iter );
                        }
                        else
                        {
                            totalFrequency -= iter->rateOfFrequency;
                        }
                    }                
                }
/*
                if( iter->frequency > cutoff )
                {
                    phraseTableCutOff.push_back( *iter );            
                }
                else
                {
                    totalFrequency -= iter->frequency;
                }
*/
            }

            for( vector< PhraseTableAtom >::iterator iter = phraseTableCutOff.begin(); iter != phraseTableCutOff.end(); ++iter )
            {
                iter->frequency = iter->rateOfFrequency;
                iter->maxFrequency = iter->rateOfFrequency = iter->rateOfFrequency / totalFrequency;
            }
            output( outfile, inverseFlag, options, scoreClassifyNum , totalFrequency);
            phraseTable.clear();
            // for cut off
            phraseTableCutOff.clear();
            finalPhraseTable.clear();
            totalFrequency = 1;
            ++pa.frequency;
            phraseTable.push_back( PhraseTableAtom( tempSourceLang, tempDestLang,tempAlignedToSourceLang, tempAlignedToDestLang, pa.rootLabel, pa.frequency, pa.frequency, pa.frequency, pa.lexicalWeighting ) );
        }
    }
    else
    {
        totalFrequency = 1;
        ++pa.frequency;
        phraseTable.push_back( PhraseTableAtom( tempSourceLang, tempDestLang,tempAlignedToSourceLang, tempAlignedToDestLang, pa.rootLabel, pa.frequency, pa.frequency, pa.frequency, pa.lexicalWeighting ) );
    }
    return true;
}

bool PhraseTable::output( ofstream &outfile, bool &inverseFlag, OptionsOfScore &options, ScoreClassifyNum &scoreClassifyNum , double totalFrequency)
{
    for( vector< PhraseTableAtom >::iterator iter_phraseTableCutOff = phraseTableCutOff.begin(); iter_phraseTableCutOff != phraseTableCutOff.end(); ++iter_phraseTableCutOff )
    {
        if( finalPhraseTable.empty() )
        {
            finalPhraseTable.insert( make_pair( make_pair( iter_phraseTableCutOff->sourceLang, iter_phraseTableCutOff->destLang ), *iter_phraseTableCutOff ) );
        }
        else
        {
            map< pair< string,string >, PhraseTableAtom >::iterator iter_findInFinalPhraseTable = finalPhraseTable.find( make_pair( iter_phraseTableCutOff->sourceLang, iter_phraseTableCutOff->destLang ) );
            if( iter_findInFinalPhraseTable != finalPhraseTable.end() )
            {
                if( iter_phraseTableCutOff->rateOfFrequency > iter_findInFinalPhraseTable->second.maxFrequency )
                {
                    iter_findInFinalPhraseTable->second.maxFrequency = iter_phraseTableCutOff->rateOfFrequency;
                    iter_findInFinalPhraseTable->second.lexicalWeight = iter_phraseTableCutOff->lexicalWeight;
					
                }
				
				iter_findInFinalPhraseTable->second.rateOfFrequency += iter_phraseTableCutOff->rateOfFrequency;
				////////////// edit by wangqiang 2015/04/09 , correct the frequency when set "printFreq = 1"  
				iter_findInFinalPhraseTable->second.frequency += iter_phraseTableCutOff->frequency;
				/*
				if( fabs(iter_findInFinalPhraseTable->second.frequency/totalFrequency - iter_findInFinalPhraseTable->second.rateOfFrequency) > 1.0e-6)
				{
					cerr<<"Error Freq In : "<<iter_phraseTableCutOff->sourceLang<<" ||| "<<iter_phraseTableCutOff->destLang<<endl;
					cerr<<"freq/total:"<<iter_findInFinalPhraseTable->second.frequency/totalFrequency<<" rate:"<<iter_findInFinalPhraseTable->second.rateOfFrequency<<endl;
					exit(-1);
				}
				*/
				////////////////
                iter_findInFinalPhraseTable->second.alignedToSourceLang = mergeAlignedInfo( iter_findInFinalPhraseTable->second.alignedToSourceLang, iter_phraseTableCutOff->alignedToSourceLang );
                iter_findInFinalPhraseTable->second.alignedToDestLang = mergeAlignedInfo( iter_findInFinalPhraseTable->second.alignedToDestLang, iter_phraseTableCutOff->alignedToDestLang );
            }
            else
            {
                finalPhraseTable.insert( make_pair( make_pair( iter_phraseTableCutOff->sourceLang, iter_phraseTableCutOff->destLang ), *iter_phraseTableCutOff ) );
            }
        }
    }

    for( map< pair< string, string >, PhraseTableAtom >::iterator iter_finalPhraseTable = finalPhraseTable.begin(); iter_finalPhraseTable != finalPhraseTable.end(); ++iter_finalPhraseTable )
    {
        if( inverseFlag )
        {
            if( iter_finalPhraseTable->second.destLang == "NULL " || iter_finalPhraseTable->second.sourceLang == "NULL " )
                ++scoreClassifyNum.nullRuleNum;
            else if( iter_finalPhraseTable->second.destLang.find( "#X" ) != string::npos )
                ++scoreClassifyNum.hieroRuleNum;
            else
                ++scoreClassifyNum.initialRuleNum;

            outfile <<iter_finalPhraseTable->second.destLang<<"||| "
                    <<iter_finalPhraseTable->second.sourceLang<<"||| "
                    <<iter_finalPhraseTable->second.alignedToDestLang<<"||| "
                    <<iter_finalPhraseTable->second.alignedToSourceLang<<"||| "
//                  <<setiosflags(ios::fixed)<<setprecision(7)
                    <<iter_finalPhraseTable->second.rateOfFrequency<<" "
                    <<iter_finalPhraseTable->second.lexicalWeight;
//                  <<"\n";
            if( options.printFreqFlag )
				// edit by wangqiang, must forced convertion, here frequency is double, use int(frequency)
				// prevent when freq is enough large, the output stream represent this float number using scientific format
                outfile<<" "<<int(iter_finalPhraseTable->second.frequency);
            if( iter_finalPhraseTable->second.rootLabel != "" )
                outfile<<" ||| "<<iter_finalPhraseTable->second.rootLabel;
            outfile<<"\n";
        }
        else
        {
            if( iter_finalPhraseTable->second.destLang == "NULL " || iter_finalPhraseTable->second.sourceLang == "NULL " )
                ++scoreClassifyNum.nullRuleNum;
            else if( iter_finalPhraseTable->second.sourceLang.find( "#X") != string::npos )
                ++scoreClassifyNum.hieroRuleNum;
            else
                ++scoreClassifyNum.initialRuleNum;

            outfile <<iter_finalPhraseTable->second.sourceLang<<"||| "
                    <<iter_finalPhraseTable->second.destLang<<"||| "
                    <<iter_finalPhraseTable->second.alignedToSourceLang<<"||| "
                    <<iter_finalPhraseTable->second.alignedToDestLang<<"||| "
//                  <<setiosflags(ios::fixed)<<setprecision(7)
                    <<iter_finalPhraseTable->second.rateOfFrequency<<" "
                    <<iter_finalPhraseTable->second.lexicalWeight;
//                  <<"\n";
            if( options.printFreqFlag )
				// edit by wangqiang, must forced convertion, here frequency is double, use int(frequency)
				// prevent when freq is enough large, the output stream represent this float number using scientific format
                outfile<<" "<<int(iter_finalPhraseTable->second.frequency);
            if( iter_finalPhraseTable->second.rootLabel != "" )
                outfile<<" ||| "<<iter_finalPhraseTable->second.rootLabel;
            outfile<<"\n";

        }
    }
    return true;
}

bool Consolidate::consolidate( 
                              string &phraseTableName, 
                              string &phraseTableInvName, 
                              string &scoreTableFile, 
                              //bool &printAlignFlag, 
                              OptionsOfScore &options,
                              ConsolidateClassifyNum &consolidateClassifyNum )
{
    ifstream infilePhraseTable( phraseTableName.c_str() );
    ifstream infilePhraseTableInv( phraseTableInvName.c_str() );
    if( !infilePhraseTable || !infilePhraseTableInv )
    {
        cerr<<"ERROR: Could not open file: "<< phraseTableName
            <<" or file: "<< phraseTableInvName<<endl;
        exit( 1 );
    }

    ofstream outfileScoreTable( scoreTableFile.c_str() );

    ofstream outfileScoreTableInv;
    if( options.generInvFileFlag )
    {
        string scoreTableInvFile = scoreTableFile + ".inv";
        outfileScoreTableInv.open( scoreTableInvFile.c_str() );

        if( !outfileScoreTableInv )
        {
            cerr<<"ERROR: Could not open file: "<<scoreTableInvFile<<"\n"<<flush;
            exit( 1 );
        }
    }

    if( !outfileScoreTable )
    {
        cerr<<"ERROR: Could not open file: "<< scoreTableFile<<endl;
        exit( 1 );
    }

    string linePhraseTable, linePhraseTableInv;
    size_t lineNo = 0;
    while( getline( infilePhraseTable, linePhraseTable ) && getline( infilePhraseTableInv, linePhraseTableInv ) )
    {
        ++lineNo;
        //        ptaOfPhraseTable = extractPhraseTable( linePhraseTable, phraseTableName, options.printAlignFlag, lineNo );
        ptaOfPhraseTable = extractPhraseTable( linePhraseTable, phraseTableName, options, lineNo );
        //        ptaOfPhraseTableInv = extractPhraseTable( linePhraseTableInv, phraseTableInvName, options.printAlignFlag, lineNo );
        ptaOfPhraseTableInv = extractPhraseTable( linePhraseTableInv, phraseTableInvName, options, lineNo );

        if( ( ptaOfPhraseTable.sourceLang == ptaOfPhraseTableInv.sourceLang ) && 
            ( ptaOfPhraseTable.destLang == ptaOfPhraseTableInv.destLang ) &&
            ( ptaOfPhraseTable.alignedToSourceLang == ptaOfPhraseTableInv.alignedToSourceLang ) &&
            ( ptaOfPhraseTable.alignedToDestLang == ptaOfPhraseTableInv.alignedToDestLang ) )
        {
            if( ptaOfPhraseTable.sourceLang == "NULL " )
                ptaOfPhraseTable.sourceLang = "<NULL> ";
            if( ptaOfPhraseTable.destLang == "NULL " )
                ptaOfPhraseTable.destLang = "<NULL> ";

            if( ptaOfPhraseTable.sourceLang == "<NULL> " || ptaOfPhraseTable.destLang == "<NULL> " )
            {
                ++consolidateClassifyNum.nullRuleNum;
            }
            else if( ptaOfPhraseTable.sourceLang.find( "#X" ) != string::npos )
            {
                ++consolidateClassifyNum.hieroRuleNum;
            }
            else
            {
                ++consolidateClassifyNum.initialRuleNum;
            }

            outfileScoreTable   <<ptaOfPhraseTable.sourceLang<<"||| "
                <<ptaOfPhraseTable.destLang<<"||| ";

            //    <<ptaOfPhraseTable.alignedToSourceLang<<"||| "
            //    <<ptaOfPhraseTable.alignedToDestLang<<"||| "
            //    <<setiosflags(ios::fixed)<<setprecision(7)

            if( ptaOfPhraseTable.rootLabel != "" )
                outfileScoreTable<<ptaOfPhraseTable.rootLabel<<"||| ";

            outfileScoreTable   <<log(ptaOfPhraseTable.rateOfFrequency)<<" "
                <<log(ptaOfPhraseTable.lexicalWeight)<<" "
                <<log(ptaOfPhraseTableInv.rateOfFrequency)<<" "
                <<log(ptaOfPhraseTableInv.lexicalWeight)<<" "
                <<1<<" "<<0;

            if( options.printFreqFlag )
                outfileScoreTable   <<" ||| "<<ptaOfPhraseTable.frequency;
            if( options.printAlignFlag )
                outfileScoreTable   <<" ||| "<<ptaOfPhraseTable.alignedToSourceLang.substr( 0, ptaOfPhraseTable.alignedToSourceLang.length() - 1 );
            outfileScoreTable   <<"\n";


            if( options.generInvFileFlag )
            {
                outfileScoreTableInv<<ptaOfPhraseTable.destLang<<"||| "
                    <<ptaOfPhraseTable.sourceLang<<"||| ";

                //                <<ptaOfPhraseTable.alignedToDestLang<<"||| "
                //                <<ptaOfPhraseTable.alignedToSourceLang<<"||| "
                //                <<setiosflags(ios::fixed)<<setprecision(7)

                if( ptaOfPhraseTable.rootLabel != "" )
                    outfileScoreTableInv<<ptaOfPhraseTable.rootLabel<<"||| ";

                outfileScoreTableInv<<log(ptaOfPhraseTableInv.rateOfFrequency)<<" "
                    <<log(ptaOfPhraseTableInv.lexicalWeight)<<" "
                    <<log(ptaOfPhraseTable.rateOfFrequency)<<" "
                    <<log(ptaOfPhraseTable.lexicalWeight)<<" "
                    <<1<<" "<<0;
                if( options.printFreqFlag )
                    outfileScoreTableInv<<" ||| "<<ptaOfPhraseTable.frequency;
                if( options.printAlignFlag )
                    outfileScoreTableInv<<" ||| "<<ptaOfPhraseTable.alignedToDestLang.substr( 0, ptaOfPhraseTable.alignedToDestLang.length() - 1 );
                outfileScoreTableInv<<"\n";
            }
        }
        else
        {
            cerr<<"\n"
                <<"WARNING: LINE:"<<lineNo<<"\n"
                <<ptaOfPhraseTable.sourceLang<<ptaOfPhraseTable.destLang
                <<ptaOfPhraseTable.alignedToSourceLang<<ptaOfPhraseTable.alignedToDestLang<<"\n"
                <<ptaOfPhraseTableInv.sourceLang<<ptaOfPhraseTableInv.destLang
                <<ptaOfPhraseTableInv.alignedToSourceLang<<ptaOfPhraseTableInv.alignedToDestLang<<"\n"<<flush;

        }
        if( lineNo % 100000 == 0 )
            cout<<"\r\tprocessed "<<lineNo<<" lines."<<flush;
    }
    cout<<"\r\tprocessed "<<lineNo<<" lines.\n"<<flush;

    infilePhraseTable.close();
    infilePhraseTableInv.close();
    outfileScoreTable.close();

    if( options.generInvFileFlag )
        outfileScoreTableInv.close();

    return true;
}

//PhraseTableAtom Consolidate::extractPhraseTable( string &line, string &fileName, bool &printAlignFlag, size_t &lineNo)
PhraseTableAtom Consolidate::extractPhraseTable( string &line, string &fileName, OptionsOfScore &options, size_t &lineNo)
{
    string tempSourceLang;
    string tempDestLang;
    string tempAlignedToSourceLang;
    string tempAlignedToDestLang;
    string tempRootLabel;
    double tempFrequency;
    double tempRateOfFrequency;
    double tempLexicalWeight;

    vector< string > tempPhraseTable;
    split( line, ' ', tempPhraseTable );

    size_t splitDomain = 0;
    size_t alignPosOfSrcPhrase = 0;
    for( vector< string >::iterator iter_tempPhraseTable = tempPhraseTable.begin();
        iter_tempPhraseTable != tempPhraseTable.end(); ++ iter_tempPhraseTable )
    {
        if( *iter_tempPhraseTable == "|||" )
        {
            ++ splitDomain;
            continue;
        }

        switch ( splitDomain )
        {
        case 0:
            tempSourceLang += ( *iter_tempPhraseTable + " " );            
            break;
        case 1:
            tempDestLang += ( *iter_tempPhraseTable + " " );
            break;
        case 2:
            {
                if( options.printAlignFlag )
                {
                    if( *iter_tempPhraseTable == "()" )
                    {
                        ++alignPosOfSrcPhrase;
                    }
                    else
                    {
                        string tempAlign = (*iter_tempPhraseTable).substr( 1, (*iter_tempPhraseTable).length()-2 );
                        if( tempAlign.find( "," ) != string::npos )
                        {
                            vector<string> tempAlignVec;
                            split( tempAlign, ',', tempAlignVec );
                            for( vector<string>::iterator iter = tempAlignVec.begin(); iter != tempAlignVec.end(); ++iter )
                            {
                                tempAlignedToSourceLang += ( size_tToString( alignPosOfSrcPhrase ) + "-" + *iter + " " );
                                tempAlignedToDestLang += ( *iter + "-" + size_tToString( alignPosOfSrcPhrase ) + " " );
                            }
                        }
                        else
                        {
                            tempAlignedToSourceLang += ( size_tToString( alignPosOfSrcPhrase ) + "-" + tempAlign + " " );
                            tempAlignedToDestLang += ( tempAlign + "-" + size_tToString( alignPosOfSrcPhrase ) + " " );
                        }
                        ++alignPosOfSrcPhrase;                
                    }
                }
                break;
                //          tempAlignedToSourceLang += ( *iter_tempPhraseTable + " " );
                //          break;
            }
        case 3:
            //          tempAlignedToDestLang += ( *iter_tempPhraseTable + " " );
            break;
        case 4:
            tempRateOfFrequency = atof( iter_tempPhraseTable->c_str() );
            ++iter_tempPhraseTable;
            tempLexicalWeight = atof( iter_tempPhraseTable->c_str() );
            if( options.printFreqFlag )
            {            
                ++iter_tempPhraseTable;
                tempFrequency = atoi( iter_tempPhraseTable->c_str() );
            }
            else
                tempFrequency = 0;
            break;
        case 5:
            tempRootLabel += ( *iter_tempPhraseTable + " " );
            break;
        default:
            break;
        }
    }
    PhraseTableAtom pta( tempSourceLang, tempDestLang, tempAlignedToSourceLang, tempAlignedToDestLang, tempRootLabel, tempRateOfFrequency, tempFrequency, tempRateOfFrequency, tempLexicalWeight );
    return pta;
}
