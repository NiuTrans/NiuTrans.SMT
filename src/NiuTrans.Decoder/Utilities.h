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
 * Hao Zhang (email: zhanghao1216@gmail.com); May 12th, 2011
 *
 */


#ifndef    __UTILITIES_H__
#define    __UTILITIES_H__


#include <stdio.h>

#include "MemManager.h"
using namespace memmanager;

#include "wchar.h"
#include "locale.h"
#if !defined( WIN32 ) && !defined( _WIN32 )
    #include "sys/time.h"
    #include "time.h"
    #include "iconv.h"
#else
    #include "time.h"
#endif

namespace utilities {

/**************************************************************************
*
* generic inner data types
*
***************************************************************************/

#if !defined( WIN32 ) && !defined( _WIN32 )
    #define __MACHINE_64_DATATYPE__
#endif

#ifdef __MACHINE_64_DATATYPE__
    typedef unsigned long long      integer_t;      /* 8 bytes */
    typedef long double             decimal_t;      /* 8 bytes */
#else
    typedef unsigned int            integer_t;      /* 4 bytes */
    typedef float                   decimal_t;      /* 4 bytes */
#endif
    typedef int       (*CompareT)( const void * vdVar1, const void * vdVar2 );
    typedef void      (*FreeT)( const void * vdVar );
    typedef integer_t (*SBNT)( const void * vdStr );
    

    /*
     *
     * data structure to simulate basic C data types, including
     * signed or unsigned char, short, int, long (Integer),
     * float, double (Decimal),
     * and void* (Object).
     *
     */
    typedef union __var_u {

    public:
        __var_u& operator=( const __var_u& unData );

    public:
        integer_t           ullInteger;
        decimal_t           ldDecimal;
        void *              mvdObject;

    }var_u;

    typedef int       (*CompVarT)( const var_u& unVal1, const var_u& unVal2 );
    

    /**************************************************************************
     *
     * foundational data structures
     *
    ***************************************************************************/

    /*
     *
     * Hash
     *
     * It simulates a hash table of dynamic growing size
     *
     */

    /*
     * structure of "hashnode" in hash table
     */
    struct HashNode {

        void *        vdKey;
        var_u *       unValue;
        HashNode *    nextNode;

    };

    class HashTable {

    public:
        /* constructors and deconstructor for "new" operator */
        HashTable( unsigned long long ullMyHashTabSz = 1024, long double ldConflictThreshold = 0.75, FreeT fnMyFreeFunc = NULL, \
            CompareT fnKeyCompFunc = NULL, SBNT fnKeyByteNumFunc = NULL );
        ~HashTable();

    public:
        /* constructors and deconstructor for "calloc, malloc, memory pool" operations */
        void Create( unsigned long long ullMyHashTabSz = 1024, FreeT fnMyFreeFunc = NULL, \
            CompareT fnKeyCompFunc = NULL, SBNT fnKeyByteNumFunc = NULL );
        void Destroy();

    public:
        void Clear();
        /* interfaces for "Integer" data types */
        void AddInt( const void* vdKey, integer_t ullValue );
        integer_t GetInt( const void* vdKey );
        /* interfaces for "Decimal" data types */
        void AddFloat( const void* vdKey, decimal_t ldValue );
        decimal_t GetFloat( const void* vdKey );
        /* interfaces for "user-defined" data types */
        void AddObject( const void* vdKey, void* vdValue );
        void* GetObject( const void* vdKey );

    public:
        void* GetKey( unsigned long long ullKeyIdx );
        unsigned long long GetKeyCnt();

    protected:
        unsigned long long ToHashCode( const void* vdKey );
        void Resize( unsigned long long ullNewHashTabSz );
        var_u* GetHashNode( const void* vdKey );
        HashNode* CreateHashNode( const void* vdKey, var_u* unValue );
        var_u* AddHashNode( const void* vdKey, var_u* unValue );

    private:
        static int ASCKeyCompFunc( const void* vdKey1, const void* vdKey2 );
        static integer_t ASCKeyByteNum( const void* vdStr );

    private:
        /* table of hashnode(s) */
        HashNode**                  mstHashTab;
        unsigned long long          ullHashTabSz;
        unsigned long long          ullHashNdCnt;
        /* table of key(s) */
        void**                      mvdKeyTab;
        unsigned long long          ullKeyCnt;
        /* threshold of average "key" conflicts */
        long double                 ldThreshold;

    private:
        MemPool*                  mcsMempool;
        FreeT                       mfnFreeFunc;
        CompareT                    mfnKeyCompFunc;
        SBNT                        mfnKeyByteNumFunc;

    };

    ////////////////////////////////////////
    // list

    typedef    int (* ListCompare)(const void * item1, const void * item2);

    // list
    class List
    {
    public:
        void **         items;
        int             count;
        int             maxNum;
        MemPool *       mem;

    public:
        List();
        List(int myMaxNum);
        List(int myMaxNum, MemPool * myMem);
        ~List();
        void Create(int myMaxNum, MemPool * myMem);
        void Add(void * item);
        void Add(void ** inputItems, int inputItemCount);
        void Insert(int pos, void * item);
        void * GetItem(int i);
		void SetItem(int i, void * item);
        void Clear();
        void Sort(int itemSize, ListCompare comp);
        void Reverse();
		List * Copy(MemPool * myMem);
    };
    
}

namespace utilities {

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
    class TimeUtil {

    public:
        TimeUtil();
        ~TimeUtil();
        /* set or get system's current time */
        void SetNowTime( time_t ulNowTime );
        time_t GetNowTime();
        char* GetNowTimeString();
        /* for program timing */
        void StartTimer();
        void EndTimer();
        double GetTimerDiff();

    private:
#if !defined( WIN32 ) && !defined( _WIN32 )
        timeval            stStartTime;
        timeval            stEndTime;
#else
        clock_t            ulStartTime;
        clock_t            ulEndTime;
#endif
        time_t             ulNowTime;

    };

    /**************************************************************************
     *
     * functions referring to "string processing", which are of static scopes
     *
    ***************************************************************************/

    /*
     * common English punctuations
     */
    static const char szEnPuncs[] = ".,:;\'\"?!()[]{}/";

    /*
     *
     * String Dealer
     *
     */
    class StringUtil {

    protected:
        static HashTable * punctDict;

    public:
        static void TrimRight( char* szLine );
        static char* ReadLine( FILE* stFileHnd );
        static ull_t Split( const char* szSrcStr, const char* szDelim, char**& szTerms );
        static void FreeStringArray( char** szStrArr, ull_t ullStrArrSz );
        static char* ToLowercase( char* szStr );
        static bool IsEnPunc( const char* szToken );

        static bool IsLiteral(const char * word);
        static void ms_mvStr( char* szStr, long lMvOff, bool bToLeft );

        static void LoadPunctDict(const char * dictFileName);
        static void UnloadPunctDict();
        static bool IsPunc(const char * word);
        static char * GeneateString(char * src, MemPool * mem);
        static char * Copy(char * string);
        static char * Copy(char * string, MemPool * mem);
        static void RemoveRightSpaces(char * line);
        static char * NormalizeText(char * line);
        static char * InternationalTokenization(const char * line);

    };

    /*
     *
     * Charset Encoding
     *
     */
    class Encoding {

    public:
        static char* Convert( const char* szSrcEnc, const char* szTgtEnc, char* szInStr, size_t ullInStrLen );

    private:
        static wchar_t* UTF8_to_UC( char* szStr_utf8 );
        static char* UC_to_GB( char* szStr_uc );
        static char* UTF8_to_GB( char* szStr_utf8 );

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
    class MathUtil {

    public:
        static bool IsPrime( ull_t ullNumber );
        static ull_t NextPrime( ull_t ullNumber );

    };

}


#endif

