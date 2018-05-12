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
 * phrase extraction and scorer; SC_phrase_table.h
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2011/6/24 add <cmath> header
 *
 *
 */


#ifndef _RULETABLE_SCORER_H_
#define _RULETABLE_SCORER_H_

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <utility>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "basic_method.h"

using namespace std;
using namespace basic_method;

namespace ruletable_scorer
{

    class LexicalTable:public BasicMethod
    {
    public:
        map< string, map< string, double > >        lexicalTable;    // the data structure of lexical table

        LexicalTable(){}
        bool loadLexicalTable( const string &fileName );
    };

    class PhraseAlignment:public BasicMethod
    {
    public:
        vector< string > sourceLanguage;
        vector< vector< size_t > > alignedToSourceLang;

        vector< string > destLanguage;
        vector< vector< size_t > > alignedToDestLang;

        string rootLabel;
        double frequency;
        double lexicalWeighting;

        PhraseAlignment():rootLabel(), frequency( 0 ), lexicalWeighting( 0 ){}

        bool create( const string &line, const size_t &lineNo );
    };

    class LexicalWeight
    {
    public:
        double lexicalWeight;
        LexicalWeight():lexicalWeight( 0 ){}
        bool caculateLexicalWeight( PhraseAlignment &pa, LexicalTable &lt, string &method );
    };

    class OptionsOfScore
    {
    public:
        int  cutoffInit;
        int  cutoffHiero;
        
        bool printFreqFlag;
        bool printAlignFlag;
        bool generInvFileFlag;

        OptionsOfScore( int &newCutoffInit       , 
                        int &newCutoffHiero      , 
                        bool &newPrintFreqFlag   , 
                        bool &newPrintAlignFlag  , 
                        bool &newGenerInvFileFlag  )
            : cutoffInit(       newCutoffInit       ), 
              cutoffHiero(      newCutoffHiero      ), 
              printFreqFlag(    newPrintFreqFlag    ), 
              printAlignFlag(   newPrintAlignFlag   ),
              generInvFileFlag( newGenerInvFileFlag ){}

        OptionsOfScore(): cutoffInit(       0     ), 
                          cutoffHiero(      0     ), 
                          printFreqFlag(    false ), 
                          printAlignFlag(   false ), 
                          generInvFileFlag( false ){}
    };


    class ScoreClassifyNum
    {
    public:
        int hieroRuleNum  ;
        int initialRuleNum;
        int nullRuleNum   ;

        ScoreClassifyNum()
            : hieroRuleNum(   0 ), 
              initialRuleNum( 0 ), 
              nullRuleNum(    0 ){};

        ScoreClassifyNum( int &newHieroRuleNum  , 
                          int &newInitialRuleNum, 
                          int &newNullRuleNum     )
            : hieroRuleNum(   newHieroRuleNum   ), 
              initialRuleNum( newInitialRuleNum ), 
              nullRuleNum(    newNullRuleNum    ){}
    };

    class PhraseTableAtom
    {
    public:
        string sourceLang         ;
        string destLang           ;
        string alignedToSourceLang;
        string alignedToDestLang  ;
        string rootLabel          ;
        double maxFrequency       ;
        double frequency          ;
        double rateOfFrequency    ;
        double lexicalWeight      ;

        PhraseTableAtom():frequency(       0 ), 
                          maxFrequency(    0 ), 
                          rateOfFrequency( 0 ), 
                          lexicalWeight(   0 ){}
        PhraseTableAtom( 
                string &sourceLang         , 
                string &destLang           , 
                string &alignedToSourceLang, 
                string &alignedToDestLang  , 
                string &rootLabel          ,
                double &maxFrequency       , 
                double &frequency          ,
                double &rateOfFrequency    , 
                double &lexicalWeight        )
        {
            this->sourceLang          = sourceLang         ;
            this->destLang            = destLang           ;
            this->alignedToSourceLang = alignedToSourceLang;
            this->alignedToDestLang   = alignedToDestLang  ;
            this->rootLabel           = rootLabel          ;
            this->maxFrequency        = maxFrequency       ;
            this->frequency           = frequency          ;
            this->rateOfFrequency     = rateOfFrequency    ;
            this->lexicalWeight       = lexicalWeight      ;
        }
    };

    class PhraseTable: public BasicMethod
    {
    public:
        typedef string::size_type STRPOS;
        vector< PhraseTableAtom > phraseTable;
        vector< PhraseTableAtom > phraseTableCutOff;
        map< pair< string, string >, PhraseTableAtom > finalPhraseTable;

        double totalFrequency;

        PhraseTable():totalFrequency( 0 ){}
        bool generatePhraseTable( PhraseAlignment  &pa              , 
                                  bool             &lastline        , 
                                  ofstream         &outfile         , 
                                  bool             &inverseFlag     , 
                                  OptionsOfScore   &options         , 
                                  ScoreClassifyNum &scoreClassifyNum  );
    private:
        bool output( ofstream         &outfile         , 
                     bool             &inverseFlag     , 
                     OptionsOfScore   &options         , 
                     ScoreClassifyNum &scoreClassifyNum  );
    };

    class ConsolidateClassifyNum
    {
    public:
        int hieroRuleNum  ;
        int initialRuleNum;
        int nullRuleNum   ;

        ConsolidateClassifyNum()
            : hieroRuleNum(   0 ), 
            initialRuleNum(   0 ), 
            nullRuleNum(      0 ){};
        ConsolidateClassifyNum( int &newHieroRuleNum  , 
                                int &newInitialRuleNum, 
                                int &newNullRuleNum     )
            : hieroRuleNum(   newHieroRuleNum   ), 
              initialRuleNum( newInitialRuleNum ), 
              nullRuleNum(    newNullRuleNum    ){}
    };

    class Consolidate: public BasicMethod
    {
    public:
        bool consolidate( string                 &phraseTableName       , 
                          string                 &phraseTableInvName    , 
                          string                 &scoreTableFile        , 
                          OptionsOfScore         &options               , 
                          ConsolidateClassifyNum &consolidateClassifyNum  );
    private:
        PhraseTableAtom ptaOfPhraseTable;
        PhraseTableAtom ptaOfPhraseTableInv;

        PhraseTableAtom extractPhraseTable( string         &line    , 
                                            string         &fileName, 
                                            OptionsOfScore &options , 
                                            size_t         &lineNo    );
    };

}

#endif
