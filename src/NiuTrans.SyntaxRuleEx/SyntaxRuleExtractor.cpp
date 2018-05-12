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
 * Program Entry: Main.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); July 3rd, 2012; add option "-varbase"
 *
 */


#include "SyntaxRuleExtractor.h"
#include "../NiuTrans.Base/Utilities.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


using namespace util;


namespace syntax_based_smt {

    /*
     *
     * Closure Span
     *
     */
    span_c::span_c() {

        this->m_lBeg = MAX_NUM_LONG_TYPE_DATA;
        this->m_lEnd = -1;

    }

    span_c::span_c( long beg, long end ) {

        this->m_lBeg = beg;
        this->m_lEnd = end;

    }

    span_c::~span_c() {
    }

    void span_c::m_expand( long index ) {

        if( index < m_lBeg )
            m_lBeg = index;
        if( index > m_lEnd )
            m_lEnd = index;

    }

    void span_c::m_expand( span_c* span ) {

        if( span->m_lBeg < this->m_lBeg )
            this->m_lBeg = span->m_lBeg;
        if( span->m_lEnd > this->m_lEnd )
            this->m_lEnd = span->m_lEnd;

    }

    bool span_c::m_isLegal() {

        return (this->m_lBeg <= this->m_lEnd);

    }

    bool span_c::m_contain( span_c* s ) {

        return ( m_isLegal() && s->m_isLegal() && m_lBeg <= s->m_lBeg && m_lEnd >= s->m_lEnd );

    }

    bool span_c::m_overlap( span_c* s ) {

        return !( !m_isLegal() || !s->m_isLegal() || m_lBeg > s->m_lEnd || m_lEnd < s->m_lBeg );

    }

    /*
     *
     * One Token's Alignment Information
     *
     */
    token_algn_info_c::token_algn_info_c( char* token ) {

        this->mp_szTok = new char[strlen( token ) + 1];
        strcpy( this->mp_szTok, token );
        this->m_lAlgnCnt = 0;
        this->mp_csAlgnIndices = new list_c();
        this->mp_csAlgnSpan = new span_c();

    }

    token_algn_info_c::~token_algn_info_c() {

        delete[] this->mp_szTok;
        this->m_lAlgnCnt = 0;
        delete this->mp_csAlgnIndices;
        delete this->mp_csAlgnSpan;

    }

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

        char **pp_szSrcToks, **pp_szTgtToks, **pp_szAlgnItems;
        long lSrcTokCnt, lTgtTokCnt, lAlgnItemCnt;

        /*** tokenize input sentence-pairs and alignments ***/
        lSrcTokCnt = string_c::ms_splitPlus( p_szSrcSent, " ", pp_szSrcToks );
        lTgtTokCnt = string_c::ms_splitPlus( p_szTgtSent, " ", pp_szTgtToks );
        lAlgnItemCnt = string_c::ms_splitPlus( p_szAlgnments, " ", pp_szAlgnItems );
        if( lSrcTokCnt == 0 || lTgtTokCnt == 0 || lAlgnItemCnt == 0 ) {
            fprintf( stderr, "[WARNING]: wrong format in %ld sentence! (SRC: %ld words   TGT: %ld words  ALIGN: %ld alignments)\n", \
                lSentId, lSrcTokCnt, lTgtTokCnt, lAlgnItemCnt );
            string_c::ms_freeStrArray( pp_szSrcToks, lSrcTokCnt );
            string_c::ms_freeStrArray( pp_szTgtToks, lTgtTokCnt );
            string_c::ms_freeStrArray( pp_szAlgnItems, lAlgnItemCnt );
            return false;
        }

        /*** allocate memory for new alignment information ***/

        /* source side */
        this->mp_csSrc2TgtAlgn = new array_c( lSrcTokCnt, algn_sent_c::ms_free_token_algn_info );
        for( long i=0; i < lSrcTokCnt; i++ ) {
            mp_csSrc2TgtAlgn->m_setObject( i, new token_algn_info_c( pp_szSrcToks[i] ) );
        }

        /* target side */
        this->mp_csTgt2SrcAlgn = new array_c( lTgtTokCnt, algn_sent_c::ms_free_token_algn_info );
        for( long i=0; i < lTgtTokCnt; i++ ) {
            mp_csTgt2SrcAlgn->m_setObject( i, new token_algn_info_c( pp_szTgtToks[i] ) );
        }

        /*** parse alignment info. of the sentence-pair ***/
        for( long i=0; i < lAlgnItemCnt; i++ ) {
            /** read in alignment info. **/
            long sid, tid;
            if( !sscanf( pp_szAlgnItems[i], "%ld-%ld", &sid, &tid ) ) {
                fprintf( stderr, "[WARNING]: wrong format in %ld sentence alignment! (ALGN: %s, index not match)\n", \
                    lSentId, pp_szAlgnItems[i] );
                string_c::ms_freeStrArray( pp_szSrcToks, lSrcTokCnt );
                string_c::ms_freeStrArray( pp_szTgtToks, lTgtTokCnt );
                string_c::ms_freeStrArray( pp_szAlgnItems, lAlgnItemCnt );
                m_clear();
                return false;
            }
            if( sid < 0 || sid >= lSrcTokCnt || tid < 0 || tid >= lTgtTokCnt ) {
                fprintf( stderr, "[WARNING]: wrong format in %ld sentence alignment! (ALGN: %s, out of bounds)\n", \
                    lSentId, pp_szAlgnItems[i] );
                string_c::ms_freeStrArray( pp_szSrcToks, lSrcTokCnt );
                string_c::ms_freeStrArray( pp_szTgtToks, lTgtTokCnt );
                string_c::ms_freeStrArray( pp_szAlgnItems, lAlgnItemCnt );
                m_clear();
                return false;
            }

            /** For every alignment, record its info. both on source and target sides **/

            /* source side */
            token_algn_info_c *st = ( token_algn_info_c* ) mp_csSrc2TgtAlgn->m_getObject( sid );
            ++st->m_lAlgnCnt;
            st->mp_csAlgnIndices->m_addInt( tid );
            st->mp_csAlgnSpan->m_expand( tid );

            /* target side */
            token_algn_info_c *tt = ( token_algn_info_c* ) mp_csTgt2SrcAlgn->m_getObject( tid );
            ++tt->m_lAlgnCnt;
            tt->mp_csAlgnIndices->m_addInt( sid );
            tt->mp_csAlgnSpan->m_expand( sid );
        }

        string_c::ms_freeStrArray( pp_szSrcToks, lSrcTokCnt );
        string_c::ms_freeStrArray( pp_szTgtToks, lTgtTokCnt );
        string_c::ms_freeStrArray( pp_szAlgnItems, lAlgnItemCnt );

        return true;

    }

    void algn_sent_c::m_clear() {

        delete this->mp_csSrc2TgtAlgn;
        delete this->mp_csTgt2SrcAlgn;

    }

    void algn_sent_c::ms_free_token_algn_info( const void* token_algn_info ) {

        token_algn_info_c* p = ( token_algn_info_c* ) token_algn_info;
        delete p;

    }

    void algn_sent_c::ms_free_span( const void* span ) {

        span_c* p = ( span_c* ) span;
        delete p;

    }

    span_c* algn_sent_c::m_getMappedSpan( span_c* span, bool isSrc2Tgt ) {

        array_c* alignment = ( isSrc2Tgt ? this->mp_csSrc2TgtAlgn : this->mp_csTgt2SrcAlgn );
        long size = alignment->m_getSize();

        span_c* mappedSpan = new span_c();
        if( span->m_isLegal() && span->m_lBeg >= 0 && span->m_lEnd < size ) {
            for( long i=span->m_lBeg; i <= span->m_lEnd; i++ ) {
                token_algn_info_c* ta = ( token_algn_info_c* ) alignment->m_getObject( i );
                mappedSpan->m_expand( ta->mp_csAlgnSpan );
            }
        }
        return mappedSpan;

    }

    bool algn_sent_c::m_isClosure( span_c* span, bool isSrc2Tgt ) {

        span_c* mappedSpan = m_getMappedSpan( span, isSrc2Tgt );
        span_c* inverseMappedSpan = m_getMappedSpan( mappedSpan, !isSrc2Tgt );
        bool isClosure = span->m_contain( inverseMappedSpan );
        delete mappedSpan;
        delete inverseMappedSpan;
        return isClosure;

    }

    span_c* algn_sent_c::m_getMinClosureSpan( span_c* span, bool isSrc2Tgt ) {

        span_c* mappedSpan = m_getMappedSpan( span, isSrc2Tgt );
        span_c* inverseMappedSpan = m_getMappedSpan( mappedSpan, !isSrc2Tgt );
        if( span->m_contain( inverseMappedSpan ) ) {
            delete inverseMappedSpan;
            return mappedSpan;
        }
        else {
            delete mappedSpan;
            delete inverseMappedSpan;
            return new span_c();
        }

    }
    
    span_c* algn_sent_c::m_getMaxClosureSpan( span_c* span, bool isSrc2Tgt ) {
    
        span_c* minSpan = m_getMinClosureSpan( span, isSrc2Tgt );
        if( minSpan->m_isLegal() ) {
            long sBeg = minSpan->m_lBeg;
            long sEnd = minSpan->m_lEnd;
            array_c* alignment = ( isSrc2Tgt ? mp_csTgt2SrcAlgn : mp_csSrc2TgtAlgn );
            while( sBeg >= 0 ) {
                token_algn_info_c* ba = ( token_algn_info_c* ) alignment->m_getObject( sBeg );
                if( sBeg != minSpan->m_lBeg && ba->m_lAlgnCnt > 0 )
                    break;
                --sBeg;
            }
            while( sEnd < alignment->m_getSize() ) {
                token_algn_info_c* ea = ( token_algn_info_c* ) alignment->m_getObject( sEnd );
                if( sEnd != minSpan->m_lEnd && ea->m_lAlgnCnt > 0 )
                    break;
                ++sEnd;
            }
            minSpan->m_expand( sBeg + 1 );
            minSpan->m_expand( sEnd - 1 );
        }
        return minSpan;
    
    }

    list_c* algn_sent_c::m_getAllClosureSpans( span_c* span, bool isSrc2Tgt, long maxUnAlgn ) {

        list_c* spans = new list_c( algn_sent_c::ms_free_span );

        span_c* ms = m_getMinClosureSpan( span, isSrc2Tgt );
        if( ms->m_isLegal() ) {
            array_c* alignment = ( isSrc2Tgt ? this->mp_csTgt2SrcAlgn : this->mp_csSrc2TgtAlgn );
            for( long beg=ms->m_lBeg; beg >= 0; --beg ) {
                token_algn_info_c* ba = ( token_algn_info_c* ) alignment->m_getObject( beg );
                if( beg != ms->m_lBeg && ba->m_lAlgnCnt > 0 )
                    break;
                for( long end=ms->m_lEnd; end < alignment->m_getSize(); end++ ) {
                    token_algn_info_c* ea = ( token_algn_info_c* ) alignment->m_getObject( end );
                    if( end != ms->m_lEnd && ea->m_lAlgnCnt > 0 )
                        break;
                    if( end - ms->m_lEnd + ms->m_lBeg - beg <= maxUnAlgn )
                        spans->m_addObject( new span_c( beg, end ) );
                }
            }
        }
        delete ms;

        return spans;

    }

    /*
     *
     * Tree Node
     *
     */
    node_c::node_c() {

        this->mp_csParent = NULL;
        this->mp_csSonList = new list_c();
        this->mp_szNodeLabel = NULL;
        this->mp_csCoveredLeavesSpan = new span_c();
        this->mp_csProjectedSpan = new span_c();
        this->m_lNodeLevel = -1;
        this->m_lNodeID = -1;
        this->m_bAdmissibleNode = false;
        this->m_bTerminalNode = false;
        this->m_bUnaryNode = false;

        this->mp_csSubtreeList = new list_c( g_free_tree, g_compare_tree );
        this->mp_csSTreeIndex_GHKM_Lex = new list_c();
        this->mp_csSTreeIndex_GHKM_NonLex = new list_c();
        this->mp_csSTreeIndex_SPMT_Lex = new list_c();

    }

    node_c::~node_c() {

        for( long i=0; i < this->mp_csSonList->m_getLength(); i++ ) {
            node_c* snode = ( node_c* ) this->mp_csSonList->m_getObject( i );
            delete snode;
        }
        delete this->mp_csSonList;
        if( this->mp_szNodeLabel ) {
            delete[] this->mp_szNodeLabel;
        }
        delete this->mp_csCoveredLeavesSpan;
        delete this->mp_csProjectedSpan;

        delete mp_csSubtreeList;
        delete mp_csSTreeIndex_GHKM_Lex;
        delete mp_csSTreeIndex_GHKM_NonLex;
        delete mp_csSTreeIndex_SPMT_Lex;

    }

    void node_c::m_printNode( char* buffer, bool simpleForm ) {

        if( simpleForm ) {
            strcat( buffer, mp_szNodeLabel );
        }
        else {
            char out[10240];
            sprintf( out, "$PID:      %d\n$SonCnt:   %d\n$NodeID:   %d\n$Level:    %d\n$Label:    %s\n$Leaf:     %d\n$Closure:  %d\n$Unary:    %d\n$Span:     [%d-%d]\n$MapSpan:  [%d-%d]\n\n", \
                mp_csParent == NULL ? -1 : mp_csParent->m_lNodeID, mp_csSonList->m_getLength(), m_lNodeID, m_lNodeLevel, mp_szNodeLabel, \
                m_bTerminalNode, m_bAdmissibleNode, m_bUnaryNode, mp_csCoveredLeavesSpan->m_lBeg, mp_csCoveredLeavesSpan->m_lEnd, mp_csProjectedSpan->m_lBeg, mp_csProjectedSpan->m_lEnd );
            strcat( buffer, out );
        }

    }

    /*
     *
     * Tree Node Pointer's Wrapper
     *
     */
    pnode_c::pnode_c( node_c* node ) {

        this->mp_csParseNode = node;
        this->mp_csParentNode = NULL;
        this->mp_csSonNodes = new list_c();

    }

    pnode_c::~pnode_c() {

        for( long i=0; i < mp_csSonNodes->m_getLength(); i++ ) {
            pnode_c* node = ( pnode_c* ) mp_csSonNodes->m_getObject( i );
            delete node;
        }
        delete mp_csSonNodes;
        mp_csParseNode = NULL;
        mp_csParentNode = NULL;

    }

    /*
     *
     * Tree Fragment
     *
     */
    tree_c::tree_c() {

        this->mp_csRootNode = NULL;
        this->mp_csLeafNodeList = new list_c();
        this->mp_csAdmissibleNodes = new list_c();
        this->m_lComposeTimes = 0;
        this->m_bLexicalized = false;
        this->m_bTypeGHKM = false;
        this->m_bTypeSPMT = false;

    }

    tree_c::~tree_c() {

        delete mp_csRootNode;
        delete mp_csLeafNodeList;
        delete mp_csAdmissibleNodes;
        m_lComposeTimes = 0;
        m_bLexicalized = false;
        m_bTypeGHKM = false;
        m_bTypeSPMT = false;

    }

    void tree_c::m_createMinGHKMSubTree( node_c* p_csParseRoot ) {

        mp_csRootNode = m_createMinSubTree( p_csParseRoot, NULL, p_csParseRoot );
        m_bTypeGHKM = true;

    }

    void tree_c::m_createMinSPMTSubTree( node_c* p_csParseRoot, span_c* tokenSpan ) {

        mp_csRootNode = m_createMinSubTree( p_csParseRoot, NULL, p_csParseRoot, tokenSpan );
        m_bTypeSPMT = true;

    }

    void tree_c::m_composeTwoSubTree( tree_c* mTree, tree_c* sTree ) {

        mp_csRootNode = m_composeTwoSubTree( mTree->mp_csRootNode, sTree->mp_csRootNode );
        m_bLexicalized = mTree->m_bLexicalized | sTree->m_bLexicalized;
        m_bTypeGHKM = mTree->m_bTypeGHKM & sTree->m_bTypeGHKM;
        m_bTypeSPMT = mTree->m_bTypeSPMT | sTree->m_bTypeSPMT;
        m_lComposeTimes = mTree->m_lComposeTimes + sTree->m_lComposeTimes + 1;

    }

    long tree_c::m_calTreeDepth() {

        long rootLevel = mp_csRootNode->mp_csParseNode->m_lNodeLevel;
        long leafMaxLevel = rootLevel + 1;
        for( long i=0; i < mp_csLeafNodeList->m_getLength(); i++ ) {
            pnode_c* leaf = ( pnode_c* ) mp_csLeafNodeList->m_getObject( i );
            long leafLevel = leaf->mp_csParseNode->m_lNodeLevel;
            if( leafLevel > leafMaxLevel ) {
                leafMaxLevel = leafLevel;
            }
        }

        return ( leafMaxLevel - rootLevel );

    }

    pnode_c* tree_c::m_createMinSubTree( node_c* p_csParseRoot, pnode_c* p_csParent, node_c* p_csParseNode, span_c* coveredSpan ) {

        pnode_c* curNode = new pnode_c( p_csParseNode );
        curNode->mp_csParentNode = ( p_csParent == NULL ? NULL : p_csParent );

        bool bLeafNodeCondition = ( p_csParseNode->m_bTerminalNode || \
            ( p_csParseNode->m_bAdmissibleNode && p_csParseNode != p_csParseRoot && \
            ( !coveredSpan || !p_csParseNode->mp_csCoveredLeavesSpan->m_overlap( coveredSpan ) ) ) );
        if( bLeafNodeCondition ) {
            /* leaf node or internal admissible branch node */
            if( p_csParseNode->m_bTerminalNode == true && p_csParseNode->mp_csProjectedSpan->m_isLegal() == true ) {
                /* lexicalized rules are rules that contains aligned leaf nodes and no un-aligned leaf nodes */
                m_bLexicalized = true;
            }
            if( p_csParseNode->m_bAdmissibleNode == true ) {
                mp_csAdmissibleNodes->m_addObject( curNode );
            }
            mp_csLeafNodeList->m_addObject( curNode );
            return curNode;
        }
        else {
            for( long i=0; i < p_csParseNode->mp_csSonList->m_getLength(); i++ ) {
                node_c* pn = ( node_c* ) p_csParseNode->mp_csSonList->m_getObject( i );
                pnode_c* rstn = m_createMinSubTree( p_csParseRoot, curNode, pn, coveredSpan );
                curNode->mp_csSonNodes->m_addObject( rstn );
            }
            return curNode;
        }

    }

    pnode_c* tree_c::m_composeTwoSubTree( pnode_c* curNode, pnode_c* sRoot ) {

        pnode_c* newCurNode;

        long sonNum = curNode->mp_csSonNodes->m_getLength();
        if( sonNum == 0 ) {
            if( curNode->mp_csParseNode == sRoot->mp_csParseNode ) {
                /* son tree */
                newCurNode = m_composeTwoSubTree( sRoot, sRoot );
            }
            else {
                /* leaf node */
                newCurNode = new pnode_c( curNode->mp_csParseNode );
                if( curNode->mp_csParseNode->m_bAdmissibleNode == true ) {
                    mp_csAdmissibleNodes->m_addObject( newCurNode );
                }
                mp_csLeafNodeList->m_addObject( newCurNode );
            }
        }
        else {
            /* branch node */
            newCurNode = new pnode_c( curNode->mp_csParseNode );
            for( long i=0; i < sonNum; i++ ) {
                pnode_c* son = ( pnode_c* ) curNode->mp_csSonNodes->m_getObject( i );
                pnode_c* newSon = m_composeTwoSubTree( son, sRoot );
                newCurNode->mp_csSonNodes->m_addObject( newSon );
            }
        }

        return newCurNode;

    }

    void g_free_tree( const void* tree ) {

        tree_c* st = ( tree_c* ) tree;
        delete st;

    }

    int g_compare_tree( const void* t1, const void* t2 ) {

        tree_c* st1 = ( tree_c* ) t1;
        tree_c* st2 = ( tree_c* ) t2;

        if( st1->m_bLexicalized != st2->m_bLexicalized || st1->mp_csRootNode->mp_csParseNode != st2->mp_csRootNode->mp_csParseNode )
            return -1;
        long leafNum = st1->mp_csLeafNodeList->m_getLength();
        if( leafNum != st2->mp_csLeafNodeList->m_getLength() )
            return -1;
        for( long i=0; i < leafNum; i++ ) {
            pnode_c* p1 = ( pnode_c* ) st1->mp_csLeafNodeList->m_getObject( i );
            pnode_c* p2 = ( pnode_c* ) st2->mp_csLeafNodeList->m_getObject( i );
            if( p1->mp_csParseNode != p2->mp_csParseNode )
                return -1;
        }
        return 0;

    }

    long g_lVarBeginIndex = 0;

    /*
     *
     * Rule's One-hand-side Information
     *
     */
    rule_xhs_c::rule_xhs_c( algn_sent_c* p_csAlignment, bool onSrcSide ) {

        /* initialization */
        this->m_lParseChartDim = -1;
        this->mppp_csParseChart = NULL;
        this->mp_csParseTreeRoot = NULL;
        this->m_lNodeNum = 0;
        this->mp_csParseTreeLeaves = NULL;
        this->mp_csAdmissibleNodes = NULL;
        this->mp_csUnalignedLeaves = NULL;
        this->mp_csAdmissibleTree = NULL;
        this->m_lAdmNodeNum = 0;

        /* alignment information */
        this->mp_csAlignment = p_csAlignment;
        this->m_bOnSrcSide = onSrcSide;

    }

    rule_xhs_c::~rule_xhs_c() {
    }

    /*
     * there are two rule extraction methods, i.e.,
     * 1). GHKM method
     * 2). SPMT method
     */
    bool rule_xhs_c::m_init( char* p_szTreeStr, char* p_szTermStr, method_e method, long lComposeTimes, long lTreeDepth ) {

        char** tokens;
        long tokenCnt = string_c::ms_splitPlus( p_szTermStr, " ", tokens );

        /* initialization */
        this->m_lParseChartDim = tokenCnt;
        m_createParseChart();
        this->mp_csParseTreeLeaves = new list_c();
        this->mp_csAdmissibleNodes = new list_c();
        this->mp_csUnalignedLeaves = new list_c();

        /* build parse tree */
        if( !m_createParseTree( p_szTreeStr, tokens, tokenCnt ) ) {
            m_destroyParseChart();
            delete mp_csParseTreeLeaves;
            delete mp_csAdmissibleNodes;
            string_c::ms_freeStrArray( tokens, tokenCnt );
            return false;
        }

        /* build admissible node tree */
        m_buildAdmissibleTree( mp_csAdmissibleTree, mp_csParseTreeRoot );

        /* generate all subtrees, that is, all rule sides */
        if( method == GHKM ) {
            m_genMinGHKMSubTree( mp_csAdmissibleTree );
        }
        else if( method == SPMT ) {
            m_genMinGHKMSubTree( mp_csAdmissibleTree, true );
            m_genMinSPMTSubTree( tokenCnt );
        }
        if( lComposeTimes > 0 )
            m_composeSubTree( mp_csAdmissibleTree, lComposeTimes, lTreeDepth );

        string_c::ms_freeStrArray( tokens, tokenCnt );

        return true;

    }
    
    void rule_xhs_c::m_clear() {

        m_destroyParseChart();
        m_lParseChartDim = -1;
        delete mp_csParseTreeRoot;
        m_lNodeNum = 0;
        delete mp_csParseTreeLeaves;
        delete mp_csAdmissibleNodes;
        delete mp_csUnalignedLeaves;
        delete mp_csAdmissibleTree;
        mp_csAdmissibleTree = NULL;
        m_lAdmNodeNum = 0;

    }

    void rule_xhs_c::m_genRule( bool isT2sRule, long maxVarNum, long maxWdNum, long maxUaInside, long maxUaOutside, oformat_e oformat, FILE* outF ) {

        /* generate rules for admissible nodes */
        for( long i=0; i < m_lAdmNodeNum; i++ ) {
            node_c* nd = ( node_c* ) mp_csAdmissibleNodes->m_getObject( i );
            span_c* pMaxSpan = mp_csAlignment->m_getMaxClosureSpan( nd->mp_csCoveredLeavesSpan, m_bOnSrcSide );
            span_c* pMinSpan = mp_csAlignment->m_getMinClosureSpan( nd->mp_csCoveredLeavesSpan, m_bOnSrcSide );
            for( long j=0; j < nd->mp_csSubtreeList->m_getLength(); j++ ) {
                tree_c* rTree = ( tree_c* ) nd->mp_csSubtreeList->m_getObject( j );
                m_genRule( rTree, pMaxSpan, pMinSpan, isT2sRule, maxVarNum, maxWdNum, maxUaInside, maxUaOutside, oformat, outF );
            }
            delete pMaxSpan;
            delete pMinSpan;
        }

        /* generate rules for unaligned word nodes */
        if( isT2sRule ) {
            /* tree-to-string rules */
            for( long i=0; i < mp_csUnalignedLeaves->m_getLength(); i++ ) {
                node_c* nd = ( node_c* ) mp_csUnalignedLeaves->m_getObject( i );
                char* rLabel = nd->mp_csParent->mp_szNodeLabel;
                char treeRule[MAX_CHAR_NUM_ONE_BUFF] = {0};
                m_printTree( nd->mp_csParent, treeRule, true );
                string_c::ms_mvStr( treeRule, 1, true );
                string_c::ms_mvStr( treeRule, 1, false );
                if( oformat == OFT ) {
                    fprintf( outF, "%s ||| %s ||| %s ||| %d %d ||| %s ||| %s\n", treeRule, nd->mp_szNodeLabel, "NULL", 1, 0, "NULL", "0-0" );
                }
                else {
                    fprintf( outF, "%s ||| %s ||| %s ||| %d %d ||| %s ||| %s ||| %s ||| %s\n", \
                        rLabel, nd->mp_szNodeLabel, "NULL", 1, 0, "NULL", "0-0", treeRule, "NULL" );
                }
            }
        }
        else {
            /* string-to-tree rules */
            array_c* srcTokens = ( !m_bOnSrcSide ? mp_csAlignment->mp_csSrc2TgtAlgn : mp_csAlignment->mp_csTgt2SrcAlgn );
            for( long i=0; i < srcTokens->m_getSize(); i++ ) {
                token_algn_info_c* ta = ( token_algn_info_c* ) srcTokens->m_getObject( i );
                if( ta->m_lAlgnCnt == 0 ) {
                    if( oformat == OFT ) {
                        fprintf( outF, "%s ||| %s ||| %s ||| %d %d ||| %s ||| %s\n", "NULL", "<NULL>", ta->mp_szTok, 1, 0, "<NULL>", "0-0" );
                    }
                    else {
                        fprintf( outF, "%s ||| %s ||| %s ||| %d %d ||| %s ||| %s ||| %s ||| %s\n", \
                            "NULL", ta->mp_szTok, "NULL", 1, 0, "NULL", "0-0", "NULL", "NULL" );
                    }
                }
            }
        }

    }

    void rule_xhs_c::m_printTree( node_c* root, char* buffer, bool simpleForm ) {

        if( !root ) {
            sprintf( buffer, "( () )" );
            return;
        }

        if( simpleForm ) {
            strcat( buffer, "(" );
        }
        root->m_printNode( buffer, simpleForm );
        for( long i=0; i < root->mp_csSonList->m_getLength(); i++ ) {
            if( simpleForm ) {
                strcat( buffer, " " );
            }
            node_c* son = ( node_c* ) root->mp_csSonList->m_getObject( i );
            m_printTree( son, buffer, simpleForm );
        }
        if( simpleForm ) {
            strcat( buffer, ")" );
        }

    }

    void rule_xhs_c::m_printTree( pnode_c* root, char* buffer, bool simpleForm ) {

        if( !root ) {
            sprintf( buffer, "( () )" );
            return;
        }

        if( simpleForm ) {
            strcat( buffer, "(" );
        }
        root->mp_csParseNode->m_printNode( buffer, simpleForm );
        for( long i=0; i < root->mp_csSonNodes->m_getLength(); i++ ) {
            if( simpleForm ) {
                strcat( buffer, " " );
            }
            pnode_c* son = ( pnode_c* ) root->mp_csSonNodes->m_getObject( i );
            m_printTree( son, buffer, simpleForm );
        }
        if( simpleForm ) {
            strcat( buffer, ")" );
        }

    }

    void rule_xhs_c::m_createParseChart() {

        mppp_csParseChart = new node_c**[m_lParseChartDim];
        for( long i=0; i < m_lParseChartDim; i++ ) {
            mppp_csParseChart[i] = new node_c*[m_lParseChartDim + 1];
            memset( mppp_csParseChart[i], 0, ( m_lParseChartDim + 1 ) * sizeof( node_c* ) );
        }

    }

    void rule_xhs_c::m_destroyParseChart() {

        for( long i=0; i < m_lParseChartDim; i++ ) {
            delete[] mppp_csParseChart[i];
        }
        delete[] mppp_csParseChart;

    }

    bool rule_xhs_c::m_createParseTree( char* p_szTreeStr, char** pp_szTerms, long lTermCnt ) {

        /* ( (XXX) ) => ( ( XXX ) ) */
        char* p_szRefinedTreeStr = m_refineParseTree( p_szTreeStr, pp_szTerms, lTermCnt );
        /* ( ( XXX ) ) => ( XXX ) */
        if( p_szRefinedTreeStr[0] == '(' && p_szRefinedTreeStr[1] == ' ' && p_szRefinedTreeStr[2] == '(' ) {
            /* GB2312 encoding */
            string_c::ms_mvStr( p_szRefinedTreeStr, 2, true );
            string_c::ms_mvStr( p_szRefinedTreeStr, 1, false );
        }
        else if( ((unsigned char)p_szRefinedTreeStr[0]) == 0xEF && ((unsigned char)p_szRefinedTreeStr[1]) == 0xBB \
            && ((unsigned char)p_szRefinedTreeStr[2]) == 0xBF ) {
            /* UTF8 encoding */
            string_c::ms_mvStr( p_szRefinedTreeStr, 5, true );
            string_c::ms_mvStr( p_szRefinedTreeStr, 1, false );
        }

        char** pp_szToks;
        long lTokCnt = string_c::ms_splitPlus( p_szRefinedTreeStr, " ", pp_szToks );
        long lTokIdx = 0;
        long lWdIdx = 0;

        this->mp_csParseTreeRoot = m_buildParseTree( NULL, pp_szToks, lTokCnt, lTokIdx, lWdIdx );

        string_c::ms_freeStrArray( pp_szToks, lTokCnt );
        delete[] p_szRefinedTreeStr;

        return ( this->mp_csParseTreeRoot == NULL ? false : true );

    }

    char* rule_xhs_c::m_refineParseTree( char* p_szTreeStr, char** pp_szTerms, long lTermCnt ) {

        long lTreeStrLen = strlen( p_szTreeStr );
        char* p_szNewTreeStr = new char[lTreeStrLen * 3 + 3];
        memset( p_szNewTreeStr, 0, ( lTreeStrLen * 3 + 3 ) * sizeof( char ) );

        long tid = 0;
        for( long i=0; i < lTreeStrLen; ) {
            if( tid < lTermCnt ) {
                char* tmpToken = new char[strlen( pp_szTerms[tid] ) + 1];
                strncpy( tmpToken, &p_szTreeStr[i], strlen( pp_szTerms[tid] ) );
                tmpToken[strlen( pp_szTerms[tid] )] = '\0';
                if( p_szTreeStr[i - 1] == ' ' && p_szTreeStr[i + strlen( pp_szTerms[tid] )] == ')' && \
                    ( !strcmp( tmpToken, pp_szTerms[tid] ) || !strcmp( string_c::ms_toLowercase( tmpToken ), pp_szTerms[tid] ) ) ) {
                    /* head word of leaf node */
                    strcat( p_szNewTreeStr, " " );
                    strcat( p_szNewTreeStr, pp_szTerms[tid] );
                    strcat( p_szNewTreeStr, " " );
                    i += strlen( pp_szTerms[tid] );
                    ++tid;
                    delete[] tmpToken;
                    continue;
                }
                delete[] tmpToken;
            }
            if( p_szTreeStr[i] == '(' || p_szTreeStr[i] == ')' ) {
                if( i > 2 ) {
                    strcat( p_szNewTreeStr, " " );
                }
                p_szNewTreeStr[strlen( p_szNewTreeStr )] = p_szTreeStr[i];
                if( i > 0 && i < lTreeStrLen -1 ) {
                    strcat( p_szNewTreeStr, " " );
                }
                ++i;
            }
            else {
                p_szNewTreeStr[strlen( p_szNewTreeStr )] = p_szTreeStr[i];
                ++i;
            }
        }

        return p_szNewTreeStr;

    }

    node_c* rule_xhs_c::m_buildParseTree( node_c* p_csNodeParent, char** pp_szToks, long lTokCnt, \
        long& lTokIdx, long& lWdIdx) {

        node_c* p_csNode = new node_c();
        p_csNode->m_lNodeLevel = ( p_csNodeParent == NULL ? 0 : p_csNodeParent->m_lNodeLevel + 1 );
        p_csNode->m_lNodeID = m_lNodeNum++;
        p_csNode->mp_csParent = p_csNodeParent;

        /* "(" */
        if( strcmp( pp_szToks[lTokIdx], "(" ) != 0 ) {
            delete p_csNode;
            return NULL;
        }
        ++lTokIdx;

        /* "POS TAG" */
        p_csNode->mp_szNodeLabel = new char[strlen( pp_szToks[lTokIdx] ) + 1];
        strcpy( p_csNode->mp_szNodeLabel, pp_szToks[lTokIdx] );
        ++lTokIdx;

        /* leaf node */
        if( strcmp( pp_szToks[lTokIdx], "(" ) != 0 || ( lTokIdx + 1 < lTokCnt && !strcmp( pp_szToks[lTokIdx+1], ")" ) ) ) {
            /* "Head Word" Leaf Node */
            node_c* p_csWordNode = new node_c();
            p_csWordNode->m_lNodeLevel = p_csNode->m_lNodeLevel + 1;
            p_csWordNode->m_lNodeID = m_lNodeNum++;
            p_csWordNode->mp_csParent = p_csNode;
            p_csWordNode->mp_szNodeLabel = new char[strlen( pp_szToks[lTokIdx] ) + 1];
            strcpy( p_csWordNode->mp_szNodeLabel, pp_szToks[lTokIdx] );
            p_csWordNode->mp_csCoveredLeavesSpan->m_expand( lWdIdx );
            span_c* mspan = mp_csAlignment->m_getMappedSpan( p_csWordNode->mp_csCoveredLeavesSpan, m_bOnSrcSide );
            p_csWordNode->mp_csProjectedSpan->m_expand( mspan );
            if( mspan->m_isLegal() == false ) {
                mp_csUnalignedLeaves->m_addObject( p_csWordNode );
            }
            p_csWordNode->m_bTerminalNode = true;
            mppp_csParseChart[lWdIdx][lWdIdx + 1] = p_csWordNode;
            mp_csParseTreeLeaves->m_addObject( p_csWordNode );

            /* "POS" Leaf Node */
            p_csNode->mp_csSonList->m_addObject( p_csWordNode );
            p_csNode->mp_csCoveredLeavesSpan->m_expand( lWdIdx );
            p_csNode->mp_csProjectedSpan->m_expand( mspan );
            delete mspan;
            p_csNode->m_bAdmissibleNode = mp_csAlignment->m_isClosure( p_csNode->mp_csCoveredLeavesSpan, m_bOnSrcSide );
            if( p_csNode->m_bAdmissibleNode == true )
                mp_csAdmissibleNodes->m_addObject( p_csNode );
            p_csNode->m_bTerminalNode = false;
            p_csNode->m_bUnaryNode = true;

            ++lWdIdx;
            ++lTokIdx;
            /* ")" */
            if( strcmp( pp_szToks[lTokIdx], ")" ) != 0 ) {
                delete p_csNode;
                return NULL;
            }
            ++lTokIdx;
            return p_csNode;
        }
        /* branch node */
        long blockBeg = lWdIdx;
        while( 1 ) {
            node_c* p_csSubTree = m_buildParseTree( p_csNode, pp_szToks, lTokCnt, lTokIdx, lWdIdx );  /* "Head Word" */
            if( !p_csSubTree || lTokIdx >= lTokCnt ) {  /* parse sub-tree error or tree's brackets "(" and ")" unmatch */
                delete p_csNode;
                if( p_csSubTree )
                    delete p_csSubTree;
                return NULL;
            }
            p_csNode->mp_csSonList->m_addObject( p_csSubTree );
            p_csNode->mp_csCoveredLeavesSpan->m_expand( p_csSubTree->mp_csCoveredLeavesSpan );
            p_csNode->mp_csProjectedSpan->m_expand( p_csSubTree->mp_csProjectedSpan );
            /* if it encounters a right bracket ")", it means the end of reading the parse tree */
            if( !strcmp( pp_szToks[lTokIdx], ")" ) ) {
                p_csNode->m_bTerminalNode = false;
                p_csNode->m_bAdmissibleNode = mp_csAlignment->m_isClosure( p_csNode->mp_csCoveredLeavesSpan, m_bOnSrcSide );
                if( p_csNode->m_bAdmissibleNode == true )
                    mp_csAdmissibleNodes->m_addObject( p_csNode );
                /* ")" */
                ++lTokIdx;
                break;
            }
            /* else the coming token must be left bracket "(", then it means to read another sub-tree */
        }
        long blockEnd = lWdIdx;
        if( mppp_csParseChart[blockBeg][blockEnd] == NULL ) {
            mppp_csParseChart[blockBeg][blockEnd] = p_csNode;
        }
        else {
            /* for unary subtrees, the chart only records the child node. */
            p_csNode->m_bUnaryNode = true;
        }

        return p_csNode;

    }

    void rule_xhs_c::m_buildAdmissibleTree( pnode_c*& p_csParent, node_c* p_csParseNode ) {

        if( p_csParseNode->m_bAdmissibleNode && ( p_csParent == NULL || p_csParseNode != p_csParent->mp_csParseNode ) ) {
            pnode_c* anode = new pnode_c( p_csParseNode );
            ++m_lAdmNodeNum;
            if( p_csParent == NULL ) {
                p_csParent = anode;
            }
            else {
                anode->mp_csParentNode = p_csParent;
                p_csParent->mp_csSonNodes->m_addObject( anode );
            }
            m_buildAdmissibleTree( anode, p_csParseNode );
        }
        else {
            for( long i=0; i < p_csParseNode->mp_csSonList->m_getLength(); i++ ) {
                node_c* node = ( node_c* ) p_csParseNode->mp_csSonList->m_getObject( i );
                m_buildAdmissibleTree( p_csParent, node );
            }
        }

    }

    void rule_xhs_c::m_genMinGHKMSubTree( pnode_c* adm_root, bool onlyNonLex ) {

        node_c* root = adm_root->mp_csParseNode;

        /* generate the minimal tree fragment under the given admissible node */
        tree_c* st = new tree_c();
        st->m_createMinGHKMSubTree( root );
        if( onlyNonLex && st->m_bLexicalized ) {
            delete st;
        }
        else {
            if( st->m_bLexicalized ) {
                root->mp_csSTreeIndex_GHKM_Lex->m_addObject( st );
            }
            else {
                root->mp_csSTreeIndex_GHKM_NonLex->m_addObject( st );
            }
            root->mp_csSubtreeList->m_addObject( st );
        }

        /* recursively generate the minimal tree fragments for the children nodes of the given admissible node */
        for( long i=0; i < adm_root->mp_csSonNodes->m_getLength(); i++ ) {
            pnode_c* son = ( pnode_c* ) adm_root->mp_csSonNodes->m_getObject( i );
            m_genMinGHKMSubTree( son, onlyNonLex );
        }

    }

    void rule_xhs_c::m_genMinSPMTSubTree( long tokenNum ) {

        for( long begPos=0; begPos < tokenNum; begPos++ ) {
            for( long endPos=begPos; endPos < tokenNum; endPos++ ) {
                span_c* tokenSpan = new span_c( begPos, endPos );
                if( mp_csAlignment->m_isClosure( tokenSpan, m_bOnSrcSide ) ) {
                    m_genMinSPMTSubTree( tokenSpan );
                }
                delete tokenSpan;
            }
        }

    }

    void rule_xhs_c::m_genMinSPMTSubTree( span_c* tokenSpan ) {

        /* trace up until find an admissible node which covers the given span */
        node_c* p_csRoot = mppp_csParseChart[tokenSpan->m_lBeg][tokenSpan->m_lBeg+1];
        while( p_csRoot ) {
            if( p_csRoot->m_bAdmissibleNode && p_csRoot->mp_csCoveredLeavesSpan->m_contain( tokenSpan ) ) {
                break;
            }
            p_csRoot = p_csRoot->mp_csParent;
        }

        /* generate SPMT lexical tree fragment */
        while( p_csRoot ) {
            tree_c* st = new tree_c();
            st->m_createMinSPMTSubTree( p_csRoot, tokenSpan );
            if( p_csRoot->mp_csSubtreeList->m_addObjectUniq( st ) ) {
                p_csRoot->mp_csSTreeIndex_SPMT_Lex->m_addObject( st );
            }
            else {
                delete st;
            }
            if( p_csRoot->mp_csParent && p_csRoot->mp_csParent->m_bUnaryNode ) {
                p_csRoot = p_csRoot->mp_csParent;
            }
            else {
                p_csRoot = NULL;
            }
        }

    }

    void rule_xhs_c::m_composeSubTree( pnode_c* adm_root, long maxComposeTimes, long maxTreeDepth ) {

        if( adm_root->mp_csSonNodes->m_getLength() == 0 ) {
            /* admissible leaf node */
            return;
        }
        else {
            /* recursive */
            for( long i=0; i < adm_root->mp_csSonNodes->m_getLength(); i++ ) {
                pnode_c* son = ( pnode_c* ) adm_root->mp_csSonNodes->m_getObject( i );
                m_composeSubTree( son, maxComposeTimes, maxTreeDepth );
            }
            /* compose */
            m_hockSonSubTree( adm_root, maxComposeTimes, maxTreeDepth );
        }

    }

    void rule_xhs_c::m_hockSonSubTree( pnode_c* adm_root, long maxComposeTimes, long maxTreeDepth ) {

        list_c* trees = adm_root->mp_csParseNode->mp_csSubtreeList;
        long treeNum = trees->m_getLength();

        for( long i=0; i < treeNum; i++ ) {
            /* retrieve the main seed tree */
            tree_c* mTree = ( tree_c* ) trees->m_getObject( i );
            list_c* newTrees = new list_c();
            newTrees->m_addObject( mTree );

            /* hock subtrees to generate new larger trees */
            long hockPosNum = mTree->mp_csAdmissibleNodes->m_getLength();
            for( long j=0; j < hockPosNum; j++ ) {
                pnode_c* hockNode = ( pnode_c* ) mTree->mp_csAdmissibleNodes->m_getObject( j );
                node_c* hockTNode = hockNode->mp_csParseNode;
                if( hockTNode->mp_csSonList->m_getLength() == 1 && \
                    ((node_c*)hockTNode->mp_csSonList->m_getObject( 0 ))->m_bTerminalNode && \
                    string_c::ms_isEnPunc( hockTNode->mp_szNodeLabel ) ) {
                        /* admissible POS tag node can not be part of composed rules */
                        continue;
                }
                list_c* sonTrees = hockNode->mp_csParseNode->mp_csSubtreeList;
                m_doHock( newTrees, hockNode, sonTrees, newTrees, maxComposeTimes, maxTreeDepth );
            }

            /* transfer the generated trees to the tree list of the given admissible node */
            for( long k=1; k < newTrees->m_getLength(); k++ ) { /* NOTE: newTrees[0] is a copy of the main seed tree */
                tree_c* t = ( tree_c* ) newTrees->m_getObject( k );
                if( trees->m_addObjectUniq( t ) ) {
                    if( t->m_bTypeGHKM ) {
                        if( t->m_bLexicalized )
                            adm_root->mp_csParseNode->mp_csSTreeIndex_GHKM_Lex->m_addObject( t );
                        else
                            adm_root->mp_csParseNode->mp_csSTreeIndex_GHKM_NonLex->m_addObject( t );
                    }
                    else if( t->m_bTypeSPMT ) {
                        adm_root->mp_csParseNode->mp_csSTreeIndex_SPMT_Lex->m_addObject( t );
                    }
                }
                else {
                    delete t;
                }
            }

            delete newTrees;

        }

    }

    void rule_xhs_c::m_doHock( list_c* tmpTrees, pnode_c* hockNode, list_c* sonTrees, list_c* newTrees, long maxComposeTimes, long maxTreeDepth ) {

        long oldLen = tmpTrees->m_getLength();

        for( long i=0; i < oldLen; i++ ) {
            tree_c* st1 = ( tree_c* ) tmpTrees->m_getObject( i );
            for( long j=0; j < sonTrees->m_getLength(); j++ ) {
                tree_c* st2 = ( tree_c* ) sonTrees->m_getObject( j );
                if( st1->m_lComposeTimes + st2->m_lComposeTimes + 1 <= maxComposeTimes ) {
                    long tDepth = st1->m_calTreeDepth() + st2->m_calTreeDepth();
                    if( tDepth <= maxTreeDepth ) {
                        tree_c* nt = new tree_c();
                        nt->m_composeTwoSubTree( st1, st2 );
                        newTrees->m_addObject( nt );
                    }
                    if( newTrees->m_getLength() >= MAX_RULE_NUM_ONE_ANODE ) {
                        return;
                    }
                }
            }
        }

    }

    void rule_xhs_c::m_genRule( tree_c* rTree, span_c* pMaxSpan, span_c* pMinSpan, bool isT2sRule, \
        long maxVarNum, long maxWdNum, long maxUaInside, long maxUaOutside, oformat_e oformat, FILE* outF ) {
    
        /* there are three code type, i.e., "normal word", "variable", and "unaligned word" */
        /* we use "word index plus one" in the alignment ("1, 2, 3, ...") to represent the "normal word",
         * use numeric "-1, -2, -3, ..." to represent the "variable",
         * and use numeric "0" to represent the "unaligned word".
         * The rule generation process is based on the tree-side and string-side code arrays.
         */

        long varIndex = 0;
        /* a symbol list which records a map from variable index to variable name */
        list_c* varNameList = new list_c();
        /* tree-side code array */
        long treeArrSz = rTree->mp_csLeafNodeList->m_getLength();
        array_c* treeArr = new array_c( treeArrSz );
        /* string-side code array */
        long strArrSz = pMaxSpan->m_lEnd - pMaxSpan->m_lBeg + 1;
        array_c* strArr = new array_c( strArrSz );
        if( !configMng_c::ms_getBool( "uaperm", false ) ) {
            for( long i=pMinSpan->m_lBeg - pMaxSpan->m_lBeg; i <= pMinSpan->m_lEnd - pMaxSpan->m_lBeg; i++ )
                strArr->m_setInt( i, pMaxSpan->m_lBeg + i + 1 );
        }
        
        /* initialize tree-side code array and the symbol list, 
         * and generate intermediate-form string-side code array
         */
        for( long i=0; i < treeArrSz; i++ ) {
            pnode_c* pLeaf = ( pnode_c* ) rTree->mp_csLeafNodeList->m_getObject( i );
            node_c* leaf = pLeaf->mp_csParseNode;
            /*
             * special cases for punctuation POS tag nodes:
             * we regard those POS tag nodes as terminal nodes,
             * not admissible nodes.
            */
            bool puncCase = ( leaf->m_bAdmissibleNode && \
                leaf->mp_csSonList->m_getLength() == 1 && \
                ((node_c*)leaf->mp_csSonList->m_getObject(0))->m_bTerminalNode && \
                string_c::ms_isEnPunc(leaf->mp_szNodeLabel) );
            if( leaf->m_bTerminalNode || puncCase ) {
                treeArr->m_setInt( i, leaf->mp_csCoveredLeavesSpan->m_lBeg );
                for( long j=leaf->mp_csProjectedSpan->m_lBeg; j <= leaf->mp_csProjectedSpan->m_lEnd; j++ ) {
                    strArr->m_setInt( j - pMaxSpan->m_lBeg, j + 1 );
                }
                if( puncCase ) {
                    /* we regard rules of the POS tag node issue as lexicalized rules */
                    rTree->m_bLexicalized = true;
                }
            }
            else if( leaf->m_bAdmissibleNode ) {
                treeArr->m_setInt( i, --varIndex );
                for( long j=leaf->mp_csProjectedSpan->m_lBeg; j <= leaf->mp_csProjectedSpan->m_lEnd; j++ ) {
                    strArr->m_setInt( j - pMaxSpan->m_lBeg, varIndex );
                }
                varNameList->m_addObject( leaf->mp_szNodeLabel );
            }
        }
        
        /* generate all final-form string-side code arrays */
        list_c* strCodeArrs = m_genAllStrCodeArray( strArr, pMaxSpan, pMinSpan, maxVarNum, maxWdNum, maxUaInside, maxUaOutside );
        
        /* generate and output rules */
        if( strCodeArrs->m_getLength() > 0 ) {
            m_outputGenRule( rTree, treeArr, strCodeArrs, varNameList, isT2sRule, oformat, outF );
        }
        
        delete strCodeArrs;
        delete strArr;
        delete treeArr;
        delete varNameList;
    
    }
    
    list_c* rule_xhs_c::m_genAllStrCodeArray( array_c* seedArr, span_c* pMaxSpan, span_c* pMinSpan, \
        long maxVarNum, long maxWdNum, long maxUaInside, long maxUaOutside ) {

        list_c* codeArrs = new list_c( g_free_array );
        list_c* tmpArrs = new list_c( g_free_array );

        long uaInRuleInitCnt = 0;
        long uaOutRuleInitCnt = 0;
        long rBegIdx = pMinSpan->m_lBeg - pMaxSpan->m_lBeg;
        long rEndIdx = pMinSpan->m_lEnd - pMaxSpan->m_lBeg;
        for( long i=0; i < seedArr->m_getSize(); i++ ) {
            if( seedArr->m_getInt( i ) == 0 ) {
                if( rBegIdx <= i && i <= rEndIdx ) {
                    /* unaligned words inside a rule */
                    ++uaInRuleInitCnt;
                }
                else {
                    /* unaligned words at boundaries of a rule */
                    ++uaOutRuleInitCnt;
                }
            }
        }
        tmpArrs->m_addObject( seedArr->m_copy() );

        /* generate tempory string-side code arrays containing all unaligned word permutations from the seed string-side code array */
        if( maxUaInside > 0 || maxUaOutside > 0 ) {
            long uaPre = -1, uaBeg = 0, uaEnd = -1, uaPost = -1, arrSz = seedArr->m_getSize();
            while( 1 ) {
                long tempCodeArrNum = tmpArrs->m_getLength();
                if( tempCodeArrNum >= MAX_RULE_NUM_ONE_ANODE ) {
                    break;
                }
                /* find the start position of a consective unaligned word range */
                while( uaBeg < arrSz && seedArr->m_getInt( uaBeg ) != 0 ) {
                    ++uaBeg;
                }
                if( uaBeg >= arrSz ) {
                    /* no unaligned word left */
                    break;
                }
                /* find the end position of the consective unaligned word range */
                uaPre = uaBeg - 1;
                uaEnd = uaBeg + 1;
                while( uaEnd < arrSz && seedArr->m_getInt( uaEnd ) == 0 ) {
                    ++uaEnd;
                }
                uaPost = uaEnd;
                --uaEnd;
                /* generate code array according to unaligned word range */
                /* there are six types of the generation situation
                 * 0). headed by left boundary unaligned word
                 * 1). tailed by right boundary unaligned word
                 * 2). unaligned word range is headed and tailed by both "normal word"
                 * 3). unaligned word range is headed and tailed by a "normal word" and a "variable"
                 * 4). unaligned word range is headed and tailed by a "variable" and a "normal word"
                 * 5). unaligned word range is headed and tailed by both "variable"
                 */
                long type = 0;
                if( uaPre == -1 && uaPost < arrSz ) {
                    type = 0;
                }
                else if( uaPre >= 0 && uaPost == arrSz ) {
                    type = 1;
                }
                else if( uaPre == -1 && uaPost == arrSz ) {
                    /* impossible if the rule generation operation is imposed on admissible tree */
                    delete tmpArrs;
                    return codeArrs;
                }
                else {
                    long leftVal = seedArr->m_getInt( uaPre );
                    long rightVal = seedArr->m_getInt( uaPost );
                    bool isLeftNorm = ( leftVal >= 1 ? true : false );
                    bool isRightNorm = ( rightVal >= 1 ? true : false );
                    if( isLeftNorm && isRightNorm )
                        type = 2;
                    else if( isLeftNorm && !isRightNorm )
                        type = 3;
                    else if( !isLeftNorm && isRightNorm )
                        type = 4;
                    else if( !isLeftNorm && !isRightNorm )
                        type = 5;
                }
                m_genTempStrCodeArray( pMaxSpan->m_lBeg, uaBeg, uaEnd, type, rBegIdx, rEndIdx, \
                    uaInRuleInitCnt, uaOutRuleInitCnt, maxUaInside, maxUaOutside, tmpArrs );
                /* update indices */
                uaBeg = uaEnd + 1;
            }
        }
        
        /* simplify the tempory string-side code arrays to form final string-side code arrays */
        m_simplifyCodeArray( tmpArrs, maxVarNum, maxWdNum, codeArrs );

        delete tmpArrs;
        return codeArrs;

    }
    
    void rule_xhs_c::m_genTempStrCodeArray( long idxBeg, long uaBeg, long uaEnd, long type, \
        long rBegIdx, long rEndIdx, long initUaInside, long initUaOutside, long maxUaInside, long maxUaOutside, list_c* tmpList ) {
    
        long tmpListLen = tmpList->m_getLength();
        for( long i=0; i < tmpListLen; i++ ) {
            array_c* oArr = ( array_c* ) tmpList->m_getObject( i );
            array_c* nArr = NULL;
            switch( type ) {
            case 0:
            case 4:
                for( long j=uaEnd; j >= uaBeg; j-- ) {
                    nArr = oArr->m_copy();
                    for( long k=uaEnd; k >= j; k-- )
                        nArr->m_setInt( k, idxBeg + k + 1 );
                    if( true == m_checkUnalignNum( nArr, rBegIdx, rEndIdx, initUaInside, initUaOutside, maxUaInside, maxUaOutside ) ) {
                        tmpList->m_addObject( nArr );
                    }
                    else {
                        delete nArr;
                        break;
                    }
                }
                break;
            case 1:
            case 3:
                for( long j=uaBeg; j <= uaEnd; j++ ) {
                    nArr = oArr->m_copy();
                    for( long k=uaBeg; k <= j; k++ )
                        nArr->m_setInt( k, idxBeg + k + 1 );
                    if( true == m_checkUnalignNum( nArr, rBegIdx, rEndIdx, initUaInside, initUaOutside, maxUaInside, maxUaOutside ) ) {
                        tmpList->m_addObject( nArr );
                    }
                    else {
                        delete nArr;
                        break;
                    }
                }
                break;
            case 2:
                nArr = oArr->m_copy();
                for( long j=uaBeg; j <= uaEnd; j++ ) {
                    nArr->m_setInt( j, idxBeg + j + 1 );
                }
                if( true == m_checkUnalignNum( nArr, rBegIdx, rEndIdx, initUaInside, initUaOutside, maxUaInside, maxUaOutside ) ) {
                    tmpList->m_addObject( nArr );
                }
                else {
                    delete nArr;
                }
                break;
            case 5:
                for( long j=uaBeg; j <= uaEnd; j++ ) {
                    for( long k=j; k <= uaEnd; k++ ) {
                        nArr = oArr->m_copy();
                        for( long m=j; m <= k; m++ )
                            nArr->m_setInt( m, idxBeg + m + 1 );
                        if( true == m_checkUnalignNum( nArr, rBegIdx, rEndIdx, initUaInside, initUaOutside, maxUaInside, maxUaOutside ) ) {
                            tmpList->m_addObject( nArr );
                        }
                        else {
                            delete nArr;
                            break;
                        }
                    }
                }

                /*
                nArr = oArr->m_copy();
                nArr->m_setInt( uaBeg, idxBeg + uaBeg + 1 );
                if( true == m_checkUnalignNum( nArr, rBegIdx, rEndIdx, initUaInside, initUaOutside, maxUaInside, maxUaOutside ) ) {
                    tmpList->m_addObject( nArr );
                }
                else {
                    delete nArr;
                }

                if( uaBeg < uaEnd ) {
                    nArr = oArr->m_copy();
                    nArr->m_setInt( uaEnd, idxBeg + uaEnd + 1 );
                    if( true == m_checkUnalignNum( nArr, rBegIdx, rEndIdx, initUaInside, initUaOutside, maxUaInside, maxUaOutside ) ) {
                        tmpList->m_addObject( nArr );
                    }
                    else {
                        delete nArr;
                    }

                    nArr = oArr->m_copy();
                    nArr->m_setInt( uaBeg, idxBeg + uaBeg + 1 );
                    nArr->m_setInt( uaEnd, idxBeg + uaEnd + 1 );
                    if( true == m_checkUnalignNum( nArr, rBegIdx, rEndIdx, initUaInside, initUaOutside, maxUaInside, maxUaOutside ) ) {
                        tmpList->m_addObject( nArr );
                    }
                    else {
                        delete nArr;
                    }
                }
                */

                break;
            }
        }

    }

    void rule_xhs_c::m_simplifyCodeArray( list_c* tmpList, long maxVarNum, long maxWdNum, list_c* codeArrs ) {

        for( long i=0; i < tmpList->m_getLength(); i++ ) {
            array_c* oArr = ( array_c* ) tmpList->m_getObject( i );
            array_c* nArr = new array_c();
            long varNum = 0, wdNum = 0;
            long nArrSz = 0, preVal;
            for( long j=0; j < oArr->m_getSize(); j++ ) {
                long val = oArr->m_getInt( j );
                if( val == 0 ) {
                    continue;
                }
                if( nArrSz > 0 && val == preVal ) {
                    continue;
                }
                if( val > 0 ) {
                    nArr->m_setInt( nArrSz, val - 1 );
                    ++wdNum;
                }
                else if( val < 0 ) {
                    nArr->m_setInt( nArrSz, val );
                    ++varNum;
                }
                ++nArrSz;
                preVal = val;
            }
            if( varNum <= maxVarNum && wdNum <= maxWdNum ) {
                array_c* retArr = new array_c( nArrSz );
                for( long k=0; k < nArrSz; k++ ) {
                    retArr->m_setInt( k, nArr->m_getInt( k ) );
                }
                codeArrs->m_addObject( retArr );
            }
            delete nArr;
        }

    }

    bool rule_xhs_c::m_checkUnalignNum( array_c* codeArr, long rBegIdx, long rEndIdx, \
        long initUaInside, long initUaOutside, long maxUaInside, long maxUaOutside ) {

        long leftUaInside = 0;
        long leftUaOutside = 0;
        long codeArrSz = codeArr->m_getSize();
        for( long i=0; i < codeArrSz; i++ ) {
            long val = codeArr->m_getInt( i );
            if( 0 == val ) {
                if( rBegIdx <= i && i <= rEndIdx ) {
                    ++leftUaInside;
                }
                else {
                    ++leftUaOutside;
                }
            }
        }
        return ( initUaInside - leftUaInside <= maxUaInside ) && ( initUaOutside - leftUaOutside <= maxUaOutside );

    }
    
    void rule_xhs_c::m_outputGenRule( tree_c* tree, array_c* treeArr, list_c* strArrs, \
        list_c* varNameList, bool isT2sRule, oformat_e oformat, FILE* outF ) {

        char treeRule[MAX_CHAR_NUM_ONE_BUFF] = {0};
        char treeRuleVarTag[MAX_CHAR_NUM_ONE_BUFF] = {0};
        char treeRuleVarId[MAX_CHAR_NUM_ONE_BUFF] = {0};
        array_c* tree2StrVarMap = new array_c();
        
        /* generate string of the tree in bracket form */
        m_printTree( tree->mp_csRootNode, treeRule, true );
        string_c::ms_mvStr( treeRule, 1, true );
        string_c::ms_mvStr( treeRule, 1, false );
        /* generate string of the leaves of the tree with variables */
        m_genVarTagStr( treeArr, true, varNameList, treeRuleVarTag );
        /* generate string of the leaves of the tree with variable indices */
        if( !isT2sRule ) {
            array_c* sca = ( array_c* ) strArrs->m_getObject( 0 );
            m_mapVarIndex( sca, tree2StrVarMap );
        }
        m_genVarIdStr( treeArr, true, isT2sRule, tree2StrVarMap, treeRuleVarId );

        /* generate strings of the string-side code array */
        long strArrNum = strArrs->m_getLength();
        for( long i=0; i < strArrNum; i++ ) {
            char strRuleVarTag[MAX_CHAR_NUM_ONE_BUFF] = {0};
            char strRuleVarId[MAX_CHAR_NUM_ONE_BUFF] = {0};
            char alignment[MAX_CHAR_NUM_ONE_BUFF] = {0};
            array_c* sca = ( array_c* ) strArrs->m_getObject( i );
            m_genVarTagStr( sca, false, varNameList, strRuleVarTag );
            m_genVarIdStr( sca, false, isT2sRule, tree2StrVarMap, strRuleVarId );
            /* generate rule alignment */
            m_genRuleAlignment( treeArr, sca, isT2sRule, alignment );

            char* rLabel = tree->mp_csRootNode->mp_csParseNode->mp_szNodeLabel;
            bool bLexRule = tree->m_bLexicalized;
            bool bCompRule = ( tree->m_lComposeTimes > 0 ? true : false );
            if( isT2sRule ) {
                /* tree-to-string rules */
                if( oformat == OFT ) {
                    fprintf( outF, "%s ||| %s ||| %s ||| %d %d ||| %s ||| %s\n", treeRule, treeRuleVarTag, \
                        strRuleVarTag, bLexRule, bCompRule, strRuleVarId, alignment );
                }
                else {
                    fprintf( outF, "%s ||| %s ||| %s ||| %d %d ||| %s ||| %s ||| %s ||| %s\n", \
                        rLabel, treeRuleVarTag, strRuleVarTag, bLexRule, bCompRule, strRuleVarId, alignment, treeRule, "NULL" );
                }
            }
            else {
                /* string-to-tree rules */
                if( oformat == OFT ) {
                    fprintf( outF, "%s ||| %s ||| %s ||| %d %d ||| %s ||| %s\n", rLabel, treeRuleVarTag, \
                        strRuleVarTag, bLexRule, bCompRule, treeRuleVarId, alignment );
                }
                else {
                    fprintf( outF, "%s ||| %s ||| %s ||| %d %d ||| %s ||| %s ||| %s ||| %s\n", \
                        rLabel, strRuleVarTag, treeRuleVarTag, bLexRule, bCompRule, treeRuleVarId, alignment, "NULL", treeRule );
                }
            }
        }
        
        delete tree2StrVarMap;
    
    }
    
    void rule_xhs_c::m_genVarTagStr( array_c* codeArr, bool onTreeSide, list_c* varNameList, char* outBuff ) {

        array_c* tokens;
        if( onTreeSide )
            tokens = ( m_bOnSrcSide ? mp_csAlignment->mp_csSrc2TgtAlgn : mp_csAlignment->mp_csTgt2SrcAlgn );
        else
            tokens = ( !m_bOnSrcSide ? mp_csAlignment->mp_csSrc2TgtAlgn : mp_csAlignment->mp_csTgt2SrcAlgn );

        for( long i=0; i < codeArr->m_getSize(); i++ ) {
            long idx = codeArr->m_getInt( i );
            char* token;
            if( idx >= 0 ) {
                token_algn_info_c* ta = ( token_algn_info_c* ) tokens->m_getObject( idx );
                token = ta->mp_szTok;
            }
            else {
                token = ( char* ) varNameList->m_getObject( -idx - 1 );
                strcat( outBuff, "#" );
            }
            strcat( outBuff, token );
            strcat( outBuff, " " );
        }

        string_c::RemoveRightSpaces( outBuff );

    }
    
    void rule_xhs_c::m_mapVarIndex( array_c* codeArr, array_c* mapArr ) {
    
        long varNum = 0;
        for( long i=0; i < codeArr->m_getSize(); i++ ) {
            long idx = codeArr->m_getInt( i );
            if( idx < 0 ) {
                mapArr->m_setInt( -idx - 1, varNum );
                ++varNum;
            }
        }
    
    }
    
    void rule_xhs_c::m_genVarIdStr( array_c* codeArr, bool onTreeSide, \
        bool isT2sRule, array_c* mapArr, char* outBuff ) {

        array_c* tokens;
        if( onTreeSide )
            tokens = ( m_bOnSrcSide ? mp_csAlignment->mp_csSrc2TgtAlgn : mp_csAlignment->mp_csTgt2SrcAlgn );
        else
            tokens = ( !m_bOnSrcSide ? mp_csAlignment->mp_csSrc2TgtAlgn : mp_csAlignment->mp_csTgt2SrcAlgn );

        for( long i=0; i < codeArr->m_getSize(); i++ ) {
            long idx = codeArr->m_getInt( i );
            if( idx >= 0 ) {
                token_algn_info_c* ta = ( token_algn_info_c* ) tokens->m_getObject( idx );
                strcat( outBuff, ta->mp_szTok );
                strcat( outBuff, " " );
            }
            else {
                char buff[256];
                long newIdx;
                if( isT2sRule ) {
                    newIdx = -idx - 1;
                }
                else {
                    newIdx = mapArr->m_getInt( -idx - 1 );
                }
                sprintf( buff, "#%ld", newIdx + g_lVarBeginIndex );
                strcat( outBuff, buff );
                strcat( outBuff, " " );
            }
        }

        string_c::RemoveRightSpaces( outBuff );

    }
    
    void rule_xhs_c::m_genRuleAlignment( array_c* treeCodeArr, array_c* strCodeArr, bool isT2sRule, char* outBuff ) {
    
        array_c* srcCodeArr = ( isT2sRule ? treeCodeArr : strCodeArr );
        array_c* tarCodeArr = ( !isT2sRule ? treeCodeArr : strCodeArr );
        array_c* srcTokens;
        if( isT2sRule ) {
            srcTokens = ( m_bOnSrcSide ? mp_csAlignment->mp_csSrc2TgtAlgn : mp_csAlignment->mp_csTgt2SrcAlgn );
        }
        else {
            srcTokens = ( !m_bOnSrcSide ? mp_csAlignment->mp_csSrc2TgtAlgn : mp_csAlignment->mp_csTgt2SrcAlgn );
        }
        
        for( long i=0; i < srcCodeArr->m_getSize(); i++ ) {
            long srcIdx = srcCodeArr->m_getInt( i );
            if( srcIdx >= 0 ) {
                /* word alignment */
                token_algn_info_c* ta = ( token_algn_info_c* ) srcTokens->m_getObject( srcIdx );
                list_c* alignList = ta->mp_csAlgnIndices;
                for( long j=0; j < alignList->m_getLength(); j++ ) {
                    long tarIdx = alignList->m_getInt( j );
                    for( long k=0; k < tarCodeArr->m_getSize(); k++ ) {
                        long idxVal = tarCodeArr->m_getInt( k );
                        if( tarIdx == idxVal ) {
                            char align[256];
                            sprintf( align, "%ld-%ld", i, k );
                            strcat( outBuff, align );
                            strcat( outBuff, " " );
                            break;
                        }
                    }
                }
            }
            else {
                /* variable alignment */
                for( long k=0; k < tarCodeArr->m_getSize(); k++ ) {
                    long tarIdx = tarCodeArr->m_getInt( k );
                    if( srcIdx == tarIdx ) {
                        char align[256];
                        sprintf( align, "%ld-%ld", i, k );
                        strcat( outBuff, align );
                        strcat( outBuff, " " );
                        break;
                    }
                }
            }
        }

        string_c::RemoveRightSpaces( outBuff );

    }
    
    void g_free_array( const void* array ) {
    
        array_c* arr = ( array_c* ) array;
        delete arr;
    
    }
    
    /*
     *
     * Syntax-based Rule Extractor
     *
     */
    syntax_rule_extractor_c::syntax_rule_extractor_c( int argc, char* argv[] ) {

        if( argc < 5 ) {
            m_usage( argv[0] );
            exit( 1 );
        }
        configMng_c::ms_create( argc, argv );
        mp_csAlignment = new algn_sent_c();
        mp_csSrcRuleSide = new rule_xhs_c( mp_csAlignment, true );
        mp_csTarRuleSide = new rule_xhs_c( mp_csAlignment, false );
        m_init( argv[0] );

    }
    
    syntax_rule_extractor_c::~syntax_rule_extractor_c() {

        m_clear();
        delete mp_csAlignment;
        delete mp_csSrcRuleSide;
        delete mp_csTarRuleSide;
        configMng_c::ms_destroy();

    }
    
    void syntax_rule_extractor_c::m_extract() {
    
        time_c timer;
        long lineCnt = 0;
        
        fprintf( stderr, "Start extracting rules at time: %s\n", timer.m_getNowTimeString() );
        timer.m_startTimer();
        while( 1 ) {
            /* read sentences and parse trees from files */
            char* srcSent = string_c::ms_readLine( mp_stSrcFp );
            char* tarSent = string_c::ms_readLine( mp_stTarFp );
            char* wdAlign = string_c::ms_readLine( mp_stAlignFp );
            if( !srcSent || !tarSent || !wdAlign )
                break;
            char* srcParse = NULL;
            char* tarParse = NULL;
            if( m_enExtractType == S2T_I || m_enExtractType == T2S_S || m_enModelType == T2T ) {
                srcParse = string_c::ms_readLine( mp_stSrcParseFp );
                if( !srcParse )
                    break;
            }
            if( m_enExtractType == S2T_S || m_enExtractType == T2S_I || m_enModelType == T2T ) {
                tarParse = string_c::ms_readLine( mp_stTarParseFp );
                if( !tarParse )
                    break;
            }
            
            /* extract rules */
            if( mp_csAlignment->m_init( srcSent, tarSent, wdAlign, lineCnt ) ) {
                switch( m_enExtractType ) {
                case S2T_S:
                    if( mp_csTarRuleSide->m_init( tarParse, tarSent, m_enMethodType, m_lMaxCompose, m_lMaxTreeDepth ) ) {
                        mp_csTarRuleSide->m_genRule( false, m_lMaxVarNum, m_lMaxWordNum, m_lMaxUaInside, m_lMaxUaOutside, m_enRuleFormat, mp_stOutputFp );
                        mp_csTarRuleSide->m_clear();
                    }
                    break;
                case T2S_S:
                    if( mp_csSrcRuleSide->m_init( srcParse, srcSent, m_enMethodType, m_lMaxCompose, m_lMaxTreeDepth ) ) {
                        mp_csSrcRuleSide->m_genRule( true, m_lMaxVarNum, m_lMaxWordNum, m_lMaxUaInside, m_lMaxUaOutside, m_enRuleFormat, mp_stOutputFp );
                        mp_csSrcRuleSide->m_clear();
                    }
                    break;
                case T2T_S:
                {
                    bool leftFlag = mp_csSrcRuleSide->m_init( srcParse, srcSent, m_enMethodType, m_lMaxCompose, m_lMaxTreeDepth );
                    bool rightFlag = mp_csTarRuleSide->m_init( tarParse, tarSent, m_enMethodType, m_lMaxCompose, m_lMaxTreeDepth );
                    if( leftFlag && rightFlag ) {
                        m_genT2tRule( mp_csSrcRuleSide, mp_csTarRuleSide, m_enRuleFormat, mp_stOutputFp );
                    }
                    if( leftFlag )
                        mp_csSrcRuleSide->m_clear();
                    if( rightFlag )
                        mp_csTarRuleSide->m_clear();
                }
                    break;
                case S2T_I:
                    if( mp_csSrcRuleSide->m_init( srcParse, srcSent, m_enMethodType, m_lMaxCompose, m_lMaxTreeDepth ) ) {
                        mp_csSrcRuleSide->m_genRule( false, m_lMaxVarNum, m_lMaxWordNum, m_lMaxUaInside, m_lMaxUaOutside, m_enRuleFormat, mp_stOutputFp );
                        mp_csSrcRuleSide->m_clear();
                    }
                    break;
                case T2S_I:
                    if( mp_csTarRuleSide->m_init( tarParse, tarSent, m_enMethodType, m_lMaxCompose, m_lMaxTreeDepth ) ) {
                        mp_csTarRuleSide->m_genRule( true, m_lMaxVarNum, m_lMaxWordNum, m_lMaxUaInside, m_lMaxUaOutside, m_enRuleFormat, mp_stOutputFp );
                        mp_csTarRuleSide->m_clear();
                    }
                    break;
                case T2T_I:
                {
                    bool leftFlag = mp_csTarRuleSide->m_init( tarParse, tarSent, m_enMethodType, m_lMaxCompose, m_lMaxTreeDepth );
                    bool rightFlag = mp_csSrcRuleSide->m_init( srcParse, srcSent, m_enMethodType, m_lMaxCompose, m_lMaxTreeDepth );
                    if( leftFlag && rightFlag ) {
                        m_genT2tRule( mp_csTarRuleSide, mp_csSrcRuleSide, m_enRuleFormat, mp_stOutputFp );
                    }
                    if( leftFlag )
                        mp_csTarRuleSide->m_clear();
                    if( rightFlag )
                        mp_csSrcRuleSide->m_clear();
                }
                    break;
                }
                mp_csAlignment->m_clear();
            }
            
            delete[] srcSent;
            delete[] tarSent;
            delete[] wdAlign;
            delete[] srcParse;
            delete[] tarParse;
            
            if( ++lineCnt % MAX_LINE_NUM_ONE_HINT == 0 ) {
                fprintf( stderr, "\rprocessed %ld lines...", lineCnt );
            }
        }

        fprintf( stderr, "\rprocessed %ld lines...\n", lineCnt );
        timer.m_endTimer();
        fprintf( stderr, "End extracting rules at time: %s\n", timer.m_getNowTimeString() );
        double tm = timer.m_getTimerDiff();
        fprintf( stderr, "Total elapsed time: %f seconds\n", tm );
        fprintf( stderr, "Average rule extraction speed: %f sent/sec\n", lineCnt / tm );
    
    }
    
    void syntax_rule_extractor_c::m_genT2tRule( rule_xhs_c* leftRs, rule_xhs_c* rightRs, oformat_e oformat, FILE* out ) {

        for( long i=0; i < leftRs->m_lAdmNodeNum; i++ ) {
            /* for every admissible node in the left-hand-side admissible tree,
             * find its all corresponding projected right-hand-side spans.
             */
            node_c* leftTreeNode = ( node_c* ) leftRs->mp_csAdmissibleNodes->m_getObject( i );
            list_c* projSpanList = leftRs->mp_csAlignment->m_getAllClosureSpans( leftTreeNode->mp_csCoveredLeavesSpan, \
                        leftRs->m_bOnSrcSide, m_lMaxUaOutside );
            for( long j=0; j < projSpanList->m_getLength(); j++ ) {
                /* for every possible projected right-hand-side span,
                 * if a node on that side subsumes the span and it's an admissible node,
                 * extract tree-to-tree rules based on the left-hand-side and right-hand-side nodes.
                 */
                span_c* pSpan = ( span_c* ) projSpanList->m_getObject( j );
                node_c* rightTreeNode = rightRs->mppp_csParseChart[pSpan->m_lBeg][pSpan->m_lEnd + 1];
                while( rightTreeNode ) {
                    if( rightTreeNode->m_bAdmissibleNode ) {
                        m_genT2tRule( leftTreeNode, rightTreeNode, leftRs->m_bOnSrcSide, oformat, out );
                    }
                    /* in case of unary admissible nodes */
                    if( rightTreeNode->mp_csParent && rightTreeNode->mp_csParent->m_bUnaryNode ) {
                        rightTreeNode = rightTreeNode->mp_csParent;
                    }
                    else {
                        rightTreeNode = NULL;
                    }
                }
            }
            delete projSpanList;
        }

    }
    
    void syntax_rule_extractor_c::m_genT2tRule( node_c* leftTreeNode, node_c* rightTreeNode, bool isSrc2Tgt, oformat_e oformat, FILE* out ) {

        list_c* leftTrees = leftTreeNode->mp_csSubtreeList;
        list_c* rightTrees = rightTreeNode->mp_csSubtreeList;
        for( long i=0; i < leftTrees->m_getLength(); i++ ) {
            tree_c* leftTree = ( tree_c* ) leftTrees->m_getObject( i );
            for( long j=0; j < rightTrees->m_getLength(); j++ ) {
                tree_c* rightTree = ( tree_c* ) rightTrees->m_getObject( j );
                m_genT2tRule( leftTree, rightTree, isSrc2Tgt, oformat, out );
            }
        }

    }

    void syntax_rule_extractor_c::m_genT2tRule( tree_c* lTree, tree_c* rTree, bool isSrc2Tgt, oformat_e oformat, FILE* out ) {

        long lVarNum = lTree->mp_csAdmissibleNodes->m_getLength();
        long rVarNum = rTree->mp_csAdmissibleNodes->m_getLength();

        if( lVarNum == rVarNum ) {
            array_c* l2rVarMap = new array_c( lVarNum );
            array_c* r2lVarMap = new array_c( lVarNum );

            if( m_mapVarNodes( lTree, rTree, l2rVarMap, r2lVarMap ) ) {
                char srcVarTagStr[MAX_CHAR_NUM_ONE_BUFF] = {0};
                char tarVarTagStr[MAX_CHAR_NUM_ONE_BUFF] = {0};
                char tarVarIdStr[MAX_CHAR_NUM_ONE_BUFF] = {0};
                char alignment[MAX_CHAR_NUM_ONE_BUFF] = {0};
                bool bLexRule = ( lTree->m_bLexicalized || rTree->m_bLexicalized );
                bool bComposeRule = ( lTree->m_lComposeTimes > 0 || rTree->m_lComposeTimes > 0 );
                m_genVarTagStr( lTree->mp_csLeafNodeList, rTree->mp_csAdmissibleNodes, l2rVarMap, srcVarTagStr );
                m_genVarTagStr( rTree->mp_csLeafNodeList, lTree->mp_csAdmissibleNodes, r2lVarMap, tarVarTagStr );
                m_genVarIdStr( rTree->mp_csLeafNodeList, r2lVarMap, tarVarIdStr );
                m_genAlignment( lTree->mp_csLeafNodeList, rTree->mp_csLeafNodeList, r2lVarMap, isSrc2Tgt, alignment );

                /* generate tree rules */
                char lTreeRule[MAX_CHAR_NUM_ONE_BUFF] = {0};
                char rTreeRule[MAX_CHAR_NUM_ONE_BUFF] = {0};
                if( isSrc2Tgt ) {
                    mp_csSrcRuleSide->m_printTree( lTree->mp_csRootNode, lTreeRule, true );
                    mp_csTarRuleSide->m_printTree( rTree->mp_csRootNode, rTreeRule, true );
                }
                else {
                    mp_csTarRuleSide->m_printTree( lTree->mp_csRootNode, lTreeRule, true );
                    mp_csSrcRuleSide->m_printTree( rTree->mp_csRootNode, rTreeRule, true );
                }
                string_c::ms_mvStr( lTreeRule, 1, true );
                string_c::ms_mvStr( lTreeRule, 1, false );
                string_c::ms_mvStr( rTreeRule, 1, true );
                string_c::ms_mvStr( rTreeRule, 1, false );

                if( oformat == OFT ) {
                    fprintf( out, "%s || %s ||| %s ||| %s ||| %d %d ||| %s ||| %s\n", lTreeRule, rTreeRule, srcVarTagStr, \
                        tarVarTagStr, bLexRule, bComposeRule, tarVarIdStr, alignment );
                }
                else {
                    fprintf( out, "%s=%s ||| %s ||| %s ||| %d %d ||| %s ||| %s ||| %s ||| %s\n", \
                        rTree->mp_csRootNode->mp_csParseNode->mp_szNodeLabel, lTree->mp_csRootNode->mp_csParseNode->mp_szNodeLabel, \
                        srcVarTagStr, tarVarTagStr, bLexRule, bComposeRule, tarVarIdStr, alignment, lTreeRule, rTreeRule );
                }
            }

            delete l2rVarMap;
            delete r2lVarMap;
        }

    }

    bool syntax_rule_extractor_c::m_mapVarNodes( tree_c* lTree, tree_c* rTree, array_c* l2rVarMap, array_c* r2lVarMap ) {

        long varNum = lTree->mp_csAdmissibleNodes->m_getLength();
        for( long i=0; i < varNum; i++ ) {
            pnode_c* lNode = ( pnode_c* ) lTree->mp_csAdmissibleNodes->m_getObject( i );
            bool hasMappedNode = false;
            for( long j=0; j < varNum; j++ ) {
                if( r2lVarMap->m_getInt( j ) == 0 ) {
                    pnode_c* rNode = ( pnode_c* ) rTree->mp_csAdmissibleNodes->m_getObject( j );
                    if( m_isAlignedNode( lNode->mp_csParseNode, rNode->mp_csParseNode ) ) {
                        l2rVarMap->m_setInt( i, j + 1 );
                        r2lVarMap->m_setInt( j, i + 1 );
                        hasMappedNode = true;
                        break;
                    }
                }
            }
            if( !hasMappedNode )
                return false;
        }
        for( long i=0; i < varNum; i++ ) {
            if( r2lVarMap->m_getInt( i ) == 0 ) {
                return false;
            }
        }
        return true;

    }

    bool syntax_rule_extractor_c::m_isAlignedNode( node_c* lNode, node_c* rNode ) {

        bool straightContainFlag = rNode->mp_csCoveredLeavesSpan->m_contain( lNode->mp_csProjectedSpan );
        bool invertedContainFlag = lNode->mp_csCoveredLeavesSpan->m_contain( rNode->mp_csProjectedSpan );
        return ( straightContainFlag && invertedContainFlag );

    }

    void syntax_rule_extractor_c::m_genVarTagStr( list_c* leafNodes, list_c* pAdmNodes, array_c* varMap, char* outBuff ) {

        long varNum = 0;
        for( long i=0; i < leafNodes->m_getLength(); i++ ) {
            pnode_c* pLeafNd = ( pnode_c* ) leafNodes->m_getObject( i );
            node_c* leafNd = pLeafNd->mp_csParseNode;
            if( leafNd->m_bTerminalNode ) {
                strcat( outBuff, leafNd->mp_szNodeLabel );
            }
            else if( leafNd->m_bAdmissibleNode ) {
                long varIdx = varMap->m_getInt( varNum ) - 1;
                char* varName = ( ( pnode_c* ) pAdmNodes->m_getObject( varIdx ) )->mp_csParseNode->mp_szNodeLabel;
                strcat( outBuff, "#" );
                strcat( outBuff, varName );
                strcat( outBuff, "=" );
                strcat( outBuff, leafNd->mp_szNodeLabel );
                ++varNum;
            }
            strcat( outBuff, " " );
        }
        string_c::RemoveRightSpaces( outBuff );

    }

    void syntax_rule_extractor_c::m_genVarIdStr( list_c* leafNodes, array_c* varMap, char* outBuff ) {

        long varNum = 0;
        for( long i=0; i < leafNodes->m_getLength(); i++ ) {
            pnode_c* pLeafNd = ( pnode_c* ) leafNodes->m_getObject( i );
            node_c* leafNd = pLeafNd->mp_csParseNode;
            if( leafNd->m_bTerminalNode ) {
                strcat( outBuff, leafNd->mp_szNodeLabel );
            }
            else if( leafNd->m_bAdmissibleNode ) {
                long varIdx = varMap->m_getInt( varNum ) - 1;
                sprintf( outBuff, "%s#%d", outBuff, varIdx + g_lVarBeginIndex );
                ++varNum;
            }
            strcat( outBuff, " " );
        }
        string_c::RemoveRightSpaces( outBuff );

    }

    void syntax_rule_extractor_c::m_genAlignment( list_c* lNodes, list_c* rNodes, array_c* varMap, bool isSrc2Tgt, char* outBuff ) {

        long lNodeNum = lNodes->m_getLength();
        long rNodeNum = rNodes->m_getLength();
        array_c* lCodeArr = new array_c( lNodeNum );
        array_c* rCodeArr = new array_c( rNodeNum );
        m_genCodeArr( lNodes, NULL, lCodeArr );
        m_genCodeArr( rNodes, varMap, rCodeArr );

        array_c* srcTokens = ( isSrc2Tgt ? mp_csAlignment->mp_csSrc2TgtAlgn : mp_csAlignment->mp_csTgt2SrcAlgn );
        for( long i=0; i < lNodeNum; i++ ) {
            long srcIdx = lCodeArr->m_getInt( i );
            if( srcIdx >= 0 ) {
                token_algn_info_c* tai = ( token_algn_info_c* ) srcTokens->m_getObject( srcIdx );
                list_c* aList = tai->mp_csAlgnIndices;
                for( long j=0; j < tai->m_lAlgnCnt; j++ ) {
                    long tarIdx = aList->m_getInt( j );
                    for( long k=0; k < rNodeNum; k++ ) {
                        long idxVal = rCodeArr->m_getInt( k );
                        if( tarIdx == idxVal ) {
                            sprintf( outBuff, "%s%ld-%ld ", outBuff, i, k );
                            break;
                        }
                    }
                }
            }
            else {
                for( long k=0; k < rNodeNum; k++ ) {
                    long idxVal = rCodeArr->m_getInt( k );
                    if( srcIdx == idxVal ) {
                        sprintf( outBuff, "%s%ld-%ld ", outBuff, i, k );
                        break;
                    }
                }
            }
        }
        string_c::RemoveRightSpaces( outBuff );

        delete lCodeArr;
        delete rCodeArr;

    }

    void syntax_rule_extractor_c::m_genCodeArr( list_c* nodes, array_c* varMap, array_c* codeArr ) {

        long nodeNum = nodes->m_getLength();
        long varIdx = 0;
        for( long i=0; i < nodeNum; i++ ) {
            node_c* nd = ( ( pnode_c* ) nodes->m_getObject( i ) )->mp_csParseNode;
            if( nd->m_bTerminalNode ) {
                codeArr->m_setInt( i, nd->mp_csCoveredLeavesSpan->m_lBeg );
            }
            else if( nd->m_bAdmissibleNode ) {
                if( varMap == NULL ) {
                    codeArr->m_setInt( i, --varIdx );
                }
                else {
                    long varId = 0 - varMap->m_getInt( varIdx );
                    codeArr->m_setInt( i, varId );
                    ++varIdx;
                }
            }
        }

    }

    void syntax_rule_extractor_c::m_init( const char* proname ) {

        /* read in all parameters */
        char* modelStr = ( char* ) configMng_c::ms_getString( "model", "t2s" );
        char* methodStr = ( char* ) configMng_c::ms_getString( "method", "ghkm" );
        const char* srcFile = configMng_c::ms_getString( "src", NULL );
        const char* tarFile = configMng_c::ms_getString( "tar", NULL );
        const char* alignFile = configMng_c::ms_getString( "align", NULL );
        const char* srcParseFile = configMng_c::ms_getString( "srcparse", NULL );
        const char* tarParseFile = configMng_c::ms_getString( "tarparse", NULL );
        const char* outputFile = configMng_c::ms_getString( "output", NULL );
        m_lMaxCompose = configMng_c::ms_getInt( "compose", 2 );
        m_lMaxVarNum = configMng_c::ms_getInt( "varnum", 4 );
        m_lMaxWordNum = configMng_c::ms_getInt( "wordnum", 4 );
        m_lMaxUaInside = configMng_c::ms_getInt( "uain", 4 );
        m_lMaxUaOutside = configMng_c::ms_getInt( "uaout", 2 );
        m_lMaxTreeDepth = configMng_c::ms_getInt( "depth", MAX_NUM_LONG_TYPE_DATA );
        m_bInverseExtract = configMng_c::ms_getBool( "inverse", false );
        char* oformat = ( char* ) configMng_c::ms_getString( "oformat", "nft" );
        if( !strcmp( oformat, "oft" ) ) {
            m_enRuleFormat = OFT;
        }
        else {
            m_enRuleFormat = NFT;
        }
        g_lVarBeginIndex = configMng_c::ms_getInt( "varbase", 1 );

        /* parse parameter "-model" */
        string_c::ms_toLowercase( modelStr );
        if( !strcmp( modelStr, "s2t" ) ) {
            m_enModelType = S2T;
        }
        else if( !strcmp( modelStr, "t2s" ) ) {
            m_enModelType = T2S;
        }
        else if( !strcmp( modelStr, "t2t" ) ) {
            m_enModelType = T2T;
        }
        else {
            m_error( proname, "unrecognized value of parameter \"-model\"" );
        }
        /* parse parameter "-method" */
        string_c::ms_toLowercase( methodStr );
        if( !strcmp( methodStr, "ghkm" ) ) {
            m_enMethodType = GHKM;
        }
        else if( !strcmp( methodStr, "spmt" ) ) {
            m_enMethodType = SPMT;
        }
        else {
            m_error( proname, "unrecognized value of parameter \"-method\"" );
        }
        /* open source sentence file */
        if( !srcFile ) {
            m_error( proname, "parameter \"-src\" not specified" );
        }
        mp_stSrcFp = fopen( srcFile, "r" );
        if( !mp_stSrcFp ) {
            m_error( proname, "source file not existed", false );
        }
        /* open target sentence file */
        if( !tarFile ) {
            m_error( proname, "parameter \"-tar\" not specified" );
        }
        mp_stTarFp = fopen( tarFile, "r" );
        if( !mp_stTarFp ) {
            m_error( proname, "target file not existed", false );
        }
        /* open word alignment file */
        if( !alignFile ) {
            m_error( proname, "parameter \"-align\" not specified" );
        }
        mp_stAlignFp = fopen( alignFile, "r" );
        if( !mp_stAlignFp ) {
            m_error( proname, "word alignment file not existed", false );
        }
        /* open source parse tree file */
        mp_stSrcParseFp = NULL;
        if( !m_bInverseExtract && m_enModelType == T2S || \
                m_bInverseExtract && m_enModelType == S2T || \
                m_enModelType == T2T ) {
            if( !srcParseFile ) {
                m_error( proname, "parameter \"-srcparse\" not specified" );
            }
            mp_stSrcParseFp = fopen( srcParseFile, "r" );
            if( !mp_stSrcParseFp ) {
                m_error( proname, "source parse tree file not existed", false );
            }
        }
        /* open target parse tree file */
        mp_stTarParseFp = NULL;
        if( !m_bInverseExtract && m_enModelType == S2T || \
                m_bInverseExtract && m_enModelType == T2S || \
                m_enModelType == T2T ) {
            if( !tarParseFile ) {
                m_error( proname, "parameter \"-tarparse\" not specified" );
            }
            mp_stTarParseFp = fopen( tarParseFile, "r" );
            if( !mp_stTarParseFp ) {
                m_error( proname, "target parse tree file not existed", false );
            }
        }
        /* create output file */
        if( !outputFile ) {
            mp_stOutputFp = stdout;
        }
        else {
            mp_stOutputFp = fopen( outputFile, "w" );
            if( !mp_stOutputFp ) {
                m_error( proname, "create output file failed", false );
            }
        }
        
        /* decide type of rule extraction */
        if( !m_bInverseExtract && m_enModelType == S2T ) {
            m_enExtractType = S2T_S;
        }
        else if( !m_bInverseExtract && m_enModelType == T2S ) {
            m_enExtractType = T2S_S;
        }
        else if( !m_bInverseExtract && m_enModelType == T2T ) {
            m_enExtractType = T2T_S;
        }
        else if( m_bInverseExtract && m_enModelType == S2T ) {
            m_enExtractType = S2T_I;
        }
        else if( m_bInverseExtract && m_enModelType == T2S ) {
            m_enExtractType = T2S_I;
        }
        else if( m_bInverseExtract && m_enModelType == T2T ) {
            m_enExtractType = T2T_I;
        }

    }
    
    void syntax_rule_extractor_c::m_clear() {

        if( mp_stSrcFp ) {
            fclose( mp_stSrcFp );
        }
        if( mp_stTarFp ) {
            fclose( mp_stTarFp );
        }
        if( mp_stAlignFp ) {
            fclose( mp_stAlignFp );
        }
        if( mp_stSrcParseFp ) {
            fclose( mp_stSrcParseFp );
        }
        if( mp_stTarParseFp ) {
            fclose( mp_stTarParseFp );
        }
        if( mp_stOutputFp && mp_stOutputFp != stdout ) {
            fclose( mp_stOutputFp );
        }

    }
    
    void syntax_rule_extractor_c::m_error( const char* proname, const char* msg, bool bShowUsage ) {

        fprintf( stderr, "[ERROR]: %s!\n", msg );
        if( bShowUsage ) {
            fprintf( stderr, "please see the help infomation below\n" );
            fprintf( stderr, "\n" );
            m_usage( proname );
        }
        exit( 1 );

    }

    void syntax_rule_extractor_c::m_usage( const char* proname ) {

        fprintf( stderr, "Syntax-Based Rule Extractor (version 1.0.0.0)\n" );
        fprintf( stderr, "Copyright (C) 2011, NEU-NLPLab (http://www.nlplab.com/). All rights reserved.\n" );
        fprintf( stderr, "\n" );
        fprintf( stderr, "usage: %s params ...\n", proname );
        fprintf( stderr, "--------------------------------------------------------------------------------\n" );
        fprintf( stderr, "[params]:\n" );
        fprintf( stderr, "  -model       specify SMT translation model\n" );
        fprintf( stderr, "               the model decides what type of rules can be extracted\n" );
        fprintf( stderr, "               its value can be \"s2t\", \"t2s\" or \"t2t\", default \"t2s\"\n" );
        fprintf( stderr, "  -method      specify rule extraction method\n" );
        fprintf( stderr, "               its value can be \"GHKM\" or \"SPMT\", default \"GHKM\"\n" );
        fprintf( stderr, "  -src         specify path to the source sentence file\n" );
        fprintf( stderr, "  -tar         specify path to the target sentence file\n" );
        fprintf( stderr, "  -align       specify path to the word alignment file\n" );
        fprintf( stderr, "  -srcparse    specify path to the source sentence parse tree file\n" );
        fprintf( stderr, "               the parse tree format is like Berkeley Parser\'s output\n" );
        fprintf( stderr, "  -tarparse    specify path to the target sentence parse tree file\n" );
        fprintf( stderr, "               the parse tree format is like Berkeley Parser\'s output\n" );
        fprintf( stderr, "  -output      specify path to the output file, default \"stdout\"\n" );
        fprintf( stderr, "  -inverse     extract inversed language-pair rules [optional]\n" );
        fprintf( stderr, "  -compose     specify the maximum compose times of atom rules\n" );
        fprintf( stderr, "               the atom rules are either GHKM minimal admissible rules\n" );
        fprintf( stderr, "               or lexical rules of SPMT Model 1 [optional]\n" );
        fprintf( stderr, "  -varnum      specify the maximum number of variables in a rule [optional]\n" );
        fprintf( stderr, "  -wordnum     specify the maximum number of words in a rule [optional]\n" );
        fprintf( stderr, "  -uain        specify the maximum number of unaligned words in a rule [optional]\n" );
        fprintf( stderr, "  -uaout       specify the maximum number of unaligned words outside a rule [optional]\n" );
        fprintf( stderr, "  -uaperm      trigger permutating unaligned words inside a rule [optional]\n" );
        fprintf( stderr, "  -depth       specify the maximum depth of tree in a rule [optional]\n" );
        fprintf( stderr, "  -oformat     specify the format of generated rules [optional]\n" );
        fprintf( stderr, "               its value can be \"oft\" or \"nft\", default \"nft\"\n" );
        fprintf( stderr, "  -varbase     specify the beginning index of variables in a rule, default \"1\" [optional]\n" );
        fprintf( stderr, "--------------------------------------------------------------------------------\n" );
        fprintf( stderr, "[examples]:\n" );
        fprintf( stderr, "[1].\n" );
        fprintf( stderr, "%s -src c.txt -tar e.txt -align a.txt -srcparse c.tree > result.txt\n", proname );
        fprintf( stderr, "[2].\n" );
        fprintf( stderr, \
        "%s -model s2t -method GHKM -src c.txt -tar e.txt -align a.txt -tarparse e.tree -output result.txt\n", proname );
        fprintf( stderr, "[3].\n" );
        fprintf( stderr, "%s -model t2t -method SPMT -src c.txt -tar e.txt -align a.txt -srcparse c.tree \\\n", proname );
        fprintf( stderr, "> -tarparse e.tree -compose 2 -varnum 4 -wordnum 4 -uain 10 -uaout 1\n" );
        fprintf( stderr, "[4].\n" );
        fprintf( stderr, "%s -model s2t -src c.txt -tar e.txt -align a.txt -tarparse e.tree -inverse\n", proname );

    }

}
