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
 * Utilities; Utilities.cpp
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


#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "Utilities.h"

namespace util {

    /**************************************************************************
     *
     * member functions of class "time_c"
     *
    ***************************************************************************/

    time_c::time_c() {
    }

    time_c::~time_c() {
    }

    /*
     @ Function:
     * start counting time
     */
    void time_c::m_startTimer() {

#if !defined( WIN32 ) && !defined( _WIN32 )
        gettimeofday( &m_stStartTime, NULL );
#else
        m_ulStartTime = clock();
#endif

    }

    /*
     @ Function:
     * end counting time
     */
    void time_c::m_endTimer() {

#if !defined( WIN32 ) && !defined( _WIN32 )
        gettimeofday( &m_stEndTime, NULL );
#else
        m_ulEndTime = clock();
#endif

    }

    /*
     @ Function:
     * get the time difference between actions of end timer counting and start timer counting.
     *
     @ Return:
     * time difference in unit of "second" (double)
     */
    double time_c::m_getTimerDiff() {

        double dTimeDiff;

#if !defined( WIN32 ) && !defined( _WIN32 )
        dTimeDiff = ( m_stEndTime.tv_sec - m_stStartTime.tv_sec ) + \
            ( m_stEndTime.tv_usec - m_stStartTime.tv_usec ) / 1000000;
#else
        dTimeDiff = difftime( m_ulEndTime, m_ulStartTime ) / CLOCKS_PER_SEC;
#endif
        
        return dTimeDiff;

    }

    /*
     @ Function:
     * set current system's time
     */
    void time_c::m_setNowTime( time_t ulNowTime ) {

        this->m_ulNowTime = ulNowTime;

    }

    /*
     @ Function:
     * get current system's time in seconds
     *
     @ Return:
     * current time in digit format (time_t or long)
     */
    time_t time_c::m_getNowTime() {

#if !defined( WIN32 ) && !defined( _WIN32 )
        timeval    stCurTime;
        gettimeofday( &stCurTime, NULL );
        m_ulNowTime = ( time_t ) stCurTime.tv_sec;
        if( stCurTime.tv_usec > 1000000 * 0.5 )
            m_ulNowTime += 1L;
#else
        time( &m_ulNowTime );
#endif
        return m_ulNowTime;

    }

    /*
     @ Function:
     * get current system's time
     *
     @ Return:
     * current time in string format (char*)
     */
    char* time_c::m_getNowTimeString() {

        time_t ulNow = m_getNowTime();
        char* p_szTime = ctime( &ulNow );
        int ullTimeStrLen = (int)strlen(p_szTime);
        if( p_szTime[ullTimeStrLen - 1] == '\n' ) {
            p_szTime[ullTimeStrLen - 1] = '\0';
        }
        return p_szTime;

    }

    /**************************************************************************
     *
     * "string processing" functions, which are of static scopes
     *
    ***************************************************************************/

    /*
     @ Function:
     * read given number of bytes from a file pointer
     *
     @ Arguments:
     * 1) a pointer to a string buffer
     * 2) count of bytes to read
     * 3) file pointer
     *
     @ Return:
     * a pointer to the string buffer
     */
    char* string_c::ms_fgets( char* buff, ull_t count, FILE* p_stFileHnd ) {

        if( 0 == count ) {
            return NULL;
        }

        ull_t widx = 0;
        unsigned char ch = fgetc( p_stFileHnd );
        while( !feof( p_stFileHnd ) && widx < count ) {
            buff[widx++] = ch;
            if( '\n' == ch ) {
                break;
            }
            ch = fgetc( p_stFileHnd );
        }
        if( widx > 0 ) {
            buff[widx] = '\0';
            return buff;
        }
        else {
            return NULL;
        }

    }

    /*
     @ Function:
     * eliminate all '\r' and '\n' characters at the end of line
     *
     @ Arguments:
     * pointer to string
     *
     @ Return:
     * NULL
     */
    void string_c::ms_trimRight( char* p_szLine ) {

        long long llStrLen = strlen( p_szLine );
        long long llIdx = llStrLen - 1;
        while( llIdx >= 0 ) {
            if( p_szLine[llIdx] != '\r' && p_szLine[llIdx] != '\n' ) {
                break;
            }
            p_szLine[llIdx] = '\0';
            --llIdx;
        }

    }

    /*
     @ Function:
     * read one line from the given input stream
     *
     @ Arguments:
     * file pointer as input stream (FILE*)
     *
     @ Return:
     * pointer to the read line
     *
     @ Basic Algorithm:
     * incrementally read in data from input stream
     * until encounter the character '\n' or reach the end of file
     */
    char* string_c::ms_readLine( FILE* p_stFileHnd ) {

        ull_t ullLineLen = 128;
        char* p_szLine = new char[ullLineLen];
        if( !fgets( p_szLine, (int)ullLineLen, p_stFileHnd ) ) {
            if( feof( p_stFileHnd ) ) {
            }
            else if( ferror( p_stFileHnd ) ) {
                fprintf( stderr, "[ERROR]: reading file failed!\n" );
            }
            delete[] p_szLine;
            return NULL;
        }
        /* read the rest part of line into string buffer */
        while( p_szLine[strlen( p_szLine ) - 1] != '\n' ) {
            char* p_szTmpPtr = NULL;
            ull_t ullTmpLen = ullLineLen + 1;
            char* p_szOldLine = p_szLine;
            ullLineLen *= 2;
            p_szLine = new char[ullLineLen];
            strcpy( p_szLine, p_szOldLine );
            delete[] p_szOldLine;
            p_szTmpPtr = p_szLine + strlen( p_szLine );
            if( !fgets( p_szTmpPtr, (int)ullTmpLen, p_stFileHnd ) ) {
                if( feof( p_stFileHnd ) ) {
                    break;
                }
                else {
                    fprintf( stderr, "[ERROR]: reading file failed!\n" );
                    delete[] p_szLine;
                    return NULL;
                }
            }
        }
        /* eliminate carriage return and line feed characters at the end of line */
        ms_trimRight( p_szLine );

        return p_szLine;

    }

    /*
     @ Function:
     * split source string into several tokens which are originally separated by specified delimiter
     * the function does not break the source string
     *
     @ Arguments:
     * 1) pointer to source string (const char*)
     * 2) pointer to delimiter (const char*)
     * 3) reference to token arrays as split results (char**&)
     *
     @ Return:
     * number of tokens generated (unsigned long long)
     *
     @ Basic Algorithm:
     * 1) recognize and record all tokens between one delimiter
     * 2) construct array of tokens
     */
    ull_t string_c::ms_split( const char* p_szSrcStr, const char* p_szDelim, char**& pp_szTerms ) {

        ull_t ullSrcStrLen = strlen( p_szSrcStr );
        ull_t ullDelimLen = strlen( p_szDelim );
        /* empty source string or the source string is exactly the same with the delimiter string */
        if( ullSrcStrLen == 0 || !strcmp( p_szSrcStr, p_szDelim ) ) {
            pp_szTerms = NULL;
            return 0;
        }
        /* an empty delimiter string or delimiter string is not shorter than the source string */
        if( ullDelimLen == 0 || ullDelimLen >= ullSrcStrLen  ) {
            pp_szTerms = new char*[1];
            pp_szTerms[0] = new char[ullSrcStrLen + 1];
            strcpy( pp_szTerms[0], p_szSrcStr );
            return 1;
        }
        /* now for general case, that is, the delimiter string is shorter than the source string :) */
        pp_szTerms = new char*[ullSrcStrLen + 1];
        ull_t ullTermCnt = 0;
        bool matched = false;
        /* scan the source string from left to right */
        for( int i=0; i < ullSrcStrLen; ) {
            /* match delimiter */
            if( !strncmp( &p_szSrcStr[i], p_szDelim, ullDelimLen ) ) {
                i += (int)ullDelimLen;
                if(!matched || i == ullSrcStrLen){
                    pp_szTerms[ullTermCnt] = new char[1];
                    pp_szTerms[ullTermCnt][0] = '\0';
                    ++ullTermCnt;
                }
                matched = false;
            }
            /* match token */
            else {
                /* find end boundary of the matched token */
                ull_t j = i + 1;
                while( j < ullSrcStrLen && strncmp( &p_szSrcStr[j], p_szDelim, ullDelimLen ) ) {
                    ++j;
                }
                /* record the matched token */
                ull_t ullTokenLen = j - i;
                pp_szTerms[ullTermCnt] = new char[ullTokenLen + 1];
                strncpy( pp_szTerms[ullTermCnt], &p_szSrcStr[i], ullTokenLen );
                pp_szTerms[ullTermCnt][ullTokenLen] = '\0';
                ++ullTermCnt;
                i = (int)j;
                matched = true;
            }
        }
        return ullTermCnt;

    }

    /*
     @ Function:
     * split source string into several tokens which are originally separated by specified delimiter
     * consective delimiters will be regarded as one delimiter
     * the function does not break the source string
     *
     @ Arguments:
     * 1) pointer to source string (const char*)
     * 2) pointer to delimiter (const char*)
     * 3) reference to token arrays as split results (char**&)
     *
     @ Return:
     * number of tokens generated (unsigned long long)
     *
     @ Basic Algorithm:
     * 1) recognize and record all tokens between one or more delimiters
     * 2) construct array of tokens
     */
    ull_t string_c::ms_splitPlus( const char* p_szSrcStr, const char* p_szDelim, char**& pp_szTerms ) {

        ull_t ullSrcStrLen = strlen( p_szSrcStr );
        ull_t ullDelimLen = strlen( p_szDelim );
        /* empty source string or the source string is exactly the same with the delimiter string */
        if( ullSrcStrLen == 0 || !strcmp( p_szSrcStr, p_szDelim ) ) {
            pp_szTerms = NULL;
            return 0;
        }
        /* an empty delimiter string or delimiter string is not shorter than the source string */
        if( ullDelimLen == 0 || ullDelimLen >= ullSrcStrLen  ) {
            pp_szTerms = new char*[1];
            pp_szTerms[0] = new char[ullSrcStrLen + 1];
            strcpy( pp_szTerms[0], p_szSrcStr );
            return 1;
        }
        /* now for general case, that is, the delimiter string is shorter than the source string :) */
        pp_szTerms = new char*[ullSrcStrLen + 1];
        ull_t ullTermCnt = 0;
        /* scan the source string from left to right */
        for( int i=0; i < ullSrcStrLen; ) {
            /* match delimiter */
            if( !strncmp( &p_szSrcStr[i], p_szDelim, ullDelimLen ) ) {
                i += (int)ullDelimLen;
            }
            /* match token */
            else {
                /* find end boundary of the matched token */
                ull_t j = i + 1;
                while( j < ullSrcStrLen && strncmp( &p_szSrcStr[j], p_szDelim, ullDelimLen ) ) {
                    ++j;
                }
                /* record the matched token */
                ull_t ullTokenLen = j - i;
                pp_szTerms[ullTermCnt] = new char[ullTokenLen + 1];
                strncpy( pp_szTerms[ullTermCnt], &p_szSrcStr[i], ullTokenLen );
                pp_szTerms[ullTermCnt][ullTokenLen] = '\0';
                ++ullTermCnt;
                i = (int)j;
            }
        }
        return ullTermCnt;

    }

    /*
     @ Function:
     * free a string array
     *
     @ Arguments:
     * 1) pointer to the string array
     * 2) size of the string array
     */
    void string_c::ms_freeStrArray( char** pp_szStrArr, ull_t ullStrArrSz ) {

        for( ull_t i=0; i < ullStrArrSz; i++ ) {
            delete[] pp_szStrArr[i];
        }
        delete[] pp_szStrArr;

    }

    /*
     @ Function:
     * change a source string to a string in lowercase
     *
     @ Arguments:
     * the source string (char*)
     *
     @ Return:
     * pointer to the changed string
     *
     @ Basic Algorithm:
     * scan over the source string and change any character which is in uppercase
     */
    char* string_c::ms_toLowercase( char* p_szStr ) {

        char* p_szChPtr = p_szStr;
        while( *p_szChPtr != '\0' ) {
            if( *p_szChPtr >= 'A' && *p_szChPtr <= 'Z' ) {
                *p_szChPtr += 'a' - 'A';
            }
            ++p_szChPtr;
        }
        return p_szStr;

    }

    /*
     @ Function:
     * judge whether a token is an English (in ASCII code) punctuation
     *
     @ Arguments:
     * pointer to the token (const char*)
     *
     @ Return:
     * true if it is an English punctuation
     * flase if it is not (bool)
     *
     @ Basic Algorithm:
     * judge whether the token has only one character and is a member of pre-specified punctuations
     */
    bool string_c::ms_isEnPunc( const char* p_szToken ) {

        if( strlen( p_szToken ) <= 2 && strstr( gs_szEnPuncs, p_szToken ) )
            return true;
        return false;

    }

    hashtab_c * string_c::punctDict = NULL;

    void string_c::LoadPunctDict(const char * dictFileName)
    {
        int maxWordLen = 1024;
        int punctCount = 0;
        char * punct = new char[maxWordLen];

        punctDict = new hashtab_c(1000);

        FILE * f = fopen(dictFileName, "rb");
        if(f == NULL){
            fprintf(stderr, "ERROR: cannot open file \"%s\"!", dictFileName);
            return;
        }

        time_c timer;
        timer.m_startTimer();
        fprintf( stderr, "Loading Punctuations\n");
        fprintf( stderr, "  >> From File: %s ...\n", dictFileName );
        fprintf( stderr, "  >> " );

        while(fgets(punct, maxWordLen - 1, f)){
            for(int i = (int)strlen(punct) - 1; i >= 0; i--){
                if(punct[i] == '\r' || punct[i] == '\n')
                    punct[i] = '\0';
                else
                    break;
            }

            punctDict->m_addInt(punct, ++punctCount);
            if( punctCount % 5 == 0 ) {
                fprintf( stderr, "#" );
            }
        }

        timer.m_endTimer();
        double time = timer.m_getTimerDiff();
        fprintf( stderr, "\nDone [%d entries, %.3f sec(s)]\n", punctCount, time );

        delete[] punct;
    }

    void string_c::UnloadPunctDict()
    {
        delete punctDict;
    }

    bool string_c::IsPunc(const char * word)
    {
        if( strlen( word ) == 1 && strstr( gs_szEnPuncs, word ) )
            return true;
        if( punctDict != NULL ){
            int pid = punctDict->m_getInt(word);
            if(pid != -1){
                //fprintf(stderr, "punct: %d %s\n", punctDict->m_getInt(word), word);
                return true;
            }
        }
        return false;
    }

    char * string_c::GeneateString(char * src, mempool_c * mem)
    {
        char * s = (char *)mem->m_alloc(sizeof(char) * ((int)strlen(src) + 1));
        strcpy(s, src);
        return s;
    }

    char * string_c::Copy(char * string)
    {
        int length = (int)strlen(string);
        char * s = new char[length + 1];
        strcpy(s, string);
        return s;
    }

    char * string_c::Copy(char * string, mempool_c * mem)
    {
        int length = (int)strlen(string);
        char * s = (char*)mem->m_alloc(length + 1);
        strcpy(s, string);
        return s;
    }

    void string_c::RemoveRightSpaces(char * line)
    {
        long long llStrLen = strlen(line);
        long long llIdx = llStrLen - 1;
        while( llIdx >= 0 ) {
            if( line[llIdx] != ' ' && line[llIdx] != '\t' ) {
                break;
            }
            line[llIdx] = '\0';
            --llIdx;
        }
    }

    /*
     @ Function:
     * extract part of string as the final string
     */
    void string_c::ms_mvStr( char* p_szStr, long lMvOff, bool bToLeft ) {

        long lStrLen = (long)strlen( p_szStr );
        if( lMvOff <= 0 ) {
            return;
        }
        else if( lMvOff >= lStrLen ) {
            p_szStr[0] = '\0';
        }
        else {
            if( !bToLeft ) {
                /* trim string tail */
                p_szStr[lStrLen - lMvOff] = '\0';
            }
            else {
                /* extract string head */
                long i;
                for( i=lMvOff; i < lStrLen; i++ ) {
                    p_szStr[i-lMvOff] = p_szStr[i];
                }
                p_szStr[i-lMvOff] = '\0';
            }
        }

    }

    /*
     *
     * Charset Encoding
     *
     */
    char* iconv_c::ms_iconv( const char* p_szSrcEnc, const char* p_szTgtEnc, char* p_szInStr, size_t ullInStrLen ) {

#if !defined( WIN32 ) && !defined( _WIN32 )
        iconv_t cd = iconv_open( p_szTgtEnc, p_szSrcEnc );
        if( cd == (iconv_t)-1 ) {
            return NULL;
        }
        size_t inLen = ullInStrLen;
        size_t outLen = inLen * 3 + 1;
        char* p_szOutStr = new char[outLen];
        memset( p_szOutStr, 0, outLen );
        char* pin = p_szInStr;
        char* pout = p_szOutStr;
        if( iconv( cd, &pin, &inLen, &pout, &outLen ) == -1 ) {
            return NULL;
        }
        iconv_close( cd );
        return p_szOutStr;

#else
        if( !strcmp( p_szSrcEnc, "UTF-8" ) && !strcmp( p_szTgtEnc, "UCS-2" ) ) {
            return ( char* ) ms_win_UTF8_to_UC( p_szInStr );
        }
        else if( !strcmp( p_szSrcEnc, "UCS-2" ) && !strcmp( p_szTgtEnc, "GB2312" ) ) {
            return ms_win_UC_to_GB( p_szInStr );
        }
        else if( !strcmp( p_szSrcEnc, "UTF-8" ) && !strcmp( p_szTgtEnc, "GB2312" ) ) {
            return ms_win_UTF8_to_GB( p_szInStr );
        }
        return p_szInStr;
#endif

    }

    wchar_t* iconv_c::ms_win_UTF8_to_UC( char* p_szStr_utf8 ) {

        int i, j;
        unsigned char ch;
        unsigned char high, mid, low;
        char* p_szStr_uc = new char[strlen( p_szStr_utf8 ) * 2 + 3];
        i = 0; j = 0;
        while( ch = p_szStr_utf8[i] )
        {
            /* one byte */
            if( ch==0xEF && (unsigned char)p_szStr_utf8[i+1]==0xBB && (unsigned char)p_szStr_utf8[i+2]==0xBF )
                i += 3;
            ch = p_szStr_utf8[i];
            if( ch < 0x80 )
            {
                p_szStr_uc[j] = ch;
                p_szStr_uc[++j] = 0x00;
                ++j;
                ++i;
            }
            /* two bytes */
            else if( ( ch & 0xE0 ) == 0xC0 )
            {
                high = ch & 0x1C;
                high = high >> 2;
                mid = ch & 0x03;
                mid = mid << 6;
                low = p_szStr_utf8[i+1] & 0x3F;
                low = mid | low;
                p_szStr_uc[j] = low;
                p_szStr_uc[++j] = high;
                ++j;
                i += 2;
            }
            /* three bytes */
            else if( ( ch & 0xF0 ) == 0xE0 )
            {
                high = ch & 0x0F;
                high = high << 4;
                mid = p_szStr_utf8[i+1] & 0x3C;
                mid = mid >> 2;
                high = high | mid;
                mid = p_szStr_utf8[i+1] & 0x03;
                mid = mid << 6;
                low = p_szStr_utf8[i+2] & 0x3F;
                low = mid | low;
                p_szStr_uc[j] = low;
                p_szStr_uc[++j] = high;
                ++j;
                i += 3;
            }
        }
        p_szStr_uc[j] = 0x00;
        p_szStr_uc[j+1] = 0x00;

        return ( wchar_t* ) p_szStr_uc;

    }

    char* iconv_c::ms_win_UC_to_GB( char* p_szStr_uc ) {

        long len = (long)wcslen( ( wchar_t* ) p_szStr_uc );
        char* p_szStr_gb = new char[2 * len + 1];
        
        if( !setlocale( LC_ALL, "chs" ) )
            return NULL;
        len = (long)wcstombs( p_szStr_gb, ( wchar_t* ) p_szStr_uc, ( 2 * len + 1 ) );
        return p_szStr_gb;

    }

    char* iconv_c::ms_win_UTF8_to_GB( char* p_szStr_utf8 ) {

        wchar_t* p_szStr_uc = ms_win_UTF8_to_UC( p_szStr_utf8 );
        char* p_szStr_gb = ms_win_UC_to_GB( ( char* ) p_szStr_uc );
        delete[] p_szStr_uc;
        return p_szStr_gb;

    }

    /**************************************************************************
     *
     * mathematic functions, which are of static scopes
     *
    ***************************************************************************/

    /*
     @ Function:
     * judge whether a given number is a prime number or not
     *
     @ Arguments:
     * the given number (unsigned long long)
     *
     @ Return:
     * true or false (bool)
     *
     @ Basic Algorithm:
     * check whether any number less than "sqart(the input number)" is a divider of the input number
     * if any of those is, the number is not a prime number
     * else it is
     */
    bool math_c::ms_isPrime( ull_t ullNumber ) {

        ull_t ullDivider = 3;
        ull_t ullSquare = ullDivider * ullDivider;

        if( ullNumber < 10 ) {
            switch( ullNumber ) {
            case 2:
            case 3:
            case 5:
            case 7:
                return true;
            default:
                return false;
            }
        }
        while( ullSquare < ullNumber && ullNumber % ullDivider ) {
            ++ullDivider;
            ullSquare += 4 * ullDivider;
            ++ullDivider;
        }
        if( ullNumber % ullDivider == 0 ) {
            return false;
        }
        return true;

    }

    /*
     @ Function:
     * find the next smallest prime number which is larger than the input number
     *
     @ Arguments:
     * an input number (unsigned long long)
     *
     @ Return:
     * the wanted prime number
     *
     @ Basic Algorithm:
     * check any odd number larger than the input number until find a prime number
     * note the number might exceed the data range of the data type
     * at that time, return the maximum number the data type supports
     */
    ull_t math_c::ms_nextPrime( ull_t ullNumber ) {

        ull_t ullCandidate = ullNumber;
        /* make "candidate number" definitely odd */
        ullCandidate |= 1;
        while( !ms_isPrime( ullCandidate ) ) {
            ullCandidate += 2;
        }
        if( ullCandidate < ullNumber )
            return ((ull_t)-1);
        return ullCandidate;


    }

    bool string_c::IsLiteral(const char * word)
    {
        int len = (int)strlen(word);

        for(int i = 0; i < len; i++){
            char ch = word[i];
            if( ch < 0 )
                return false;
        }
        return true;
    }

    double math_c::ms_factorial( double dNum, long long ullFact ) {

        if( ullFact == 0 ) {
            return 1.0;
        }
        double ret = dNum;
        long long fact = ( ullFact > 0 ? ullFact : -ullFact );
        for( ull_t i=0; i < fact - 1; i++ ) {
            ret *= dNum;
        }
        if( ullFact < 0 ) {
            ret = 1.0 / ret;
        }
        return ret;

    }

    /**************************************************************************
     *
     * globals shared for the whole project
     *
    ***************************************************************************/

    /*
     @ Function:
     * free a string
     *
     @ Arguments:
     * pointer to the string (void*)
     */
    void configMng_c::ms_freeAsciiStr( const void* p_vdVar ) {

        char* p_szStr = ( char* ) p_vdVar;
        delete[] p_szStr;

    }

    /**************************************************************************
     *
     * class "configMng_c" for managing all system's settings
     *
    ***************************************************************************/

    hashtab_c* configMng_c::mp_csHashTab = NULL;

    /*
     @ Function:
     * create and initialize a new configuration manager
     * load all system's settings
     *
     @ Arguments:
     * 1) number of command-line arguments (int)
     * 2) array of command-line arguments (char**)
     * 3) configuration file's path (char*)
     */
    void configMng_c::ms_create( int argc, char** argv ) {

        time_c timer;
        mp_csHashTab = new hashtab_c( 1024, 0.75, configMng_c::ms_freeAsciiStr );
        char* configFN = NULL;

        for(int i = 0; i < argc - 1; i++){
            if(strcmp(argv[i], "-config") == 0){
                configFN = argv[i + 1];
            }
        }

        fprintf( stderr, "Loading system settings\n" );
        timer.m_startTimer();
        if( configFN != NULL && strcmp( configFN, "" ) != 0 )
            ms_loadArgsFromConfig( configFN );
        ms_loadArgsFromCmdLine( argc, argv );
        timer.m_endTimer();
        fprintf( stderr, "Done [%.2f s]\n", timer.m_getTimerDiff() );

    }

    /*
     @ Function:
     * destroy the configuration manager
     */
    void configMng_c::ms_destroy() {

        delete mp_csHashTab;

    }

    /*
     @ Function:
     * get the total parameter number
     *
     @ Return:
     * the total parameter number (unsigned long long)
     */
    unsigned long long configMng_c::ms_getParaNum() {

        return mp_csHashTab->m_getKeyCnt();

    }

    /*
     @ Function:
     * load system's settings from configuration file
     *
     @ Arguments:
     * configuration file's path (char*)
     */
    void configMng_c::ms_loadArgsFromConfig( const char* p_szConfigFile ) {

       fprintf( stderr, "  >> From File: %s ...\n", p_szConfigFile );
       /* open configuration file */
       FILE* p_stFileHnd = fopen( p_szConfigFile, "r" );
       if( p_stFileHnd == NULL ) {
           fprintf( stderr, "[ERROR]: file %s not existed!\n", p_szConfigFile );
           return;
       }
       /* record <key, value> pairs */
       char* p_szLine = NULL;
       while( p_szLine = string_c::ms_readLine( p_stFileHnd ) ) {
            char* p_szKeyPtr = strstr( p_szLine, "param=\"" );
            char* p_szValPtr = strstr( p_szLine, "value=\"" );
            /* find the <key, value> pair */
            if( p_szKeyPtr && p_szValPtr ) {
                int iLen = (int)strlen( p_szLine );
                char* p_szKey = new char[iLen + 1];
                char* p_szVal = new char[iLen + 1];
                p_szKeyPtr += strlen( "param=\"" );
                p_szValPtr += strlen( "value=\"" );
                /* get key string */
                int i = 0;
                while( *p_szKeyPtr != '\"' ) {
                    p_szKey[i++] = *p_szKeyPtr++;
                }
                p_szKey[i] = '\0';
                /* get value string */
                i = 0;
                while( *p_szValPtr != '\"' ) {
                    p_szVal[i++] = *p_szValPtr++;
                }
                p_szVal[i] = '\0';
                mp_csHashTab->m_addObject( string_c::ms_toLowercase( p_szKey ), p_szVal );
                delete[] p_szKey;
                /* Note: do not "delete p_szVal" because we use shallow copy for complex data types */
            }
            delete[] p_szLine;
       }
       /* close configuration file */
       fclose( p_stFileHnd );

    }

    /*
     @ Function:
     * load system's settings from command-line
     *
     @ Arguments:
     * 1) number of command-line arguments (int)
     * 2) array of command-line arguments (char**)
     */
    void configMng_c::ms_loadArgsFromCmdLine( int argc, char** argv ) {

        if( argc < 2 ) {
            return;
        }
        fprintf( stderr, "  >> From command-line ...\n" );
        for( int i=1; i < argc; i++ ) {
            if( argv[i][0] == '-' ) {
                if( i == argc - 1 || argv[i+1][0] == '-' ){
                    char * defaultSetting = new char[2];
                    defaultSetting[0] = '1';
                    defaultSetting[1] = '\0';
                    mp_csHashTab->m_addObject( string_c::ms_toLowercase( &argv[i][1] ), defaultSetting );
                }
                else{
                    int iValLen = (int)strlen( argv[i+1] );
                    char* p_szVal = ( char* ) new char[iValLen+1];
                    strcpy( p_szVal, argv[i+1] );
                    mp_csHashTab->m_addObject( string_c::ms_toLowercase( &argv[i][1] ), p_szVal );
                }
            }
        }

    }

    /*
     @ Function:
     * get the string value of a given parameter
     *
     @ Arguments:
     * 1) parameter name (char*)
     * 2) parameter default value (char*)
     *
     @ Return:
     * parameter value (char*)
     */
    const char* configMng_c::ms_getString( const char* p_szKey, const char* p_szDefaultVal ) {

        const char * defaultVal = p_szDefaultVal;
        char* p_szKeyLc = new char[strlen( p_szKey ) + 1];
        strcpy( p_szKeyLc, p_szKey );
        string_c::ms_toLowercase( p_szKeyLc );
        char* p_szVal = ( char* ) mp_csHashTab->m_getObject( p_szKeyLc );
        delete[] p_szKeyLc;

        if( p_szVal == NULL ) {
            return p_szDefaultVal;
        }
        else {
            return p_szVal;
        }

    }

    /*
     @ Function:
     * get the boolean value of a given parameter
     *
     @ Arguments:
     * 1) parameter name (char*)
     * 2) parameter default value (bool)
     *
     @ Return:
     * parameter value (bool)
     */
    bool configMng_c::ms_getBool( const char* p_szKey, bool bDefaultVal ) {

        char* p_szVal = ( char* ) ms_getString( p_szKey, NULL );
        if( p_szVal == NULL || strcmp( p_szVal, "" ) == 0 ) {
            return bDefaultVal;
        }
        else {
            if( strcmp( p_szVal, "0" ) == 0 || strcmp( string_c::ms_toLowercase( p_szVal ), "false" ) == 0 ) {
                return false;
            }
            else {
                return true;
            }
        }

    }

    /*
     @ Function:
     * get the integer value of a given parameter
     *
     @ Arguments:
     * 1) parameter name (char*)
     * 2) parameter default value (int)
     *
     @ Return:
     * parameter value (int)
     */
    int configMng_c::ms_getInt( const char* p_szKey, int iDefaultVal ) {

        const char* p_szVal = ms_getString( p_szKey, NULL );
        if( p_szVal == NULL ) {
            return iDefaultVal;
        }
        else {
            return atoi( p_szVal );
        }

    }

    /*
     @ Function:
     * get the decimal value of a given parameter
     *
     @ Arguments:
     * 1) parameter name (char*)
     * 2) parameter default value (float)
     *
     @ Return:
     * parameter value (float)
     */
    float configMng_c::ms_getFloat( const char* p_szKey, float fDefaultVal ) {

        const char* p_szVal = ms_getString( p_szKey, NULL );
        if( p_szVal == NULL ) {
            return fDefaultVal;
        }
        else {
            return ( float ) atof( p_szVal );
        }

    }

    /*
     @ Function:
     * get the decimal value of a given parameter
     *
     @ Arguments:
     * 1) parameter name (char*)
     * 2) parameter default value (double)
     *
     @ Return:
     * parameter value (double)
     */
    double configMng_c::ms_getDouble( const char* p_szKey, double dDefaultVal ) {

        const char* p_szVal = ms_getString( p_szKey, NULL );
        if( p_szVal == NULL ) {
            return dDefaultVal;
        }
        else {
            return atof( p_szVal );
        }

    }

}

