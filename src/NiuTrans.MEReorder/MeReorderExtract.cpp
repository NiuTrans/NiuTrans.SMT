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
 * Maxent reorder model trainer; MeReorderExtract.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); July 21th, 2011; fix a bug about parse format (i.e., original program is not able to parse something lik "(NN NN)")
 * Hao Zhang (email: zhanghao1216@gmail.com); July 20th, 2011; fix a bug about parse format (i.e., original program is not able to parse something lik "(NN abc()def)")
 * Hao Zhang (email: zhanghao1216@gmail.com); July 13th, 2011; fix a bug about parse format (i.e., "( () )" when parsing failed) in function "m_meSampleExtract()"
 * Hao Zhang (email: zhanghao1216@gmail.com); June 24th, 2011; fix a bug caused by variable "mp_stSrcParseFileHnd" initialization
 * Hao Zhang (email: zhanghao1216@gmail.com); May 12th, 2011
 *
 */


#include "MeReorderExtract.h"
#include "../NiuTrans.Base/Utilities.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

using namespace util;

namespace smt {

    /*
     *
     * Aligned Sentence-Pair
     *
     */
    algn_sent_c::algn_sent_c() {
    }

    algn_sent_c::~algn_sent_c() {
    }

    bool algn_sent_c::m_init( char* p_szSrcSent, char* p_szTgtSent, char* p_szAlgnments, long lSentId ) {

        char** pp_szAlgnItems;
        long lAlgnItemCnt;
        /* tokenize input sentence-pairs and alignments */
        m_lSrcTokCnt = string_c::ms_split( p_szSrcSent, " ", mpp_szSrcToks );
        m_lTgtTokCnt = string_c::ms_split( p_szTgtSent, " ", mpp_szTgtToks );
        lAlgnItemCnt = string_c::ms_split( p_szAlgnments, " ", pp_szAlgnItems );
        if( m_lSrcTokCnt == 0 || m_lTgtTokCnt == 0 ) {
            fprintf( stderr, "[WARNING]: wrong format in %ld sentence! (SRC: %ld words   TGT: %ld words)\n", \
                lSentId, m_lSrcTokCnt, m_lTgtTokCnt );
            string_c::ms_freeStrArray( mpp_szSrcToks, m_lSrcTokCnt );
            string_c::ms_freeStrArray( mpp_szTgtToks, m_lTgtTokCnt );
            string_c::ms_freeStrArray( pp_szAlgnItems, lAlgnItemCnt );
            return false;
        }
        /* allocate memory for source-side-alignment-count-array and target-side-alignment-list-array */
        mp_lSrc2TgtCnt = new long[m_lSrcTokCnt];
        memset( mp_lSrc2TgtCnt, 0, m_lSrcTokCnt * sizeof( long ) );
        mp_lTgt2SrcCnt = new long[m_lTgtTokCnt];
        memset( mp_lTgt2SrcCnt, 0, m_lTgtTokCnt * sizeof( long ) );
        mpp_csSrc2TgtIdxLst = new list_c*[m_lSrcTokCnt];
        for( long i=0; i < m_lSrcTokCnt; i++ ) {
            mpp_csSrc2TgtIdxLst[i] = new list_c();
        }
        mpp_csTgt2SrcIdxLst = new list_c*[m_lTgtTokCnt];
        for( long i=0; i < m_lTgtTokCnt; i++ ) {
            mpp_csTgt2SrcIdxLst[i] = new list_c();
        }
        /* parse info of sentence-pair alignments */
        for( long i=0; i < lAlgnItemCnt; i++ ) {
            long sid, tid;
            if( !sscanf( pp_szAlgnItems[i], "%ld-%ld", &sid, &tid ) ) {
                fprintf( stderr, "[WARNING]: wrong format in %ld sentence alignment! (ALGN: %s, index not match)\n", \
                    lSentId, pp_szAlgnItems[i] );
                string_c::ms_freeStrArray( pp_szAlgnItems, lAlgnItemCnt );
                m_clear();
                return false;
            }
            if( sid < 0 || sid >= m_lSrcTokCnt || tid < 0 || tid >= m_lTgtTokCnt ) {
                fprintf( stderr, "[WARNING]: wrong format in %ld sentence alignment! (ALGN: %s, out of bounds)\n", \
                    lSentId, pp_szAlgnItems[i] );
                string_c::ms_freeStrArray( pp_szAlgnItems, lAlgnItemCnt );
                m_clear();
                return false;
            }
            /* record counts of alignment points for tokens on source or target sides */
            ++mp_lSrc2TgtCnt[sid];
            ++mp_lTgt2SrcCnt[tid];
            /* append indices of alignment points to tokens' index list on source and target sides */
            mpp_csTgt2SrcIdxLst[tid]->m_addInt( sid );
            mpp_csSrc2TgtIdxLst[sid]->m_addInt( tid );
        }
        string_c::ms_freeStrArray( pp_szAlgnItems, lAlgnItemCnt );
        return true;

    }

    void algn_sent_c::m_clear() {

        string_c::ms_freeStrArray( mpp_szSrcToks, m_lSrcTokCnt );
        string_c::ms_freeStrArray( mpp_szTgtToks, m_lTgtTokCnt );
        delete[] mp_lSrc2TgtCnt;
        delete[] mp_lTgt2SrcCnt;
        for( long i=0; i < m_lSrcTokCnt; i++ ) {
            delete mpp_csSrc2TgtIdxLst[i];
        }
        delete[] mpp_csSrc2TgtIdxLst;
        for( long i=0; i < m_lTgtTokCnt; i++ ) {
            delete mpp_csTgt2SrcIdxLst[i];
        }
        delete[] mpp_csTgt2SrcIdxLst;

    }

    /*
     * Parse Tree Node
     */
    parse_node_c::parse_node_c() {

        this->m_lNodeBlockBeg = -1;
        this->m_lNodeBlockEnd = -1;
        this->m_lNodeLevel = -1;
        this->mp_szNodeNonTermsStack = NULL;
        this->mp_szNodeNonTerm = NULL;
        this->mp_szNodeHeadWord = NULL;
        this->mp_csNodeParent = NULL;
        this->mp_csNodeSonList = new list_c();

    }

    parse_node_c::~parse_node_c() {

        if( mp_szNodeNonTermsStack ) {
            delete[] mp_szNodeNonTermsStack;
        }
        if( mp_szNodeNonTerm ) {
            delete[] mp_szNodeNonTerm;
        }
        if( mp_szNodeHeadWord ) {
            delete[] mp_szNodeHeadWord;
        }
        delete mp_csNodeSonList;

    }

    /*
     *
     * Berkeley Parse Tree Reader
     *
     */
    berk_parse_tree_c::berk_parse_tree_c( long lMyDim, bool bMySm ) {

        this->mppp_csParseChart = new parse_node_c**[lMyDim];
        for( long i=0; i < lMyDim; i++ ) {
            this->mppp_csParseChart[i] = new parse_node_c*[lMyDim + 1];
            memset( this->mppp_csParseChart[i], 0, ( lMyDim + 1 ) * sizeof( parse_node_c* ) );
        }
        this->m_lDimension = lMyDim;
        this->m_bSimpleMode = bMySm;

    }

    berk_parse_tree_c::~berk_parse_tree_c() {

        for( long i=0; i < m_lDimension; i++ ) {
            for( long j=0; j < m_lDimension + 1; j++ ) {
                if( mppp_csParseChart[i][j] != NULL ) {
                    delete mppp_csParseChart[i][j];
                }
            }
            delete[] mppp_csParseChart[i];
        }
        delete[] mppp_csParseChart;

    }

    bool berk_parse_tree_c::m_initParseChart( char* p_szTreeStr, bool baddDelim ) {

        /* ( (XXX) ) => (XXX) */
        if( p_szTreeStr[0] == '(' && p_szTreeStr[1] == ' ' && p_szTreeStr[2] == '(' ) {
            /* GB2312 encoding */
            string_c::ms_mvStr( p_szTreeStr, 2, true );
            string_c::ms_mvStr( p_szTreeStr, 1, false );
        }
        else if( ((unsigned char)p_szTreeStr[0]) == 0xEF && ((unsigned char)p_szTreeStr[1]) == 0xBB \
            && ((unsigned char)p_szTreeStr[2]) == 0xBF ) {
            /* UTF8 encoding */
            string_c::ms_mvStr( p_szTreeStr, 5, true );
            string_c::ms_mvStr( p_szTreeStr, 1, false );
        }
        /* (XXX) => ( XXX ) */
        long lStrLen = strlen( p_szTreeStr );
        char* p_szNewTs = new char[lStrLen * 3 + 1];
        if( baddDelim ) {
            long j = 0;
            for( long i=0; i < lStrLen; i++ ) {
                if( p_szTreeStr[i] == '(' || p_szTreeStr[i] == ')' ) {
                    p_szNewTs[j] = ' ';
                    p_szNewTs[j+1] = p_szTreeStr[i];
                    p_szNewTs[j+2] = ' ';
                    j += 3;
                }
                else {
                    p_szNewTs[j] = p_szTreeStr[i];
                    ++j;
                }
            }
            p_szNewTs[j] = '\0';
        }
        else {
            strcpy( p_szNewTs, p_szTreeStr );
        }
        /* trim */
        long idx = strlen( p_szNewTs ) - 1;
        while( idx >= 0 && p_szNewTs[idx] != ')' ) {
            p_szNewTs[idx] = '\0';
            --idx;
        }
        char** pp_szToks;
        long lTokCnt = string_c::ms_splitPlus( p_szNewTs, " ", pp_szToks );
        long lTokIdx = 0;
        long lWdIdx = 0;
        parse_node_c* p_csTree = m_buildParseTree( NULL, pp_szToks, lTokCnt, lTokIdx, lWdIdx );
        delete[] pp_szToks;
        delete[] p_szNewTs;
        return ( p_csTree == NULL ? false : true );

    }

    void berk_parse_tree_c::m_clearParseChart() {

        for( long i=0; i < m_lDimension; i++ ) {
            for( long j=0; j < m_lDimension + 1; j++ ) {
                if( mppp_csParseChart[i][j] != NULL ) {
                    delete mppp_csParseChart[i][j];
                    mppp_csParseChart[i][j] = NULL;
                }
            }
        }

    }

    parse_node_c* berk_parse_tree_c::m_buildParseTree( parse_node_c* p_csNodeParent, char** pp_szToks, long lTokCnt, \
        long& lTokIdx, long& lWdIdx) {

        parse_node_c* p_csNode = new parse_node_c();
        p_csNode->m_lNodeLevel = ( p_csNodeParent == NULL ? 0 : p_csNodeParent->m_lNodeLevel + 1 );
        p_csNode->mp_csNodeParent = p_csNodeParent;

        if( strcmp( pp_szToks[lTokIdx], "(" ) != 0 ) {
            delete p_csNode;
            return NULL;
        }
        else {
            delete[] pp_szToks[lTokIdx];    /* "(" */
        }
        ++lTokIdx;
        if( !m_bSimpleMode ) {
            p_csNode->mp_szNodeNonTerm = pp_szToks[lTokIdx];        /* "POS TAG" */
            p_csNode->mp_szNodeNonTermsStack = new char[strlen( pp_szToks[lTokIdx] ) + 1];
            strcpy( p_csNode->mp_szNodeNonTermsStack, pp_szToks[lTokIdx] );
        }
        else {
            delete[] pp_szToks[lTokIdx];
        }
        ++lTokIdx;
        /* leaf node */
        if( strcmp( pp_szToks[lTokIdx], "(" ) != 0 || ( lTokIdx + 1 < lTokCnt && !strcmp( pp_szToks[lTokIdx+1], ")" ) ) ) {
            p_csNode->m_lNodeBlockBeg = lWdIdx;
            p_csNode->m_lNodeBlockEnd = ++lWdIdx;
            if( !m_bSimpleMode ) {
                p_csNode->mp_szNodeHeadWord = pp_szToks[lTokIdx];   /* "Head Word" */
            }
            else {
                delete[] pp_szToks[lTokIdx];
            }
            ++lTokIdx;
            if( strcmp( pp_szToks[lTokIdx], ")" ) != 0 ) {
                delete p_csNode;
                return NULL;
            }
            else {
                delete[] pp_szToks[lTokIdx];    /* ")" */
            }
            ++lTokIdx;
            mppp_csParseChart[p_csNode->m_lNodeBlockBeg][p_csNode->m_lNodeBlockEnd] = p_csNode;
            return p_csNode;
        }
        /* intermediate node */
        p_csNode->m_lNodeBlockBeg = lWdIdx;
        while( 1 ) {
            parse_node_c* p_csSubTree = m_buildParseTree( p_csNode, pp_szToks, lTokCnt, lTokIdx, lWdIdx );  /* "Head Word" */
            if( !p_csSubTree || lTokIdx >= lTokCnt ) {  /* parse sub-tree error or tree's brackets "(" and ")" unmatch */
                delete p_csNode;
                return NULL;
            }
            p_csNode->mp_csNodeSonList->m_addObject( p_csSubTree );
            if( !m_bSimpleMode ) {
                if( p_csNode->mp_szNodeHeadWord == NULL ) {
                    p_csNode->mp_szNodeHeadWord = new char[strlen( p_csSubTree->mp_szNodeHeadWord ) + 1];
                    strcpy( p_csNode->mp_szNodeHeadWord, p_csSubTree->mp_szNodeHeadWord );
                }
                else {
                    char* p_szBuff = new char[strlen( p_csNode->mp_szNodeHeadWord ) + strlen( p_csSubTree->mp_szNodeHeadWord ) + 2];
                    sprintf( p_szBuff, "%s %s", p_csNode->mp_szNodeHeadWord, p_csSubTree->mp_szNodeHeadWord );
                    delete[] p_csNode->mp_szNodeHeadWord;
                    p_csNode->mp_szNodeHeadWord = p_szBuff;
                }
            }
            /* if it encounters a right bracket ")", it means the end of reading the parse tree */
            if( !strcmp( pp_szToks[lTokIdx], ")" ) ) {
                delete[] pp_szToks[lTokIdx];    /* ")" */
                ++lTokIdx;
                break;
            }
            /* else the coming token must be left bracket "(", then it means to read another sub-tree */
        }
        p_csNode->m_lNodeBlockEnd = lWdIdx;
        if( mppp_csParseChart[p_csNode->m_lNodeBlockBeg][p_csNode->m_lNodeBlockEnd] == NULL ) {
            /* non-unary subtrees */
            mppp_csParseChart[p_csNode->m_lNodeBlockBeg][p_csNode->m_lNodeBlockEnd] = p_csNode;
        }
        else {
            /* unary subtrees */
            parse_node_c* p_csChildNode = mppp_csParseChart[p_csNode->m_lNodeBlockBeg][p_csNode->m_lNodeBlockEnd];
            if( !m_bSimpleMode ) {
                char* p_szBuff = new char[strlen( p_csNode->mp_szNodeNonTerm ) + strlen( p_csChildNode->mp_szNodeNonTermsStack ) + 2];
                sprintf( p_szBuff, "%s:%s", p_csNode->mp_szNodeNonTerm, p_csChildNode->mp_szNodeNonTermsStack );
                delete[] p_csChildNode->mp_szNodeNonTermsStack;
                p_csChildNode->mp_szNodeNonTermsStack = p_szBuff;
            }
            p_csChildNode->mp_csNodeParent = p_csNode->mp_csNodeParent;
            p_csChildNode->m_lNodeLevel = p_csNode->m_lNodeLevel;
            delete p_csNode;
            p_csNode = p_csChildNode;
        }

        return p_csNode;

    }

    bool berk_parse_tree_c::m_isSatisfyTreeConstraint( long lLeftBlkBeg, long lLeftBlkEnd, long lRightBlkBeg, long lRightBlkEnd ) {

        return m_isSatisfyTreeConstraint( mppp_csParseChart[lLeftBlkBeg][lLeftBlkEnd], mppp_csParseChart[lRightBlkBeg][lRightBlkEnd] );

    }

    bool berk_parse_tree_c::m_isSatisfyTreeConstraint( parse_node_c* p_csLeftSubTree, parse_node_c* p_csRightSubTree ) {

        if( p_csLeftSubTree == NULL || p_csRightSubTree == NULL ) {
            return false;
        }
        if( p_csLeftSubTree->mp_csNodeParent != p_csRightSubTree->mp_csNodeParent ) {
            return false;
        }
        return true;

    }

    /*
     *
     * MaxEnt Reorder Model
     *
     */
    me_extract_c::me_extract_c( int argc, char* argv[] ) {

        this->mp_szSrcFilePath = NULL;
        this->mp_szTgtFilePath = NULL;
        this->mp_szAlgnFilePath = NULL;
        this->mp_szRltFilePath = NULL;
        this->m_bSrcParsePruning = false;
        this->mp_szSrcParseFilePath = NULL;
        this->mp_szTgtParseFilePath = NULL;
        this->m_lMaxSrcPhraseWordNum = 0x7fffffff;
        this->m_lMaxTgtPhraseWordNum = 0x7fffffff;
        this->m_lMaxTgtUnalgnWordNum = 0x7fffffff;
        this->m_lMaxExtractedSampleNum = 0x7fffffff;
        /* parse command-line user options */
        this->m_parseCmdLineOptions( argc, argv );
        if( !this->mp_szRltFilePath ) {
            this->mp_szRltFilePath = "PhrDistorSamples.txt";
        }
        this->mp_szStraightSampleMark = "STRAIGHT";
        this->mp_szInvertedSampleMark = "INVERTED";
        this->mp_szCharset = "UTF-8";
        this->m_lExtractedSampleNum = 0;

    }

    me_extract_c::~me_extract_c() {

        

    }

    void me_extract_c::m_extractInit() {

        if( !mp_szSrcFilePath || !mp_szTgtFilePath || !mp_szAlgnFilePath ) {
            m_usage();
            exit( 1 );
        }
        if( this->m_bSrcParsePruning == true ) {
            if( this->mp_szSrcParseFilePath == NULL ) {
                m_usage();
                exit( 1 );
            }
            else {
                this->mp_stSrcParseFileHnd = fopen( mp_szSrcParseFilePath, "r" );
                if( !mp_stSrcParseFileHnd ) {
                    fprintf( stderr, "[ERROR]: file %s not existed!\n", mp_szSrcParseFilePath );
                    exit( 1 );
                }
            }
        }
        else if( this->mp_szSrcParseFilePath != NULL ) {
            m_usage();
            exit( 1 );
        }
        else {
            this->mp_stSrcParseFileHnd = NULL;
        }
        this->mp_stSrcFileHnd = fopen( mp_szSrcFilePath, "r" );
        if( !mp_stSrcFileHnd ) {
            fprintf( stderr, "[ERROR]: file %s not existed!\n", mp_szSrcFilePath );
            exit( 1 );
        }
        this->mp_stTgtFileHnd = fopen( mp_szTgtFilePath, "r" );
        if( !mp_stTgtFileHnd ) {
            fprintf( stderr, "[ERROR]: file %s not existed!\n", mp_szTgtFilePath );
            exit( 1 );
        }
        this->mp_stAlgnFileHnd = fopen( mp_szAlgnFilePath, "r" );
        if( !mp_stAlgnFileHnd ) {
            fprintf( stderr, "[ERROR]: file %s not existed!\n", mp_szAlgnFilePath );
            exit( 1 );
        }
        this->mp_stRltFileHnd = fopen( mp_szRltFilePath, "w" );
        if( !mp_stRltFileHnd ) {
            fprintf( stderr, "[ERROR]: file %s not existed!\n", mp_szRltFilePath );
            exit( 1 );
        }

    }

    void me_extract_c::m_extractClear() {

        fclose( mp_stSrcFileHnd );
        fclose( mp_stTgtFileHnd );
        fclose( mp_stAlgnFileHnd );
        fclose( mp_stRltFileHnd );
        if( mp_stSrcParseFileHnd ) {
            fclose( mp_stSrcParseFileHnd );
        }

    }

    void me_extract_c::m_parseCmdLineOptions( int argc, char* argv[] ) {

        for( int i=0; i < argc; i++ ) {
            if( argv[i][0] == '-' ) {
                if( i == argc -1 || argv[i+1][0] == '-' ) {
                    if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "srcparsepruning" ) ) {
                        m_bSrcParsePruning = true;
                    }
                }
                else {
                    if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "src" ) ) {
                        mp_szSrcFilePath = argv[i+1];
                    }
                    else if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "tgt" ) ) {
                        mp_szTgtFilePath = argv[i+1];
                    }
                    else if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "algn" ) ) {
                        mp_szAlgnFilePath = argv[i+1];
                    }
                    else if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "output" ) ) {
                        mp_szRltFilePath = argv[i+1];
                    }
                    else if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "srcparse" ) ) {
                        mp_szSrcParseFilePath = argv[i+1];
                    }
                    else if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "maxsrcphrwdnum" ) ) {
                        m_lMaxSrcPhraseWordNum = atoi( argv[i+1] );
                    }
                    else if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "maxtgtphrwdnum" ) ) {
                        m_lMaxTgtPhraseWordNum = atoi( argv[i+1] );
                    }
                    else if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "maxtgtgapwdnum" ) ) {
                        m_lMaxTgtUnalgnWordNum = atoi( argv[i+1] );
                    }
                    else if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "tgtparse" ) ) {
                        mp_szTgtParseFilePath = argv[i+1];
                    }
                    else if( !strcmp( string_c::ms_toLowercase( &argv[i][1] ), "maxsamplenum" ) ) {
                        m_lMaxExtractedSampleNum = atol( argv[i+1] );
                    }
                    ++i;
                }
            }
        }

    }

    void me_extract_c::m_usage() {

        fprintf( stderr, "NiuTrans.MEReorder, version 0.0.1\n" );
        fprintf( stderr, "\n" );
        fprintf( stderr, "usage: NiuTrans.MEReorder [options]...\n" );
        fprintf( stderr, "\n" );
        fprintf( stderr, "       -src                       source-language file path\n" );
        fprintf( stderr, "       -tgt                       target-language file paht\n" );
        fprintf( stderr, "       -algn                      alignments file path\n" );
        fprintf( stderr, "       -output                    result file path, default \"PhrDistorSamples.txt\" [optional]\n" );
        fprintf( stderr, "       -srcParsePruning           impose source parse tree constraints for extracting samples [optional]\n" );
        fprintf( stderr, "                                  if it is specified, option \"-srcParse\" must be specified\n" );
        fprintf( stderr, "       -srcParse                  source-language parse trees file path\n" );
        fprintf( stderr, "                                  valid only when option \"-srcParsePruning\" is specified\n" );
        fprintf( stderr, "       -maxSrcPhrWdNum            maximum word number in a source-side phrase\n" );
        fprintf( stderr, "                                  default \"unlimited\" [optional]\n" );
        fprintf( stderr, "       -maxTgtPhrWdNum            maximum word number in a target-side phrase\n" );
        fprintf( stderr, "                                  default \"unlimited\" [optional]\n" );
        fprintf( stderr, "       -maxTgtGapWdNum            maximum unaligned word number on target-side between two blocks\n" );
        fprintf( stderr, "                                  default \"unlimited\" [optional]\n" );
        fprintf( stderr, "       -maxSampleNum              maximum number of extracted samples\n" );
        fprintf( stderr, "                                  default \"unlimited\" [optional]\n" );
        fprintf( stderr, "       -tgtParse                  target-language parse trees file path [optional]\n" );
        fprintf( stderr, "\n" );
        fprintf( stderr, "example:\n" );
        fprintf( stderr, "\n" );
        fprintf( stderr, "       NiuTrans.MEReorder -src c.txt -tgt e.txt -algn a.txt\n" );

    }

    void me_extract_c::m_meSampleExtract() {

        m_extractInit();
        time_c timer;
        timer.m_startTimer();
        fprintf( stderr, "start extracting maximum-entropy-based reordering samples...\n" );
        algn_sent_c* p_csAlgnSent = new algn_sent_c();
        long lSentNum = 0;
        long lErrorNum = 0;
        while( 1 ) {
            char* p_szSrcLine = string_c::ms_readLine( mp_stSrcFileHnd );
            char* p_szTgtLine = string_c::ms_readLine( mp_stTgtFileHnd );
            char* p_szAlgnLine = string_c::ms_readLine( mp_stAlgnFileHnd );
            if( !p_szSrcLine || !p_szTgtLine || !p_szAlgnLine ) {
                break;
            }
            char* p_szSrcParseLine = NULL;
            if( m_bSrcParsePruning ) {
                p_szSrcParseLine = string_c::ms_readLine( mp_stSrcParseFileHnd );
                if( !p_szSrcParseLine ) {
                    break;
                }
                if( lSentNum == 0 ) {
                    if( ((unsigned char)p_szSrcParseLine[0]) == 0xEF && ((unsigned char)p_szSrcParseLine[1]) == 0xBB \
                        && ((unsigned char)p_szSrcParseLine[2]) == 0xBF ) {
                        mp_szCharset = "UTF-8";
                    }
                    else if( p_szSrcParseLine[0] == '(' ) {
                        mp_szCharset = "GB2312";
                    }
                }
            }
            if( p_csAlgnSent->m_init( p_szSrcLine, p_szTgtLine, p_szAlgnLine, lSentNum + 1 ) ) {
                /* read source-side parse tree */
                bool bParseTreeGood = true;
                berk_parse_tree_c* p_csBerkTree = NULL;
                if( m_bSrcParsePruning ) {
                    p_csBerkTree = new berk_parse_tree_c( p_csAlgnSent->m_lSrcTokCnt, true );
                    if( !strcmp( p_szSrcParseLine, "(())" ) || !strcmp( p_szSrcParseLine, "( () )") ) { /* empty parse tree */
                        ++lErrorNum;
                        bParseTreeGood = false;
                    }
                    else {
                        /* use string to refine parse tree */
                        char* p_szRefinedParseTree = m_refineParseTree( p_szSrcLine, p_szSrcParseLine );
                        if( !p_szRefinedParseTree || !p_csBerkTree->m_initParseChart( p_szRefinedParseTree, false ) ) {
                            ++lErrorNum;
                            bParseTreeGood = false;
                        }
                        delete[] p_szRefinedParseTree;
                    }
                }
                /* parse tree is ok, now extract samples */
                if( bParseTreeGood == true ) {
                    m_extract( p_csAlgnSent, p_csBerkTree );
                }
                if( m_bSrcParsePruning ) {
                    delete p_csBerkTree;
                }
                p_csAlgnSent->m_clear();
            }

            delete[] p_szSrcLine;
            delete[] p_szTgtLine;
            delete[] p_szAlgnLine;
            if( m_bSrcParsePruning ) {
                delete[] p_szSrcParseLine;
            }
            ++lSentNum;
            if( lSentNum % 10000 == 0 ) {
                fprintf( stderr, "\rprocessed %12ld sentences...", lSentNum );
            }
            if( m_lExtractedSampleNum >= m_lMaxExtractedSampleNum ) {
                break;
            }
        }
        fprintf( stderr, "\rprocessed %12ld sentences...", lSentNum );
        delete p_csAlgnSent;
        timer.m_endTimer();
        if( lSentNum >= 10000 ) {
            fprintf( stderr, "\n" );
        }
        double dSpeed = lSentNum / timer.m_getTimerDiff();
        fprintf( stderr, "done [%.2f seconds, avg: %.2f sent/sec]\n", timer.m_getTimerDiff(), dSpeed );
        m_extractClear();

    }

    char* me_extract_c::m_refineParseTree( char* p_szString, char* p_szOriginParseTree ) {

        char* p_szParseStr;
        long lStrLen = strlen( p_szOriginParseTree );
        if( !strcmp( mp_szCharset, "UTF-8" ) ) {
            p_szParseStr = iconv_c::ms_iconv( "UTF-8", "GB2312", p_szOriginParseTree, lStrLen );
            if( !p_szParseStr ) {
                return NULL;
            }
        }
        else if( !strcmp( mp_szCharset, "GB2312" ) ) {
            p_szParseStr = p_szOriginParseTree;
        }
        lStrLen = strlen( p_szParseStr );
        char* p_szRpt = new char[lStrLen * 3 + 3];
        memset( p_szRpt, 0, ( lStrLen * 3 + 3 ) * sizeof( char ) );
        char** pp_szToks;
        long tokCnt = string_c::ms_split( p_szString, " ", pp_szToks );
        //strcat( p_szRpt, "(" );
        long tid = 0;
        for( long i=0; i < lStrLen; ) {
            if( tid < tokCnt && !strncmp( &p_szParseStr[i], pp_szToks[tid], strlen( pp_szToks[tid] ) ) ) {
                strcat( p_szRpt, " " );
                strcat( p_szRpt, pp_szToks[tid] );
                strcat( p_szRpt, " " );
                i += strlen( pp_szToks[tid] );
                ++tid;
            }
            else if( p_szParseStr[i] == '(' || p_szParseStr[i] == ')' ) {
                if( i > 2 ) {
                    strcat( p_szRpt, " " );
                }
                p_szRpt[strlen( p_szRpt )] = p_szParseStr[i];
                if( i > 0 && i < lStrLen -1 ) {
                    strcat( p_szRpt, " " );
                }
                ++i;
            }
            else {
                p_szRpt[strlen( p_szRpt )] = p_szParseStr[i];
                ++i;
            }
        }
        //strcat( p_szRpt, ")" );
        string_c::ms_freeStrArray( pp_szToks, tokCnt );
        if( strcmp( mp_szCharset, "GB2312" ) != 0 ) {
            delete[] p_szParseStr;
        }

        return p_szRpt;

    }

    void me_extract_c::m_extract( algn_sent_c* p_csAlgnSent, berk_parse_tree_c* p_csSrcTree ) {

        for( long span=2; span <= p_csAlgnSent->m_lSrcTokCnt; span++ ) {
            for( long beg=0; beg + span <= p_csAlgnSent->m_lSrcTokCnt; beg++ ) {
                long end = beg + span;
                for( long mid=beg+1; mid < end; mid++ ) {
                    if( mid-beg > m_lMaxSrcPhraseWordNum || end - mid > m_lMaxSrcPhraseWordNum ) {
                        /* violate the maximum word number constraint of source-side phrase */
                        continue;
                    }
                    if( m_bSrcParsePruning && !p_csSrcTree->m_isSatisfyTreeConstraint( beg, mid, mid, end ) ) {
                        /* violate the source parse tree constraint */
                        continue;
                    }
                    /* find phrase's target-side boundaries */
                    long leftTgtBeg, leftTgtEnd;
                    if( !m_findPhrTgtBnd( p_csAlgnSent, beg, mid-1, leftTgtBeg, leftTgtEnd ) ) {
                        /* violate word-alignment consistency */
                        continue;
                    }
                    else if( leftTgtEnd - leftTgtBeg + 1 > m_lMaxTgtPhraseWordNum ) {
                        /* violate the maximum word number constraint of target-side phrase */
                        continue;
                    }
                    long rightTgtBeg, rightTgtEnd;
                    if( !m_findPhrTgtBnd( p_csAlgnSent, mid, end-1, rightTgtBeg, rightTgtEnd ) ) {
                        /* violate word-alignment consistency */
                        continue;
                    }
                    else if( rightTgtEnd - rightTgtBeg + 1 > m_lMaxTgtPhraseWordNum ) {
                        /* violate the maximum word number constraint of target-side phrase */
                        continue;
                    }
                    /* check whether two phrases are adjacent */
                    if( leftTgtEnd <= rightTgtBeg ) {
                        /* check gap between two phrases */
                        bool bUnaligned = true;
                        long lUnalignedCnt = 0;
                        for( long i=leftTgtEnd + 1; i < rightTgtBeg; i++ ) {
                            if( p_csAlgnSent->mp_lTgt2SrcCnt[i] > 0 ) {
                                bUnaligned = false;
                            }
                            ++lUnalignedCnt;
                        }
                        if( !bUnaligned ||  lUnalignedCnt > m_lMaxTgtUnalgnWordNum ) {
                            continue;
                        }
                        /* straight orientation samples */
                        m_outputSample( p_csAlgnSent, beg, mid-1, leftTgtBeg, leftTgtEnd, \
                            mid, end-1, rightTgtBeg, rightTgtEnd, true );
                    }
                    else if( rightTgtEnd <= leftTgtBeg ) {
                        /* check gap between two phrases */
                        bool bUnaligned = true;
                        long lUnalignedCnt = 0;
                        for( long i=rightTgtEnd + 1; i < leftTgtBeg; i++ ) {
                            if( p_csAlgnSent->mp_lTgt2SrcCnt[i] > 0 ) {
                                bUnaligned = false;
                            }
                            ++lUnalignedCnt;
                        }
                        if( !bUnaligned ||  lUnalignedCnt > m_lMaxTgtUnalgnWordNum ) {
                            continue;
                        }
                        /* straight orientation samples */
                        m_outputSample( p_csAlgnSent, beg, mid-1, leftTgtBeg, leftTgtEnd, \
                            mid, end-1, rightTgtBeg, rightTgtEnd, false );
                    }
                    if( m_lExtractedSampleNum >= m_lMaxExtractedSampleNum ) {
                        return;
                    }

                }/* end for */
            }/* end for */
        }/* end for */

    }

    bool me_extract_c::m_findPhrTgtBnd( algn_sent_c* p_csAlgnSent, long srcBeg, long srcEnd, long& tgtBeg, long& tgtEnd ) {

        long* p_lTgt2SrcCnt = new long[p_csAlgnSent->m_lTgtTokCnt];
        memcpy( p_lTgt2SrcCnt, p_csAlgnSent->mp_lTgt2SrcCnt, p_csAlgnSent->m_lTgtTokCnt * sizeof( long ) );
        long tMin = 0x7fffffff;
        long tMax = -1;
        for( long i=srcBeg; i <= srcEnd; i++ ) {
            list_c* p_csTgtIdxLst = p_csAlgnSent->mpp_csSrc2TgtIdxLst[i];
            for( long j=0; j < p_csTgtIdxLst->m_getLength(); j++ ) {
                long tIdx = p_csTgtIdxLst->m_getInt( j );
                if( tIdx < tMin ) {
                    tMin = tIdx;
                }
                if( tIdx > tMax ) {
                    tMax = tIdx;
                }
                --p_lTgt2SrcCnt[tIdx];
            }
        }
        if( tMax >= 0 ) {
            bool bOob = false;
            for( long i=tMin; i <= tMax; i++ ) {
                if( p_lTgt2SrcCnt[i] > 0 ) {
                    bOob = true;
                    break;
                }
            }
            if( !bOob ) {
                tgtBeg = tMin;
                tgtEnd = tMax;
                delete[] p_lTgt2SrcCnt;
                return true;
            }
        }
        tgtBeg = -1;
        tgtEnd = -1;
        delete[] p_lTgt2SrcCnt;
        return false;

    }

    void me_extract_c::m_outputSample( algn_sent_c* p_csAlgnSent, long leftSrcBeg, long leftSrcEnd, \
        long leftTgtBeg, long leftTgtEnd, long rightSrcBeg, long rightSrcEnd, \
        long rightTgtBeg, long rightTgtEnd, bool bIsStraight ) {

            if( bIsStraight == true ) {
                /* straight samples */
                fprintf( mp_stRltFileHnd, "%s ", mp_szStraightSampleMark );
            }
            else {
                /* inverse samples */
                fprintf( mp_stRltFileHnd, "%s ", mp_szInvertedSampleMark );
            }
            char** pp_szSrcToks = p_csAlgnSent->mpp_szSrcToks;
            char** pp_szTgtToks = p_csAlgnSent->mpp_szTgtToks;
            fprintf( mp_stRltFileHnd, "SLL=%s SLR=%s TLL=%s TLR=%s ", pp_szSrcToks[leftSrcBeg], pp_szSrcToks[leftSrcEnd], \
                pp_szTgtToks[leftTgtBeg], pp_szTgtToks[leftTgtEnd] );
            fprintf( mp_stRltFileHnd, "SRL=%s SRR=%s TRL=%s TRR=%s ", pp_szSrcToks[rightSrcBeg], pp_szSrcToks[rightSrcEnd], \
                pp_szTgtToks[rightTgtBeg], pp_szTgtToks[rightTgtEnd] );
            fprintf( mp_stRltFileHnd, "\n" );

            ++m_lExtractedSampleNum;
            if( m_lExtractedSampleNum >= m_lMaxExtractedSampleNum ) {
                return;
            }

    }

}

