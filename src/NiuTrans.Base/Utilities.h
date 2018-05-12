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
 * Utilities; Utilities.h
 * This header file defines the most general utility tools,
 * including some global macros, functions and classes.
 * They refer to the constant operations about
 * "time",
 * "string processing",
 * "mathematics"
 * and some "miscellanea".
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); Nov. 23th, 2011; add class "configMng_c"
 * Hao Zhang (email: zhanghao1216@gmail.com); May 12th, 2011
 *
 */


#ifndef    __UTILITIES_H__
#define    __UTILITIES_H__


#include <stdio.h>

#include "DataStruts.h"
using namespace datastructs;

#include "wchar.h"
#include "locale.h"
#if !defined( WIN32 ) && !defined( _WIN32 )
    #include "sys/time.h"
    #include "time.h"
    #include "iconv.h"
#else
    #include "time.h"
#endif

namespace util {

    /*
     * define inner data types
     */
    typedef unsigned long long ull_t;

    /**************************************************************************
     *
     * operations referring to "time" for platform Windows or Linux
     *
    ***************************************************************************/

    /*
     *
     * Time
     *
     */
    class time_c {

    public:
        time_c();
        ~time_c();
        /* set or get system's current time */
        void m_setNowTime( time_t ulNowTime );
        time_t m_getNowTime();
        char* m_getNowTimeString();
        /* for program timing */
        void m_startTimer();
        void m_endTimer();
        double m_getTimerDiff();

    private:
#if !defined( WIN32 ) && !defined( _WIN32 )
        timeval            m_stStartTime;
        timeval            m_stEndTime;
#else
        clock_t            m_ulStartTime;
        clock_t            m_ulEndTime;
#endif
        time_t            m_ulNowTime;

    };

    /**************************************************************************
     *
     * functions referring to "string processing", which are of static scopes
     *
    ***************************************************************************/

    /*
     * common English punctuations
     */
    static const char gs_szEnPuncs[] = ".,:;?!$()[]{}/\"\'\'``";

    /*
     *
     * String Dealer
     *
     */
    class string_c {

    protected:
        static hashtab_c * punctDict;

    public:
    	  static char* ms_fgets( char* buff, ull_t count, FILE* p_stFileHnd );
        static void ms_trimRight( char* p_szLine );
        static char* ms_readLine( FILE* p_stFileHnd );
        static ull_t ms_split( const char* p_szSrcStr, const char* p_szDelim, char**& pp_szTerms );
        static ull_t ms_splitPlus( const char* p_szSrcStr, const char* p_szDelim, char**& pp_szTerms );
        static void ms_freeStrArray( char** pp_szStrArr, ull_t ullStrArrSz );
        static char* ms_toLowercase( char* p_szStr );
        static bool ms_isEnPunc( const char* p_szToken );

        static bool IsLiteral(const char * word);
        static void ms_mvStr( char* p_szStr, long lMvOff, bool bToLeft );

        static void LoadPunctDict(const char * dictFileName);
        static void UnloadPunctDict();
        static bool IsPunc(const char * word);
        static char * GeneateString(char * src, mempool_c * mem);
        static char * Copy(char * string);
        static char * Copy(char * string, mempool_c * mem);
        static void RemoveRightSpaces(char * line);

    };

    /*
     *
     * Charset Encoding
     *
     */
    class iconv_c {

    public:
        static char* ms_iconv( const char* p_szSrcEnc, const char* p_szTgtEnc, char* p_szInStr, size_t ullInStrLen );

    private:
        static wchar_t* ms_win_UTF8_to_UC( char* p_szStr_utf8 );
        static char* ms_win_UC_to_GB( char* p_szStr_uc );
        static char* ms_win_UTF8_to_GB( char* p_szStr_utf8 );

    };

    /**************************************************************************
     *
     * mathematic functions, which are of static scopes
     *
    ***************************************************************************/

    /*
     *
     * Math
     *
     */
    class math_c {

    public:
        static bool ms_isPrime( ull_t ullNumber );
        static ull_t ms_nextPrime( ull_t ullNumber );
        static double ms_factorial( double dNum, long long ullFact );

    };

    /**************************************************************************
     *
     * configuration manager for managing all system's settings
     *
    ***************************************************************************/

    class configMng_c {

    public:
        static void ms_create( int argc, char** argv );
        static void ms_destroy();
        
    public:
        static unsigned long long ms_getParaNum();

    public:
        static const char* ms_getString( const char* p_szKey, const char* p_szDefaultVal );
        static bool ms_getBool( const char* p_szKey, bool bDefaultVal );
        static int ms_getInt( const char* p_szKey, int iDefaultVal );
        static float ms_getFloat( const char* p_szKey, float fDefaultVal );
        static double ms_getDouble( const char* p_szKey, double dDefaultVal );

    protected:
        static void ms_loadArgsFromConfig( const char* p_szConfigFile );
        static void ms_loadArgsFromCmdLine( int argc, char** argv );

    private:
        static void ms_freeAsciiStr( const void* p_vdVar );

    private:
        static hashtab_c*   mp_csHashTab;

    };

}


#endif

