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
 * phrase extraction and scorer; PE_word_alignment.h
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2012/09/24 bug fixes
 *
 */


#ifndef _WORD_ALIGNMENT_H_
#define _WORD_ALIGNMENT_H_

#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "basic_method.h"
#include "n_ary_tree.h"

using namespace std;
using namespace basic_method;
using namespace n_ary_tree;


namespace word_alignment
{

    class TargetLangAndAlignedInfo
    {
    public:
        vector< string >            destLanguage;                // store dest words
        vector< vector< size_t > >  alignedToDestLanguage;
        vector< size_t >            alignedCountDestLang;
        TargetLangAndAlignedInfo(){}
    };

    class SourceLangAndAlignedInfo
    {
    public:
        vector< string >            sourceLanguage;              // store source words
        vector< vector< size_t > >  alignedToSourceLanguage;
        vector< size_t >            alignedCountSourceLang;
    };

    class WordAlignment:public BasicMethod
    {
    public:
        TargetLangAndAlignedInfo    destLangAndAlignedInfo  ;    // dest language and aligned information store structure
        SourceLangAndAlignedInfo    sourceLangAndAlignedInfo;    // source language and aligned information store structure
        Tree srcParseTree;
        Tree tgtParseTree;

        WordAlignment(){};

        bool createWordAlignment( 
                const string &targetLangSentence,
                const string &sourceLangSentence,
                const string &wordAlignmentStoD ,
                const size_t &sentenceLineNo    ,
                const Tree   &treeofSrc         ,
                const Tree   &treeofTgt           );

        bool createWordAlignment( 
                const string &targetLangSentence,
                const string &sourceLangSentence,
                const string &wordAlignmentStoD ,
                const size_t &sentenceLineNo    ,
                const Tree   &tree              ,
                const bool   &treedir             );

        bool createWordAlignment(
                const string &targetLangSentence, 
                const string &sourceLangSentence, 
                const string &wordAlignmentStoD , 
                const size_t &sentenceLineNo      );

    private:
        bool createTargetLanguage( const string &destLangSentence, const size_t &sentenceLineNo );
        bool createTargetLanguage( const string &destLangSentence, const size_t &sentenceLineNo, const Tree &tree );

        bool createSourceLanguage( const string &sourceLangSentence, const size_t &sentenceLineNo );
        bool createSourceLanguage( const string &sourceLangSentence, const size_t &sentenceLineNo, const Tree &tree );

        bool createAlignedInformation(
                const string &wordAligmentStoD, 
                const size_t &sentenceLineNo    );
    };

}

#endif
