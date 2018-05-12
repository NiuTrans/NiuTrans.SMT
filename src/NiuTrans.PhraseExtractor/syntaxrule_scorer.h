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
 * SYNSC_scoring_for_synrule.h
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

#ifndef _SYNTAXRULE_SCORER_H_
#define _SYNTAXRULE_SCORER_H_

#include <ctime>
#include <string>
#include <vector>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include "ruletable_scorer.h"
#include "basic_method.h"

using namespace std;
using namespace ruletable_scorer;
using namespace basic_method;
//#define __DEBUGSYNSC__

namespace syntaxrule_scorer
{

    class OptionsOfScoringforSynRule: public OptionsOfScore
    {
    public:
        int    cutoff       ;
        int    lowerFreq    ;

        string model        ;
        string speedorMemory;
        

        OptionsOfScoringforSynRule( int &newCutoff, string &newModel, string &newSpeedorMemory, int &newLowerFreq )
            : cutoff( newCutoff ), model( newModel ), speedorMemory( newSpeedorMemory ), lowerFreq( newLowerFreq ){} 
    };

    class SynRuleAtom
    {
    public:
        string rootLabel          ;
        string srcPhrase          ;
        string tgtPhrase          ;
        string rawScore           ;
        string tgtPhraseFormatConv;
        string alignment          ;
        string srcTree            ;
        string tgtTree            ;
        bool   isLowFrequency     ;

        double scoreofSrcGivenRoot      ;
        double scoreofSrcandTgtGivenRoot;
        double scoreofTgtGivenRoot      ;
        
        SynRuleAtom():
            rootLabel()             , srcPhrase()                   ,
            tgtPhrase()             , rawScore()                    ,
            tgtPhraseFormatConv()   , alignment()                   ,
            srcTree()               , tgtTree()                     ,
            scoreofSrcGivenRoot( 0 ), scoreofSrcandTgtGivenRoot( 0 ), 
            scoreofTgtGivenRoot( 0 ), isLowFrequency(        false )  {}

        SynRuleAtom( string &newRootLabel          , string &newSrcPhrase, 
                     string &newTgtPhrase          , string &newRawScore , 
                     string &newTgtPhraseFormatConv, string &newAlignment, 
                     string &newSrcTree            , string &newTgtTree    ):
            rootLabel(           newRootLabel           ), srcPhrase( newSrcPhrase      ),
            tgtPhrase(           newTgtPhrase           ), rawScore(  newRawScore       ),
            tgtPhraseFormatConv( newTgtPhraseFormatConv ), alignment( newAlignment      ),
            srcTree(             newSrcTree             ), tgtTree(   newTgtTree        ),
            scoreofSrcGivenRoot( 0                      ), scoreofSrcandTgtGivenRoot( 0 ),
            scoreofTgtGivenRoot( 0                      ), isLowFrequency(        false )  {}

    };

    /*
    class BasicContentofSynRule
    {
    public:
        string rootLabel;
        string srcPhrase;
        string tgtPhrase;
        BasicContentofSynRule(){}
        BasicContentofSynRule( string &newRootLabel, 
                               string &newSrcPhrase, 
                               string &newTgtPhrase  )
            :rootLabel( newRootLabel ), 
             srcPhrase( newSrcPhrase ), 
             tgtPhrase( newTgtPhrase ){}
    };
    */

    class ScoresofSynRuleMemory
    {
    public:
        map< string, double > rootLabelCount      ;
        map< string, double > srcPhraseCount      ;
        map< string, double > tgtPhraseCount      ;
        map< string, double > srcandTgtPhraseCount;
        ScoresofSynRuleMemory(){};
    };

    class ScoresofSynRuleSpeed
    {
    public:
        double                rootLabelCount      ;
        map< string, double > srcPhraseCount      ;
        map< string, double > tgtPhraseCount      ;
        map< string, double > srcandTgtPhraseCount;
    };

    class ScoringforSynRule
    {
    public:
    //  double                rootLabelCount      ;
    //  map< string, double > srcPhraseCount      ;
    //  map< string, double > tgtPhraseCount      ;
    //  map< string, double > srcandTgtPhraseCount;

        map< string, SynRuleAtom > scoresforSynRule;

        ScoringforSynRule(){};


        ScoresofSynRuleSpeed  scoresofSynRuleSpeed ;
        ScoresofSynRuleMemory scoresofSynRuleMemory;


        bool calcuSrcandTgtScoreGivenRootSpeed( 
            string                     &synRule      , 
            string                     &output       ,
            OptionsOfScoringforSynRule &options      , 
            SystemCommand              &systemCommand  );

        bool calcuSrcandTgtScoreGivenRootMemory(
            string                     &synRule      ,
            string                     &output       ,
            OptionsOfScoringforSynRule &options      ,
            SystemCommand              &systemCommand  );

        bool consolidatePhraseTabandSynRule( 
            string                     &phraseTab    , 
            string                     &synRule      , 
            SystemCommand              &systemCommand  );

    private:
        bool outputSrcandTgtScoreGivenRoot(
            SynRuleAtom                &synRuleAtom,
            OptionsOfScoringforSynRule &options    ,
            ofstream                   &outfile      );

        bool outputConsolidatePhrTabandSynRule(
            vector< string >           &synRuleVec       ,
            vector< string >           &phraseTabScoreVec,
            vector< string >           &synRuleScoreVec  ,
            ofstream                   &ofile_output       );

    };

}

#endif
