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
 * SCFG Rule Binarization: SCFGRuleBinarization.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 */


#include "SCFGRuleBinarization.h"
#include "stdio.h"
#include "string.h"

#define MAX_LINE_NUM_ONE_HINT   10000


/*
 *
 * span class
 *
 */

span_c::span_c( long sbeg, long send, long sid ) {

    this->m_lSpanBeg = sbeg;
    this->m_lSpanEnd = send;
    this->m_lSpanID = sid;

}

span_c::~span_c() {
}

/*
 *
 * mono-rule class
 *
 */

const char* mono_rule_c::msp_szTokenDelim = " ";

void mono_rule_c::ms_freeCharStr( const void* ptr ) {

    char* str = ( char* ) ptr;
    delete[] str;

}

mono_rule_c::mono_rule_c( char* rhs_str, bool onTarSide, long baseVarIdx ) {

    this->mp_stBlkSymList = new list_c( mono_rule_c::ms_freeCharStr );
    this->mp_stBlkVarSymIdList = new list_c();
    this->mp_stBlkVarSequence = ( onTarSide ? new list_c() : NULL );
    this->m_bOnTarSide = onTarSide;

    m_init( rhs_str, baseVarIdx );

}

mono_rule_c::~mono_rule_c() {

    delete this->mp_stBlkSymList;
    delete this->mp_stBlkVarSymIdList;
    delete this->mp_stBlkVarSequence;

}

void mono_rule_c::m_init( char* rhs_str, long baseVarIdx ) {

    char** toks = NULL;
    long tc = string_c::ms_split( rhs_str, this->msp_szTokenDelim, toks );
    char* buff = new char[strlen( rhs_str ) + 2];

    buff[0] = '\0';
    for( long i=0; i < tc; i++ ) {
        if( true == m_isVar( toks[i] ) ) {
            if( '\0' != buff[0] ) {
                string_c::RemoveRightSpaces( buff );
                this->mp_stBlkSymList->m_addObject( string_c::Copy( buff ) );
                buff[0] = '\0';
            }
            this->mp_stBlkSymList->m_addObject( string_c::Copy( toks[i] ) );
            long blkSymCnt = this->mp_stBlkSymList->m_getLength();
            this->mp_stBlkVarSymIdList->m_addInt( blkSymCnt - 1 );
            if( true == this->m_bOnTarSide ) {
                long varId = -1;
                sscanf( toks[i], "#%ld", &varId );
                this->mp_stBlkVarSequence->m_addInt( varId - baseVarIdx );
            }
        }
        else {
            strcat( buff, toks[i] );
            strcat( buff, " " );
        }
    }
    if( '\0' != buff[0] ) {
        string_c::RemoveRightSpaces( buff );
        this->mp_stBlkSymList->m_addObject( string_c::Copy( buff ) );
        buff[0] = '\0';
    }
    
    delete[] buff;
    string_c::ms_freeStrArray( toks, tc );

}

bool mono_rule_c::m_isVar( char* tok ) {

    long len = strlen( tok );
    if( len > 1 && tok[0] == '#' ) {
        /* #NN, #0, etc. */
        return true;
    }
    return false;

}

/*
 *
 * rule binary tree node class
 *
 */

bin_tnode_c::bin_tnode_c( span_c* span ) {

    this->mp_csSpan = span;
    this->mp_szLhsRule = NULL;
    this->mp_szRhsSrcRule = NULL;
    this->mp_szRhsTarRule = NULL;
    this->mp_csLeftChild = NULL;
    this->mp_csRightChild = NULL;
    this->mp_csParent = NULL;

}

bin_tnode_c::~bin_tnode_c() {

    delete this->mp_csSpan;
    delete[] this->mp_szLhsRule;
    delete[] this->mp_szRhsSrcRule;
    delete[] this->mp_szRhsTarRule;

}

/*
 *
 * bilingual-rule class
 *
 */

bin_rule_c::bin_rule_c( char* lhs, char* rhs_src, char* rhs_tar ) {

    this->mp_szLhsRule = string_c::Copy( lhs );
    this->mp_szRhsSrcRule = string_c::Copy( rhs_src );
    this->mp_szRhsTarRule = string_c::Copy( rhs_tar );

}

bin_rule_c::~bin_rule_c() {

    delete[] this->mp_szLhsRule;
    delete[] this->mp_szRhsSrcRule;
    delete[] this->mp_szRhsTarRule;

}

/*
 *
 * rule binarization class
 *
 */

const char* rule_binarizer_c::msp_szTermDelim = " ||| ";
long rule_binarizer_c::ms_lMinTermNum = 5;

void g_freeBinRule( const void* ptr ) {

    bin_rule_c* rule = ( bin_rule_c* ) ptr;
    delete rule;

}

rule_binarizer_c::rule_binarizer_c( int cmd_argc, char* cmd_argv[] ) {

    configMng_c::ms_create( cmd_argc, cmd_argv );
    this->mp_szProgramName = cmd_argv[0];

}

rule_binarizer_c::~rule_binarizer_c() {

    configMng_c::ms_destroy();

}

void rule_binarizer_c::m_processFile() {

    /* print program usage when users ask for help */
    if( configMng_c::ms_getBool( "h", false ) || configMng_c::ms_getBool( "help", false ) ) {
        m_usage();
        return;
    }

    /* initialize program arguments */
    if( false == m_initArguments() ) {
        return;
    }

    /* loop */
    char* p_szLine = NULL;
    long lineNum = 0;
    time_c timer;
    timer.m_startTimer();
    fprintf( stderr, "start binarizing rules at time: %s\n", timer.m_getNowTimeString() );
    while( p_szLine = string_c::ms_readLine( this->mp_stFileReader ) ) {
        ++lineNum;
        char** terms = NULL;
        long termCnt = string_c::ms_split( p_szLine, rule_binarizer_c::msp_szTermDelim, terms );
        if( termCnt >= rule_binarizer_c::ms_lMinTermNum ) {
            bool retFlag = false;
            list_c* binRuleList = new list_c( g_freeBinRule );
            char rootLabel[2048];
            sprintf( rootLabel, "#%s", terms[LHS] );
            /* call rule binarization function */
            switch( this->m_enBinarizeMethod ) {
                case LEFT_HEAVY:
                    retFlag = m_rbin_left_heavy( rootLabel, terms[RHS_SRC], \
                        terms[RHS_TAR], lineNum, binRuleList );
                    break;
                case COST_REDUCTION:
                    break;
                default:
                    break;
            }
            /* dump binarization rules */
            if( false == retFlag ) {
                fprintf( this->mp_stFileWriter, "%s\n", p_szLine );
                fprintf( this->mp_stFileWriter, "<null>\n" );
                fprintf( this->mp_stFileWriter, "\n" );
            }
            else {
                fprintf( this->mp_stFileWriter, "%s\n", p_szLine );
                m_dumpBinRule( binRuleList );
                fprintf( this->mp_stFileWriter, "\n" );
            }
            delete binRuleList;
        }
        string_c::ms_freeStrArray( terms, termCnt );
        delete[] p_szLine;
        if( 0 == lineNum % MAX_LINE_NUM_ONE_HINT ) {
            fprintf( stderr, "\rprocessed %ld lines...", lineNum );
        }
    }
    fprintf( stderr, "\rprocessed %ld lines...\n", lineNum );
    fprintf( stderr, "end binarizing rules at time: %s\n", timer.m_getNowTimeString() );
    timer.m_endTimer();
    fprintf( stderr, "average speed: %.3f [sent(s)/sec]\n", ( double ) lineNum / timer.m_getTimerDiff() );

    /* clear program arguments */
    m_clearArguments();

    return;

}

bool rule_binarizer_c::m_initArguments() {

    /* open input file */
    const char* inFn = configMng_c::ms_getString( "input", NULL );
    if( NULL == inFn ) {
        this->mp_stFileReader = stdin;
    }
    else {
        this->mp_stFileReader = fopen( inFn, "r" );
        if( NULL == this->mp_stFileReader ) {
            fprintf( stderr, "[ERROR]: file %s does not exist!\n", inFn );
            return false;
        }
    }

    /* open output file */
    const char* outFn = configMng_c::ms_getString( "output", NULL );
    if( NULL == outFn ) {
        this->mp_stFileWriter = stdout;
    }
    else {
        this->mp_stFileWriter = fopen( outFn, "w" );
        if( NULL == this->mp_stFileWriter ) {
            fprintf( stderr, "[ERROR]: create file %s failed!\n", outFn );
            return false;
        }
    }

    /* decide rule binarization method type */
    this->m_enBinarizeMethod = ( method_e ) configMng_c::ms_getInt( "mtype", LEFT_HEAVY );

    /* set the value of the beginning index number of variables */
    this->m_lBaseVarIndex = configMng_c::ms_getInt( "varbase", 1 );

    return true;

}

void rule_binarizer_c::m_clearArguments() {

    /* close input file */
    if( this->mp_stFileReader != stdin ) {
        fclose( this->mp_stFileReader );
    }

    /* close output file */
    if( this->mp_stFileWriter != stdout ) {
        fclose( this->mp_stFileWriter );
    }

}

void rule_binarizer_c::m_reIndex( char* str ) {

    long len = strlen( str );
    for( long i=0; i < len - 1; i++ ) {
        if( '#' == str[i] && ( '0' == str[i+1] || '1' == str[i+1] ) ) {
            str[i+1] += this->m_lBaseVarIndex;
        }
    }

}

void rule_binarizer_c::m_dumpBinRule( list_c* binRuleList ) {

    long listLen = binRuleList->m_getLength();
    for( long i=0; i < listLen; i++ ) {
        bin_rule_c* rule = ( bin_rule_c* ) binRuleList->m_getObject( i );
        m_reIndex( rule->mp_szRhsTarRule );
        fprintf( this->mp_stFileWriter, "%s%s%s%s%s\n", rule->mp_szRhsSrcRule, rule_binarizer_c::msp_szTermDelim, \
            rule->mp_szRhsTarRule, rule_binarizer_c::msp_szTermDelim, &rule->mp_szLhsRule[1] );
    }

}

void rule_binarizer_c::m_createChart( bin_tnode_c***& binTNodeChart, long chartSize ) {

    binTNodeChart = new bin_tnode_c**[chartSize];
    for( long i=0; i < chartSize; i++ ) {
        binTNodeChart[i] = new bin_tnode_c*[chartSize];
        memset( binTNodeChart[i], 0, sizeof( bin_tnode_c* ) * chartSize );
    }

}

void rule_binarizer_c::m_releaseChart( bin_tnode_c*** binTNodeChart, long chartSize ) {

    for( long i=0; i < chartSize; i++ ) {
        for( long j=0; j < chartSize; j++ ) {
            if( binTNodeChart[i][j] != NULL ) {
                delete binTNodeChart[i][j];
            }
        }
        delete[] binTNodeChart[i];
    }
    delete[] binTNodeChart;

}

bool rule_binarizer_c::m_isVar( char* tok ) {

    long len = strlen( tok );
    if( len > 1 && tok[0] == '#' ) {
        /* #NN, #0, etc. */
        return true;
    }
    return false;

}

bool rule_binarizer_c::m_canAttach( char* tok, bool onlyVar ) {

    if( '#' == tok[0] && '\0' == tok[1] ) {
        /*
         * this is for the case of word "#".
         * in this case, the program should deal it specially,
         * because it is a reserved word.
        */
        return false;
    }

    bool flag1 = m_isVar( tok );
    bool flag2 = ( strlen( tok ) >= 1 && '$' == tok[0] );
    if( true == onlyVar ) {
        return ( false == flag1 );
    }
    else {
        return ( ( false == flag1 ) | ( true == flag2 ) );
    }

}

void rule_binarizer_c::m_replaceVarId( char* str, char varId ) {

    char* ptr = strstr( str, "#0" );
    if( ptr ) {
        *( ptr + 1 ) = varId;
    }

}

void rule_binarizer_c::m_genBinRule( bin_tnode_c* binTreeRoot, char* p_szLhs, mono_rule_c* rhsSrcMonoRule, \
    mono_rule_c* rhsTarMonoRule, long ruleID, list_c* binRuleList ) {

        char buff[4096];
        bool canGenRule = true;

        if( NULL == binTreeRoot->mp_csLeftChild && NULL == binTreeRoot->mp_csRightChild ) { /* leaf node */
            char* leftSym = NULL;
            char* rightSym = NULL;

            /* generate source part on the right hand side of the binarization rule */
            /* current source-side symbol position and the symbol itself */
            long srcMonoRulePos = rhsSrcMonoRule->mp_stBlkVarSymIdList->m_getInt( binTreeRoot->mp_csSpan->m_lSpanBeg );
            char* srcSymbol = ( char* ) rhsSrcMonoRule->mp_stBlkSymList->m_getObject( srcMonoRulePos );
            /* attach left or right boundary words */
            if( srcMonoRulePos - 1 >= 0 \
                && ( leftSym = ( char* ) rhsSrcMonoRule->mp_stBlkSymList->m_getObject( srcMonoRulePos - 1 ) ) \
                && '\0' != leftSym[0] \
                && true == m_canAttach( leftSym, true ) ) {
                    /* attach unused left boundary word */
                    sprintf( buff, "%s %s", leftSym, srcSymbol );
                    binTreeRoot->mp_szRhsSrcRule = string_c::Copy( buff );
                    leftSym[0] = '\0';
            }
            else if ( srcMonoRulePos + 1 < rhsSrcMonoRule->mp_stBlkSymList->m_getLength() \
                && ( rightSym = ( char* ) rhsSrcMonoRule->mp_stBlkSymList->m_getObject( srcMonoRulePos + 1 ) ) \
                && '\0' != rightSym[0] \
                && true == m_canAttach( rightSym, true ) ) {
                    /* attach unused right boundary word */
                    sprintf( buff, "%s %s", srcSymbol, rightSym );
                    binTreeRoot->mp_szRhsSrcRule = string_c::Copy( buff );
                    rightSym[0] = '\0';
            }
            else {
                binTreeRoot->mp_szRhsSrcRule = string_c::Copy( srcSymbol );
                binTreeRoot->mp_szLhsRule = string_c::Copy( srcSymbol );
                canGenRule = false;
            }

            /* generate target part on the right hand side of the binarization rule */
            /* current source-side symbol position and the symbol itself */
            long tarMonoRulePos = rhsTarMonoRule->mp_stBlkVarSymIdList->m_getInt( binTreeRoot->mp_csSpan->m_lSpanID );
            char* tarSymbol = ( char* ) rhsTarMonoRule->mp_stBlkSymList->m_getObject( tarMonoRulePos );
            /* attach left and right boundary words */
            buff[0] = '\0';
            bool mode = ( NULL == strstr( binTreeRoot->mp_szRhsSrcRule, "$" ) );
            if( tarMonoRulePos - 1 >= 0 \
                && ( leftSym = ( char* ) rhsTarMonoRule->mp_stBlkSymList->m_getObject( tarMonoRulePos - 1 ) ) \
                && '\0' != leftSym[0] \
                && true == m_canAttach( leftSym, mode ) ) {
                    /* attach unused left boundary word */
                    sprintf( buff, "%s #0", leftSym );
                    leftSym[0] = '\0';
            }
            if ( tarMonoRulePos + 1 < rhsTarMonoRule->mp_stBlkSymList->m_getLength() \
                && ( rightSym = ( char* ) rhsTarMonoRule->mp_stBlkSymList->m_getObject( tarMonoRulePos + 1 ) ) \
                && '\0' != rightSym[0] \
                && true == m_canAttach( rightSym, mode ) ) {
                    /* attach unused right boundary word */
                    if( '\0' == buff[0] ) {
                        strcpy( buff, "#0" );
                    }
                    sprintf( buff, "%s %s", buff, rightSym );
                    rightSym[0] = '\0';
            }
            if( '\0' == buff[0] ) {
                strcpy( buff, "#0" );
            }
            binTreeRoot->mp_szRhsTarRule = string_c::Copy( buff );
        }
        else {  /* branch node */
            m_genBinRule( binTreeRoot->mp_csLeftChild, p_szLhs, rhsSrcMonoRule, rhsTarMonoRule, ruleID, binRuleList );
            m_genBinRule( binTreeRoot->mp_csRightChild, p_szLhs, rhsSrcMonoRule, rhsTarMonoRule, ruleID, binRuleList );

            /* generate source part on the right hand side of the binarization rule */
            if( binTreeRoot->mp_csLeftChild->mp_csSpan->m_lSpanBeg < binTreeRoot->mp_csRightChild->mp_csSpan->m_lSpanBeg ) {
                /* straight orientation */
                sprintf( buff, "%s %s", binTreeRoot->mp_csLeftChild->mp_szLhsRule, binTreeRoot->mp_csRightChild->mp_szLhsRule );
            }
            else {
                /* inverted orientation */
                sprintf( buff, "%s %s", binTreeRoot->mp_csRightChild->mp_szLhsRule, binTreeRoot->mp_csLeftChild->mp_szLhsRule );
            }
            binTreeRoot->mp_szRhsSrcRule = string_c::Copy( buff );

            /* generate target part on the right hand side of the binarization rule */
            char s1[2048] = "#0", s2[2048] = "#0";
            char varId;
            if( NULL == binTreeRoot->mp_csLeftChild->mp_csLeftChild && NULL == strstr( binTreeRoot->mp_csLeftChild->mp_szRhsSrcRule, " " ) ) {
                strcpy( s1, binTreeRoot->mp_csLeftChild->mp_szRhsTarRule );
            }
            varId = ( binTreeRoot->mp_csLeftChild->mp_csSpan->m_lSpanBeg < binTreeRoot->mp_csRightChild->mp_csSpan->m_lSpanBeg ? '0' : '1' );
            m_replaceVarId( s1, varId );
            if( NULL == binTreeRoot->mp_csRightChild->mp_csLeftChild && NULL == strstr( binTreeRoot->mp_csRightChild->mp_szRhsSrcRule, " " ) ) {
                strcpy( s2, binTreeRoot->mp_csRightChild->mp_szRhsTarRule );
            }
            varId = ( binTreeRoot->mp_csLeftChild->mp_csSpan->m_lSpanBeg < binTreeRoot->mp_csRightChild->mp_csSpan->m_lSpanBeg ? '1' : '0' );
            m_replaceVarId( s2, varId );
            sprintf( buff, "%s %s", s1, s2 );
            binTreeRoot->mp_szRhsTarRule = string_c::Copy( buff );
        }

        /* generate left hand side of the binarization rule */
        if( true == canGenRule ) {
            /* generate left hand side of the binarization rule */
            if( 0 == binTreeRoot->mp_csSpan->m_lSpanBeg && rhsTarMonoRule->mp_stBlkVarSymIdList->m_getLength() - 1 == binTreeRoot->mp_csSpan->m_lSpanEnd ) {
                char* headSymbol = ( char* ) rhsSrcMonoRule->mp_stBlkSymList->m_getObject( 0 );
                char* tailSymbol = ( char* ) rhsSrcMonoRule->mp_stBlkSymList->m_getObject( rhsSrcMonoRule->mp_stBlkSymList->m_getLength() - 1 );
                if( ( '\0' != headSymbol[0] && true == m_canAttach( headSymbol, true ) ) \
                    || ( '\0' != tailSymbol[0] && true == m_canAttach( tailSymbol, true ) ) ) {
                    sprintf( buff, "%s%ld:%ld%ld", p_szLhs, ruleID, binTreeRoot->mp_csSpan->m_lSpanBeg, binTreeRoot->mp_csSpan->m_lSpanEnd );
                    binTreeRoot->mp_szLhsRule = string_c::Copy( buff );
                    /* generate binarization rule and add it to the rule list */
                    bin_rule_c* binRule = new bin_rule_c( binTreeRoot->mp_szLhsRule, binTreeRoot->mp_szRhsSrcRule, binTreeRoot->mp_szRhsTarRule );
                    binRuleList->m_addObject( binRule );

                    if( '\0' != headSymbol[0] && true == m_canAttach( headSymbol, true ) ) {
                        sprintf( buff, "%s %s", headSymbol, binTreeRoot->mp_szLhsRule );
                    }
                    else {
                        sprintf( buff, "%s %s", binTreeRoot->mp_szLhsRule, tailSymbol );
                    }
                    /* generate binarization rule and add it to the rule list */
                    binRule = new bin_rule_c( p_szLhs, buff, ( char* ) "#0" );
                    binRuleList->m_addObject( binRule );
                }
                else {
                    binTreeRoot->mp_szLhsRule = string_c::Copy( p_szLhs );
                    /* generate binarization rule and add it to the rule list */
                    bin_rule_c* binRule = new bin_rule_c( binTreeRoot->mp_szLhsRule, binTreeRoot->mp_szRhsSrcRule, binTreeRoot->mp_szRhsTarRule );
                    binRuleList->m_addObject( binRule );
                }
            }
            else {
                sprintf( buff, "%s%ld:%ld%ld", p_szLhs, ruleID, binTreeRoot->mp_csSpan->m_lSpanBeg, binTreeRoot->mp_csSpan->m_lSpanEnd );
                binTreeRoot->mp_szLhsRule = string_c::Copy( buff );
                /* generate binarization rule and add it to the rule list */
                bin_rule_c* binRule = new bin_rule_c( binTreeRoot->mp_szLhsRule, binTreeRoot->mp_szRhsSrcRule, binTreeRoot->mp_szRhsTarRule );
                binRuleList->m_addObject( binRule );
            }
            
        }

}

bool rule_binarizer_c::m_rbin_left_heavy( char* p_szLhs, char* p_szRhsSrc, char* p_szRhsTar, long ruleID, list_c* binRuleList ) {

    /* parse source-side and target-side right hand side monolingual rule */
    mono_rule_c* rhsSrcMonoRule = new mono_rule_c( p_szRhsSrc, false, this->m_lBaseVarIndex );
    long rSymNum = rhsSrcMonoRule->mp_stBlkSymList->m_getLength();
    if( rSymNum < 3 ) {
        delete rhsSrcMonoRule;
        return false;
    }
    mono_rule_c* rhsTarMonoRule = new mono_rule_c( p_szRhsTar, true, this->m_lBaseVarIndex );
    long rVarNum = rhsTarMonoRule->mp_stBlkVarSequence->m_getLength();

    /* new a binary tree chart */
    bin_tnode_c*** binTNodeChart = NULL;
    m_createChart( binTNodeChart, rVarNum );

    /* new a span stack for shift-reduce binarization algorithm */
    stack_c* stack = new stack_c();

    /* shift-reduce binarization algorithm */
    for( long i=0; i < rVarNum; i++ ) {
        long leafBeg = rhsTarMonoRule->mp_stBlkVarSequence->m_getInt( i );
        long leafEnd = leafBeg;
        span_c* leafSpan = new span_c( leafBeg, leafEnd, i );
        bin_tnode_c* leafNode = new bin_tnode_c( leafSpan );
        stack->m_pushObject( leafSpan );
        binTNodeChart[leafBeg][leafEnd] = leafNode;

        while( stack->m_getSize() > 1 ) {
            span_c* s2 = ( span_c* ) stack->m_popObject();
            span_c* s1 = ( span_c* ) stack->m_popObject();
            if( s1->m_lSpanEnd + 1 != s2->m_lSpanBeg && s2->m_lSpanEnd + 1 != s1->m_lSpanBeg ) {
                stack->m_pushObject( s1 );
                stack->m_pushObject( s2 );
                break;
            }
            else {
                long branchBeg = -1;
                long branchEnd = -1;
                if( s1->m_lSpanBeg < s2->m_lSpanBeg ) {
                    /* straight orientation */
                    branchBeg = s1->m_lSpanBeg;
                    branchEnd = s2->m_lSpanEnd;
                }
                else {
                    /* inverted orientation */
                    branchBeg = s2->m_lSpanBeg;
                    branchEnd = s1->m_lSpanEnd;
                }
                span_c* branchSpan = new span_c( branchBeg, branchEnd, -1 );
                stack->m_pushObject( branchSpan );
                bin_tnode_c* branchNode = new bin_tnode_c( branchSpan );
                branchNode->mp_csLeftChild = binTNodeChart[s1->m_lSpanBeg][s1->m_lSpanEnd];
                branchNode->mp_csRightChild = binTNodeChart[s2->m_lSpanBeg][s2->m_lSpanEnd];
                binTNodeChart[s1->m_lSpanBeg][s1->m_lSpanEnd]->mp_csParent = branchNode;
                binTNodeChart[s2->m_lSpanBeg][s2->m_lSpanEnd]->mp_csParent = branchNode;
                binTNodeChart[branchBeg][branchEnd] = branchNode;
            }
        }
    }

    /* generate binarized rules */
    bool retFlag = false;
    if( stack->m_getSize() == 1 ) {
        m_genBinRule( binTNodeChart[0][rhsTarMonoRule->mp_stBlkVarSequence->m_getLength() - 1], \
            p_szLhs, rhsSrcMonoRule, rhsTarMonoRule, ruleID, binRuleList );
        retFlag = true;
    }
    else {
        //fprintf( stderr, "[WARNING]: rule NO. %d: \"%s -> %s ||| %s\" is not binarizable!\n", \
            ruleID, p_szLhs, p_szRhsSrc, p_szRhsTar );
        retFlag = false;
    }

    /* release the span stack */
    delete stack;

    /* release the binary tree chart */
    m_releaseChart( binTNodeChart, rVarNum );

    /* release the monolingual rules */
    delete rhsSrcMonoRule;
    delete rhsTarMonoRule;

    return retFlag;

}

void rule_binarizer_c::m_usage() {

    fprintf( stderr, "\n" );
    fprintf( stderr, "[usage]:\n" );
    fprintf( stderr, "%s [options]...\n", this->mp_szProgramName );
    fprintf( stderr, "-----------------------------------------------------------------------------\n" );
    fprintf( stderr, "[options]:\n" );
    fprintf( stderr, " -input        specify path to the input file, default \"stdin\"\n" );
    fprintf( stderr, " -output       specify path to the output file, default \"stdout\"\n" );
    fprintf( stderr, " -mtype        specify type of rule binarization method, default \"0\"\n" );
    fprintf( stderr, "                 effective types include [0:left heavy], [1:cost reduction]\n" );
    fprintf( stderr, " -varbase      specify the start index value of variables in a rule, default \"1\"\n" );
    fprintf( stderr, "-----------------------------------------------------------------------------\n" );
    fprintf( stderr, "[example]:\n" );
    fprintf( stderr, "%s -input scfg.rule -output scfg.rule.bi -mtype 0\n", this->mp_szProgramName );
    fprintf( stderr, "%s < scfg.rule > scfg.rule.bi\n", this->mp_szProgramName );

}
