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
 * Basic Data Structures; DataStruts.h
 * This header file defines the foundational data structures used in this project.
 * The data structures are designed for generic programming,
 * including "array", "list", "priority heap", "stack", "queue", "hash".
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); Nov. 27th, 2011; add a new class constructor and "m_copy()" function for class "array_c"
 * Hao Zhang (email: zhanghao1216@gmail.com); Nov. 23th, 2011; add "addItemUniq()" function for class "list_c"
 * Hao Zhang (email: zhanghao1216@gmail.com); Nov. 16th, 2011; add function "m_setFreeFunc" in class "list_c"
 * Hao Zhang (email: zhanghao1216@gmail.com); May 9th, 2011
 *
 */


#ifndef    __DATASTRUCTS_H__
#define    __DATASTRUCTS_H__

#include "OServ.h"
using namespace oserv;

namespace datastructs {

    /**************************************************************************
     *
     * generic inner data types for all following foundational data structures
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
    typedef    int (*compare_t)( const void* p_vdVar1, const void* p_vdVar2 );
    typedef    void (*free_t)( const void* p_vdVar );
    typedef integer_t (*sbn_t)( const void* p_vdStr );

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
        integer_t           m_ullInteger;
        decimal_t            m_ldDecimal;
        void*               mp_vdObject;

    }var_u;

    typedef int (*compvar_t)( const var_u& unVal1, const var_u& unVal2 );

    /**************************************************************************
     *
     * foundational data structures
     *
    ***************************************************************************/

    /*
     *
     * Array
     *
     * It simulates two types of arrays:
     * 1) array of fixed size
     * 2) array of dynamic growing size
     *
     * Note:
     * IF specified element free function, the class frees all elements contained when it destroys itself.
     * ELSE the user should manually free all memory occupied by the elements
     */
    class array_c {

    public:
        /* constructors and deconstructor for "new" operator */
        array_c( free_t p_fnMyFreeFunc = NULL );
        array_c( integer_t ullMyArraySize, free_t p_fnMyFreeFunc = NULL );
        array_c( integer_t ullMyArraySize, bool bSizeFixed, free_t p_fnMyFreeFunc = NULL );
        ~array_c();

    public:
        /* constructors and deconstructor for "calloc, malloc, memory pool" operations */
        void m_create( integer_t ullMyArraySize, bool bSizeFixed = false, free_t p_fnMyFreeFunc = NULL );
        void m_destroy();

    public:
        void m_clear();
        integer_t m_getSize();
        array_c* m_copy();

    public:
        /* interfaces for "Integer" data types */
        integer_t m_getInt( integer_t ullMyIndex );
        void m_setInt( integer_t ullMyIndex, integer_t ullMyValue );
        /* interfaces for "Decimal" data types */
        decimal_t m_getFloat( integer_t ullMyIndex );
        void m_setFloat( integer_t ullMyIndex, decimal_t ldMyValue );
        /* interfaces for complex "user-defined" complex data types */
        void* m_getObject( integer_t ullMyIndex );
        void m_setObject( integer_t ullMyIndex, void* p_vdMyValue );

    protected:
        var_u& operator[]( integer_t ullMyIndex );

    private:
        var_u*              mp_unItems;
        integer_t           m_ullArraySize;
        bool                m_bSizeFixed;

    private:
        mempool_c*          mp_csMempool;
        free_t              mp_fnFreeFunc;

    };

    /*
     *
     * List
     *
     * It simulates two types of list
     * 1) list of fixed size
     * 2) list of dynamic growing size
     *
     * Note:
     * IF specified element free function, the class frees all elements contained when it destroys itself.
     * ELSE the user should manually free all memory occupied by the elements
     */
    class list_c {

    public:
        /* constructors and deconstructor for "new" operator */
        list_c( free_t p_fnMyFreeFunc = NULL, compare_t p_fnMyObjItemCompFunc = NULL );
        list_c( integer_t ullMyListSize, free_t p_fnMyFreeFunc = NULL, compare_t p_fnMyObjItemCompFunc = NULL );
        ~list_c();

    public:
        /* constructors and deconstructor for "calloc, malloc, memory pool" operations */
        void m_create( integer_t ullMyListSize, bool bSizeFixed = false, \
            free_t p_fnMyFreeFunc = NULL, compare_t p_fnMyObjItemCompFunc = NULL );
        void m_destroy();

    public:
        void m_clear();
        integer_t m_getLength();
        void m_reverse();
        void m_setFreeFunc( free_t p_fnNewFreeFunc );

    public:
        /* interfaces for "Integer" data types */
        void m_addInt( integer_t ullMyValue );
        bool m_addIntUniq( integer_t ullMyValue );
        integer_t m_getInt( integer_t ullMyIndex );
        void m_sortInt( bool bIsSmall2Big = true );
        /* interfaces for "Decimal" data types */
        void m_addFloat( decimal_t ldMyValue );
        bool m_addFloatUniq( decimal_t ldMyValue );
        decimal_t m_getFloat( integer_t ullMyIndex );
        void m_sortFloat( bool bIsSmall2Big = true );
        /* interfaces for complex "user-defined" data types */
        void m_addObject( void* p_vdMyValue );
        bool m_addObjectUniq( void* p_vdMyValue );
        void* m_getObject( integer_t ullMyIndex );
        void m_sortObject( bool bIsSmall2Big = true );

    protected:
        var_u& operator[]( integer_t ullMyIndex );
        void m_addItem( const var_u& unMyValue );
        bool m_addItemUniq( const var_u& unMyValue, compvar_t p_fnCompFunc );
        void m_sort( compvar_t p_fnCompFunc );

    private:
        void m_swap( integer_t ullIdx1, integer_t ullIdx2 );
        integer_t m_partition( integer_t ullLeftIdx, integer_t ullRightIdx, compvar_t p_fnCompFunc );
        void m_qsort( long long ullLeftIdx, long long ullRightIdx, compvar_t p_fnCompFunc );

    private:
        static int ms_compInt(  const var_u& stVal1, const var_u& stVal2  );
        static int ms_compFloat(  const var_u& stVal1, const var_u& stVal2  );
        static int ms_compObject(  const var_u& stVal1, const var_u& stVal2  );

    private:
        var_u*              mp_unItems;
        integer_t           m_ullListSize;
        integer_t           m_ullItemCnt;
        bool                m_bSizeFixed;

    private:
        mempool_c*          mp_csMempool;
        free_t              mp_fnFreeFunc;
        compare_t           mp_fnObjItemCompFunc;

    private:
        static compare_t    msp_fnClassItemCompFunc;

    private:
        static mutex_c      ms_csMutex;

    };

    /*
     *
     * Priority Heap
     * 1) It has a fixed maximum size which is specified by users
     * 2) It supports both "min-heap" and "max-heap"
     *
     * Note:
     * 1) IF specified element free function, the class frees all elements contained when it destroys itself.
     * ELSE the user should manually free all memory occupied by the elements
     * 2) users MUST free the data (of "OBJECT" type) got from the priority heap
     */
    enum heaptype_en { MINHEAP, MAXHEAP };

    class priorheap_c {

    public:
        /* constructors and deconstructor for "new" operator */
        priorheap_c( free_t p_fnMyFreeFunc = NULL, compare_t p_fnMyObjIdCompFunc = NULL, compare_t p_fnMyObjDataCompFunc = NULL );
        priorheap_c( integer_t ullMyHeapSize, heaptype_en enMyHeapType, free_t p_fnMyFreeFunc = NULL, \
            compare_t p_fnMyObjIdCompFunc = NULL, compare_t p_fnMyObjDataCompFunc = NULL );
        ~priorheap_c();

    public:
        /* constructors and deconstructor for "calloc, malloc, memory pool" operations */
        void m_create( integer_t ullMyHeapSize, heaptype_en enMyHeapType, free_t p_fnMyFreeFunc = NULL, \
            compare_t p_fnMyObjIdCompFunc = NULL, compare_t p_fnMyObjDataCompFunc = NULL );
        void m_destroy();

    public:
        void m_clear();
        bool m_isEmpty();
        integer_t m_getSize();

    public:
        /* interfaces for "Integer" data types */
        bool m_pushInt( integer_t ullMyValue );
        integer_t m_popInt();
        /* interfaces for "Decimal" data types */
        bool m_pushFloat( decimal_t ldMyValue );
        decimal_t m_popFloat();
        /* interfaces for complex "user-defined" data types */
        bool m_pushObject( void* p_vdMyValue );
        void* m_popObject();
        bool m_updateObject( void* p_vdMyNewVal );

    protected:
        bool m_push( var_u& unMyValue, compvar_t p_fnCompFunc );
        bool m_pop( var_u* p_unRetData, compvar_t p_fnCompFunc );

    private:
        bool m_compare( var_u& unVal1, var_u& unVal2, compvar_t p_fnCompFunc );
        void m_swap( integer_t ullIndex1, integer_t ullIndex2 );
        void m_trickleDown( integer_t ullIndex, compvar_t p_fnCompFunc );
        void m_bubbleUp( integer_t ullIndex, compvar_t p_fnCompFunc );

    private:
        static int ms_compInt(  const var_u& stVal1, const var_u& stVal2  );
        static int ms_compFloat(  const var_u& stVal1, const var_u& stVal2  );
        static int ms_compObject(  const var_u& stVal1, const var_u& stVal2  );

    private:
        var_u*              mp_unItems;
        integer_t           m_ullHeapSize;
        integer_t           m_ullItemCnt;
        heaptype_en         m_enHeapType;

    private:
        mempool_c*          mp_csMempool;
        free_t              mp_fnFreeFunc;
        compare_t           mp_fnObjIdCompFunc;
        compare_t           mp_fnObjDataCompFunc;

    private:
        static compare_t    msp_fnClassDataCompFunc;

    private:
        static mutex_c      ms_csMutex;

    };

    /*
     *
     * Stack
     *
     * It simulates a stack of dynamic growing size
     *
     * Note:
     * 1) IF specified element free function, the class frees all elements contained when it destroys itself.
     * ELSE the user should manually free all memory occupied by the elements
     * 2) users MUST free the data (of "OBJECT" type) got from the stack
     *
     */
    enum datatype_en { INT, FLOAT, USR_DEF };

    class stack_c {

    public:
        /* constructors and deconstructor for "new" operator */
        stack_c( integer_t ullMyStackSize = 32, free_t p_fnMyFreeFunc = NULL, compare_t p_fnMyObjItemCompFunc = NULL );
        ~stack_c();

    public:
        /* constructors and deconstructor for "calloc, malloc, memory pool" operations */
        void m_create( integer_t ullMyStackSize = 32, free_t p_fnMyFreeFunc = NULL, compare_t p_fnMyObjItemCompFunc = NULL );
        void m_destroy();

    public:
        void m_clear();
        bool m_isEmpty();
        integer_t m_getSize();
        stack_c* m_duplicate();
        bool m_isSame( stack_c* p_csStack, datatype_en enItemType );

    public:
        /* interfaces for "Integer" data types */
        bool m_pushInt( integer_t ullMyValue );
        integer_t m_popInt();
        /* interfaces for "Decimal" data types */
        bool m_pushFloat( decimal_t ldMyValue );
        decimal_t m_popFloat();
        /* interfaces for complex "user-defined" data types */
        bool m_pushObject( void* p_vdMyValue );
        void* m_popObject();

    protected:
        bool m_push( var_u& unValue );
        bool m_pop( var_u* p_unValue );
        bool m_isSame( stack_c* p_csStack, compvar_t p_fnCompFunc );

    private:
        static int ms_compInt(  const var_u& stVal1, const var_u& stVal2  );
        static int ms_compFloat(  const var_u& stVal1, const var_u& stVal2  );
        static int ms_compObject(  const var_u& stVal1, const var_u& stVal2  );

    private:
        var_u*              mp_unItems;
        integer_t           m_ullStackSize;
        integer_t           m_ullItemCnt;

    private:
        mempool_c*          mp_csMempool;
        free_t              mp_fnFreeFunc;
        compare_t           mp_fnObjItemCompFunc;

    private:
        static compare_t    msp_fnClassItemCompFunc;

    private:
        static mutex_c      ms_csMutex;

    };

    /*
     *
     * Queue
     *
     * It simulates a queue of fixed size
     *
     * Note:
     * 1) IF specified element free function, the class frees all elements contained when it destroys itself.
     * ELSE the user should manually free all memory occupied by the elements
     * 2) users MUST free the data (of "OBJECT" type) got from the queue
     *
     */
    class queue_c {

    public:
        /* constructors and deconstructor for "new" operator */
        queue_c( integer_t ullMyQueueSz = 32, free_t p_fnMyFreeFunc = NULL );
        ~queue_c();

    public:
        /* constructors and deconstructor for "calloc, malloc, memory pool" operations */
        void m_create( integer_t ullMyQueueSz = 32, free_t p_fnMyFreeFunc = NULL );
        void m_destroy();

    public:
        void m_clear();
        bool m_isEmpty();
        bool m_isFull();
        integer_t m_getSize();

    public:
        /* interfaces for "Integer" data types */
        bool m_enQueInt( integer_t ullValue );
        integer_t m_deQueInt();
        /* interfaces for "Decimal" data types */
        bool m_enQueFloat( decimal_t ldValue );
        decimal_t m_deQueFloat();
        /* interfaces for complex "user-defined" data types */
        bool m_enQueObject( void* p_vdValue );
        void* m_deQueObject();

    protected:
        bool m_enqueue( var_u& unValue );
        bool m_dequeue( var_u* p_unValue );

    private:
        var_u*              mp_unItems;
        integer_t           m_ullQueueSz;
        integer_t           m_ullLeftIdx;
        integer_t           m_ullRightIdx;

    private:
        mempool_c*        mp_csMempool;
        free_t            mp_fnFreeFunc;

    };

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
    typedef struct __hashnode_s {

        void*           mp_vdKey;
        var_u*            mp_unValue;
        __hashnode_s*    mp_stNextNd;

    }hashnode_s;

    class hashtab_c {

    public:
        /* constructors and deconstructor for "new" operator */
        hashtab_c( unsigned long long ullMyHashTabSz = 1024, long double ms_ldConflictThreshold = 0.75, free_t p_fnMyFreeFunc = NULL, \
            compare_t p_fnKeyCompFunc = NULL, sbn_t p_fnKeyByteNumFunc = NULL );
        ~hashtab_c();

    public:
        /* constructors and deconstructor for "calloc, malloc, memory pool" operations */
        void m_create( unsigned long long ullMyHashTabSz = 1024, free_t p_fnMyFreeFunc = NULL, \
            compare_t p_fnKeyCompFunc = NULL, sbn_t p_fnKeyByteNumFunc = NULL );
        void m_destroy();

    public:
        void m_clear();
        /* interfaces for "Integer" data types */
        void m_addInt( const void* p_vdKey, integer_t ullValue );
        integer_t m_getInt( const void* p_vdKey );
        /* interfaces for "Decimal" data types */
        void m_addFloat( const void* p_vdKey, decimal_t ldValue );
        decimal_t m_getFloat( const void* p_vdKey );
        /* interfaces for "user-defined" data types */
        void m_addObject( const void* p_vdKey, void* p_vdValue );
        void* m_getObject( const void* p_vdKey );

    public:
        void* m_getKey( unsigned long long ullKeyIdx );
        unsigned long long m_getKeyCnt();

    protected:
        unsigned long long m_toHashCode( const void* p_vdKey );
        void m_resize( unsigned long long ullNewHashTabSz );
        var_u* m_getHashNode( const void* p_vdKey );
        hashnode_s* m_newHashNode( const void* p_vdKey, var_u* p_unValue );
        var_u* m_addHashNode( const void* p_vdKey, var_u* p_unValue );

    private:
        static int ms_ascKeyCompFunc( const void* p_vdKey1, const void* p_vdKey2 );
        static integer_t ms_ascKeyByteNum( const void* p_vdStr );

    private:
        /* table of hashnode(s) */
        hashnode_s**                mpp_stHashTab;
        unsigned long long          m_ullHashTabSz;
        unsigned long long          m_ullHashNdCnt;
        /* table of key(s) */
        void**                      mpp_vdKeyTab;
        unsigned long long          m_ullKeyCnt;
        /* threshold of average "key" conflicts */
        static long double          ms_ldThreshold;

    private:
        mempool_c*                  mp_csMempool;
        free_t                      mp_fnFreeFunc;
        compare_t                   mp_fnKeyCompFunc;
        sbn_t                       mp_fnKeyByteNumFunc;

    };
    
}


#endif

