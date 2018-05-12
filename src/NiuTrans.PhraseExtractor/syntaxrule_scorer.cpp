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
 * SYNSC_scoring_for_synrule.cpp
 *
 * $Version:
 * 0.4.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2012/05/23, add this file, scoring for syntax-rule, modified by Qiang Li.
 *
 */

#include "syntaxrule_scorer.h"
using namespace syntaxrule_scorer;

bool ScoringforSynRule::calcuSrcandTgtScoreGivenRootSpeed( 
string                     &synRule      ,
string                     &output       ,
OptionsOfScoringforSynRule &options      ,
SystemCommand              &systemCommand  )
{
    cerr<<"\n####################################################################\n"
        <<  "# Calculate Rule Scores Given Root (version 4.0)  --www.nlplab.com #\n"
        <<  "####################################################################\n"
        <<flush;

#ifdef __DEBUGSYNSC__
    cerr<<"ScoringforSynRule::calcuSrcScoreGivenRoot:step1\n"<<flush;
    cerr<<"synRule:"<<synRule<<"\n"<<flush;
    cerr<<"output :"<<output <<"\n"<<flush;
    cerr<<"options:"<<options.cutoff<<" "<<options.model<<"\n"<<flush;
    cerr<<"systemCommand.sortFile:"<<systemCommand.sortFile<<"\n"
        <<"systemCommand.del     :"<<systemCommand.del     <<"\n"
        <<flush;
#endif

    string synRuleSorted = synRule + ".sorted";
    string command = systemCommand.sortFile + synRule + " > " + synRuleSorted;

#ifdef __DEBUGSYNSC__
    cerr<<"command:"<<command<<"\n"<<flush;
#endif

    clock_t start, finish;

    start = clock();                                 // start time
    cerr<<"Sort\n"
        <<"        command : "<<command      <<"\n"    
        <<"        input   : "<<synRule      <<"\n"
        <<"        output  : "<<synRuleSorted<<"\n"
        <<flush;
    system( command.c_str() );                      // start to sort input syntax-rule. 
    finish = clock();                               // finish time
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;

    ifstream infile_synRuleSorted( synRuleSorted.c_str() );
    if( !infile_synRuleSorted )
    {
        cerr<<"Error: can not open file "<<synRuleSorted<<"\n"<<flush;
        exit( 1 );
    }

    string ofile_synRuleScore = output + ".syntaxscore";
    ofstream outfile_synRuleScore( ofile_synRuleScore.c_str() );
    if( !outfile_synRuleScore )
    {
        cerr<<"Error: can not open file "<<ofile_synRuleScore<<"\n"<<flush;
        exit( 1 );
    }

    BasicMethod bm                       ;
    string      line_synRuleSorted       ;
    string      lastRootLabel            ;
    bool        firstLine          = true;
    size_t      lineNo             = 0   ;
    size_t      ruleNo             = 0   ;

    start = clock();                                 // start time
    cerr<<"Calculate Pr(r|root(r)), Pr(s(r)|root(r)) and Pr(t(r)|root(r)):"<<"\n"<<flush;
    while( getline( infile_synRuleSorted, line_synRuleSorted ) )
    {
        ++lineNo;
        bm.clearIllegalChar( line_synRuleSorted );
        bm.rmEndSpace( line_synRuleSorted );

        vector< string > synRuleSortedVec;
        bm.splitWithStr( line_synRuleSorted, " ||| ", synRuleSortedVec );

        if( synRuleSortedVec.size() != 8 )
        {
            cerr<<"\n"
                <<"Warning: line="<<lineNo<<"\n"
                <<"         sent="<<line_synRuleSorted<<"\n"
                <<flush;
            continue;
        }

        SynRuleAtom synRuleAtom( synRuleSortedVec.at( 0 ), synRuleSortedVec.at( 1 ),
                                 synRuleSortedVec.at( 2 ), synRuleSortedVec.at( 3 ), 
                                 synRuleSortedVec.at( 4 ), synRuleSortedVec.at( 5 ),
                                 synRuleSortedVec.at( 6 ), synRuleSortedVec.at( 7 )  );

        string srcandTgtandRootLabel = synRuleSortedVec.at( 0 ) + " ||| " +
                                       synRuleSortedVec.at( 1 ) + " ||| " +
                                       synRuleSortedVec.at( 2 )             ;

        if( firstLine )
        {
            lastRootLabel = synRuleSortedVec.at( 0 );
            firstLine = false;
            
            scoresofSynRuleSpeed.rootLabelCount = 1;                 // first use, initial value.
            ++scoresofSynRuleSpeed.srcPhraseCount[ synRuleSortedVec.at( 1 ) ];
            ++scoresofSynRuleSpeed.tgtPhraseCount[ synRuleSortedVec.at( 2 ) ];

            string srcandTgtPhrase = synRuleSortedVec.at( 1 ) + " ||| " + synRuleSortedVec.at( 2 );
            ++scoresofSynRuleSpeed.srcandTgtPhraseCount[ srcandTgtPhrase ];
    
            scoresforSynRule.insert( make_pair( srcandTgtandRootLabel, synRuleAtom ) );
        }
        else
        {
            if( synRuleSortedVec.at( 0 ) != lastRootLabel )
            {
                lastRootLabel = synRuleSortedVec.at( 0 );

                // start calculate
                for( map< string, SynRuleAtom >::iterator iter  = scoresforSynRule.begin();
                                                          iter != scoresforSynRule.end()  ; 
                                                        ++iter                              )
                {
                    iter->second.scoreofSrcGivenRoot       = scoresofSynRuleSpeed.srcPhraseCount[ iter->second.srcPhrase ]/scoresofSynRuleSpeed.rootLabelCount;
                    iter->second.scoreofTgtGivenRoot       = scoresofSynRuleSpeed.tgtPhraseCount[ iter->second.tgtPhrase ]/scoresofSynRuleSpeed.rootLabelCount;
                    string srcandTgtPhrase = iter->second.srcPhrase + " ||| " + iter->second.tgtPhrase;
                    iter->second.scoreofSrcandTgtGivenRoot = scoresofSynRuleSpeed.srcandTgtPhraseCount[ srcandTgtPhrase ]/scoresofSynRuleSpeed.rootLabelCount; 

                    // add in 2012-05-29, assign IsLowFrequency feature in synrule.
//                  iter->second.isLowFrequency            = ( scoresofSynRuleSpeed.srcandTgtPhraseCount[ srcandTgtPhrase ] <= 3 ) ? true : false;        
                    iter->second.isLowFrequency            = ( scoresofSynRuleSpeed.srcandTgtPhraseCount[ srcandTgtPhrase ] <= options.lowerFreq ) ? true : false;        

                    ++ruleNo;
                    outputSrcandTgtScoreGivenRoot( iter->second, options, outfile_synRuleScore );
                }

                scoresofSynRuleSpeed.rootLabelCount = 0;
                scoresofSynRuleSpeed.srcPhraseCount.clear();
                scoresofSynRuleSpeed.tgtPhraseCount.clear();
                scoresofSynRuleSpeed.srcandTgtPhraseCount.clear();
                scoresforSynRule.clear();

                ++scoresofSynRuleSpeed.rootLabelCount;
                ++scoresofSynRuleSpeed.srcPhraseCount[ synRuleSortedVec.at( 1 ) ];
                ++scoresofSynRuleSpeed.tgtPhraseCount[ synRuleSortedVec.at( 2 ) ];
                string srcandTgtPhrase = synRuleSortedVec.at( 1 ) + " ||| " + synRuleSortedVec.at( 2 );
                ++scoresofSynRuleSpeed.srcandTgtPhraseCount[ srcandTgtPhrase ];

                if( scoresforSynRule.find( srcandTgtandRootLabel ) == scoresforSynRule.end() )
                    scoresforSynRule.insert( make_pair( srcandTgtandRootLabel, synRuleAtom ) );
            }
            else
            {
                ++scoresofSynRuleSpeed.rootLabelCount;
                ++scoresofSynRuleSpeed.srcPhraseCount[ synRuleSortedVec.at( 1 ) ];
                ++scoresofSynRuleSpeed.tgtPhraseCount[ synRuleSortedVec.at( 2 ) ];
                string srcandTgtPhrase = synRuleSortedVec.at( 1 ) + " ||| " + synRuleSortedVec.at( 2 );
                ++scoresofSynRuleSpeed.srcandTgtPhraseCount[ srcandTgtPhrase ];
                if( scoresforSynRule.find( srcandTgtandRootLabel ) == scoresforSynRule.end() )
                    scoresforSynRule.insert( make_pair( srcandTgtandRootLabel, synRuleAtom ) );
            }
        }

        if( lineNo % 10000 == 0 )
            cerr<<"\r        processed "<<lineNo<<" lines."<<flush;
    }

    for( map< string, SynRuleAtom >::iterator iter = scoresforSynRule.begin(); iter != scoresforSynRule.end(); ++iter )
    {
        iter->second.scoreofSrcGivenRoot       = scoresofSynRuleSpeed.srcPhraseCount[ iter->second.srcPhrase ]/scoresofSynRuleSpeed.rootLabelCount;
        iter->second.scoreofTgtGivenRoot       = scoresofSynRuleSpeed.tgtPhraseCount[ iter->second.tgtPhrase ]/scoresofSynRuleSpeed.rootLabelCount;
        string srcandTgtPhrase = iter->second.srcPhrase + " ||| " + iter->second.tgtPhrase;
        iter->second.scoreofSrcandTgtGivenRoot = scoresofSynRuleSpeed.srcandTgtPhraseCount[ srcandTgtPhrase ]/scoresofSynRuleSpeed.rootLabelCount; 

        // add in 2012-05-29, assign IsLowFrequency feature in synrule.
        iter->second.isLowFrequency            = ( scoresofSynRuleSpeed.srcandTgtPhraseCount[ srcandTgtPhrase ] <= options.lowerFreq ) ? true : false;

        ++ruleNo;
        outputSrcandTgtScoreGivenRoot( iter->second, options, outfile_synRuleScore );
        outfile_synRuleScore<<flush;
    }
    cerr<<"\r        processed "<<lineNo<<" lines.\n"<<flush;
    finish = clock();                               // finish time
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n\n"<<flush; 
    cerr<<"Final Results\n"
        <<"        Syntax Rule Number : "<<ruleNo<<"\n"<<flush;

    infile_synRuleSorted.close();
    outfile_synRuleScore.close();

    return true;
}

bool ScoringforSynRule::calcuSrcandTgtScoreGivenRootMemory(
string                     &synRule      ,
string                     &output       ,
OptionsOfScoringforSynRule &options      ,
SystemCommand              &systemCommand  )
{
    cerr<<"\n####################################################################\n"
        <<  "# Calculate Rule Scores Given Root (version 4.0)  --www.nlplab.com #\n"
        <<  "####################################################################\n"
        <<flush;

#ifdef __DEBUGSYNSC__
    cerr<<"ScoringforSynRule::calcuSrcScoreGivenRootMemory:step1\n"<<flush;
    cerr<<"synRule:"<<synRule<<"\n"<<flush;
    cerr<<"output :"<<output <<"\n"<<flush;
    cerr<<"options:"<<options.cutoff<<" "<<options.model<<"\n"<<flush;
    cerr<<"systemCommand.sortFile:"<<systemCommand.sortFile<<"\n"
        <<"systemCommand.del     :"<<systemCommand.del     <<"\n"
        <<flush;
#endif

    string synRuleSorted = synRule + ".sorted";
    string command = systemCommand.sortFile + synRule + " > " + synRuleSorted;

#ifdef __DEBUGSYNSC__
    cerr<<"command:"<<command<<"\n"<<flush;
#endif

    clock_t start, finish;

    start = clock();                                 // start time
    cerr<<"Sort\n"
        <<"        command : "<<command      <<"\n"    
        <<"        input   : "<<synRule      <<"\n"
        <<"        output  : "<<synRuleSorted<<"\n"
        <<flush;
    system( command.c_str() );                      // start to sort input syntax-rule. 
    finish = clock();                               // finish time
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;

    ifstream infile_synRuleSorted( synRuleSorted.c_str() );
    if( !infile_synRuleSorted )
    {
        cerr<<"Error: can not open file "<<synRuleSorted<<"\n"<<flush;
        exit( 1 );
    }

    string ofile_synRuleScore = output + ".syntaxscore";
    ofstream outfile_synRuleScore( ofile_synRuleScore.c_str() );
    if( !outfile_synRuleScore )
    {
        cerr<<"Error: can not open file "<<ofile_synRuleScore<<"\n"<<flush;
        exit( 1 );
    }

    BasicMethod      bm                    ;
    size_t           lineNo             = 0;
    size_t           ruleNo             = 0;
    vector< string > synRuleSortedVec      ;
    string           line_synRuleSorted    ;

    // 2012/05/25, count for root label.
    cerr<<"Count for Root Label:"<<"\n"<<flush;
    while( getline( infile_synRuleSorted, line_synRuleSorted ) )
    {
        ++lineNo;
        bm.clearIllegalChar( line_synRuleSorted );
        bm.rmEndSpace(       line_synRuleSorted );
/*
#ifdef __DEBUGSYNSC__
        cerr<<line_synRuleSorted<<"\n"<<flush;
#endif
*/

        if( !synRuleSortedVec.empty() )
        {
            synRuleSortedVec.clear();
        }

        bm.splitWithStr(    line_synRuleSorted, " ||| ", synRuleSortedVec );

        if( synRuleSortedVec.size() != 8 )
        {
            cerr<<"\n"
                <<"Warning: line="<<lineNo<<"\n"
                <<"         sent="<<line_synRuleSorted<<"\n"
                <<flush;
            continue;
        }
        
        ++scoresofSynRuleMemory.rootLabelCount[ synRuleSortedVec.at( 0 ) ];

        if( lineNo % 10000 == 0 )
            cerr<<"\r        processed "<<lineNo<<" lines."<<flush;
    }
    cerr<<"\r        processed "<<lineNo<<" lines.\n"<<flush;

    // close the input file, open later...
    infile_synRuleSorted.clear();
    infile_synRuleSorted.close();

    infile_synRuleSorted.open( synRuleSorted.c_str() );

    lineNo               = 0            ;
    line_synRuleSorted   = ""           ;
//  string lastSrcPhrase                ;
    string lastRootLabelandSrcPhr       ;
    bool   firstLine     = true         ;       

    start = clock();                                 // start time
    cerr<<"Calculate Pr(r|root(r)) and Pr(s(r)|root(r)):"<<"\n"<<flush;
    while( getline( infile_synRuleSorted, line_synRuleSorted ) )
    {
        ++lineNo;
        bm.clearIllegalChar( line_synRuleSorted );
        bm.rmEndSpace(       line_synRuleSorted );

        if( !synRuleSortedVec.empty() )
        {
            synRuleSortedVec.clear();
        }

        bm.splitWithStr( line_synRuleSorted, " ||| ", synRuleSortedVec );
        if( synRuleSortedVec.size() != 8 )
        {
            cerr<<"\n"
                <<"Warning: line="<<lineNo<<"\n"
                <<"         sent="<<line_synRuleSorted<<"\n"
                <<flush;
            continue;
        }

        SynRuleAtom synRuleAtom( synRuleSortedVec.at( 0 ), synRuleSortedVec.at( 1 ),
                                 synRuleSortedVec.at( 2 ), synRuleSortedVec.at( 3 ),
                                 synRuleSortedVec.at( 4 ), synRuleSortedVec.at( 5 ),
                                 synRuleSortedVec.at( 6 ), synRuleSortedVec.at( 7 )  );

        string srcandTgtandRootLabel = synRuleSortedVec.at( 0 ) + " ||| " +
                                       synRuleSortedVec.at( 1 ) + " ||| " +
                                       synRuleSortedVec.at( 2 )             ;

        if( firstLine )
        {
//          lastSrcPhrase = synRuleSortedVec.at( 1 );
            lastRootLabelandSrcPhr = synRuleSortedVec.at( 0 ) + " ||| " + synRuleSortedVec.at( 1 );
            firstLine     = false;

            ++scoresofSynRuleMemory.srcPhraseCount[ synRuleSortedVec.at( 1 ) ];
            //++scoresofSynRuleMemory.tgtPhraseCount[ synRuleSortedVec.at( 2 ) ];

            string srcandTgtPhrase = synRuleSortedVec.at( 1 ) + " ||| " + synRuleSortedVec.at( 2 );
            ++scoresofSynRuleMemory.srcandTgtPhraseCount[ srcandTgtPhrase ];

            scoresforSynRule.insert( make_pair( srcandTgtandRootLabel, synRuleAtom ) );
        }
        else
        {
            if( ( synRuleSortedVec.at( 0 ) + " ||| " + synRuleSortedVec.at( 1 ) ) != lastRootLabelandSrcPhr )
//            if( synRuleSortedVec.at( 1 ) != lastSrcPhrase )
            {
                lastRootLabelandSrcPhr = synRuleSortedVec.at( 0 ) + " ||| " + synRuleSortedVec.at( 1 );
//              lastSrcPhrase = synRuleSortedVec.at( 1 );

                // start calculate 
                for( map< string, SynRuleAtom >::iterator iter  = scoresforSynRule.begin();
                                                          iter != scoresforSynRule.end()  ; 
                                                        ++iter                              )
                {
                    iter->second.scoreofSrcGivenRoot       = scoresofSynRuleMemory.srcPhraseCount[ iter->second.srcPhrase ]/scoresofSynRuleMemory.rootLabelCount[ iter->second.rootLabel ];
                    string srcandTgtPhrase                 = iter->second.srcPhrase + " ||| " + iter->second.tgtPhrase;
                    iter->second.scoreofSrcandTgtGivenRoot = scoresofSynRuleMemory.srcandTgtPhraseCount[ srcandTgtPhrase ]/scoresofSynRuleMemory.rootLabelCount[ iter->second.rootLabel ];

                    // add in 2012-05-29, assign IsLowFrequency feature in synrule.
                    iter->second.isLowFrequency            = ( scoresofSynRuleMemory.srcandTgtPhraseCount[ srcandTgtPhrase ] <= options.lowerFreq ) ? true : false;

                    ++ruleNo;
                    outputSrcandTgtScoreGivenRoot( iter->second, options, outfile_synRuleScore );
                }
                
                scoresofSynRuleMemory.srcPhraseCount.clear();
                scoresofSynRuleMemory.srcandTgtPhraseCount.clear();
                scoresforSynRule.clear();

                ++scoresofSynRuleMemory.srcPhraseCount[ synRuleSortedVec.at( 1 ) ];
                string srcandTgtPhrase = synRuleSortedVec.at( 1 ) + " ||| " + synRuleSortedVec.at( 2 );
                ++scoresofSynRuleMemory.srcandTgtPhraseCount[ srcandTgtPhrase ];

                if( scoresforSynRule.find( srcandTgtandRootLabel ) == scoresforSynRule.end() )
                {
                    scoresforSynRule.insert( make_pair( srcandTgtandRootLabel, synRuleAtom ) );
                }
            }
            else
            {
                ++scoresofSynRuleMemory.srcPhraseCount[ synRuleSortedVec.at( 1 ) ];
                //++scoresofSynRuleMemory.tgtPhraseCount[ synRuleSortedVec.at( 2 ) ];
                string srcandTgtPhrase = synRuleSortedVec.at( 1 ) + " ||| " + synRuleSortedVec.at( 2 );
                ++scoresofSynRuleMemory.srcandTgtPhraseCount[ srcandTgtPhrase ];
                if( scoresforSynRule.find( srcandTgtandRootLabel ) == scoresforSynRule.end() )
                {
                    scoresforSynRule.insert( make_pair( srcandTgtandRootLabel, synRuleAtom ) );
                }
            }
        }

        if( lineNo % 10000 == 0 )
            cerr<<"\r        processed "<<lineNo<<" lines."<<flush;
    }

    for( map< string, SynRuleAtom >::iterator iter = scoresforSynRule.begin(); iter != scoresforSynRule.end(); ++iter )
    {
        iter->second.scoreofSrcGivenRoot = scoresofSynRuleMemory.srcPhraseCount[ iter->second.srcPhrase ]/scoresofSynRuleMemory.rootLabelCount[ iter->second.rootLabel ];
        string srcandTgtPhrase = iter->second.srcPhrase + " ||| " + iter->second.tgtPhrase;
        iter->second.scoreofSrcandTgtGivenRoot = scoresofSynRuleMemory.srcandTgtPhraseCount[ srcandTgtPhrase ]/scoresofSynRuleMemory.rootLabelCount[ iter->second.rootLabel ];
        // add in 2012-05-29, assign IsLowFrequency feature in synrule.
        iter->second.isLowFrequency            = ( scoresofSynRuleMemory.srcandTgtPhraseCount[ srcandTgtPhrase ] <= options.lowerFreq ) ? true : false;

        ++ruleNo;
        outputSrcandTgtScoreGivenRoot( iter->second, options, outfile_synRuleScore );
        outfile_synRuleScore<<flush;
    }
    cerr<<"\r        processed "<<lineNo<<" lines.\n"<<flush;
    finish = clock();                               // finish time
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n\n"<<flush; 
    cerr<<"Final Results\n"
        <<"        Syntax Rule Number : "<<ruleNo<<"\n"<<flush;

    infile_synRuleSorted.close();
    outfile_synRuleScore.close();

    return true;
}

bool ScoringforSynRule::outputSrcandTgtScoreGivenRoot(
SynRuleAtom                &synRuleAtom,
OptionsOfScoringforSynRule &options    ,
ofstream                   &outfile      )
{
    if( options.model == "t2s" )
    {
        if( synRuleAtom.tgtPhrase == "NULL" )
            synRuleAtom.tgtPhrase = "<NULL>";
        if( synRuleAtom.tgtPhraseFormatConv == "NULL" )
            synRuleAtom.tgtPhraseFormatConv = "<NULL>";

        outfile<<synRuleAtom.srcPhrase                <<" ||| "
               <<synRuleAtom.tgtPhrase                <<" ||| "
               <<synRuleAtom.rootLabel                <<" ||| "
               <<synRuleAtom.srcTree                  <<" ||| "
               <<synRuleAtom.tgtPhraseFormatConv      <<" ||| ";

        outfile<<log(synRuleAtom.scoreofSrcandTgtGivenRoot)<<" "
               <<log(synRuleAtom.scoreofSrcGivenRoot      )<<" ";

        if( options.speedorMemory == "speed" )
            outfile<<log(synRuleAtom.scoreofTgtGivenRoot  )<<" ";
//          outfile<<"0"<<" ";
        else
            outfile<<"0"<<" ";

        outfile<<synRuleAtom.rawScore                 <<" "
               <<synRuleAtom.isLowFrequency           <<" ||| "
               <<synRuleAtom.alignment                <<"\n"   ;
    }
    else if( options.model == "s2t" )
    {
        if( synRuleAtom.tgtPhrase == "NULL" )
            synRuleAtom.tgtPhrase = "<NULL>";
        if( synRuleAtom.tgtPhraseFormatConv == "NULL" )
            synRuleAtom.tgtPhraseFormatConv = "<NULL>";
        
        outfile<<synRuleAtom.srcPhrase                <<" ||| "
               <<synRuleAtom.tgtPhrase                <<" ||| "
               <<synRuleAtom.rootLabel                <<" ||| "
               <<synRuleAtom.tgtTree                  <<" ||| "
               <<synRuleAtom.tgtPhraseFormatConv      <<" ||| ";

        outfile<<log(synRuleAtom.scoreofSrcandTgtGivenRoot)<<" "
               <<log(synRuleAtom.scoreofSrcGivenRoot      )<<" ";
        
        if( options.speedorMemory == "speed" )
            outfile<<log(synRuleAtom.scoreofTgtGivenRoot  )<<" ";
//          outfile<<"0"<<" ";
        else
            outfile<<"0"<<" ";

        outfile<<synRuleAtom.rawScore                 <<" "
               <<synRuleAtom.isLowFrequency           <<" ||| "
               <<synRuleAtom.alignment                <<"\n"   ;
    }
    else if( options.model == "t2t" )
    {
        if( synRuleAtom.tgtPhrase == "NULL" )
            synRuleAtom.tgtPhrase = "<NULL>";
        if( synRuleAtom.tgtPhraseFormatConv == "NULL" )
            synRuleAtom.tgtPhraseFormatConv = "<NULL>";
        
        outfile<<synRuleAtom.srcPhrase                <<" ||| "
               <<synRuleAtom.tgtPhrase                <<" ||| "
               <<synRuleAtom.rootLabel                <<" ||| "
               <<synRuleAtom.srcTree                  <<" || "
               <<synRuleAtom.tgtTree                  <<" ||| "
               <<synRuleAtom.tgtPhraseFormatConv      <<" ||| ";

        outfile<<log(synRuleAtom.scoreofSrcandTgtGivenRoot)<<" "
               <<log(synRuleAtom.scoreofSrcGivenRoot      )<<" ";

        if( options.speedorMemory == "speed" )
            outfile<<log(synRuleAtom.scoreofTgtGivenRoot  )<<" ";
//          outfile<<"0"<<" ";
        else
            outfile<<"0"<<" ";

        outfile<<synRuleAtom.rawScore                 <<" "
               <<synRuleAtom.isLowFrequency           <<" ||| "
               <<synRuleAtom.alignment                <<"\n"   ;
    }
    else
    {
        cerr<<"Error: parameter \"-model\" is wrong!\n"<<flush;
        exit( 1 );
    }
    return true;
}



bool ScoringforSynRule::consolidatePhraseTabandSynRule( 
string        &phraseTab    , 
string        &synRule      , 
SystemCommand &systemCommand  )
{
    cerr<<"\n####################################################################\n"
        <<  "# Consolidate PhraseTab and SynRule(version 4.0)  --www.nlplab.com #\n"
        <<  "####################################################################\n"
        <<flush;

#ifdef __DEBUGSYNSC__
    cerr<<"phraseTab:"<<phraseTab<<"\n"<<flush;
    cerr<<"synRule  :"<<synRule  <<"\n"<<flush;
    cerr<<"sortFile :"<<systemCommand.sortFile<<"\n"<<flush;
    cerr<<"del      :"<<systemCommand.del     <<"\n"<<flush;
#endif


#ifdef __DEBUGSYNSC__
    string phr1 = "#ADJP #NN ||| #ADJP #NN that";
    string phr2 = "#ADJP #NN ||| #ADJP #NN";
    cerr<<"~~"<<phr1<<"~~"<<phr2<<"~~\n";
    cerr<<"phr1<phr2:"
        <<(phr1<phr2? "true\n":"false\n")
        <<flush;
    cerr<<"phr1=phr2:"
        <<(phr1==phr2? "true\n":"false\n")
        <<flush;
    cerr<<"phr1>phr2:"
        <<(phr1>phr2? "true\n":"false\n")
        <<flush;
    cerr<<"phr1<phr2:c.str:"
        <<(phr1.c_str() < phr2.c_str() ? "true\n" : "false\n" )
        <<flush;
    cerr<<"phr1=phr2:c.str:"
        <<(phr1.c_str() == phr2.c_str() ? "true\n" : "false\n" )
        <<flush;
    cerr<<"phr1>phr2:c.str:"
        <<(phr1.c_str() > phr2.c_str() ? "true\n" : "false\n" )
        <<flush;
    cerr<<"strcmp(phr1,phr2):"
        <<( strcmp( phr1.c_str(), phr2.c_str() ) == 0 ? "equal\n" : ( strcmp( phr1.c_str(), phr2.c_str() ) == 1 ? "big\n" : "small\n"));  

    phr1 = "#ADJP #NN ||| #ADJP #NN that |||";
    phr2 = "#ADJP #NN ||| #ADJP #NN |||";
    cerr<<"~~"<<phr1<<"~~"<<phr2<<"~~\n";
    cerr<<"phr1<phr2:"
        <<(phr1<phr2? "true\n":"false\n")
        <<flush;
    cerr<<"phr1=phr2:"
        <<(phr1==phr2? "true\n":"false\n")
        <<flush;
    cerr<<"phr1>phr2:"
        <<(phr1>phr2? "true\n":"false\n")
        <<flush;
    cerr<<"phr1<phr2:c.str:"
        <<(phr1.c_str() < phr2.c_str() ? "true\n" : "false\n" )
        <<flush;
    cerr<<"phr1=phr2:c.str:"
        <<(phr1.c_str() == phr2.c_str() ? "true\n" : "false\n" )
        <<flush;
    cerr<<"phr1>phr2:c.str:"
        <<(phr1.c_str() > phr2.c_str() ? "true\n" : "false\n" )
        <<flush;
    cerr<<"strcmp(phr1,phr2):"
        <<( strcmp( phr1.c_str(), phr2.c_str() ) == 0 ? "equal\n" : ( strcmp( phr1.c_str(), phr2.c_str() ) == 1 ? "big\n" : "small\n"));  

    phr1 = "#ADJP #NN ||| #ADJP #NN";
    phr2 = "#ADJP #NN ||| #ADJP #NN";
    cerr<<"~~"<<phr1<<"~~"<<phr2<<"~~\n";
    cerr<<"phr1<phr2:"
        <<(phr1<phr2? "true\n":"false\n")
        <<flush;
    cerr<<"phr1=phr2:"
        <<(phr1==phr2? "true\n":"false\n")
        <<flush;
    cerr<<"phr1>phr2:"
        <<(phr1>phr2? "true\n":"false\n")
        <<flush;
    cerr<<"phr1<phr2:c.str:"
        <<(phr1.c_str() < phr2.c_str() ? "true\n" : "false\n" )
        <<flush;
    cerr<<"phr1=phr2:c.str:"
        <<(phr1.c_str() == phr2.c_str() ? "true\n" : "false\n" )
        <<flush;
    cerr<<"phr1>phr2:c.str:"
        <<(phr1.c_str() > phr2.c_str() ? "true\n" : "false\n" )
        <<flush;
    cerr<<"strcmp(phr1,phr2):"
        <<( strcmp( phr1.c_str(), phr2.c_str() ) == 0 ? "equal\n" : ( strcmp( phr1.c_str(), phr2.c_str() ) == 1 ? "big\n" : "small\n"));  

#endif

    string output = phraseTab + ".syntaxscore";
    ofstream ofile_output( output.c_str() );
    if( !ofile_output )
    {
        cerr<<"Error: can not open file "<<output<<"\n";
        exit( 1 );
    }

    string command;
    clock_t start, finish;
    
    string phraseTabSorted = phraseTab + ".sorted";
    command = systemCommand.sortFile + phraseTab + " > " + phraseTabSorted;
    start = clock();
    cerr<<"Sort\n"
        <<"        command : "<<command        <<"\n"    
        <<"        input   : "<<phraseTab      <<"\n"
        <<"        output  : "<<phraseTabSorted<<"\n"
        <<flush;
    system( command.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;

    command = systemCommand.del + phraseTab;
    start = clock();
    cerr<<"Delete\n"
        <<"        command : "<<command  <<"\n"
        <<"        input   : "<<phraseTab<<"\n"
        <<flush;
    system( command.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n\n"<<flush; 

    string synRuleSorted = synRule + ".sorted";
    command = systemCommand.sortFile + synRule + " > " + synRuleSorted;
    start = clock();
    cerr<<"Sort\n"
        <<"        command : "<<command      <<"\n"    
        <<"        input   : "<<synRule      <<"\n"
        <<"        output  : "<<synRuleSorted<<"\n"
        <<flush;
    system( command.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;

    command = systemCommand.del + synRule;
    start = clock();
    cerr<<"Delete\n"
        <<"        command : "<<command<<"\n"
        <<"        input   : "<<synRule<<"\n"
        <<flush;
    system( command.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n\n"<<flush; 

    ifstream infile_phraseTabSorted( phraseTabSorted.c_str() );
    ifstream infile_synRuleSorted(   synRuleSorted.c_str()   );
    if( !infile_phraseTabSorted || !infile_synRuleSorted )
    {
        cerr<<"Error: can not open file \""<<phraseTabSorted<<"\" or \""<<synRuleSorted<<"\""<<"\n"<<flush;
        exit( 1 );
    }

    string   line_phraseTabSorted;
    string   line_synRuleSorted  ;

    size_t lineNo = 0;
    size_t ruleNo = 0;
    BasicMethod bm;

    vector< string > phraseTabVec;
    vector< string > phraseTabScoreVec;
    vector< string > synRuleVec;
    vector< string > synRuleScoreVec;
    string phraseTabSrcandTgt;
    string synRuleSrcandTgt;

    // firstly, read one line in synRuleSorted file, initial.
    getline( infile_synRuleSorted, line_synRuleSorted );
    bm.clearIllegalChar( line_synRuleSorted );
    if( !synRuleVec.empty() )
    {
        synRuleVec.clear();
        synRuleScoreVec.clear();
    }
    bm.splitWithStr( line_synRuleSorted, " ||| ", synRuleVec );
    bm.split( synRuleVec.at( 5 ), ' ', synRuleScoreVec );
    synRuleSrcandTgt = synRuleVec.at( 0 ) + " ||| " + synRuleVec.at( 1 ) + " ||| ";              // note: must add the last " ||| " for sort as ascii

    start = clock();
    while( getline( infile_phraseTabSorted, line_phraseTabSorted ) )
    {
        ++lineNo;
        bm.clearIllegalChar( line_phraseTabSorted );

#ifdef __DEBUGSYNSC__
        cerr<<"line_phraseTabSorted:"<<line_phraseTabSorted<<"\n"<<flush;
#endif
        if( !phraseTabVec.empty() )
        {
            phraseTabVec.clear();
            phraseTabScoreVec.clear();
        }        
        bm.splitWithStr( line_phraseTabSorted, " ||| ", phraseTabVec );
        bm.split( phraseTabVec.at( 2 ), ' ', phraseTabScoreVec );
        phraseTabSrcandTgt = phraseTabVec.at( 0 ) + " ||| " + phraseTabVec.at( 1 ) + " ||| ";    // note: must add the last " ||| " for sort as ascii

        if( synRuleSrcandTgt == phraseTabSrcandTgt )
        {
            while( synRuleSrcandTgt == phraseTabSrcandTgt )
            {
                ++ruleNo;
                outputConsolidatePhrTabandSynRule( synRuleVec, phraseTabScoreVec, synRuleScoreVec, ofile_output );
            
                if( getline( infile_synRuleSorted, line_synRuleSorted ) )
                {
                    bm.clearIllegalChar( line_synRuleSorted );
                    if( !synRuleVec.empty() )
                    {
                        synRuleVec.clear();
                        synRuleScoreVec.clear();
                    }
                    bm.splitWithStr( line_synRuleSorted, " ||| ", synRuleVec );
                    bm.split( synRuleVec.at( 5 ), ' ', synRuleScoreVec );
                    synRuleSrcandTgt = synRuleVec.at( 0 ) + " ||| " + synRuleVec.at( 1 ) + " ||| ";// note: must add the last " ||| " for sort as ascii
                }
                else
                {
                    break;
                }
            }
        }
        else if( synRuleSrcandTgt < phraseTabSrcandTgt )
        {
            while( synRuleSrcandTgt < phraseTabSrcandTgt )
            {
                if( getline( infile_synRuleSorted, line_synRuleSorted ) )
                {
                    bm.clearIllegalChar( line_synRuleSorted );
                    if( !synRuleVec.empty() )
                    {
                        synRuleVec.clear();
                        synRuleScoreVec.clear();
                    }
                    bm.splitWithStr( line_synRuleSorted, " ||| ", synRuleVec );
                    bm.split( synRuleVec.at( 5 ), ' ', synRuleScoreVec );
                    synRuleSrcandTgt = synRuleVec.at( 0 ) + " ||| " + synRuleVec.at( 1 ) + " ||| ";// note: must add the last " ||| " for sort as ascii

                }
                else
                {
                    break;
                }
            }

            if( synRuleSrcandTgt == phraseTabSrcandTgt )
            {
                while( synRuleSrcandTgt == phraseTabSrcandTgt )
                {
                    ++ruleNo;
                    outputConsolidatePhrTabandSynRule( synRuleVec, phraseTabScoreVec, synRuleScoreVec, ofile_output );

                    if( getline( infile_synRuleSorted, line_synRuleSorted ) )
                    {
                        bm.clearIllegalChar( line_synRuleSorted );
                        if( !synRuleVec.empty() )
                        {
                            synRuleVec.clear();
                            synRuleScoreVec.clear();
                        }
                        bm.splitWithStr( line_synRuleSorted, " ||| ", synRuleVec );
                        bm.split( synRuleVec.at( 5 ), ' ', synRuleScoreVec );
                        synRuleSrcandTgt = synRuleVec.at( 0 ) + " ||| " + synRuleVec.at( 1 ) + " ||| "; // note: must add the last " ||| " for sort as ascii
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        else if( synRuleSrcandTgt > phraseTabSrcandTgt )
        {
            ;
        }
        
        if( lineNo % 10000 == 0 )
        {
            cerr<<"\r        processed "<<lineNo<<" lines in PhraseTab."<<flush;
        }
    }
    finish = clock();
    cerr<<"\r        processed "<<lineNo<<" lines in PhraseTab.\n"<<flush;
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n\n"<<flush; 
    cerr<<"Final Results\n"
        <<"        Syntax Rule Number : "<<ruleNo<<"\n"<<flush;

    infile_phraseTabSorted.close();
    infile_synRuleSorted.close();
    ofile_output.close();

    return true;
}

bool ScoringforSynRule::outputConsolidatePhrTabandSynRule( 
vector< string > &synRuleVec       ,
vector< string > &phraseTabScoreVec,
vector< string > &synRuleScoreVec  ,
ofstream         &ofile_output       )

{
    ofile_output<<synRuleVec.at( 0 )       <<" ||| "
                <<synRuleVec.at( 4 )       <<" ||| "
                <<synRuleVec.at( 2 )       <<" ||| "
                <<phraseTabScoreVec.at( 0 )<<" "          // Pr(t|s).  s->t translation probablity
                <<phraseTabScoreVec.at( 1 )<<" "          // Lex(t|s). s->t lexical weight
                <<phraseTabScoreVec.at( 2 )<<" "          // Pr(s|t).  t->s translation probablity
                <<phraseTabScoreVec.at( 3 )<<" "          // Lex(s|t). t->s lexical weight
                <<phraseTabScoreVec.at( 4 )<<" "          
                <<synRuleScoreVec.at( 0 )  <<" "          // Pr(r|root(r)).
                <<synRuleScoreVec.at( 1 )  <<" "          // Pr(s(r)|root(r)).
    //          <<synRuleScoreVec.at( 2 )  <<" "          // Pr(t(r)|root(r)).
                <<synRuleScoreVec.at( 3 )  <<" "          // IsLexicalized
                <<synRuleScoreVec.at( 4 )  <<" "          // IsComposed
                <<synRuleScoreVec.at( 5 )  <<" "          // IsLowFrequency
                <<"0"                      <<" ||| "      // bilex
                <<synRuleVec.at( 6 )       <<"\n";
    return true;
}
