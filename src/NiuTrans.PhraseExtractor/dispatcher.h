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
 * phrase extraction and scorer; dispatcher.h
 *
 * $Version:
 * 0.4.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 # 2012/09/17, add namespace dispatcher
 * 2012/05/22, add scoring for syntax-rule, modified by Qiang Li.
 * 2011/11/15, add this file, modified by Qiang Li.
 *
 */

#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <map>
#include "basic_method.h"
#include "word_alignment.h"
#include "ruletable_scorer.h"
#include "lexicaltable_extractor.h"
#include "phrasetable_extractor.h"
#include "hierarchyrule_extractor.h"
#include "ruletable_filter.h"
#include "syntaxrule_scorer.h"

using namespace std;
using namespace hierarchyrule_extractor;
using namespace lexicaltable_extractor;
using namespace phrasetable_extractor;
using namespace ruletable_scorer;
using namespace syntaxrule_scorer;
using namespace ruletable_filter;
using namespace basic_method;

//#define __DEBUG__

namespace dispatcher
{

    class Dispatcher
    {
    public:
        bool resolve_parameter( int argc, char * argv[] );

        bool identify_function( 
                string                &function, 
                map< string, string > &param   ,
                string                &sortFile,
                string                &del       );

        void generateLexicalTable_function(   map< string, string > &param, string &sortFile, string &del );
        void phraseExtract_function(          map< string, string > &param                                );
        void extractHieroRule_function(       map< string, string > &param                                );
        void score_function(                  map< string, string > &param, string &sortFile, string &del );
        void scoringforSynRule_function(      map< string, string > &param, string &sortFile, string &del );  // add in 2012-05-22, scoring for syntax-rule.
        void filterTableWithNum_function(     map< string, string > &param                                );
        void filterPhraseTabWithDev_function( map< string, string > &param                                );

        bool call_generateLexicalTable( 
                string            &lexFile      , 
                string            &stem         ,
                OptionsOfLexTable &options      ,
                SystemCommand     &systemCommand  );

        bool call_phraseExtract( OptionsOfPhraseTable &options );

        bool call_extractHieroRule( 
                string         &sourceFile   , 
                string         &destFile     , 
                string         &alignedFile  , 
                string         &extractResult, 
                OptionsOfHiero &options        );

        bool call_score( 
                string         &extractFile    , 
                string         &lexFile        , 
                string         &phraseTableFile, 
                bool           &inverseFlag    , 
                OptionsOfScore &options        ,
                string         &method         ,
                SystemCommand  &systemCommand    );

        bool call_scoringforSynRule( 
                map< string, string >      &param        ,
                OptionsOfScoringforSynRule &options      ,
                SystemCommand              &systemCommand  );

        bool call_consolidate( 
                string         &infileS2D    , 
                string         &infileD2S    , 
                string         &outfile      ,
                OptionsOfScore &options      ,    
                SystemCommand  &systemCommand  );

        bool call_filterTableWithNum( 
                const OptionsOfFilterNum &options );

        bool call_filterPhraseTabWithDev( 
                string &devSet     , 
                string &phraseTable, 
                string &filterTable, 
                int    &maxlen     , 
                int    &refnum       );

        // add in 2012-05-22, scoring for syntax-rule.
        bool changeSynRuletoPhraseTabFormat(
                string                     &rule   ,
                OptionsOfScoringforSynRule &options  );

        bool generScoringForPhraseTabParam( const map< string, string > &param         ,
                                                  map< string, string > &paramPhraseTab  );

    };

}

#endif
