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
 * phrase extraction and scorer; PE_word_alignment.cpp
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


#include "word_alignment.h"
using namespace word_alignment;


bool WordAlignment::createTargetLanguage( const string &destLangSentence, const size_t &sentenceLineNo )
{
    if ( destLangSentence == "" )
    {
        cerr<<"\n"
            <<"        WARNING : NULL sentence in "<<sentenceLineNo<<" line in target file."<<"\n";
        return false;
    }
    if ( split( destLangSentence, ' ', destLangAndAlignedInfo.destLanguage ) )
        return true;
    else 
        return false;
}

bool WordAlignment::createTargetLanguage( const string &destLangSentence, const size_t &sentenceLineNo, const Tree &tree )
{
    if ( destLangSentence == "" )
    {
        cerr<<"\n"
            <<"        WARNING : NULL sentence in "<<sentenceLineNo<<" line in target file."<<"\n";
        return false;
    }
    if ( split( destLangSentence, ' ', destLangAndAlignedInfo.destLanguage ) )
    {
        if( destLangAndAlignedInfo.destLanguage.size() != tree.leaf.size() )
            return false;
        else
        {
            vector< string >::iterator iterDestLang = destLangAndAlignedInfo.destLanguage.begin();
            for( map< size_t, TreeNode >::const_iterator iter = tree.leaf.begin(); iter != tree.leaf.end(); ++iter )
            {
                string leaf = iter->second.lable;
                toLower( leaf );
                if( leaf != *iterDestLang )
                    return false;
                ++iterDestLang;
            }
            return true;
        }
    }
    else 
        return false;
}

bool WordAlignment::createSourceLanguage( const string &sourceLangSentence, const size_t &sentenceLineNo )
{
    if ( sourceLangSentence == "" )
    {
        cerr<<"\n"
            <<"        WARNING : NULL sentence in "<<sentenceLineNo<<" line in source file."<<"\n";
        return false;
    }
    if( split( sourceLangSentence, ' ', sourceLangAndAlignedInfo.sourceLanguage ) )
        return true;
    else
        return false;
}

bool WordAlignment::createSourceLanguage( const string &sourceLangSentence, const size_t &sentenceLineNo, const Tree &tree )
{
    if ( sourceLangSentence == "" )
    {
        cerr<<"\n"
            <<"        WARNING : NULL sentence in "<<sentenceLineNo<<" line in source file."<<"\n";
        return false;
    }
    if( split( sourceLangSentence, ' ', sourceLangAndAlignedInfo.sourceLanguage ) )
    {
        if( sourceLangAndAlignedInfo.sourceLanguage.size() != tree.leaf.size() )
            return false;
        else
        {
            vector< string >::iterator iterSourceLang = sourceLangAndAlignedInfo.sourceLanguage.begin();
            for( map< size_t, TreeNode >::const_iterator iter = tree.leaf.begin(); iter != tree.leaf.end(); ++iter )
            {
                string leaf = iter->second.lable;
                //toLower( leaf );
                if( leaf != *iterSourceLang )
                    return false;
                ++iterSourceLang;
            }
        }
        return true;
    }
    else
        return false;
}

bool WordAlignment::createAlignedInformation(
const string &wordAlignmentStoD, 
const size_t &sentenceLineNo )
{
    if( wordAlignmentStoD == "" )
    {
        cerr<<"\n"
            <<"        WARNING : NULL sentence in "<<sentenceLineNo<<" line in aligned file."<<"\n";
        return false;
    }
    vector< string > alignmentTable;
    if( !split( wordAlignmentStoD, ' ', alignmentTable ) )
        return false;
    // init alignedCountSourceLang
    sourceLangAndAlignedInfo.alignedCountSourceLang.assign( sourceLangAndAlignedInfo.sourceLanguage.size(), 0 );
    // init alignedCountDestLang
    destLangAndAlignedInfo.alignedCountDestLang.assign( destLangAndAlignedInfo.destLanguage.size(), 0 );
    // init alignedToSourceLanguage
    sourceLangAndAlignedInfo.alignedToSourceLanguage.assign( sourceLangAndAlignedInfo.sourceLanguage.size(), vector< size_t >() );
    // init alignedToDestLanguage
    destLangAndAlignedInfo.alignedToDestLanguage.assign( destLangAndAlignedInfo.destLanguage.size(), vector< size_t >() );

    for( vector< string >::iterator iter_alignmentTable = alignmentTable.begin(); 
         iter_alignmentTable != alignmentTable.end(); ++iter_alignmentTable )
    {
        unsigned int sourceWordNo, destWordNo;

#ifdef WIN32
        if( ! sscanf_s( iter_alignmentTable->c_str(), "%d-%d", &sourceWordNo, &destWordNo ) )
#else
        if( ! sscanf( iter_alignmentTable->c_str(), "%u-%u", &sourceWordNo, &destWordNo ) )
#endif
        {
            cerr<<"\n"
                <<"        WARNING : "<<*iter_alignmentTable<<" is not num-num in line:"<<sentenceLineNo<<endl;
            return false;
        }
        if( sourceWordNo >= sourceLangAndAlignedInfo.sourceLanguage.size() || destWordNo >= destLangAndAlignedInfo.destLanguage.size() )
        {
            cerr<<"\n"
                <<"        WARNING : "<<"out of bound in "<<sentenceLineNo<<" line.\n"
                <<"                  srcLang.size:"<<sourceLangAndAlignedInfo.sourceLanguage.size()<<"\tnow position:"     <<sourceWordNo
                <<" (valid: 0-"          <<sourceLangAndAlignedInfo.sourceLanguage.size()-1  <<")\n"
                <<"                  tgtLang.size:"  <<destLangAndAlignedInfo.destLanguage.size()<<"\tnow position:"     <<destWordNo
                <<" (valid: 0-"          <<destLangAndAlignedInfo.destLanguage.size()-1      <<")\n"<<flush;
            return false;
        }
        ++ sourceLangAndAlignedInfo.alignedCountSourceLang[ sourceWordNo ];
        ++ destLangAndAlignedInfo.alignedCountDestLang[ destWordNo ];
        sourceLangAndAlignedInfo.alignedToSourceLanguage[ sourceWordNo ].push_back( destWordNo );
        destLangAndAlignedInfo.alignedToDestLanguage[ destWordNo ].push_back( sourceWordNo );
    }
    return true;
}

bool WordAlignment::createWordAlignment(
const string &targetLangSentence, 
const string &sourceLangSentence, 
const string &wordAlignmentStoD, 
const size_t &sentenceLineNo )
{
    if( createTargetLanguage( targetLangSentence, sentenceLineNo ) &&
        createSourceLanguage( sourceLangSentence, sentenceLineNo ) &&
        createAlignedInformation( wordAlignmentStoD, sentenceLineNo ) )
        return true;
    else 
        return false;
}

bool WordAlignment::createWordAlignment(
const string &targetLangSentence, 
const string &sourceLangSentence, 
const string &wordAlignmentStoD, 
const size_t &sentenceLineNo,
const Tree &tree,
const bool &treedir )
{
    if( treedir )
    {
        tgtParseTree = tree;
        if( createTargetLanguage( targetLangSentence, sentenceLineNo, tree ) &&
            createSourceLanguage( sourceLangSentence, sentenceLineNo ) &&
            createAlignedInformation( wordAlignmentStoD, sentenceLineNo ) )
            return true;
        else 
            return false;    
    }
    if( !treedir )
    {
        srcParseTree = tree;
        if( createTargetLanguage( targetLangSentence, sentenceLineNo ) &&
            createSourceLanguage( sourceLangSentence, sentenceLineNo, tree ) &&
            createAlignedInformation( wordAlignmentStoD, sentenceLineNo ) )
            return true;
        else 
            return false;
    }
    return false;
}

bool WordAlignment::createWordAlignment(
const string &targetLangSentence, 
const string &sourceLangSentence, 
const string &wordAlignmentStoD, 
const size_t &sentenceLineNo,
const Tree &treeofSrc,
const Tree &treeofTgt )
{
    srcParseTree = treeofSrc;
    tgtParseTree = treeofTgt;

    if( createTargetLanguage( targetLangSentence, sentenceLineNo, treeofTgt ) &&
        createSourceLanguage( sourceLangSentence, sentenceLineNo, treeofSrc ) &&
        createAlignedInformation( wordAlignmentStoD, sentenceLineNo ) )
        return true;
    else 
        return false;    
}


