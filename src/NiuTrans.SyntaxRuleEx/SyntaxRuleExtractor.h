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
 * Syntax Rule Extractor: SyntaxRuleExtractor.h
 * This header file defines a general syntax-based rule extractor. It can be used in several syntax-based SMT models, including:
 * 1) string-to-tree SMT model
 * 2) tree-to-string SMT model
 * 3) tree-to-tree SMT model
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn); Oct 20, 2014, define MAX_CHAR_NUM_ONE_BUFF = 2048 * 16
 * Hao Zhang (email: zhanghao1216@gmail.com); July 3rd, 2012; add option "-varbase"
 * Hao Zhang (email: zhanghao1216@gmail.com); June 5th, 2012; fix a bug in function
 * * "pnode_c* tree_c::m_composeTwoSubTree( pnode_c* curNode, pnode_c* sRoot )"
 * Hao Zhang (email: zhanghao1216@gmail.com); May 28th, 2012; unify rule new format to
 * * "root-label ||| src-scfg ||| tar-scfg ||| isLexical isCompose ||| tar-scfg-varID ||| alignment ||| src-tree ||| tar-tree"
 * Hao Zhang (email: zhanghao1216@gmail.com); May 12th, 2012; fix a bug about permulating un-aligned words in case "3" and case "4"
 * Hao Zhang (email: zhanghao1216@gmail.com); May 9th, 2012; re-define lexicalized rules, count un-aligned words inside or outside separately
 * Hao Zhang (email: zhanghao1216@gmail.com); Jan. 19th, 2012; add function "m_checkUnalignNum()", add program option "-uaperm"
 * Hao Zhang (email: zhanghao1216@gmail.com); Dec. 17th, 2011
 *
 */


#ifndef    __SYNTAXRULEEXTRACTOR_H__
#define    __SYNTAXRULEEXTRACTOR_H__


#include "../NiuTrans.Base/DataStruts.h"

#define     MAX_NUM_LONG_TYPE_DATA       0x7fffffff
#define     MAX_CHAR_NUM_ONE_BUFF        2048 * 16
#define     MAX_LINE_NUM_ONE_HINT        1000
#define     MAX_RULE_NUM_ONE_ANODE       MAX_NUM_LONG_TYPE_DATA

using namespace datastructs;


namespace syntax_based_smt {

    /*
     *
     * Closure Span. It only records the leftmost and rightmost indices.
     *
     */
    class span_c {

    public:
        span_c();
        span_c( long beg, long end );
        ~span_c();

    public:
        void m_expand( long index );
        void m_expand( span_c* span );
        bool m_isLegal();
        bool m_contain( span_c* s );
        bool m_overlap( span_c* s );

    public:
        long            m_lBeg;
        long            m_lEnd;

    };

    /*
     *
     * Alignment Information of One Token
     *
     */
    class token_algn_info_c {

    public:
        token_algn_info_c( char* token );
        ~token_algn_info_c();

    public:
        char*           mp_szTok;               /* token */
        long            m_lAlgnCnt;             /* alignment count */
        list_c*         mp_csAlgnIndices;       /* alignment indices */
        span_c*         mp_csAlgnSpan;          /* leftmost and rightmost indices */

    };

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
        span_c* m_getMappedSpan( span_c* span, bool isSrc2Tgt );
        bool m_isClosure( span_c* span, bool isSrc2Tgt );
        span_c* m_getMinClosureSpan( span_c* span, bool isSrc2Tgt );
        span_c* m_getMaxClosureSpan( span_c* span, bool isSrc2Tgt );
        list_c* m_getAllClosureSpans( span_c* span, bool isSrc2Tgt, long maxUnAlgn = MAX_NUM_LONG_TYPE_DATA );

    protected:
        static void ms_free_token_algn_info( const void* token_algn_info );
        static void ms_free_span( const void* span );

    public:
        /* source sentence info */
        array_c*        mp_csSrc2TgtAlgn;
        /* target sentence info */
        array_c*        mp_csTgt2SrcAlgn;

    };

    /*
     *
     * Tree Node
     *
     */
    class node_c {

    public:
        node_c();
        ~node_c();

    public:
        void m_printNode( char* buffer, bool simpleForm = true );

    public:
        node_c*         mp_csParent;
        list_c*         mp_csSonList;

    public:
        char*           mp_szNodeLabel;
        span_c*         mp_csCoveredLeavesSpan;
        span_c*         mp_csProjectedSpan;
        long            m_lNodeLevel;
        long            m_lNodeID;
        bool            m_bAdmissibleNode;
        bool            m_bTerminalNode;
        bool            m_bUnaryNode;

    public:
        list_c*         mp_csSubtreeList;
        list_c*         mp_csSTreeIndex_GHKM_Lex;
        list_c*         mp_csSTreeIndex_GHKM_NonLex;
        list_c*         mp_csSTreeIndex_SPMT_Lex;

    };

    /*
     *
     * Tree Node Pointer's Wrapper
     *
     */
    class pnode_c {

    public:
        pnode_c( node_c* node );
        ~pnode_c();

    public:
        pnode_c*        mp_csParentNode;
        list_c*         mp_csSonNodes;

    public:
        node_c*         mp_csParseNode;

    };

    /*
     *
     * Tree Fragment
     *
     */
    class tree_c {

    public:
        tree_c();
        ~tree_c();

    public:
        void m_createMinGHKMSubTree( node_c* p_csParseRoot );
        void m_createMinSPMTSubTree( node_c* p_csParseRoot, span_c* tokenSpan );
        void m_composeTwoSubTree( tree_c* mTree, tree_c* sTree );
        long m_calTreeDepth();

    protected:
        pnode_c* m_createMinSubTree( node_c* p_csParseRoot, pnode_c* p_csParent, node_c* p_csParseNode, span_c* coveredSpan = NULL );
        pnode_c* m_composeTwoSubTree( pnode_c* curNode, pnode_c* sRoot );

    public:
        pnode_c*        mp_csRootNode;
        list_c*         mp_csLeafNodeList;
        list_c*         mp_csAdmissibleNodes;
        long            m_lComposeTimes;
        bool            m_bLexicalized;
        bool            m_bTypeGHKM;
        bool            m_bTypeSPMT;

    };

    void g_free_tree( const void* tree );
    int g_compare_tree( const void* t1, const void* t2 );
    extern long g_lVarBeginIndex;

    /*
     *
     * Rule's One-hand-side Information
     *
     */
    enum method_e{ GHKM, SPMT };
    enum oformat_e{ OFT, NFT };
    
    class rule_xhs_c {

    public:
        rule_xhs_c( algn_sent_c* p_csAlignment, bool onSrcSide );
        ~rule_xhs_c();

    public:
        bool m_init( char* p_szTreeStr, char* p_szTermStr, method_e method, long lComposeTimes, long lTreeDepth );
        void m_clear();
        void m_genRule( bool isT2sRule, long maxVarNum, long maxWdNum, long maxUaInside, long maxUaOutside, oformat_e oformat, FILE* outF );
        void m_printTree( node_c* root, char* buffer, bool simpleForm = true );
        void m_printTree( pnode_c* root, char* buffer, bool simpleForm = true );

    protected:
        /* indexing chart */
        void m_createParseChart();
        void m_destroyParseChart();

    protected:
        /* parse tree */
        bool m_createParseTree( char* p_szTreeStr, char** pp_szTerms, long lTermCnt );
        char* m_refineParseTree( char* p_szTreeStr, char** pp_szTerms, long lTermCnt );
        node_c* m_buildParseTree( node_c* p_csNodeParent, char** pp_szToks, long lTokCnt, long& lTokIdx, long& lWdIdx );

    protected:
        /* admissible tree */
        void m_buildAdmissibleTree( pnode_c*& p_csParent, node_c* p_csParseNode );
        void m_genMinGHKMSubTree( pnode_c* adm_root, bool onlyNonLex = false );
        void m_genMinSPMTSubTree( long tokenNum );
        void m_composeSubTree( pnode_c* adm_root, long maxComposeTimes, long maxTreeDepth );
    private:
        void m_genMinSPMTSubTree( span_c* tokenSpan );
        void m_hockSonSubTree( pnode_c* adm_root, long maxComposeTimes, long maxTreeDepth );
        void m_doHock( list_c* tmpTrees, pnode_c* hockNode, list_c* sonTrees, list_c* newTrees, long maxComposeTimes, long maxTreeDepth );

    private:
        /* generate tree-to-string or string-to-tree rules */
        void m_genRule( tree_c* rTree, span_c* pMaxSpan, span_c* pMinSpan, bool isT2sRule, \
            long maxVarNum, long maxWdNum, long maxUaInside, long maxUaOutside, oformat_e oformat, FILE* outF );
        list_c* m_genAllStrCodeArray( array_c* seedArr, span_c* pMaxSpan, span_c* pMinSpan, \
            long maxVarNum, long maxWdNum, long maxUaInside, long maxUaOutside );
        void m_genTempStrCodeArray( long idxBeg, long uaBeg, long uaEnd, long type, \
            long rBegIdx, long rEndIdx, long initUaInside, long initUaOutside, long maxUaInside, long maxUaOutside, list_c* tmpList );
        void m_simplifyCodeArray( list_c* tmpList, long maxVarNum, long maxWdNum, list_c* codeArrs );
        bool m_checkUnalignNum( array_c* codeArr, long rBegIdx, long rEndIdx, \
            long initUaInside, long initUaOutside, long maxUaInside, long maxUaOutside );
        void m_outputGenRule( tree_c* tree, array_c* treeArr, list_c* strArrs, list_c* varNameList, bool isT2sRule, oformat_e oformat, FILE* outF );
        void m_genVarTagStr( array_c* codeArr, bool onTreeSide, list_c* varNameList, char* outBuff );
        void m_mapVarIndex( array_c* codeArr, array_c* mapArr );
        void m_genVarIdStr( array_c* codeArr, bool onTreeSide, bool isT2sRule, array_c* mapArr, char* outBuff );
        void m_genRuleAlignment( array_c* treeCodeArr, array_c* strCodeArr, bool isT2sRule, char* outBuff );

    public:
        /* indexing chart */
        node_c***           mppp_csParseChart;          /* a tree node indexing chart, used to accelerate locating nodes given their covered span */
        long                m_lParseChartDim;           /* chart dimension, indicating the number of leaf nodes */
        /* parse tree */
        node_c*             mp_csParseTreeRoot;         /* a parse tree */
        long                m_lNodeNum;                 /* total node number in the parse tree */
        list_c*             mp_csParseTreeLeaves;       /* pointers to leaf nodes */
        list_c*             mp_csAdmissibleNodes;       /* pointers to admissible nodes */
        list_c*             mp_csUnalignedLeaves;       /* pointers to unaligned words */
        /* admissible tree */
        pnode_c*            mp_csAdmissibleTree;        /* an admissible node tree */
        long                m_lAdmNodeNum;              /* admissible node number */

    public:
        algn_sent_c*        mp_csAlignment;             /* pointer to a word-alignment class */
        bool                m_bOnSrcSide;               /* indicator showing this part-of-rule is a source part or a target part */

    };
    
    void g_free_array( const void* array );

    /*
     *
     * Syntax-based Rule Extractor
     *
     */
    enum model_e{ S2T, T2S, T2T };
    enum extract_e{ S2T_S, T2S_S, T2T_S, S2T_I, T2S_I, T2T_I };

    class syntax_rule_extractor_c {

    public:
        syntax_rule_extractor_c( int argc, char* argv[] );
        ~syntax_rule_extractor_c();
        
    public:
        void m_extract();
        
    protected:
        void m_genT2tRule( rule_xhs_c* leftRs, rule_xhs_c* rightRs, oformat_e oformat, FILE* out );
    private:
        void m_genT2tRule( node_c* leftTreeNode, node_c* rightTreeNode, bool isSrc2Tgt, oformat_e oformat, FILE* out );
        void m_genT2tRule( tree_c* lTree, tree_c* rTree, bool isSrc2Tgt, oformat_e oformat, FILE* out );
        bool m_mapVarNodes( tree_c* lTree, tree_c* rTree, array_c* l2rVarMap, array_c* r2lVarMap );
        bool m_isAlignedNode( node_c* lNode, node_c* rNode );
        void m_genVarTagStr( list_c* leafNodes, list_c* pAdmNodes, array_c* varMap, char* outBuff );
        void m_genVarIdStr( list_c* leafNodes, array_c* varMap, char* outBuff );
        void m_genAlignment( list_c* lNodes, list_c* rNodes, array_c* varMap, bool isSrc2Tgt, char* outBuff );
        void m_genCodeArr( list_c* nodes, array_c* varMap, array_c* codeArr );
        
    private:
        void m_init( const char* proname );
        void m_clear();
        void m_error( const char* proname, const char* msg, bool bShowUsage = true );
        void m_usage( const char* proname );
        
    private:
        algn_sent_c*        mp_csAlignment;
        rule_xhs_c*         mp_csSrcRuleSide;
        rule_xhs_c*         mp_csTarRuleSide;
        extract_e           m_enExtractType;

    private:
        model_e             m_enModelType;
        method_e            m_enMethodType;
        FILE*               mp_stSrcFp;
        FILE*               mp_stTarFp;
        FILE*               mp_stAlignFp;
        FILE*               mp_stSrcParseFp;
        FILE*               mp_stTarParseFp;
        FILE*               mp_stOutputFp;
        long                m_lMaxCompose;
        long                m_lMaxVarNum;
        long                m_lMaxWordNum;
        long                m_lMaxUaInside;
        long                m_lMaxUaOutside;
        long                m_lMaxTreeDepth;
        bool                m_bInverseExtract;
        oformat_e           m_enRuleFormat;

    };

}


#endif
