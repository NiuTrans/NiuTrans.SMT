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
 * Maxent reorder model trainer; MeReorderExtract.h
 * This header file defines a class for extracting distortion samples
 * from an aligned bilingual corpus. The extracted samples are used to
 * train a maximum entropy lexicalized reordering model.
 * It supports either GB2312 or UTF-8 charset encoded files of parse trees.
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); June 23th, 2011; add option "-maxSampleNum"
 * Hao Zhang (email: zhanghao1216@gmail.com); May 12th, 2011;
 *
 */


#ifndef __MEREORDEREXTRACT_H__
#define __MEREORDEREXTRACT_H__


#include "stdio.h"
#include "../NiuTrans.Base/DataStruts.h"
using namespace datastructs;


namespace smt {

    /*
     *
     * Aligned Sentence-Pair (storing info of an aligned sentence-pair)
     *
     */
    class algn_sent_c {

    public:
        algn_sent_c();
        ~algn_sent_c();

    public:
        bool m_init( char* p_szSrcSent, char* p_szTgtSent, char* p_szAlgnments, long lSentId );
        void m_clear();

    public:
        /* source sentence info */
        char**          mpp_szSrcToks;
        long            m_lSrcTokCnt;
        long*           mp_lSrc2TgtCnt;
        list_c**        mpp_csSrc2TgtIdxLst;
        /* target sentence info */
        char**          mpp_szTgtToks;
        long            m_lTgtTokCnt;
        long*           mp_lTgt2SrcCnt;
        list_c**        mpp_csTgt2SrcIdxLst;

    };

    /*
     * Parse Tree Node
     */
    class parse_node_c {

    public:
        parse_node_c();
        ~parse_node_c();

    public:
        long            m_lNodeBlockBeg;
        long            m_lNodeBlockEnd;
        long            m_lNodeLevel;
        char*           mp_szNodeNonTermsStack;
        char*           mp_szNodeNonTerm;
        char*           mp_szNodeHeadWord;
        parse_node_c*   mp_csNodeParent;
        list_c*         mp_csNodeSonList;

    };

    /*
     *
     * Berkeley Parse Tree Reader
     *
     */
    class berk_parse_tree_c {

    public:
        berk_parse_tree_c( long lMyDim, bool bMySm = false );
        ~berk_parse_tree_c();

    public:
        bool m_initParseChart( char* p_szTreeStr, bool baddDelim = true );
        void m_clearParseChart();
        bool m_isSatisfyTreeConstraint( long lLeftBlkBeg, long lLeftBlkEnd, long lRightBlkBeg, long lRightBlkEnd );

    private:
        parse_node_c* m_buildParseTree( parse_node_c* p_csNodeParent, char** pp_szToks, long lTokCnt, long& lTokIdx, long& lWdIdx );
        bool m_isSatisfyTreeConstraint( parse_node_c* p_csLeftSubTree, parse_node_c* p_csRightSubTree );

    public:
        parse_node_c*** mppp_csParseChart;
        long            m_lDimension;
        bool            m_bSimpleMode;

    };

    /*
     *
     * MaxEnt Reorder Model
     *
     */
    class me_extract_c {

    public:
        me_extract_c( int argc, char* argv[] );
        ~me_extract_c();

    public:
        void m_meSampleExtract();

    protected:
        void m_extractInit();
        void m_extractClear();
        char* m_refineParseTree( char* p_szString, char* p_szOriginParseTree );
        void m_extract( algn_sent_c* p_csAlgnSent, berk_parse_tree_c* p_csSrcTree );
        bool m_findPhrTgtBnd( algn_sent_c* p_csAlgnSent, long srcBeg, long srcEnd, long& tgtBeg, long& tgtEnd );
        void m_outputSample( algn_sent_c* p_csAlgnSent, long leftSrcBeg, long leftSrcEnd, long leftTgtBeg, long leftTgtEnd, \
            long rightSrcBeg, long rightSrcEnd, long rightTgtBeg, long rightTgtEnd, bool bIsStraight );

    private:
        void m_parseCmdLineOptions( int argc, char* argv[] );
        void m_usage();

    private:
        const char*     mp_szSrcFilePath;
        const char*     mp_szSrcParseFilePath;
        const char*     mp_szTgtFilePath;
        const char*     mp_szTgtParseFilePath;
        const char*     mp_szAlgnFilePath;
        const char*     mp_szRltFilePath;
        bool            m_bSrcParsePruning;
        long            m_lMaxSrcPhraseWordNum;
        long            m_lMaxTgtPhraseWordNum;
        long            m_lMaxTgtUnalgnWordNum;
        long            m_lMaxExtractedSampleNum;

    private:
        FILE*           mp_stSrcFileHnd;
        FILE*           mp_stSrcParseFileHnd;
        FILE*           mp_stTgtFileHnd;
        FILE*           mp_stTgtParseFileHnd;
        FILE*           mp_stAlgnFileHnd;
        FILE*           mp_stRltFileHnd;
        const char*     mp_szStraightSampleMark;
        const char*     mp_szInvertedSampleMark;
        long            m_lExtractedSampleNum;

    private:
        const char*     mp_szCharset;

    };

}


#endif

