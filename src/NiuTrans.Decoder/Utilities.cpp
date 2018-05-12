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
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn): Dec 10, 2012 useless items removed
 * Hao Zhang (email: zhanghao1216@gmail.com); May 12th, 2011
 *
 */


#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "Utilities.h"

namespace utilities {

#define MTEVAL_NORMALIZATION

/**************************************************************************
 *
 * assignment (i.e., =) operator overloading for generic inner data types
 *
***************************************************************************/

__var_u& var_u::operator =( const __var_u &unData ) {

    this->ullInteger = unData.ullInteger;
    this->ldDecimal = unData.ldDecimal;
    this->mvdObject = unData.mvdObject;
    return *this;

}

/********************************************
 *
 * Hash Table's Member Functions
 *
 ********************************************/

/*
 @ Function:
 * compare two ASCII keys
 *
 @ Arguments:
 * 1) the first key (void*)
 * 2) the second key (void*)
 *
 @ Return:
 * 1) "-1" means the first key is smaller than the second key
 * 2) "0" means the first key is equal to the second key
 * 3) "1" means the first key is larger than the second key
 */
int HashTable::ASCKeyCompFunc( const void* vdKey1, const void* vdKey2 ) {

    return strcmp( ( const char* ) vdKey1, ( const char* ) vdKey2 );

}

/*
 @ Function:
 * get number of bytes occupied by an ASCII key
 *
 @ Arguments:
 * pointer to the key (void*)
 *
 @ Return:
 * number of bytes (integer_t)
 */
integer_t HashTable::ASCKeyByteNum( const void* vdStr ) {

    return (integer_t)( strlen( ( const char* ) vdStr ) + 1 );

}

/*
 @ Function:
 * constructor of class "HashTable"
 *
 @ Arguments:
 * 1) size of hash table (unsigned long long)
 * 2) element free function (FreeT)
 * 3) key comparison function (CompareT)
 * 4) key length calculation function (SBNT)
 */
HashTable::HashTable( unsigned long long ullMyHashTabSz, long double ldConflictThreshold, FreeT fnMyFreeFunc, \
    CompareT fnKeyCompFunc, SBNT fnKeyByteNumFunc ) {

    ldThreshold = ldConflictThreshold;

    Create( ullMyHashTabSz, fnMyFreeFunc, fnKeyCompFunc, fnKeyByteNumFunc );

}

/*
 @ Function:
 * create and initialize a new hash table
 *
 @ Arguments:
 * 1) size of hash table (unsigned long long)
 * 2) element free function (FreeT)
 * 3) key comparison function (CompareT)
 * 4) key length calculation function (SBNT)
 */
void HashTable::Create( unsigned long long ullMyHashTabSz, FreeT fnMyFreeFunc, \
    CompareT fnKeyCompFunc, SBNT fnKeyByteNumFunc ) {

    this->ullHashTabSz = ( ullMyHashTabSz == 0 ? 1024 : ullMyHashTabSz );
    this->mcsMempool = new MemPool( (ull_t)(ullHashTabSz * ldThreshold * sizeof( HashNode )) );
    this->mstHashTab = ( HashNode** ) mcsMempool->Alloc( ullHashTabSz * sizeof( HashNode* ) );
    this->ullHashNdCnt = 0;
    this->mvdKeyTab = ( void** ) mcsMempool->Alloc( ullHashTabSz * sizeof( void* ) );
    this->ullKeyCnt = 0;
    this->mfnFreeFunc = fnMyFreeFunc;
    this->mfnKeyCompFunc = ( fnKeyCompFunc == NULL ? HashTable::ASCKeyCompFunc : fnKeyCompFunc );
    this->mfnKeyByteNumFunc = ( fnKeyByteNumFunc == NULL ? HashTable::ASCKeyByteNum : fnKeyByteNumFunc );

}

/*
 @ Function:
 * deconstructor of class "HashTable"
 */
HashTable::~HashTable() {

    Destroy();

}

/*
 @ Function:
 * destroy the hash table
 */
void HashTable::Destroy() {

    for( unsigned long long i=0; i < ullHashTabSz; i++ ) {
        HashNode* stCurHashNd = mstHashTab[i];
        while( stCurHashNd != NULL ) {
            HashNode* stTmpHashNd = stCurHashNd;
            stCurHashNd = stCurHashNd->nextNode;
            if( stTmpHashNd->unValue->mvdObject != NULL && mfnFreeFunc != NULL ) {
                mfnFreeFunc( stTmpHashNd->unValue->mvdObject );
            }
        }
    }
    delete mcsMempool;

}

/*
 @ Function:
 * clear the hash table for reuse
 */
void HashTable::Clear() {

    Destroy();
    Create( ullHashTabSz, mfnFreeFunc, mfnKeyCompFunc, mfnKeyByteNumFunc );

}

/*
 @ Function:
 * code a given "string" to an "integer" data
 *
 @ Arguments:
 * pointer to the string ended with character '\0' (void*)
 *
 @ Return:
 * a hash code (unsigned long long)
 */
unsigned long long HashTable::ToHashCode( const void* vdKey ) {

    unsigned long long ullHashCode = 0, g = 0;
    const char* szStr = ( const char* ) vdKey;

    while( *szStr != '\0' ) {
        ullHashCode <<= 4;
        ullHashCode += ( unsigned long long ) *szStr;
        g = ullHashCode & ( ( unsigned long long ) 0xf << ( 64 - 4 ) );
        if( g != 0 ) {
            ullHashCode ^= g >> ( 64 - 8 );
            ullHashCode ^= g;
        }
        ++szStr;
    }
    
    return ullHashCode;

}

/*
 @ Function:
 * Resize the hash table
 *
 @ Arguments:
 * new size of hash table (unsigned long long)
 */
void HashTable::Resize( unsigned long long ullNewHashTabSz ) {

    if( ullNewHashTabSz <= ullHashTabSz ) {
        return;
    }

    /* backup info of old hash table */
    unsigned long long ullOldHashTabSz = ullHashTabSz;
    MemPool* csOldMempool = mcsMempool;
    HashNode** stOldHashTab = mstHashTab;

    /* update info of new hash table */
    ullHashTabSz = ullNewHashTabSz;
    mcsMempool = new MemPool( (ull_t)(ullHashTabSz * ldThreshold * sizeof( HashNode )) );
    mstHashTab = ( HashNode** ) mcsMempool->Alloc( ullHashTabSz * sizeof( HashNode* ) );
    mvdKeyTab = ( void** ) mcsMempool->Alloc( ullHashTabSz * sizeof( void* ) );
    ullHashNdCnt = 0;
    ullKeyCnt = 0;
    /* transfer hash nodes in old hash table to new hash table */
    for( unsigned long long i=0; i < ullOldHashTabSz; i++ ) {
        HashNode* stHashNd = stOldHashTab[i];
        while( stHashNd != NULL ) {
            HashNode* stHashNdLast = stHashNd;
            stHashNd = stHashNd->nextNode;
            AddHashNode( stHashNdLast->vdKey, stHashNdLast->unValue );
        }
    }

    /* eliminate old hash table */
    delete csOldMempool;

}

/*
 @ Function:
 * get a hash node specified by "key"
 *
 @ Arguments:
 * key (void*)
 *
 @ Return:
 * pointer to the result data of inner data type (var_u*)
 */
var_u* HashTable::GetHashNode( const void* vdKey ) {

    unsigned long long ullHashCode = ToHashCode( vdKey );
    HashNode* stHashNd = mstHashTab[ullHashCode % ullHashTabSz];

    while( stHashNd != NULL ) {
        int r = mfnKeyCompFunc( stHashNd->vdKey, vdKey );
        if( r < 0 ) {
            stHashNd = stHashNd->nextNode;
        }
        else if( r == 0 ) {
            return stHashNd->unValue;
        }
        else {
            break;
        }
    }
    return NULL;

}

/*
 @ Function:
 * create and initialize a new hash node
 *
 @ Arguments:
 * 1) key (void*)
 * 2) value (var_u*)
 *
 @ Return:
 * pointer to the new hash node
 */
HashNode* HashTable::CreateHashNode( const void* vdKey, var_u* unValue ) {

    /* new hash node */
    HashNode* stNewHashNd = ( HashNode* ) mcsMempool->Alloc( sizeof( HashNode ) );
    /* initialize "key" of new hash node */
    integer_t iByteNum = mfnKeyByteNumFunc( vdKey );
    stNewHashNd->vdKey = ( void* ) mcsMempool->Alloc( iByteNum * sizeof( char ) );
    memcpy( stNewHashNd->vdKey, vdKey, iByteNum );
    /* initialize "value" of new hash node */
    stNewHashNd->unValue = ( var_u* ) mcsMempool->Alloc( sizeof( var_u ) );
    *stNewHashNd->unValue = *unValue;
    /* initialize pointer to the next hash node */
    stNewHashNd->nextNode = NULL;

    return stNewHashNd;

}

/*
 @ Function:
 * add a hash node identified by "key" into the hash table
 * if the node of the specified "key" is already exists, update its "value"
 *
 @ Arguments:
 * 1) key (void*)
 * 2) value (var_u*)
 *
 @ Return:
 * pointer to the value (var_u*)
 */
var_u* HashTable::AddHashNode( const void* vdKey, var_u* unValue ) {

    /* if there are too many "key" conflicts */
    long double r = ( ( long double ) ullHashNdCnt / ( long double ) ullHashTabSz );
    if( r >= ldThreshold ) {
        Resize( ullHashTabSz * 2 + 1 );
    }

    /* search the hash node identified by "key" */
    unsigned long long ullHashCode = ToHashCode( vdKey );
    HashNode* stHashNd = mstHashTab[ullHashCode % ullHashTabSz];
    HashNode* stHashNdLast = stHashNd;

    while( stHashNd != NULL ) {
        int r = mfnKeyCompFunc( stHashNd->vdKey, vdKey );
        if( r < 0 ) {
            stHashNdLast = stHashNd;
            stHashNd = stHashNd->nextNode;
        }
        else if( r == 0 ) {
            /* already existed in hash table */
            if( stHashNd->unValue->mvdObject != NULL && mfnFreeFunc != NULL ) {
                mfnFreeFunc( stHashNd->unValue->mvdObject );
            }
            *stHashNd->unValue = *unValue;
            return stHashNd->unValue;
        }
        else {
            break;
        }
    }

    /* no such hash node */
    HashNode* stNewHashNd = CreateHashNode( vdKey, unValue );
    if( stHashNd == mstHashTab[ullHashCode % ullHashTabSz] ) {
        stNewHashNd->nextNode = mstHashTab[ullHashCode % ullHashTabSz];
        mstHashTab[ullHashCode % ullHashTabSz] = stNewHashNd;
    }
    else {
        stNewHashNd->nextNode = stHashNdLast->nextNode;
        stHashNdLast->nextNode = stNewHashNd;
    }
    ++ullHashNdCnt;
    mvdKeyTab[ullKeyCnt++] = stNewHashNd->vdKey;
    return stNewHashNd->unValue;

}

/*
 @ Function:
 * add a new ( key, value<Integer> ) pair into the hash table
 *
 @ Arguments:
 * 1) key (void*)
 * 2) value (unsigned long long)
 */
void HashTable::AddInt( const void* vdKey, integer_t ullValue ) {

    var_u unTmpVal = {0};
    unTmpVal.ullInteger = ullValue;
    AddHashNode( vdKey, &unTmpVal );

}

/*
 @ Function:
 * get an "Integer" value from the hash table
 *
 @ Arguments:
 * key (void*)
 *
 @ Return:
 * value (unsigned long long)
 */
integer_t HashTable::GetInt( const void* vdKey ) {

    var_u* unVal = GetHashNode( vdKey );
    if( unVal != NULL ) {
        return unVal->ullInteger;
    }
    else {
        return ( integer_t ) -1;
    }

}

/*
 @ Function:
 * add a new ( key, value<Decimal> ) pair into the hash table
 *
 @ Arguments:
 * 1) key (void*)
 * 2) value (long double)
 */
void HashTable::AddFloat( const void* vdKey, decimal_t ldValue ) {

    var_u unTmpVal = {0};
    unTmpVal.ldDecimal = ldValue;
    AddHashNode( vdKey, &unTmpVal );

}

/*
 @ Function:
 * get a "Decimal" value from the hash table
 *
 @ Arguments:
 * key (void*)
 *
 @ Return:
 * value (long double)
 */
decimal_t HashTable::GetFloat( const void* vdKey ) {

    var_u* unVal = GetHashNode( vdKey );
    if( unVal != NULL ) {
        return unVal->ldDecimal;
    }
    else {
        return -1.0;
    }

}

/*
 @ Function:
 * add a new ( key, value<user-defined complex objects> ) pair into the hash table
 *
 @ Arguments:
 * 1) key (void*)
 * 2) value (void*)
 */
void HashTable::AddObject( const void* vdKey, void* vdValue ) {

    var_u unTmpVal = {0};
    unTmpVal.mvdObject = vdValue;
    AddHashNode( vdKey, &unTmpVal );

}

/*
 @ Function:
 * get a complex "user-defined" object from the hash table
 *
 @ Arguments:
 * key (void*)
 *
 @ Return:
 * pointer to the value (void*)
 */
void* HashTable::GetObject( const void* vdKey ) {

    var_u* unVal = GetHashNode( vdKey );
    if( unVal != NULL ) {
        return unVal->mvdObject;
    }
    else {
        return NULL;
    }

}

/*
 @ Function:
 * get the number of different keys
 *
 @ Return:
 * different key number (unsigned long long)
 */
unsigned long long HashTable::GetKeyCnt() {

    return ullKeyCnt;

}

/*
 @ Function:
 * get a key by its index which indicates the timestamp
 * the key was added
 *
 @ Arguments:
 * index of the key (unsigned long long)
 *
 @ Return:
 * pointer to the key (void*)
 */
void* HashTable::GetKey( unsigned long long ullKeyIdx ) {

    if( ullKeyIdx < 0 || ullKeyIdx >= ullKeyCnt ) {
        return NULL;
    }
    return mvdKeyTab[ullKeyIdx];

}

////////////////////////////////////////
////////////////////////////////////////
// list

List::List()
{
    mem    = NULL;
    maxNum = 0;
    count  = 0;
    items  = NULL;
}

List::List(int myMaxNum)
{
    mem    = NULL;
    maxNum = myMaxNum;
    count  = 0;
    items  = new void*[myMaxNum];
}

List::List(int myMaxNum, MemPool * myMem)
{
    mem    = myMem;
    maxNum = myMaxNum;
    count  = 0;
    items  = (void**)mem->Alloc(sizeof(void*) * maxNum);
}

List::~List()
{
    if(mem == NULL)
        delete[] items;
}

void List::Create(int myMaxNum, MemPool * myMem)
{
    mem    = myMem;
    maxNum = myMaxNum;
    count  = 0;
    items  = (void**)mem->Alloc(sizeof(void*) * maxNum);
}

void List::Add(void * item)
{
    if( count == maxNum ){
        void ** newItems;
        if( mem == NULL )
            newItems = new void*[maxNum * 2 + 1];
        else
            newItems = (void**)mem->Alloc(sizeof(void*) * (maxNum * 2 + 1));
        memcpy(newItems, items, sizeof(void*) * maxNum);
        if( mem == NULL )
            delete[] items;
        items = newItems;
        maxNum = maxNum * 2 + 1;
    }
    
    items[count++] = item;

}

void List::Add(void ** inputItems, int inputItemCount)
{
    if( count + inputItemCount >= maxNum ){
        int newMaxNum = (count + inputItemCount) * 2 + 1;
        void ** newItems;
        if( mem == NULL )
            newItems = new void*[newMaxNum];
        else
            newItems = (void**)mem->Alloc(sizeof(void*) * newMaxNum);
        memcpy(newItems, items, sizeof(void*) * maxNum);
        if( mem == NULL )
            delete[] items;
        items = newItems;
        maxNum = newMaxNum;
    }
    memcpy(items + count, inputItems, sizeof(void*) * inputItemCount);
    count += inputItemCount;
}

void List::Insert(int pos, void * item)
{
    if( count == maxNum ){
        void ** newItems;
        if( mem == NULL )
            newItems = new void*[maxNum * 2 + 1];
        else
            newItems = (void**)mem->Alloc(sizeof(void*) * (maxNum * 2 + 1));
        memcpy(newItems, items, sizeof(void*) * maxNum);
        if( mem == NULL )
            delete[] items;
        items = newItems;
        maxNum = maxNum * 2 + 1;
    }

    for(int i = count - 1; i >= pos; i--)
        items[i + 1] = items[i];
    items[pos] = item;
    count++;
}

void * List::GetItem(int i)
{
    if( i >= 0 && i < count )
        return items[i];
    else
        return NULL;
}

void List::SetItem(int i, void * item)
{
	 if( i >= 0 && i < count )
        items[i] = item;
}

void List::Clear()
{
    count = 0;
}

void List::Sort(int itemSize, ListCompare comp)
{
    qsort(items, count, itemSize, comp);
}

void List::Reverse()
{
    int half = count/2;
    for(int i = 0; i < half; i++){
        void * tmp = items[i];
        items[i] = items[count - i - 1];
        items[count - i - 1] = tmp;
    }
}

List * List::Copy(MemPool * myMem)
{
	List * newList = new List(maxNum, myMem);
	for(int i = 0; i < count; i++){
		newList->Add(GetItem(i));
	}
	return newList;
}

}


namespace utilities {

/**************************************************************************
 *
 * member functions of class "TimeUtil"
 *
***************************************************************************/

TimeUtil::TimeUtil() {
}

TimeUtil::~TimeUtil() {
}

/*
 @ Function:
 * start counting time
 */
void TimeUtil::StartTimer() {

#if !defined( WIN32 ) && !defined( _WIN32 )
    gettimeofday( &stStartTime, NULL );
#else
    ulStartTime = clock();
#endif

}

/*
 @ Function:
 * end counting time
 */
void TimeUtil::EndTimer() {

#if !defined( WIN32 ) && !defined( _WIN32 )
    gettimeofday( &stEndTime, NULL );
#else
    ulEndTime = clock();
#endif

}

/*
 @ Function:
 * get the time difference between actions of end timer counting and start timer counting.
 *
 @ Return:
 * time difference in unit of "second" (double)
 */
double TimeUtil::GetTimerDiff() {

    double dTimeDiff;

#if !defined( WIN32 ) && !defined( _WIN32 )
    dTimeDiff = (double)( stEndTime.tv_sec - stStartTime.tv_sec ) + \
        (double)( stEndTime.tv_usec - stStartTime.tv_usec ) / 1000000;
#else
    dTimeDiff = difftime( ulEndTime, ulStartTime ) / CLOCKS_PER_SEC;
#endif
    
    return dTimeDiff;

}

/*
 @ Function:
 * set current system's time
 */
void TimeUtil::SetNowTime( time_t ulNowTime ) {

    this->ulNowTime = ulNowTime;

}

/*
 @ Function:
 * get current system's time in seconds
 *
 @ Return:
 * current time in digit format (time_t or long)
 */
time_t TimeUtil::GetNowTime() {

#if !defined( WIN32 ) && !defined( _WIN32 )
    timeval    stCurTime;
    gettimeofday( &stCurTime, NULL );
    ulNowTime = ( time_t ) stCurTime.tv_sec;
    if( stCurTime.tv_usec > 1000000 * 0.5 )
        ulNowTime += 1L;
#else
    time( &ulNowTime );
#endif
    return ulNowTime;

}

/*
 @ Function:
 * get current system's time
 *
 @ Return:
 * current time in string format (char*)
 */
char* TimeUtil::GetNowTimeString() {

    time_t ulNow = GetNowTime();
    char* szTime = ctime( &ulNow );
    int ullTimeStrLen = (int)strlen(szTime);
    if( szTime[ullTimeStrLen - 1] == '\n' ) {
        szTime[ullTimeStrLen - 1] = '\0';
    }
    return szTime;

}

/**************************************************************************
 *
 * "string processing" functions, which are of static scopes
 *
***************************************************************************/

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
void StringUtil::TrimRight( char* IsEnPuncszLine ) {

    long long llStrLen = strlen( IsEnPuncszLine );
    long long llIdx = llStrLen - 1;
    while( llIdx >= 0 ) {
        if( IsEnPuncszLine[llIdx] != '\r' && IsEnPuncszLine[llIdx] != '\n' ) {
            break;
        }
        IsEnPuncszLine[llIdx] = '\0';
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
char* StringUtil::ReadLine( FILE* IsEnPuncstFileHnd ) {

    ull_t ullLineLen = 128;
    char* IsEnPuncszLine = new char[ullLineLen];
    if( !fgets( IsEnPuncszLine, (int)ullLineLen, IsEnPuncstFileHnd ) ) {
        if( feof( IsEnPuncstFileHnd ) ) {
        }
        else if( ferror( IsEnPuncstFileHnd ) ) {
            fprintf( stderr, "[ERROR]: reading file failed!\n" );
        }
        delete[] IsEnPuncszLine;
        return NULL;
    }
    /* read the rest part of line into string buffer */
    while( IsEnPuncszLine[strlen( IsEnPuncszLine ) - 1] != '\n' ) {
        char* IsEnPuncszTmpPtr = NULL;
        ull_t ullTmpLen = ullLineLen + 1;
        char* IsEnPuncszOldLine = IsEnPuncszLine;
        ullLineLen *= 2;
        IsEnPuncszLine = new char[ullLineLen];
        strcpy( IsEnPuncszLine, IsEnPuncszOldLine );
        delete[] IsEnPuncszOldLine;
        IsEnPuncszTmpPtr = IsEnPuncszLine + strlen( IsEnPuncszLine );
        if( !fgets( IsEnPuncszTmpPtr, (int)ullTmpLen, IsEnPuncstFileHnd ) ) {
            if( feof( IsEnPuncstFileHnd ) ) {
                break;
            }
            else {
                fprintf( stderr, "[ERROR]: reading file failed!\n" );
                delete[] IsEnPuncszLine;
                return NULL;
            }
        }
    }
    /* eliminate carriage return and line feed characters at the end of line */
    TrimRight( IsEnPuncszLine );

    return IsEnPuncszLine;

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
 * 1) recognize and record all tokens between one or more delimiters
 * 2) construct array of tokens
 */
ull_t StringUtil::Split( const char* szSrcStr, const char* szDelim, char**& szTerms ) {

    ull_t ullSrcStrLen = strlen( szSrcStr );
    ull_t ullDelimLen = strlen( szDelim );
    /* empty source string or the source string is exactly the same with the delimiter string */
    if( ullSrcStrLen == 0 || !strcmp( szSrcStr, szDelim ) ) {
        szTerms = NULL;
        return 0;
    }
    /* an empty delimiter string or delimiter string is not shorter than the source string */
    if( ullDelimLen == 0 || ullDelimLen >= ullSrcStrLen  ) {
        szTerms = new char*[1];
        szTerms[0] = new char[ullSrcStrLen + 1];
        strcpy( szTerms[0], szSrcStr );
        return 1;
    }
    /* now for general case, that is, the delimiter string is shorter than the source string :) */
    szTerms = new char*[ullSrcStrLen + 1];
    ull_t ullTermCnt = 0;
    bool matched = false;
    /* scan the source string from left to right */
    for( int i=0; i < ullSrcStrLen; ) {
        /* match delimiter */
        if( !strncmp( &szSrcStr[i], szDelim, ullDelimLen ) ) {
            i += (int)ullDelimLen;
            if(!matched || i == ullSrcStrLen){
                szTerms[ullTermCnt] = new char[1];
                szTerms[ullTermCnt][0] = '\0';
                ++ullTermCnt;
            }
            matched = false;
        }
        /* match token */
        else {
            /* find end boundary of the matched token */
            ull_t j = i + 1;
            while( j < ullSrcStrLen && strncmp( &szSrcStr[j], szDelim, ullDelimLen ) ) {
                ++j;
            }
            /* record the matched token */
            ull_t ullTokenLen = j - i;
            szTerms[ullTermCnt] = new char[ullTokenLen + 1];
            strncpy( szTerms[ullTermCnt], &szSrcStr[i], ullTokenLen );
            szTerms[ullTermCnt][ullTokenLen] = '\0';
            ++ullTermCnt;
            i = (int)j;
            matched = true;
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
void StringUtil::FreeStringArray( char** szStrArr, ull_t ullStrArrSz ) {

    for( ull_t i=0; i < ullStrArrSz; i++ ) {
        delete[] szStrArr[i];
    }
    delete[] szStrArr;

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
char* StringUtil::ToLowercase( char* szStr ) {

    char* szChPtr = szStr;
    while( *szChPtr != '\0' ) {
        if( *szChPtr >= 'A' && *szChPtr <= 'Z' ) {
            *szChPtr += 'a' - 'A';
        }
        ++szChPtr;
    }
    return szStr;

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
bool StringUtil::IsEnPunc( const char* szToken ) {

    if( strlen( szToken ) == 1 && strstr( szEnPuncs, szToken ) )
        return true;
    return false;

}

HashTable * StringUtil::punctDict = NULL;

void StringUtil::LoadPunctDict(const char * dictFileName)
{
    int maxWordLen = 1024;
    int punctCount = 0;
    char * punct = new char[maxWordLen];

    punctDict = new HashTable(1000);

    FILE * f = fopen(dictFileName, "rb");
    if(f == NULL){
        fprintf(stderr, "ERROR: cannot open file \"%s\"!", dictFileName);
        return;
    }

    TimeUtil timer;
    timer.StartTimer();
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

        punctDict->AddInt(punct, ++punctCount);
        if( punctCount % 5 == 0 ) {
            fprintf( stderr, "#" );
        }
    }

    timer.EndTimer();
    double time = timer.GetTimerDiff();
    fprintf( stderr, "\nDone [%d entries, %.3f sec(s)]\n", punctCount, time );

    delete[] punct;
}

void StringUtil::UnloadPunctDict()
{
    delete punctDict;
}

bool StringUtil::IsPunc(const char * word)
{
    if( strlen( word ) == 1 && strstr( szEnPuncs, word ) )
        return true;
    if( punctDict != NULL ){
        int pid = punctDict->GetInt(word);
        if(pid != -1){
            //fprintf(stderr, "punct: %d %s\n", punctDict->GetInt(word), word);
            return true;
        }
    }
    return false;
}

char * StringUtil::GeneateString(char * src, MemPool * mem)
{
    char * s = (char *)mem->Alloc(sizeof(char) * ((int)strlen(src) + 1));
    strcpy(s, src);
    return s;
}

char * StringUtil::Copy(char * string)
{
    int length = (int)strlen(string);
    char * s = new char[length + 1];
    strcpy(s, string);
    return s;
}

char * StringUtil::Copy(char * string, MemPool * mem)
{
    int length = (int)strlen(string);
    char * s = (char*)mem->Alloc(length + 1);
    strcpy(s, string);
    return s;
}

void StringUtil::RemoveRightSpaces(char * line)
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

char * StringUtil::NormalizeText(char * line)
{
	int length = (int)strlen(line);
	char * newText = new char[length * 3 + 1];
	int i = 0, j = 0;

	newText[0] = '\0';

	if(length == 0)
		return newText;

	for(; i < length; i++)
		if( line[i] != ' ' && line[i] != '\t' )
			break;

	for(; i < length; i++){
		int op = 0;

		if( line[i] == '\n' || line[i] == '\r' )
			op = 1;
#ifdef MTEVAL_NORMALIZATION
		else if(line[i] != ' '){
			if( line[i] >= '{' && line[i] <= '~' ) // 0x7b-0x7e
				op = 2;
			else if( line[i] >= '[' && line[i] <= '`') // 0x5b-0x60
				op = 2;
			else if( line[i] >= ' ' && line[i] <= '&') // 0x20-0x26
				op = 2;
			else if( line[i] >= '(' && line[i] <= '+') // 0x28-0x2b
				op = 2;
			else if( line[i] >= ':' && line[i] <= '@') // 0x3a-0x40
				op = 2;
			else if( line[i] == '/') // 0x2f
				op = 2;
		}
        
		if(i > 0){
			if((line[i-1] < '0' || line[i-1] > '9') && (line[i] == '.' || line[i] == ',')) // ([^0-9])([\.,])"
				op = 3;
			else if( (line[i-1] >= '0' && line[i-1] <= '9') && line[i] == '-') // "([0-9])(-)"
				op = 3;
			else if( (line[i-1] == '.' || line[i-1] == ',') && (line[i] < '0' || line[i] > '9')) // "([\.,])([^0-9])"
				op = 4;
		}
#else
		else if(line[i] == '%')
			op = 1;
		else if(i > 0 && i < length - 2 && line[i-1] == ' ' && line[i] == '\'' &&
				line[i+1] == 's' && line[i+2] == ' ')
			op = 5;
		else if(i > 1 && i < length - 1 && line[i-2] == 's' && line[i-1] == ' ' &&
				line[i] == '\'' && line[i+1] == ' ')
			op = 5;
		else if(i > 2 && ((line[i-3] >= 'a' && line[i-3] <= 'z') || (line[i-3] >= 'A' && line[i-3] <= 'Z')) &&
				line[i-2] == ' ' && line[i-1] == '-' && line[i] == ' ')
			op = 6;
#endif

		if( op == 0 ){              // "$1" => "$1" where "$1" = line[i]
			newText[j++] = line[i];
		}
		else if( op == 1){          // "$1" => " $1"  where "$1" = line[i]
			newText[j++] = ' ';
			newText[j++] = line[i];
		}
		else if( op == 2 ){         // "$1" => " $1 "  where "$1" = line[i]
			newText[j++] = ' ';
			newText[j++] = line[i];
			newText[j++] = ' ';
		}
		else if(op == 3){           // "$1$2" => "$1 $2 "  where "$1" = line[i-1] and "$2" = line[i]
			newText[j++] = ' ';
			newText[j++] = line[i];
			newText[j++] = ' ';
		}
		else if(op == 4){           // "$1$2" => " $1 $2"  where "$1" = line[i-1] and "$2" = line[i]
			if( newText[j-1] == line[i-1] ){
				newText[j] = newText[j-1];
				newText[j-1] = ' ';
				j++;
				newText[j++] = ' ';
				newText[j++] = line[i];
			}
			else
				newText[j++] = line[i];
		}
		else if(op == 5){          // " $1" => "$1" where where "$1" = line[i] 
			newText[j-1] = line[i];
		}
		else if(op == 6){          // " $1 " => "$1" where where "$1" = line[i-1]
			newText[j-2] = newText[j-1];
			j--;
		}
	}

	// remove sequencial spaces
	int p = 1, q = 1;
	for(; q < j;  q++){
		if( newText[p-1] != ' ' || newText[q] != ' ' )
			newText[p++] = newText[q];
	}

	// remove head spaces
	q = 0;
	while( q < p && newText[q] == ' ' ) ++q;
	if( q == p ) {
		newText[0] = '\0';
		return newText;
	}
	else {
		strncpy( newText, &newText[q], p-q );
		newText[p-q] = '\0';
		p = p - q;
	}

	// remove trailing spaces
	while( p > 0 && newText[p-1] == ' ' ) --p;

	if( length > 1)
		newText[p] = '\0';

	return newText;
}

struct TokenUTF8{
    char * word;
    TokenUTF8 * prev;
    TokenUTF8 * next;
};

bool IsUTF8Space(const char * word)
{
    if(word[0] == ' ')
            return true;
    return false;
}

bool IsUTF8ASCII(const char * word)
{
    if( (word[0] & 0x80) == 0 )
            return true;
    return false;
}

TokenUTF8 * CreateUTF8Space()
{
    TokenUTF8 * newNode = new TokenUTF8;
    newNode->word = new char[2];
    strcpy(newNode->word, " ");
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

void InsertSpacePrev(TokenUTF8 * t)
{
    TokenUTF8 * last = t->prev;
    
    if(IsUTF8Space(last->word))
            return;

    TokenUTF8 * space =CreateUTF8Space();
    space->prev = last;
    space->next = t;
    last->next = space;
    t->prev = space;
}

void InsertSpaceNext(TokenUTF8 * t)
{
    TokenUTF8 * next = t->next;
    
    if(next != NULL && IsUTF8Space(next->word))
            return;

    TokenUTF8 * space =CreateUTF8Space();
    space->prev = t;
    space->next = t->next;
    t->next = space;
    if(next != NULL)
            next->prev = space;
}

const char * puncts = ",.:;!?\"'-/";

bool IsUTF8Punct(const char * word)
{
    // consider ascii only
    // TODO: more punctuations in Chinese
    if((word[0] & 0x80) != 0)
            return false;

    if(strchr(puncts, word[0]) == NULL)
            return false;

    return true;
}

bool IsUTF8Digit(const char * word)
{
    // consider ascii only
    // TODO: consider wide-character digits 
    if((word[0] & 0x80) != 0)
            return false;

    if(word[0] < '0' || word[0] > '9')
            return false;
    
    return true;
}

bool IsUTF8Symbol(const char * word)
{
    // consider ascii only
    if((word[0] & 0x80) != 0)
            return false;

    if(IsUTF8Punct(word))
            return false;
    if(IsUTF8Digit(word))
            return false;
    if(word[0] >= 'a' && word[0] <= 'z')
            return false;
    if(word[0] >= 'A' && word[0] <= 'Z')
            return false;
    return true;
}

bool IsUTF8DP(const char * word)
{
    if(IsUTF8Digit(word))
            return true;
    if(IsUTF8Punct(word))
            return true;
    return false;
}

void MergeTwoUTF8Tokens(TokenUTF8 * token1, TokenUTF8 * token2)
{
    int newLen = strlen(token1->word) + strlen(token2->word) + 1;
    char * newWord = new char[newLen];
    newWord[newLen - 1] = '\0';
    sprintf(newWord, "%s%s", token1->word, token2->word);

    delete[] token1->word;
    token1->word = newWord;
    token1->next = token2->next;
    if(token1->next != NULL)
        token1->next->prev = token1;

    delete[] token2->word;
    delete token2;
}

char * TokenizationInternationalUTF8(const char * input, bool preserveSpace)
{
    int len = strlen(input);
    TokenUTF8 * firstW = NULL;
    TokenUTF8 * lastW = NULL;

    // create a virtual head
    firstW = CreateUTF8Space();
    lastW = firstW;

    // read utf8 character sequence
    for(int i = 0; i < len; i++){
            int seqLen = 0;
            char c1 = input[i];
            if((c1 & 0x80) == 0) 
                    seqLen = 1;
            else if((c1 & 0xE0) == 0xC0)
                    seqLen = 2;
            else if((c1 & 0xF0) == 0xE0)
                    seqLen = 3;
            else if((c1 & 0xF8) == 0xF0)
                    seqLen = 4;
            else // malformed data
                    seqLen = -1;

            if(seqLen <= 0){
                    fprintf(stderr, "Illegal UTF8 char in %s\n", input);
                    continue;
            }

            // read a utf8 character
            char * word = new char[seqLen + 1];
            strncpy(word, input + i, seqLen);
            word[seqLen] = '\0';

            // remove all spaces
            if(!preserveSpace && IsUTF8Space(word)){
                    delete[] word;
                    continue;
            }

            TokenUTF8 * curToken = new TokenUTF8;
            curToken->word = word;
            curToken->next = NULL;
            curToken->prev = NULL;

            // append the list
            lastW->next = curToken;
            curToken->prev = lastW;
            lastW = curToken;

            i += seqLen - 1;
            //fprintf(stderr, "%d ||| %d ||| %s\n", i - seqLen + 1, (int)word[0], word);
    }

    // tokenization
    TokenUTF8 * t = firstW->next;
    while(t != NULL){
            if(IsUTF8Space(t->word)){
                    // do nothing
                    t = t->next;
                    continue;
            }

            // tokenize non-ascii
        // code in perl: $norm_text =~ s/([^[:ascii:]])/ $1 /g if ( $split_non_ASCII );
            if(!IsUTF8ASCII(t->word)){
                    InsertSpaceNext(t);
                    InsertSpacePrev(t);
            }

            // tokenize punctuations that are preceded by a non-digit 
        // code in perl: $norm_text =~ s/(\P{N})(\p{P})/$1 $2 /g;
            if(!IsUTF8Digit(t->prev->word) && IsUTF8Punct(t->word)){
                    InsertSpaceNext(t);
                    InsertSpacePrev(t);
            }

            // tokenize punctuations that are followed by a non-digit 
            // code in perl: $norm_text =~ s/(\p{P})(\P{N})/ $1 $2/g;
            if(t->next != NULL && !IsUTF8Digit(t->next->word) && IsUTF8Punct(t->word)){
                    InsertSpaceNext(t);
                    InsertSpacePrev(t);
            }
            
            // tokenize symbols
        // code in perl: $norm_text =~ s/(\p{S})/ $1 /g; # tokenize symbols
            if(IsUTF8Symbol(t->word)){
                    InsertSpaceNext(t);
                    InsertSpacePrev(t);
            }

            t = t->next;
    }

    // dump the result
    int newLen = 1;
    t = firstW;
    while(t != NULL){
            newLen += strlen(t->word);
            t = t->next;
    }

    char * result = new char[newLen];
    t = firstW;
    newLen = 0;
    while(t != NULL){
            if(t != firstW){
                    strcpy(result + newLen, t->word);
                    newLen += strlen(t->word);
            }
            lastW = t;
            t = t->next;
            delete[] lastW->word;
            delete lastW;
    }

    result[newLen] = '\0';

    for(int i = newLen - 1; i >= 0; i--){
            if(IsUTF8Space(result + i)){
                    result[i] = '\0';
            }
            else{
                    break;
            }
    }

    // post-processing
    char * p = strchr(result, '$');
    char * q = NULL;
    while(p != NULL){
        q = p + 1;
        if(strncmp(q, " number", 7) == 0 || strncmp(q, " date", 5) == 0 || strncmp(q, " time", 5) == 0){
            *p = ' ';
            *q = '$';
            p = strchr(q + 1, '$');
        }
        else
            p = strchr(q, '$');
    }

    return result;
}

char * StringUtil::InternationalTokenization(const char * line)
{
    //fprintf(stderr, "token: %s\n", line);
    // only for UTF8 string
    // TODO: support of other encodings
    return TokenizationInternationalUTF8(line, false);
}

/*
 @ Function:
 * extract part of string as the final string
 */
void StringUtil::ms_mvStr( char* p_szStr, long lMvOff, bool bToLeft ) {

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

bool StringUtil::IsLiteral(const char * word)
{
    int len = (int)strlen(word);

    for(int i = 0; i < len; i++){
        if(word[i] < 0 || word[i] > 127)
            return false;
    }
    return true;
}

/*
 *
 * Charset Encoding
 *
 */
char* Encoding::Convert( const char* szSrcEnc, const char* szTgtEnc, char* szInStr, size_t ullInStrLen ) {

#if !defined( WIN32 ) && !defined( _WIN32 )
    iconv_t cd = iconv_open( szTgtEnc, szSrcEnc );
    if( cd == (iconv_t)-1 ) {
        return NULL;
    }
    size_t inLen = ullInStrLen;
    size_t outLen = inLen * 3 + 1;
    char* szOutStr = new char[outLen];
    memset( szOutStr, 0, outLen );
    char* pin = szInStr;
    char* pout = szOutStr;
    if( iconv( cd, &pin, &inLen, &pout, &outLen ) == -1 ) {
        return NULL;
    }
    iconv_close( cd );
    return szOutStr;

#else
    if( !strcmp( szSrcEnc, "UTF-8" ) && !strcmp( szTgtEnc, "UCS-2" ) ) {
        return ( char* ) UTF8_to_UC( szInStr );
    }
    else if( !strcmp( szSrcEnc, "UCS-2" ) && !strcmp( szTgtEnc, "GB2312" ) ) {
        return UC_to_GB( szInStr );
    }
    else if( !strcmp( szSrcEnc, "UTF-8" ) && !strcmp( szTgtEnc, "GB2312" ) ) {
        return UTF8_to_GB( szInStr );
    }
    return szInStr;
#endif

}

wchar_t* Encoding::UTF8_to_UC( char* szStr_utf8 ) {

int i, j;
unsigned char ch;
unsigned char high, mid, low;
char* szStr_uc = new char[strlen( szStr_utf8 ) * 2 + 3];
i = 0; j = 0;
while( ch = szStr_utf8[i] )
{
    /* one byte */
    if( ch==0xEF && (unsigned char)szStr_utf8[i+1]==0xBB && (unsigned char)szStr_utf8[i+2]==0xBF )
        i += 3;
    ch = szStr_utf8[i];
    if( ch < 0x80 )
    {
        szStr_uc[j] = ch;
        szStr_uc[++j] = 0x00;
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
        low = szStr_utf8[i+1] & 0x3F;
        low = mid | low;
        szStr_uc[j] = low;
        szStr_uc[++j] = high;
        ++j;
        i += 2;
    }
    /* three bytes */
    else if( ( ch & 0xF0 ) == 0xE0 )
    {
        high = ch & 0x0F;
        high = high << 4;
        mid = szStr_utf8[i+1] & 0x3C;
        mid = mid >> 2;
        high = high | mid;
        mid = szStr_utf8[i+1] & 0x03;
        mid = mid << 6;
        low = szStr_utf8[i+2] & 0x3F;
        low = mid | low;
        szStr_uc[j] = low;
        szStr_uc[++j] = high;
        ++j;
        i += 3;
    }
}
szStr_uc[j] = 0x00;
szStr_uc[j+1] = 0x00;

return ( wchar_t* ) szStr_uc;

}

char* Encoding::UC_to_GB( char* szStr_uc ) {

long len = (long)wcslen( ( wchar_t* ) szStr_uc );
char* szStr_gb = new char[2 * len + 1];

if( !setlocale( LC_ALL, "chs" ) )
    return NULL;
len = (long)wcstombs( szStr_gb, ( wchar_t* ) szStr_uc, ( 2 * len + 1 ) );
return szStr_gb;

}

char* Encoding::UTF8_to_GB( char* szStr_utf8 ) {

wchar_t* szStr_uc = UTF8_to_UC( szStr_utf8 );
char* szStr_gb = UC_to_GB( ( char* ) szStr_uc );
delete[] szStr_uc;
return szStr_gb;

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
bool MathUtil::IsPrime( ull_t ullNumber ) {

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
ull_t MathUtil::NextPrime( ull_t ullNumber ) {

    ull_t ullCandidate = ullNumber;
    /* make "candidate number" definitely odd */
    ullCandidate |= 1;
    while( !IsPrime( ullCandidate ) ) {
        ullCandidate += 2;
    }
    if( ullCandidate < ullNumber )
        return ((ull_t)-1);
    return ullCandidate;


}

}
