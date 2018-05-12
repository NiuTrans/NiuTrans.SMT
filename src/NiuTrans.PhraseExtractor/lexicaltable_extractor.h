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
 * phrase extraction and scorer; LEX_generate_lexical_table.h
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 7/26/2011 add Stemming class for stemming method by Qiang Li.
 *
 */


#ifndef _LEXICALTABLE_EXTRACTOR_H_
#define _LEXICALTABLE_EXTRACTOR_H_

#include <map>
#include <set>
#include <utility>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include "basic_method.h"
#include "multi_thread.h"
#include "lock.h"

using namespace std;
using namespace lock;
using namespace multi_thread;
using namespace basic_method;

//#define __DEBUGLEXTAB__
namespace lexicaltable_extractor
{

    class OptionsOfLexTable
    {
    public:
        string sourceLangFileName ;
        string targetLangFileName ;
        string alignedInfoFileName;
        string lexTabS2TFileName  ;
        string lexTabT2SFileName  ;

        int nthread;
    };

    class Stemming:public BasicMethod
    {
    public:
        map< string, pair< string, double > > stemmingList;
        map< string, double > stemmed;
        map< string, map< string, double > > wordTranslationStem;
        map< string, double > totalDestLangStem;
    };


    class GeneLexTabParam{
    public:
        string lineOfSourceLang;
        string lineOfTargetLang;
        string lineOfAlignedInfo;
        size_t threadId;
        size_t lineNo;
        int    nthread;

        GeneLexTabParam()
            : lineOfSourceLang(),
              lineOfTargetLang(),
              lineOfAlignedInfo(),
              threadId( 0 ),
              lineNo( 0 ),
              nthread( 0 ){}

        ~GeneLexTabParam(){}
    };

    class GenerateLexicalTable:public BasicMethod, public MultiThread
    {
    public:
        GeneLexTabParam gltp;

        GenerateLexicalTable():gltp(){}

        ~GenerateLexicalTable(){}
        Stemming stem;

        bool generateLexicalTable( OptionsOfLexTable &options );
        bool generateLexicalTable( OptionsOfLexTable &options         , 
                                   string            &stemmingFileName  );

        bool readLinesFromCorpusNormal( ifstream &infileSourceLang , 
                                        ifstream &infileTargetLang , 
                                        ifstream &infileAlignedInfo, 
                                        OptionsOfLexTable &options   );
        bool statisticalWordCountNormal( string &lineOfSourceLang , 
                                         string &lineOfTargetLang , 
                                         string &lineOfAlignedInfo, 
                                         size_t &lineNo             );
        bool outputLexicalTableNormal( ofstream &outfileLexS2D, 
                                       ofstream &outfileLexD2S  );


        bool readLinesFromCorpusMultiThread( ifstream &infileSourceLang , 
                                             ifstream &infileTargetLang ,
                                             ifstream &infileAlignedInfo,
                                             OptionsOfLexTable &options   );
        bool statisticalWordCountMultiThread( GeneLexTabParam &gltp );
        bool outputLexicalTableMultiThread( ofstream &outfileLexS2D, 
                                            ofstream &outfileLexD2S  );
        void Run();

    private:
        map< string, double > totalSourceLang;
        map< string, double > totalDestLang;
        map< string, map< string, double > > wordTranslation;

        static map< string, double > globalTotalSourceLang;
        static map< string, double > globalTotalTargetLang;
        static map< string, map< string, double > > globalWordTranslation;

        static Mutex g_Lock;  

    };

}

#endif
