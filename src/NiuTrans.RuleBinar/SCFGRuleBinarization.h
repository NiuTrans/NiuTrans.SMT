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
 * SCFG Rule Binarization: SCFGRuleBinarization.h
 * This header file defines a SCFG rule binarization class.
 * It is used to binarize SCFG rules(e.g., syntax rules, hiero rules, etc.) for efficient decoding.
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 */


#ifndef    __SCFG_RULE_BINARIZATION_H__
#define    __SCFG_RULE_BINARIZATION_H__


#include "../NiuTrans.Base/DataStruts.h"
#include "../NiuTrans.Base/Utilities.h"
using namespace datastructs;
using namespace util;


enum method_e{ LEFT_HEAVY, COST_REDUCTION };
enum termcont_e{ RHS_SRC, RHS_TAR, LHS, FEATVEC, BLKALGN };


/*
 *
 * span class
 *
 */

class span_c {

public:
    span_c( long sbeg, long send, long sid );
    ~span_c();

public:
    long                    m_lSpanBeg;                 /* the beginning index of the span */
    long                    m_lSpanEnd;                 /* the end index of the span */
    long                    m_lSpanID;                  /* span position in the origin monolingual rule */

};

/*
 *
 * mono-rule class
 *
 */

class mono_rule_c {

public:
    mono_rule_c( char* rhs_str, bool onTarSide, long baseVarIdx );
    ~mono_rule_c();

private:
    static void ms_freeCharStr( const void* ptr );
    void m_init( char* rhs_str, long baseVarIdx );
    bool m_isVar( char* tok );

public:
    list_c*                 mp_stBlkSymList;            /* strings of non-terminals and consective terminals */
    list_c*                 mp_stBlkVarSymIdList;       /* indices of non-terminals in "mp_stBlkSymList" */
    list_c*                 mp_stBlkVarSequence;        /* non-terminal index sequence to be binarized */
    bool                    m_bOnTarSide;               /* a flag showing whether it is a target-side monolingual rule */

private:
    static const char*      msp_szTokenDelim;

};

/*
 *
 * rule binary tree node class
 *
 */

class bin_tnode_c {

public:
    bin_tnode_c( span_c* span );
    ~bin_tnode_c();

public:
    span_c*                 mp_csSpan;                  /* converge span of the binarization tree node */
    char*                   mp_szLhsRule;               /* the left hand side rule */
    char*                   mp_szRhsSrcRule;            /* source part of the right hand side rule */
    char*                   mp_szRhsTarRule;            /* target part of the right hand side rule */
    bin_tnode_c*            mp_csLeftChild;             /* left child binary tree node */
    bin_tnode_c*            mp_csRightChild;            /* right child binary tree node */
    bin_tnode_c*            mp_csParent;                /* parent binary tree node */

};

/*
 *
 * bilingual-rule class
 *
 */

class bin_rule_c {

public:
    bin_rule_c( char* lhs, char* rhs_src, char* rhs_tar );
    ~bin_rule_c();

public:
    char*                   mp_szLhsRule;               /* the left hand side rule */
    char*                   mp_szRhsSrcRule;            /* source part of the right hand side rule */
    char*                   mp_szRhsTarRule;            /* target part of the right hand side rule */

};

/*
 *
 * rule binarization class
 *
 */

extern void g_freeBinRule( const void* ptr );

class rule_binarizer_c {

public:
    rule_binarizer_c( int cmd_argc, char* cmd_argv[] );
    ~rule_binarizer_c();

public:
    void m_processFile();

protected:
    bool m_rbin_left_heavy( char* p_szLhs, char* p_szRhsSrc, char* p_szRhsTar, long ruleID, list_c* binRuleList );
    void m_createChart( bin_tnode_c***& binTNodeChart, long chartSize );
    void m_releaseChart( bin_tnode_c*** binTNodeChart, long chartSize );
    bool m_isVar( char* tok );
    bool m_canAttach( char* tok, bool onlyVar );
    void m_replaceVarId( char* str, char varId );
    void m_genBinRule( bin_tnode_c* binTreeRoot, char* p_szLhs, mono_rule_c* rhsSrcMonoRule, \
        mono_rule_c* rhsTarMonoRule, long ruleID, list_c* binRuleList );
    void m_reIndex( char* str );
    void m_dumpBinRule( list_c* binRuleList );

private:
    bool m_initArguments();
    void m_clearArguments();
    void m_usage();

private:
    char*                   mp_szProgramName;           /* program name */
    FILE*                   mp_stFileReader;            /* input file pointer */
    FILE*                   mp_stFileWriter;            /* output file pointer */
    method_e                m_enBinarizeMethod;         /* binarization method */
    long                    m_lBaseVarIndex;            /* the beginning index number of variables */

private:
    static const char*      msp_szTermDelim;
    static long             ms_lMinTermNum;

};


#endif
