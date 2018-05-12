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
 * Basic Data Structures; DataStruts.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); Nov. 23th, 2011; add "addItemUniq()" function for class "list_c"
 * Hao Zhang (email: zhanghao1216@gmail.com); Nov. 16th, 2011; add function "m_setFreeFunc" in class "list_c"
 * Hao Zhang (email: zhanghao1216@gmail.com); May 9th, 2011
 *
 */


#include "DataStruts.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"


namespace datastructs {

    /**************************************************************************
     *
     * assignment (i.e., =) operator overloading for generic inner data types
     *
    ***************************************************************************/

    __var_u& var_u::operator =( const __var_u &unData ) {

        this->m_ullInteger = unData.m_ullInteger;
        this->m_ldDecimal = unData.m_ldDecimal;
        this->mp_vdObject = unData.mp_vdObject;
        return *this;

    }

    /**************************************************************************
     *
     * implementations of class array_c's member functions
     *
    ***************************************************************************/

    /********************************************
     *
     * Array's Member Functions
     *
     ********************************************/

    /*
     @ Function:
     * default constructor of class "array_c"
     * the array's size is "expandable".
     *
     @ Arguments:
     * user-defined memory free function to free complex item, including "struct", "union", "char*" (free_t)
     */
    array_c::array_c( free_t p_fnMyFreeFunc ) {

        m_create( 32, false, p_fnMyFreeFunc );

    }

    /*
     @ Function:
     * constructor of class "array_c"
     * the array's size is "fixed".
     *
     @ Arguments:
     * 1) array size (unsigned long long)
     * 2) free function for "user-defined" element, including "struct", "union", "char*" and "class" (free_t)
     */
    array_c::array_c( integer_t ullMyArraySize, free_t p_fnMyFreeFunc ) {

        m_create( ullMyArraySize, true, p_fnMyFreeFunc );

    }
    
    /*
     @ Function:
     * constructor of class "array_c"
     *
     @ Arguments:
     * 1) array size (unsigned long long)
     * 2) flag to decide array's type, whether size fixed or not (bool)
     * 3) free function for "user-defined" element, including "struct", "union", "char*" and "class" (free_t)
     */
    array_c::array_c( integer_t ullMyArraySize, bool bSizeFixed, free_t p_fnMyFreeFunc ) {
    
        m_create( ullMyArraySize, bSizeFixed, p_fnMyFreeFunc );
    
    }

    /*
     @ Function:
     * deconstructor of class "array_c"
     */
    array_c::~array_c() {

        m_destroy();

    }

    /*
     @ Function:
     * create and initialize a new array
     *
     @ Arguments:
     * 1) array size (unsigned long long)
     * 2) flag to decide array's type, whether size fixed or not (bool)
     * 3) free function for "user-defined" element, including "struct", "union", "char*" and "class" (free_t)
     */
    void array_c::m_create( integer_t ullMyArraySize, bool bSizeFixed, free_t p_fnMyFreeFunc ) {

        this->m_ullArraySize = ( ullMyArraySize == 0 ? 32 : ullMyArraySize );
        this->mp_csMempool = new mempool_c( m_ullArraySize * sizeof( var_u ) );
        this->mp_unItems = ( var_u* ) mp_csMempool->m_alloc( m_ullArraySize * sizeof( var_u ) );
        this->m_bSizeFixed = bSizeFixed;
        this->mp_fnFreeFunc = p_fnMyFreeFunc;

    }

    /*
     @ Function:
     * destroy an array
     */
    void array_c::m_destroy() {

        for( integer_t i=0; i < m_ullArraySize; i++ ) {
            if( mp_unItems[i].mp_vdObject != NULL ) {
                /* free the complex user-defined data structures, including "struct", "union", "char*" and "class" */
                if( mp_fnFreeFunc != NULL ) {
                    mp_fnFreeFunc( mp_unItems[i].mp_vdObject );
                }
            }
        }
        delete mp_csMempool;

    }

    /*
     @ Function:
     * clear an array for reuse
     */
    void array_c::m_clear() {

        m_destroy();
        m_create( m_ullArraySize, m_bSizeFixed, mp_fnFreeFunc );

    }

    /*
     @ Function:
     * get the number of element in the array
     */
    integer_t array_c::m_getSize() {

        return m_ullArraySize;

    }
    
    /*
     @ Function:
     * make a copy of the array itself
     */
    array_c* array_c::m_copy() {
    
        array_c* na = new array_c( m_ullArraySize, m_bSizeFixed, mp_fnFreeFunc );
        memcpy( na->mp_unItems, mp_unItems, m_ullArraySize * sizeof( var_u ) );
        return na;
    
    }

    /*
     @ Function:
     * get an "Integer" data from the array
     *
     @ Arguments:
     * index of the data in the array (unsigned long long)
     *
     @ Return:
     * an "Integer" data (unsigned long long)
     */
    integer_t array_c::m_getInt( integer_t ullMyIndex ) {

        return (*this)[ullMyIndex].m_ullInteger;

    }

    /*
     @ Function:
     * assign an "Integer" data to an element of the array
     *
     @ Arguments:
     * 1) index of the data in the array (unsigned long long)
     * 2) value to be assigned to the element in the array (unsigned long long)
     */
    void array_c::m_setInt( integer_t ullMyIndex, integer_t ullMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.m_ullInteger = ullMyValue;
        (*this)[ullMyIndex] = unTmpVar;

    }

    /*
     @ Function:
     * get a "Decimal" data from the array
     *
     @ Arguments:
     * index of the data in the array (unsigned long long)
     *
     @ Return:
     * a "Decimal" data (long double)
     */
    decimal_t array_c::m_getFloat( integer_t ullMyIndex ) {

        return (*this)[ullMyIndex].m_ldDecimal;

    }

    /*
     @ Function:
     * assign a "Decimal" data to an element of the array
     *
     @ Arguments:
     * 1) index of the data in the array (unsigned long long)
     * 2) value to be assigned to the element in the array (long double)
     */
    void array_c::m_setFloat( integer_t ullMyIndex, decimal_t ldMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.m_ldDecimal = ldMyValue;
        (*this)[ullMyIndex] = unTmpVar;

    }

    /*
     @ Function:
     * get a complex "user-defined" data from the array
     *
     @ Arguments:
     * index of the data in the array (unsigned long long)
     *
     @ Return:
     * a pointer to a complex "user-defined" data (void*)
     */
    void* array_c::m_getObject( integer_t ullMyIndex ) {

        return (*this)[ullMyIndex].mp_vdObject;

    }

    /*
     @ Function:
     * assign a complex "user-defined" data pointer to an element of the array
     *
     @ Arguments:
     * 1) index of the data in the array (unsigned long long)
     * 2) value to be assigned to the element in the array (void*)
     */
    void array_c::m_setObject( integer_t ullMyIndex, void* p_vdMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.mp_vdObject = p_vdMyValue;
        (*this)[ullMyIndex] = unTmpVar;

    }

    /*
     @ Function:
     * "[]" operator overloading function
     */
    var_u& array_c::operator []( integer_t ullMyIndex ) {

        if( this->m_bSizeFixed == true ) {
            if( ullMyIndex < 0 || ullMyIndex >= m_ullArraySize ) {
                fprintf( stderr, "[ERROR]: index out of range!\n" );
                return *( ( var_u* ) NULL );
            }
            return mp_unItems[ullMyIndex];
        }
        else {
            if( ullMyIndex >= m_ullArraySize ) {
                var_u* p_unNewItems = ( var_u* ) mp_csMempool->m_alloc( ( ullMyIndex * 2 ) * sizeof( var_u ) );
                memcpy( p_unNewItems, mp_unItems, m_ullArraySize * sizeof( var_u ) );
                mp_unItems = p_unNewItems;
                m_ullArraySize = ullMyIndex * 2;
            }
            return mp_unItems[ullMyIndex];
        }

    }

    /********************************************
     *
     * List's Member Functions
     *
     ********************************************/

    compare_t list_c::msp_fnClassItemCompFunc = NULL;
    mutex_c list_c::ms_csMutex;

     /*
     @ Function:
     * "Integer" data comparison function
     * the function defines how to compare two "Integer" data
     *
     @ Arguments:
     * 1) reference to the first data (var_u&)
     * 2) reference to the second data (var_u&)
     *
     @ Return:
     * 1) "< 0" means that the first data is smaller than the second one
     * 2) "= 0" means that the two data are equal
     * 3) "> 0" means that the first data is larger than the second one
     */
    int list_c::ms_compInt(  const var_u& stVal1, const var_u& stVal2  ) {

        long long llValDiff = stVal1.m_ullInteger - stVal2.m_ullInteger;
        return (int)llValDiff;

    }

    /*
     @ Function:
     * "Decimal" data comparison function
     * the function defines how to compare two "Decimal" data
     *
     @ Arguments:
     * 1) reference to the first data (var_u&)
     * 2) reference to the second data (var_u&)
     *
     @ Return:
     * 1) "< 0" means that the first data is smaller than the second one
     * 2) "= 0" means that the two data are equal
     * 3) "> 0" means that the first data is larger than the second one
     */
    int list_c::ms_compFloat( const var_u& stVal1, const var_u& stVal2 ) {

        long double ldValDiff = stVal1.m_ldDecimal - stVal2.m_ldDecimal;
        return (int)ldValDiff;

    }

    /*
     @ Function:
     * complex "user-defined" data comparison function
     * the function defines how to compare two complex "user-defined" data
     *
     @ Arguments:
     * 1) reference to the first data (var_u&)
     * 2) reference to the second data (var_u&)
     *
     @ Return:
     * 1) "< 0" means that the first data is smaller than the second one
     * 2) "= 0" means that the two data are equal
     * 3) "> 0" means that the first data is larger than the second one
     */
    int list_c::ms_compObject( const var_u& stVal1, const var_u& stVal2 ) {

        if( list_c::msp_fnClassItemCompFunc != NULL )
            return list_c::msp_fnClassItemCompFunc( stVal1.mp_vdObject, stVal2.mp_vdObject );
        else
            return 0;

    }

    /*
     @ Function:
     * default constructor of class "list_c"
     * the size of list is non-fixed
     *
     @ Arguments:
     * 1) user-defined memory free function to free complex item, including "struct", "union", "char*" and "class" (free_t)
     * 2) item comparison function pointer (compare_t)
     */
    list_c::list_c( free_t p_fnMyFreeFunc, compare_t p_fnMyObjItemCompFunc ) {

        m_create( 32, false, p_fnMyFreeFunc, p_fnMyObjItemCompFunc );

    }

    /*
     @ Function:
     * constructor of class "list_c"
     * the size of list is fixed
     *
     @ Arguments:
     * 1) list size (unsigned long long)
     * 2) free function for "user-defined" element, including "struct", "union", "char*" and "class" (free_t)
     * 3) item comparison function pointer (compare_t)
     */
    list_c::list_c( integer_t ullMyListSize, free_t p_fnMyFreeFunc, compare_t p_fnMyObjItemCompFunc ) {

        m_create( ullMyListSize, true, p_fnMyFreeFunc, p_fnMyObjItemCompFunc );

    }

    /*
     @ Function:
     * deconstructor of class "list_c"
     */
    list_c::~list_c() {

        m_destroy();

    }

    /*
     @ Function:
     * create and initialize a new list
     *
     @ Arguments:
     * 1) list size (unsigned long long)
     * 2) flag to decide list's type, whether size fixed or not (bool)
     * 3) free function for "user-defined" element, including "struct", "union", "char*" and "class" (free_t)
     * 4) item comparison function pointer (compare_t)
     */
    void list_c::m_create( integer_t ullMyListSize, bool bSizeFixed, free_t p_fnMyFreeFunc, compare_t p_fnMyObjItemCompFunc ) {

        this->m_ullListSize = ( ullMyListSize == 0 ? 32 : ullMyListSize );
        this->mp_csMempool = new mempool_c( m_ullListSize * sizeof( var_u ) );
        this->mp_unItems = ( var_u* ) mp_csMempool->m_alloc( m_ullListSize * sizeof( var_u ) );
        this->m_ullItemCnt = 0;
        this->m_bSizeFixed = bSizeFixed;
        this->mp_fnFreeFunc = p_fnMyFreeFunc;
        this->mp_fnObjItemCompFunc = p_fnMyObjItemCompFunc;

    }

    /*
     @ Function:
     * destroy a list
     */
    void list_c::m_destroy() {

        for( integer_t i=0; i < m_ullItemCnt; i++ ) {
            if( mp_unItems[i].mp_vdObject != NULL ) {
                if( mp_fnFreeFunc != NULL ) {
                    mp_fnFreeFunc( mp_unItems[i].mp_vdObject );
                }
            }
        }
        delete mp_csMempool;

    }

    /*
     @ Function:
     * clear a list for reuse
     */
    void list_c::m_clear() {

        m_destroy();
        m_create( m_ullListSize, m_bSizeFixed, mp_fnFreeFunc, mp_fnObjItemCompFunc );

    }

    /*
     @ Function:
     * get number of elements in the list
     *
     @ Arguments:
     * NULL
     *
     @ Return:
     * element number (unsigned long long)
     */
    integer_t list_c::m_getLength() {

        return m_ullItemCnt;

    }

    /*
     @ Function:
     * reverse elements in the list
     */
    void list_c::m_reverse() {

        if( m_ullItemCnt == 0 )
            return;
        integer_t ullLeftIdx = 0;
        integer_t ullRightIdx = m_ullItemCnt - 1;
        while( ullLeftIdx < ullRightIdx ) {
            m_swap( ullLeftIdx, ullRightIdx );
            ++ullLeftIdx;
            --ullRightIdx;
        }

    }

    /*
     @ Function:
     * reset the element free function
     */
    void list_c::m_setFreeFunc( free_t p_fnNewFreeFunc ) {

        mp_fnFreeFunc = p_fnNewFreeFunc;

    }

    /*
     @ Function:
     * add an "Integer" data into list
     *
     @ Arguments:
     * "Integer" data to be added (unsigned long long)
     */
    void list_c::m_addInt( integer_t ullMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.m_ullInteger = ullMyValue;
        m_addItem( unTmpVar );

    }

    /*
     @ Function:
     * uniquely add an "Integer" data into list
     *
     @ Arguments:
     * "Integer" data to be added (unsigned long long)
     *
     @ Return:
     * whether element insertion is successful or not
     */
    bool list_c::m_addIntUniq( integer_t ullMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.m_ullInteger = ullMyValue;
        return m_addItemUniq( unTmpVar, list_c::ms_compInt );

    }

    /*
     @ Function:
     * get an "Integer" data from list
     *
     @ Arguments:
     * index of the data in the list (unsigned long long)
     *
     @ Return:
     * an "Integer" data (unsigned long long)
     */
    integer_t list_c::m_getInt( integer_t ullMyIndex ) {

        return (*this)[ullMyIndex].m_ullInteger;

    }

    /*
     @ Function:
     * sort list whose elements are of "Integer" data type
     */
    void list_c::m_sortInt( bool bIsSmall2Big ) {

        m_sort( list_c::ms_compInt );
        if( bIsSmall2Big == false )
            m_reverse();

    }

    /*
     @ Function:
     * add a "Decimal" data into list
     *
     @ Arguments:
     * "Decimal" data to be added (long double)
     */
    void list_c::m_addFloat( decimal_t ldMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.m_ldDecimal = ldMyValue;
        m_addItem( unTmpVar );

    }

    /*
     @ Function:
     * uniquely add a "Decimal" data into list
     *
     @ Arguments:
     * "Decimal" data to be added (long double)
     *
     @ Return:
     * whether element insertion is successful or not
     */
    bool list_c::m_addFloatUniq( decimal_t ldMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.m_ldDecimal = ldMyValue;
        return m_addItemUniq( unTmpVar, list_c::ms_compFloat );

    }

    /*
     @ Function:
     * get a "Decimal" data from list
     *
     @ Arguments:
     * index of the data in the list (unsigned long long)
     *
     @ Return:
     * a "Decimal" data (long double)
     */
    decimal_t list_c::m_getFloat( integer_t ullMyIndex ) {

        return (*this)[ullMyIndex].m_ldDecimal;

    }

    /*
     @ Function:
     * sort list whose elements are of "Decimal" data type
     */
    void list_c::m_sortFloat( bool bIsSmall2Big ) {

        m_sort( list_c::ms_compFloat );
        if( bIsSmall2Big == false )
            m_reverse();

    }

    /*
     @ Function:
     * add a complex "user-defined" data into list
     *
     @ Arguments:
     * the data to be added (void*)
     */
    void list_c::m_addObject( void* p_vdMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.mp_vdObject = p_vdMyValue;
        m_addItem( unTmpVar );

    }

    /*
     @ Function:
     * uniquely add a complex "user-defined" data into list
     *
     @ Arguments:
     * the data to be added (void*)
     *
     @ Return:
     * whether element insertion is successful or not
     */
    bool list_c::m_addObjectUniq( void* p_vdMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.mp_vdObject = p_vdMyValue;
        list_c::msp_fnClassItemCompFunc = mp_fnObjItemCompFunc;
        return m_addItemUniq( unTmpVar, list_c::ms_compObject );

    }

    /*
     @ Function:
     * get a complex "user-defined" data from list
     *
     @ Arguments:
     * index of the data in the list (unsigned long long)
     *
     @ Return:
     * a pointer to the complex "user-defined" data (void*)
     */
    void* list_c::m_getObject( integer_t ullMyIndex ) {

        return (*this)[ullMyIndex].mp_vdObject;

    }

    /*
     @ Function:
     * sort list whose elements are of complex "user-defined" data type
     */
    void list_c::m_sortObject( bool bIsSmall2Big ) {

        list_c::ms_csMutex.m_lock();
        list_c::msp_fnClassItemCompFunc = mp_fnObjItemCompFunc;
        m_sort( list_c::ms_compObject );
        list_c::ms_csMutex.m_unlock();
        if( bIsSmall2Big == false )
            m_reverse();

    }

    /*
     @ Function:
     * operator "[]" overloading function
     */
    var_u& list_c::operator[]( integer_t ullMyIndex ) {

        if( ullMyIndex < 0 || ullMyIndex >= m_ullItemCnt ) {
            fprintf( stderr, "[ERROR]: index out of range!\n" );
            return *( (var_u*)NULL );
        }
        return mp_unItems[ullMyIndex];

    }

    /*
     @ Function:
     * add an element into the list
     *
     @ Arguments:
     * element value (var_u)
     */
    void list_c::m_addItem( const var_u &unMyValue ) {

        if( m_ullItemCnt == m_ullListSize ) {
            if( m_bSizeFixed == true ) {
                fprintf( stderr, "[Warning]: list is full!\n" );
                return;
            }
            else {
                var_u* unNewItems = ( var_u* ) mp_csMempool->m_alloc( m_ullListSize * 2 * sizeof( var_u ) );
                memcpy( unNewItems, mp_unItems, m_ullListSize * sizeof( var_u ) );
                mp_unItems = unNewItems;
                m_ullListSize *= 2;
            }
        }
        (*this)[m_ullItemCnt++] = unMyValue;

    }

    /*
     @ Function:
     * uniquely add an element into the list
     *
     @ Arguments:
     * 1) element value (var_u)
     * 2) element comparison function's address
     *
     @ Return:
     * operation judgement. "true" if the element is added, "false" otherwise.
     */
    bool list_c::m_addItemUniq( const var_u& unMyValue, compvar_t p_fnCompFunc ) {

        /* check whether the new element is existed in the list */
        for( integer_t i=0; i < m_ullItemCnt; i++ ) {
            if( p_fnCompFunc( mp_unItems[i], unMyValue ) == 0 ) {
                /* the element has been already existed in the list */
                return false;
            }
        }

        /* add the element when it is not existed in the list */
        if( m_ullItemCnt == m_ullListSize ) {
            if( m_bSizeFixed == true ) {
                fprintf( stderr, "[Warning]: list is full!\n" );
                return false;
            }
            else {
                var_u* unNewItems = ( var_u* ) mp_csMempool->m_alloc( m_ullListSize * 2 * sizeof( var_u ) );
                memcpy( unNewItems, mp_unItems, m_ullListSize * sizeof( var_u ) );
                mp_unItems = unNewItems;
                m_ullListSize *= 2;
            }
        }
        (*this)[m_ullItemCnt++] = unMyValue;
        return true;

    }

    /*
     @ Function:
     * sort elements in the list
     *
     @ Arguments:
     * data comparison function's address
     * it defines how to compare two elements in the list (compvar_t)
     *
     @ Basic Algorithm:
     * invoke a sort function implemented by using quicksort algorithm
     */
    void list_c::m_sort( compvar_t p_fnCompFunc ) {

        m_qsort( 0, m_ullItemCnt - 1, p_fnCompFunc );

    }

    /*
     @ Function:
     * swap two elements in the list
     *
     @ Arguments:
     * 1) the first element's index
     * 2) the second element's index
     */
    void list_c::m_swap( integer_t ullIdx1, integer_t ullIdx2 ) {

        var_u unTmpVar = mp_unItems[ullIdx1];
        mp_unItems[ullIdx1] = mp_unItems[ullIdx2];
        mp_unItems[ullIdx2] = unTmpVar;

    }

    /*
     @ Function:
     * partition for quicksort algorithm
     *
     @ Arguments:
     * 1) index of the most left element (unsigned long long)
     * 2) index of the most right element (unsigned long long)
     * 3) elements comparing function (compvar_t)
     *
     @ Return:
     * boundary index of the two partitions (unsigned long long)
     */
    integer_t list_c::m_partition( integer_t ullLeftIdx,  integer_t ullRightIdx, compvar_t p_fnCompFunc ) {

        var_u& unPivotVal = mp_unItems[ullRightIdx];
        integer_t ullStoreIdx = ullLeftIdx;
        for( integer_t i=ullLeftIdx; i < ullRightIdx; i++ ) {
            /* item[i] <= pivot value */
            if( p_fnCompFunc( mp_unItems[i], unPivotVal ) <= 0 ) {
                m_swap( i, ullStoreIdx );
                ++ullStoreIdx;
            }
        }
        m_swap( ullStoreIdx, ullRightIdx );
        return ullStoreIdx;

    }

    /*
     @ Function:
     * sort elements in the list
     *
     @ Arguments:
     * 1) index of the most left element (long long)
     * 2) index of the most right element (long long)
     * 3) elements comparing function (compvar_t)
     *
     @ Basic Algorithm:
     * quicksort algorithm
     */
    void list_c::m_qsort( long long ullLeftIdx, long long ullRightIdx, compvar_t p_fnCompFunc ) {

        if( ullLeftIdx < ullRightIdx ) {
            /* partition */
            integer_t ullMidIdx = m_partition( (integer_t)ullLeftIdx, (integer_t)ullRightIdx, p_fnCompFunc );
            /* recursion */
            m_qsort( ullLeftIdx, ullMidIdx - 1, p_fnCompFunc );
            m_qsort( ullMidIdx + 1, ullRightIdx, p_fnCompFunc );
        }

    }

    /********************************************
     *
     * Priority Heap's Member Functions
     *
     ********************************************/

    compare_t priorheap_c::msp_fnClassDataCompFunc = NULL;
    mutex_c priorheap_c::ms_csMutex;

    /*
     @ Function:
     * "Integer" data comparison function
     * the function defines how to compare two "Integer" data
     *
     @ Arguments:
     * 1) reference to the first data (var_u&)
     * 2) reference to the second data (var_u&)
     *
     @ Return:
     * 1) "< 0" means that the first data is smaller than the second one
     * 2) "= 0" means that the two data are equal
     * 3) "> 0" means that the first data is larger than the second one
     */
    int priorheap_c::ms_compInt(  const var_u& stVal1, const var_u& stVal2  ) {

        long long llValDiff = stVal1.m_ullInteger - stVal2.m_ullInteger;
        return (int)llValDiff; // ??? Why not return a "long long" type

    }

    /*
     @ Function:
     * "Decimal" data comparison function
     * the function defines how to compare two "Decimal" data
     *
     @ Arguments:
     * 1) reference to the first data (var_u&)
     * 2) reference to the second data (var_u&)
     *
     @ Return:
     * 1) "< 0" means that the first data is smaller than the second one
     * 2) "= 0" means that the two data are equal
     * 3) "> 0" means that the first data is larger than the second one
     */
    int priorheap_c::ms_compFloat( const var_u& stVal1, const var_u& stVal2 ) {

        long double ldValDiff = stVal1.m_ldDecimal - stVal2.m_ldDecimal;
        return (int)ldValDiff; // ??? Why not return a "long long" type
    }

    /*
     @ Function:
     * complex "user-defined" data comparison function
     * the function defines how to compare two complex "user-defined" data
     *
     @ Arguments:
     * 1) reference to the first data (var_u&)
     * 2) reference to the second data (var_u&)
     *
     @ Return:
     * 1) "< 0" means that the first data is smaller than the second one
     * 2) "= 0" means that the two data are equal
     * 3) "> 0" means that the first data is larger than the second one
     */
    int priorheap_c::ms_compObject( const var_u& stVal1, const var_u& stVal2 ) {

        if( priorheap_c::msp_fnClassDataCompFunc != NULL )
            return priorheap_c::msp_fnClassDataCompFunc( stVal1.mp_vdObject, stVal2.mp_vdObject );
        else
            return 0;

    }

    /*
     @ Function:
     * default constructor of class "priorheap_c" object
     */
    priorheap_c::priorheap_c( free_t p_fnMyFreeFunc, compare_t p_fnMyObjIdCompFunc, compare_t p_fnMyObjDataCompFunc ) {

        m_create( 32, MINHEAP, p_fnMyFreeFunc, p_fnMyObjIdCompFunc, p_fnMyObjDataCompFunc );

    }

    /*
     @ Function:
     * constructor of class "priorheap_c" object
     *
     @ Arguments:
     * 1) size of priority heap (unsigned long long)
     * 2) the priority heap's type (heaptype_en)
     * 3) element free function defined by the user (free_t)
     * 4) comparison function of complex object's "ID" field (compare_t)
     * 5) comparison function of complex object's "DATA" field (compare_t)
     */
    priorheap_c::priorheap_c( integer_t ullMyHeapSize, heaptype_en enMyHeapType, free_t p_fnMyFreeFunc, \
        compare_t p_fnMyObjIdCompFunc, compare_t p_fnMyObjDataCompFunc ) {

        m_create( ullMyHeapSize, enMyHeapType, p_fnMyFreeFunc, p_fnMyObjIdCompFunc, p_fnMyObjDataCompFunc );

    }

    /*
     @ Function:
     * create and initialize a new priority heap
     *
     @ Arguments:
     * 1) size of priority heap (unsigned long long)
     * 2) the priority heap's type (heaptype_en)
     * 3) element free function defined by the user (free_t)
     * 4) comparison function of complex object's "ID" field (compare_t)
     * 5) comparison function of complex object's "DATA" field (compare_t)
     */
    void priorheap_c::m_create( integer_t ullMyHeapSize, heaptype_en enMyHeapType, free_t p_fnMyFreeFunc, \
        compare_t p_fnMyObjIdCompFunc, compare_t p_fnMyObjDataCompFunc ) {

        this->m_ullHeapSize = ( ullMyHeapSize == 0 ? 32 : ullMyHeapSize );
        this->mp_csMempool = new mempool_c( m_ullHeapSize * sizeof( var_u ) );
        this->mp_unItems = ( var_u* ) mp_csMempool->m_alloc( m_ullHeapSize * sizeof( var_u ) );
        this->m_ullItemCnt = 0;
        this->m_enHeapType = enMyHeapType;
        this->mp_fnFreeFunc = p_fnMyFreeFunc;
        this->mp_fnObjIdCompFunc = p_fnMyObjIdCompFunc;
        this->mp_fnObjDataCompFunc = p_fnMyObjDataCompFunc;

    }

    /*
     @ Function:
     * deconstructor of the class "priorheap_c"
     */
    priorheap_c::~priorheap_c() {

        m_destroy();

    }

    /*
     @ Function:
     * destroy the priority heap
     */
    void priorheap_c::m_destroy() {

        for( integer_t i=0; i < m_ullItemCnt; i++ ) {
            if( mp_unItems[i].mp_vdObject != NULL ) {
                if( mp_fnFreeFunc != NULL ) {
                    mp_fnFreeFunc( mp_unItems[i].mp_vdObject );
                }
            }
        }
        delete mp_csMempool;

    }

    /*
     @ Function:
     * clear the priority heap for reuse
     */
    void priorheap_c::m_clear() {

        m_destroy();
        m_create( m_ullHeapSize, m_enHeapType, mp_fnFreeFunc, mp_fnObjIdCompFunc, mp_fnObjDataCompFunc );

    }

    /*
     @ Function:
     * judge whether the priority heap is empty or not
     *
     @ Return:
     * "true" if the priority heap is empty, else "false" (bool)
     */
    bool priorheap_c::m_isEmpty() {

        return m_ullItemCnt == 0;

    }

    /*
     @ Function:
     * get the element number in the priority heap
     */
    integer_t priorheap_c::m_getSize() {

        return m_ullItemCnt;

    }

    /*
     @ Function:
     * push an element into the priority heap
     *
     @ Arguments:
     * 1) value of the element (var_u&)
     * 2) element comparison function (compvar_t)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool priorheap_c::m_push( var_u& unMyValue, compvar_t p_fnCompFunc ) {

        if( m_ullItemCnt < m_ullHeapSize ) {
            mp_unItems[m_ullItemCnt] = unMyValue;
            ++m_ullItemCnt;
            m_bubbleUp( m_ullItemCnt - 1, p_fnCompFunc );
            return true;
        }
        return false;

    }

    /*
     @ Function:
     * remove the top element in the priority heap
     *
     @ Arguments:
     * 1) address of returned value (var_u*)
     * 2) element comparison function (compvar_t)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool priorheap_c::m_pop( var_u* p_unRetData, compvar_t p_fnCompFunc ) {

        if( m_isEmpty() ) {
            return false;
        }
        *p_unRetData = mp_unItems[0];
        --m_ullItemCnt;
        mp_unItems[0] = mp_unItems[m_ullItemCnt];
        m_trickleDown( 0, p_fnCompFunc );
        return true;

    }

    /*
     @ Function:
     * compare two elements in the priority heap
     *
     @ Arguments:
     * 1) value of the first element (var_u&)
     * 2) value of the second element (var_u&)
     * 3) element comparison function (compvar_t)
     *
     @ Return:
     * "true" if the first element is "smaller" than the second one, else "false"
     */
    bool priorheap_c::m_compare( var_u& unVal1, var_u& unVal2, compvar_t p_fnCompFunc ) {

        /* min-heap */
        if( m_enHeapType == MINHEAP ) {
            if( p_fnCompFunc( unVal1, unVal2 ) < 0 ) {
                return true;
            }
            else {
                return false;
            }
        }
        /* max-heap */
        else {
            if( p_fnCompFunc( unVal1, unVal2 ) > 0 ) {
                return true;
            }
            else {
                return false;
            }
        }

    }

    /*
     @ Function:
     * swap two elements in the priority heap
     *
     @ Arguments:
     * 1) index of the first element (unsigned long long)
     * 2) index of the second element (unsigned long long)
     */
    void priorheap_c::m_swap( integer_t ullIndex1, integer_t ullIndex2 ) {

        if( ullIndex1 < m_ullItemCnt && ullIndex2 < m_ullItemCnt ) {
            var_u unTmpVal = mp_unItems[ullIndex1];
            mp_unItems[ullIndex1] = mp_unItems[ullIndex2];
            mp_unItems[ullIndex2] = unTmpVal;
        }

    }

    /*
     @ Function:
     * reformat the priority heap from a parent element
     *
     @ Arguments:
     * 1) index of the parent element (unsigned long long)
     * 2) element comparison function (compvar_t)
     */
    void priorheap_c::m_trickleDown( integer_t ullIndex, compvar_t p_fnCompFunc ) {

        integer_t ullParentIdx = ullIndex;
        integer_t ullLeftChildIdx = ullParentIdx * 2 + 1;
        integer_t ullRightChildIdx = ullLeftChildIdx + 1;
        integer_t ullSwapIndex;

        while( ullLeftChildIdx < m_ullItemCnt ) {
            /* find the child element to be swapped */
            ullSwapIndex = ( ( ullRightChildIdx >= m_ullItemCnt || \
                m_compare( mp_unItems[ullLeftChildIdx], mp_unItems[ullRightChildIdx], p_fnCompFunc ) ) ? \
                ullLeftChildIdx : ullRightChildIdx );
            /* loop-out condition */
            if( m_compare( mp_unItems[ullParentIdx], mp_unItems[ullSwapIndex], p_fnCompFunc ) ) {
                break;
            }
            /* swap the parent and the child elements, and update their indices */
            m_swap( ullParentIdx, ullSwapIndex );
            ullParentIdx = ullSwapIndex;
            ullLeftChildIdx = ullParentIdx * 2 + 1;
            ullRightChildIdx = ullLeftChildIdx + 1;
        }

    }

    /*
     @ Function:
     * reformat the priority heap from a child element
     *
     @ Arguments:
     * 1) index of the child element (unsigned long long)
     * 2) element comparison function (compvar_t)
     */
    void priorheap_c::m_bubbleUp( integer_t ullIndex, compvar_t p_fnCompFunc ) {

        long long llChildIdx = ullIndex;
        long long llParentIdx = ( llChildIdx - 1 ) / 2;

        while( llChildIdx > 0 && !m_compare( mp_unItems[llParentIdx], mp_unItems[llChildIdx], p_fnCompFunc ) ) {
            m_swap( (integer_t)llParentIdx, (integer_t)llChildIdx );
            llChildIdx = llParentIdx;
            llParentIdx = ( llChildIdx - 1 ) / 2;
        }

    }

    /*
     @ Function:
     * push an "Integer" data into the priority heap
     *
     @ Arguments:
     * value of the "Integer" data (unsigned long long)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool priorheap_c::m_pushInt( integer_t ullMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.m_ullInteger = ullMyValue;
        return m_push( unTmpVar, priorheap_c::ms_compInt );

    }

    /*
     @ Function:
     * pop the top element of "Integer" data type from the priority heap
     *
     @ Return:
     * data value (unsigned long long)
     */
    integer_t priorheap_c::m_popInt() {

        var_u unTmpVar = {0};
        m_pop( &unTmpVar, priorheap_c::ms_compInt );
        return unTmpVar.m_ullInteger;

    }

    /*
     @ Function:
     * push a "Decimal" data into the priority heap
     *
     @ Arguments:
     * value of the "Decimal" data (long double)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool priorheap_c::m_pushFloat( decimal_t ldMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.m_ldDecimal = ldMyValue;
        return m_push( unTmpVar, priorheap_c::ms_compFloat );

    }

    /*
     @ Function:
     * pop the top element of "Decimal" data type from the priority heap
     *
     @ Return:
     * data value (long double)
     */
    decimal_t priorheap_c::m_popFloat() {

        var_u unTmpVar = {0};
        m_pop( &unTmpVar, priorheap_c::ms_compFloat );
        return unTmpVar.m_ldDecimal;

    }

    /*
     @ Function:
     * push a complex "user-defined" data into the priority heap
     *
     @ Arguments:
     * pointer to the complex "user-defined" data (void*)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool priorheap_c::m_pushObject( void* p_vdMyValue ) {

        var_u unTmpVar = {0};
        unTmpVar.mp_vdObject = p_vdMyValue;
        priorheap_c::ms_csMutex.m_lock();
        priorheap_c::msp_fnClassDataCompFunc = mp_fnObjDataCompFunc;
        bool bFlag = m_push( unTmpVar, priorheap_c::ms_compObject );
        priorheap_c::ms_csMutex.m_unlock();
        return bFlag;

    }

    /*
     @ Function:
     * pop the top element of complex "user-defined" data type from the priority heap
     *
     @ Return:
     * data value (void*)
     */
    void* priorheap_c::m_popObject() {

        var_u unTmpVar = {0};
        priorheap_c::ms_csMutex.m_lock();
        priorheap_c::msp_fnClassDataCompFunc = mp_fnObjDataCompFunc;
        bool bFlag = m_pop( &unTmpVar, priorheap_c::ms_compObject );
        priorheap_c::ms_csMutex.m_unlock();
        if( bFlag == true ) {
            return unTmpVar.mp_vdObject;
        }
        else {
            return NULL;
        }

    }

    /*
     @ Function:
     * update an existed element in the priority heap,
     * or add a new element to the priority heap.
     *
     @ Arguments:
     * pointer to the complex "user-defined" data (void*)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool priorheap_c::m_updateObject( void* p_vdMyNewVal ) {

        var_u unTmpVar = {0};
        unTmpVar.mp_vdObject = p_vdMyNewVal;

        /* first check whether the data is already existed in the priority heap */
        for( integer_t i=0; i < m_ullItemCnt; i++ ) {
            if( mp_fnObjIdCompFunc == NULL || mp_fnObjIdCompFunc( mp_unItems[i].mp_vdObject, p_vdMyNewVal ) != 0 ) {
                continue;
            }
            /* exist the same "ID" in the priority heap */
            else {
                priorheap_c::ms_csMutex.m_lock();
                priorheap_c::msp_fnClassDataCompFunc = mp_fnObjDataCompFunc;
                if( m_compare( unTmpVar, mp_unItems[i], priorheap_c::ms_compObject ) == true ) {
                    if( mp_fnFreeFunc )
                        mp_fnFreeFunc( mp_unItems[i].mp_vdObject );
                    mp_unItems[i] = unTmpVar;
                    m_bubbleUp( i, priorheap_c::ms_compObject );
                }
                else {
                    if( mp_fnFreeFunc )
                        mp_fnFreeFunc( p_vdMyNewVal );
                }
                priorheap_c::ms_csMutex.m_unlock();
                return true;
            }
        }
        /* it's a new "ID" to be added */
        priorheap_c::ms_csMutex.m_lock();
        priorheap_c::msp_fnClassDataCompFunc = mp_fnObjDataCompFunc;
        /* the priority heap is not full */
        if( m_ullItemCnt < m_ullHeapSize ) {
            m_push( unTmpVar, priorheap_c::ms_compObject );
            priorheap_c::ms_csMutex.m_unlock();
            return true;
        }
        /* the priority heap is full */
        else {
            integer_t ullItemIdx = m_ullItemCnt - 1;
            if( m_ullItemCnt % 2 == 1 && m_ullItemCnt > 1 ) {
                if( m_compare( mp_unItems[m_ullItemCnt-1], mp_unItems[m_ullItemCnt-2], priorheap_c::ms_compObject ) == true ) {
                    ullItemIdx = m_ullItemCnt - 2;
                }
            }
            if( m_compare( unTmpVar, mp_unItems[ullItemIdx], priorheap_c::ms_compObject ) == true ) {
                if( mp_fnFreeFunc )
                    mp_fnFreeFunc( mp_unItems[ullItemIdx].mp_vdObject );
                mp_unItems[ullItemIdx] = unTmpVar;
                m_bubbleUp( ullItemIdx, priorheap_c::ms_compObject );
                priorheap_c::ms_csMutex.m_unlock();
                return true;
            }
            else {
                if( mp_fnFreeFunc )
                    mp_fnFreeFunc( p_vdMyNewVal );
                priorheap_c::ms_csMutex.m_unlock();
                return true;
            }
        }

    }

    /********************************************
     *
     * Stack's Member Functions
     *
     ********************************************/

    compare_t stack_c::msp_fnClassItemCompFunc = NULL;
    mutex_c stack_c::ms_csMutex;

    /*
     @ Function:
     * "Integer" data comparison function
     * the function defines how to compare two "Integer" data
     *
     @ Arguments:
     * 1) reference to the first data (var_u&)
     * 2) reference to the second data (var_u&)
     *
     @ Return:
     * 1) "< 0" means that the first data is smaller than the second one
     * 2) "= 0" means that the two data are equal
     * 3) "> 0" means that the first data is larger than the second one
     */
    int stack_c::ms_compInt(  const var_u& stVal1, const var_u& stVal2  ) {

        long long llValDiff = stVal1.m_ullInteger - stVal2.m_ullInteger;
        return (int)llValDiff; // ??? Why not return a "long long" type

    }

    /*
     @ Function:
     * "Decimal" data comparison function
     * the function defines how to compare two "Decimal" data
     *
     @ Arguments:
     * 1) reference to the first data (var_u&)
     * 2) reference to the second data (var_u&)
     *
     @ Return:
     * 1) "< 0" means that the first data is smaller than the second one
     * 2) "= 0" means that the two data are equal
     * 3) "> 0" means that the first data is larger than the second one
     */
    int stack_c::ms_compFloat( const var_u& stVal1, const var_u& stVal2 ) {

        long double ldValDiff = stVal1.m_ldDecimal - stVal2.m_ldDecimal;
        return (int)ldValDiff; // ??? Why not return a "long long" type

    }

    /*
     @ Function:
     * complex "user-defined" data comparison function
     * the function defines how to compare two complex "user-defined" data
     *
     @ Arguments:
     * 1) reference to the first data (var_u&)
     * 2) reference to the second data (var_u&)
     *
     @ Return:
     * 1) "< 0" means that the first data is smaller than the second one
     * 2) "= 0" means that the two data are equal
     * 3) "> 0" means that the first data is larger than the second one
     */
    int stack_c::ms_compObject( const var_u& stVal1, const var_u& stVal2 ) {

        if( stack_c::msp_fnClassItemCompFunc != NULL )
            return stack_c::msp_fnClassItemCompFunc( stVal1.mp_vdObject, stVal2.mp_vdObject );
        else
            return 0;

    }

    /*
     @ Function:
     * constructor of class "stack_c"
     *
     @ Arguments:
     * 1) stack size (unsigned long long)
     * 2) element free function (free_t)
     * 3) element comparison function (compare_t)
     */
    stack_c::stack_c( integer_t ullMyStackSize, free_t p_fnMyFreeFunc, compare_t p_fnMyObjItemCompFunc ) {

        m_create( ullMyStackSize, p_fnMyFreeFunc, p_fnMyObjItemCompFunc );

    }

    /*
     @ Function:
     * create and initialize a new stack
     *
     @ Arguments:
     * 1) stack size (unsigned long long)
     * 2) element free function (free_t)
     * 3) element comparison function (compare_t)
     */
    void stack_c::m_create( integer_t ullMyStackSize, free_t p_fnMyFreeFunc, compare_t p_fnMyObjItemCompFunc ) {

        this->m_ullStackSize = ( ullMyStackSize == 0 ? 32 : ullMyStackSize );
        this->mp_csMempool = new mempool_c( m_ullStackSize * sizeof( var_u ) );
        this->mp_unItems = ( var_u* ) mp_csMempool->m_alloc( m_ullStackSize * sizeof( var_u ) );
        this->m_ullItemCnt = 0;
        this->mp_fnFreeFunc = p_fnMyFreeFunc;
        this->mp_fnObjItemCompFunc = p_fnMyObjItemCompFunc;

    }

    /*
     @ Function:
     * deconstructor of class "stack_c"
     */
    stack_c::~stack_c() {

        m_destroy();

    }

    /*
     @ Function:
     * destroy the stack itself
     */
    void stack_c::m_destroy() {

        for( integer_t i=0; i < m_ullItemCnt; i++ ) {
            if( mp_unItems[i].mp_vdObject != NULL ) {
                if( mp_fnFreeFunc != NULL ) {
                    mp_fnFreeFunc( mp_unItems[i].mp_vdObject );
                }
            }
        }
        delete mp_csMempool;

    }

    /*
     @ Function:
     * clear the stack for reuse
     */
    void stack_c::m_clear() {

        m_destroy();
        m_create( m_ullStackSize, mp_fnFreeFunc, mp_fnObjItemCompFunc );

    }

    /*
     @ Function:
     * judge whether the stack is empty
     *
     @ Return
     * "true" if the stack is empty, else "false" (bool)
     */
    bool stack_c::m_isEmpty() {

        return m_ullItemCnt == 0;

    }

    /*
     @ Function:
     * get the stack size
     *
     @ Return:
     * element number (unsigned long long)
     */
    integer_t stack_c::m_getSize() {

        return m_ullItemCnt;

    }

    /*
     @ Function:
     * duplicate the stack itself
     *
     @ Return:
     * pointer to a shallow copy of the stack (stack_c*)
     */
    stack_c* stack_c::m_duplicate() {

        stack_c* p_csStack = new stack_c( m_ullStackSize, mp_fnFreeFunc, mp_fnObjItemCompFunc );
        for( integer_t i=0; i < m_ullItemCnt; i++ ) {
            /* it can break the "protected" mode to invoke function "m_push()", why??? A bug of C++? */
            p_csStack->m_push( mp_unItems[i] );
        }
        return p_csStack;

    }

    /*
     @ Function:
     * judge whether two stacks are exactly same or not
     *
     @ Arguments:
     * 1) pointer to a stack (stack_c*)
     * 2) type of elements in the stacks (datatype_en)
     *
     @ Return:
     * "true" if the two stacks are same, else "false" (bool)
     */
    bool stack_c::m_isSame( stack_c* p_csStack, datatype_en enItemType ) {

        if( enItemType == INT ) {
            return m_isSame( p_csStack, stack_c::ms_compInt );
        }
        else if( enItemType == FLOAT ) {
            return m_isSame( p_csStack, stack_c::ms_compFloat );
        }
        else if( enItemType == USR_DEF ) {
            stack_c::ms_csMutex.m_lock();
            stack_c::msp_fnClassItemCompFunc = mp_fnObjItemCompFunc;
            bool bFlag = m_isSame( p_csStack, stack_c::ms_compObject );
            stack_c::ms_csMutex.m_unlock();
            return bFlag;
        }

    }

    /*
     @ Function:
     * push an element into the stack
     *
     @ Arguments:
     * value of the new to be added element (var_u)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool stack_c::m_push( var_u& unValue ) {

        if( m_ullItemCnt == m_ullStackSize ) {
            var_u* p_unNewItems = ( var_u* ) mp_csMempool->m_alloc( m_ullStackSize * 2 * sizeof( var_u ) );
            memcpy( p_unNewItems, mp_unItems, m_ullStackSize * sizeof( var_u ) );
            mp_unItems = p_unNewItems;
            m_ullStackSize *= 2;
        }
        mp_unItems[m_ullItemCnt++] = unValue;
        return true;

    }

    /*
     @ Function:
     * pop an element from the stack
     *
     @ Arguments:
     * address of the result data returned to the user (var_u*)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool stack_c::m_pop( var_u* p_unValue ) {

        if( !m_isEmpty() ) {
            *p_unValue = mp_unItems[--m_ullItemCnt];
            return true;
        }
        return false;

    }

    /*
     @ Function:
     * judge whether two stacks are exactly same or not
     *
     @ Arguments:
     * pointer to the stack to be compared (stack_s*)
     *
     @ Return:
     * "true" if same, "false" if not (bool)
     */
    bool stack_c::m_isSame( stack_c* p_csStack, compvar_t p_fnCompFunc ) {

        if( m_ullItemCnt != p_csStack->m_getSize() ) {
            return false;
        }
        else {
            for( integer_t i=0; i < m_ullItemCnt; i++ ) {
                if( p_fnCompFunc( mp_unItems[i], p_csStack->mp_unItems[i] ) != 0 ) {
                    return false;
                }
            }
            return true;
        }

    }

    /*
     @ Function:
     * push an "Integer" data into the stack
     *
     @ Arguments:
     * data's value (unsigned long long)
     */
    bool stack_c::m_pushInt( integer_t ullMyValue ) {

        var_u unTmpVal = {0};
        unTmpVal.m_ullInteger = ullMyValue;
        return m_push( unTmpVal );

    }

    /*
     @ Function:
     * pop an "Integer" data from the stack
     *
     @ Return:
     * the data's value (unsigned long long)
     */
    integer_t stack_c::m_popInt() {

        var_u unTmpVal = {0};
        m_pop( &unTmpVal );
        return unTmpVal.m_ullInteger;

    }

    /*
     @ Function:
     * push a "Decimal" data into the stack
     *
     @ Arguments:
     * data's value (long double)
     */
    bool stack_c::m_pushFloat( decimal_t ldMyValue ) {

        var_u unTmpVal = {0};
        unTmpVal.m_ldDecimal = ldMyValue;
        return m_push( unTmpVal );

    }

    /*
     @ Function:
     * pop a "Decimal" data from the stack
     *
     @ Return:
     * the data's value (long double)
     */
    decimal_t stack_c::m_popFloat() {

        var_u unTmpVal = {0};
        m_pop( &unTmpVal );
        return unTmpVal.m_ldDecimal;

    }

    /*
     @ Function:
     * push a complex "user-defined" data into the stack
     *
     @ Arguments:
     * pointer to the data (void*)
     */
    bool stack_c::m_pushObject( void* p_vdMyValue ) {

        var_u unTmpVal = {0};
        unTmpVal.mp_vdObject = p_vdMyValue;
        return m_push( unTmpVal );

    }

    /*
     @ Function:
     * pop a complex "user-defined" data from the stack
     *
     @ Return:
     * pointer to the data (void*)
     */
    void* stack_c::m_popObject() {

        var_u unTmpVal = {0};
        bool bFlag = m_pop( &unTmpVal );
        if( bFlag == true ) {
            return unTmpVal.mp_vdObject;
        }
        else {
            return NULL;
        }

    }

    /********************************************
     *
     * Queue's Member Functions
     *
     ********************************************/

    /*
     @ Function:
     * constructor of class "queue_c"
     *
     @ Arguments:
     * 1) queue size (unsigned long long)
     * 2) element free function (free_t)
     */
    queue_c::queue_c( integer_t ullMyQueueSz, free_t p_fnMyFreeFunc ) {

        m_create( ullMyQueueSz, p_fnMyFreeFunc );

    }

    /*
     @ Function:
     * create and initialize a new queue
     *
     @ Arguments:
     * 1) queue size (unsigned long long)
     * 2) element free function (free_t)
     */
    void queue_c::m_create( integer_t ullMyQueueSz, free_t p_fnMyFreeFunc ) {

        this->m_ullQueueSz = ullMyQueueSz + 1;
        this->mp_csMempool = new mempool_c( m_ullQueueSz * sizeof( var_u ) );
        this->mp_unItems = ( var_u* ) mp_csMempool->m_alloc( m_ullQueueSz * sizeof( var_u ) );
        this->m_ullLeftIdx = 0;
        this->m_ullRightIdx = 0;
        this->mp_fnFreeFunc = p_fnMyFreeFunc;

    }

    /*
     @ Function:
     * deconstructor of class "queue_c"
     */
    queue_c::~queue_c() {

        m_destroy();

    }

    /*
     @ Function:
     * destroy the queue itself
     */
    void queue_c::m_destroy() {

        while( !m_isEmpty() ) {
            var_u unTmpVal = {0};
            m_dequeue( &unTmpVal );
            if( unTmpVal.mp_vdObject != NULL && mp_fnFreeFunc != NULL ) {
                mp_fnFreeFunc( unTmpVal.mp_vdObject );
            }
        }
        delete mp_csMempool;

    }

    /*
     @ Function:
     * clear the queue for reuse
     */
    void queue_c::m_clear() {

        m_destroy();
        m_create( m_ullQueueSz - 1, mp_fnFreeFunc );

    }

    /*
     @ Function:
     * judge whether the queue is empty or not
     *
     @ Return:
     * "true" if the queue is empty, else "false" (bool)
     */
    bool queue_c::m_isEmpty() {

        return m_ullLeftIdx == m_ullRightIdx;

    }

    /*
     @ Function:
     * judge whether the queue is full or not
     *
     @ Return:
     * "true" if the queue is full, else "false" (bool)
     */
    bool queue_c::m_isFull() {

        return ( ( m_ullRightIdx + 1 ) % m_ullQueueSz ) == m_ullLeftIdx;

    }

    /*
     @ Function:
     * get the queue's size
     *
     @ Return:
     * element number (unsigned long long)
     */
    integer_t queue_c::m_getSize() {

        if( m_ullRightIdx >= m_ullLeftIdx ) {
            return m_ullRightIdx - m_ullLeftIdx;
        }
        else {
            return m_ullRightIdx + m_ullQueueSz - m_ullLeftIdx;
        }

    }

    /*
     @ Function:
     * add an element into the queue
     *
     @ Arguments:
     * data's value (var_u&)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool queue_c::m_enqueue( var_u& unValue ) {

        if( m_isFull() == true ) {
            return false;
        }
        mp_unItems[m_ullRightIdx] = unValue;
        m_ullRightIdx = ( m_ullRightIdx + 1 ) % m_ullQueueSz;
        return true;

    }

    /*
     @ Function:
     * get an element from the queue
     *
     @ Arguments:
     * address of the result data returned to the user (var_u*)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool queue_c::m_dequeue( var_u* p_unValue ) {

        if( m_isEmpty() == true ) {
            p_unValue = NULL;
            return false;
        }
        *p_unValue = mp_unItems[m_ullLeftIdx];
        m_ullLeftIdx = ( m_ullLeftIdx + 1 ) % m_ullQueueSz;
        return true;

    }

    /*
     @ Function:
     * put an "Integer" data into the queue
     *
     @ Arguments:
     * data value (unsigned long long)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool queue_c::m_enQueInt( integer_t ullValue ) {

        var_u unTmpVal = {0};
        unTmpVal.m_ullInteger = ullValue;
        return m_enqueue( unTmpVal );

    }

    /*
     @ Function:
     * get an "Integer" data from the queue
     *
     @ Return:
     * data's value (unsigned long long)
     */
    integer_t queue_c::m_deQueInt() {

        var_u unTmpVal = {0};
        if( m_dequeue( &unTmpVal ) == true ) {
            return unTmpVal.m_ullInteger;
        }
        return 0;

    }

    /*
     @ Function:
     * put a "Decimal" data into the queue
     *
     @ Arguments:
     * data value (long double)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool queue_c::m_enQueFloat( decimal_t ldValue ) {

        var_u unTmpVal = {0};
        unTmpVal.m_ldDecimal = ldValue;
        return m_enqueue( unTmpVal );

    }

    /*
     @ Function:
     * get a "Decimal" data from the queue
     *
     @ Return:
     * data's value (long double)
     */
    decimal_t queue_c::m_deQueFloat() {

        var_u unTmpVal = {0};
        if( m_dequeue( &unTmpVal ) == true ) {
            return unTmpVal.m_ldDecimal;
        }
        return 0.0;

    }

    /*
     @ Function:
     * put a complex "user-defined" data into the queue
     *
     @ Arguments:
     * pointer to the data (void*)
     *
     @ Return:
     * "true" if the operation is successful, else "false" (bool)
     */
    bool queue_c::m_enQueObject( void* p_vdValue ) {

        var_u unTmpVal = {0};
        unTmpVal.mp_vdObject = p_vdValue;
        return m_enqueue( unTmpVal );

    }

    /*
     @ Function:
     * get a complex "user-defined" data from the queue
     *
     @ Return:
     * pointer to the data (void*)
     */
    void* queue_c::m_deQueObject() {

        var_u unTmpVal = {0};
        if( m_dequeue( &unTmpVal ) == true ) {
            return unTmpVal.mp_vdObject;
        }
        return NULL;

    }

    /********************************************
     *
     * Hash Table's Member Functions
     *
     ********************************************/

    long double hashtab_c::ms_ldThreshold = 0.75;

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
    int hashtab_c::ms_ascKeyCompFunc( const void* p_vdKey1, const void* p_vdKey2 ) {

        return strcmp( ( const char* ) p_vdKey1, ( const char* ) p_vdKey2 );

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
    integer_t hashtab_c::ms_ascKeyByteNum( const void* p_vdStr ) {

        return (integer_t)( strlen( ( const char* ) p_vdStr ) + 1 );

    }

		/*
     @ Function:
     * constructor of class "hashtab_c"
     *
     @ Arguments:
     * 1) size of hash table (unsigned long long)
     * 2) element free function (free_t)
     * 3) key comparison function (compare_t)
     * 4) key length calculation function (sbn_t)
     */
    hashtab_c::hashtab_c( unsigned long long ullMyHashTabSz, long double ms_ldConflictThreshold, free_t p_fnMyFreeFunc, \
        compare_t p_fnKeyCompFunc, sbn_t p_fnKeyByteNumFunc ) {

        ms_ldThreshold = ms_ldConflictThreshold;

        m_create( ullMyHashTabSz, p_fnMyFreeFunc, p_fnKeyCompFunc, p_fnKeyByteNumFunc );

    }

    /*
     @ Function:
     * create and initialize a new hash table
     *
     @ Arguments:
     * 1) size of hash table (unsigned long long)
     * 2) element free function (free_t)
     * 3) key comparison function (compare_t)
     * 4) key length calculation function (sbn_t)
     */
    void hashtab_c::m_create( unsigned long long ullMyHashTabSz, free_t p_fnMyFreeFunc, \
        compare_t p_fnKeyCompFunc, sbn_t p_fnKeyByteNumFunc ) {

        this->m_ullHashTabSz = ( ullMyHashTabSz == 0 ? 1024 : ullMyHashTabSz );
        this->mp_csMempool = new mempool_c( (ull_t)(m_ullHashTabSz * ms_ldThreshold * sizeof( hashnode_s )) );
        this->mpp_stHashTab = ( hashnode_s** ) mp_csMempool->m_alloc( m_ullHashTabSz * sizeof( hashnode_s* ) );
        this->m_ullHashNdCnt = 0;
        this->mpp_vdKeyTab = ( void** ) mp_csMempool->m_alloc( m_ullHashTabSz * sizeof( void* ) );
        this->m_ullKeyCnt = 0;
        this->mp_fnFreeFunc = p_fnMyFreeFunc;
        this->mp_fnKeyCompFunc = ( p_fnKeyCompFunc == NULL ? hashtab_c::ms_ascKeyCompFunc : p_fnKeyCompFunc );
        this->mp_fnKeyByteNumFunc = ( p_fnKeyByteNumFunc == NULL ? hashtab_c::ms_ascKeyByteNum : p_fnKeyByteNumFunc );

    }

    /*
     @ Function:
     * deconstructor of class "hashtab_c"
     */
    hashtab_c::~hashtab_c() {

        m_destroy();

    }

    /*
     @ Function:
     * destroy the hash table
     */
    void hashtab_c::m_destroy() {

        for( unsigned long long i=0; i < m_ullHashTabSz; i++ ) {
            hashnode_s* p_stCurHashNd = mpp_stHashTab[i];
            while( p_stCurHashNd != NULL ) {
                hashnode_s* p_stTmpHashNd = p_stCurHashNd;
                p_stCurHashNd = p_stCurHashNd->mp_stNextNd;
                if( p_stTmpHashNd->mp_unValue->mp_vdObject != NULL && mp_fnFreeFunc != NULL ) {
                    mp_fnFreeFunc( p_stTmpHashNd->mp_unValue->mp_vdObject );
                }
            }
        }
        delete mp_csMempool;

    }

    /*
     @ Function:
     * clear the hash table for reuse
     */
    void hashtab_c::m_clear() {

        m_destroy();
        m_create( m_ullHashTabSz, mp_fnFreeFunc, mp_fnKeyCompFunc, mp_fnKeyByteNumFunc );

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
    unsigned long long hashtab_c::m_toHashCode( const void* p_vdKey ) {

        unsigned long long ullHashCode = 0, g = 0;
        const char* p_szStr = ( const char* ) p_vdKey;

        while( *p_szStr != '\0' ) {
            ullHashCode <<= 4;
            ullHashCode += ( unsigned long long ) *p_szStr;
            g = ullHashCode & ( ( unsigned long long ) 0xf << ( 64 - 4 ) );
            if( g != 0 ) {
                ullHashCode ^= g >> ( 64 - 8 );
                ullHashCode ^= g;
            }
            ++p_szStr;
        }
        
        return ullHashCode;

    }

    /*
     @ Function:
     * resize the hash table
     *
     @ Arguments:
     * new size of hash table (unsigned long long)
     */
    void hashtab_c::m_resize( unsigned long long ullNewHashTabSz ) {

        if( ullNewHashTabSz <= m_ullHashTabSz ) {
            return;
        }

        /* backup info of old hash table */
        unsigned long long ullOldHashTabSz = m_ullHashTabSz;
        mempool_c* p_csOldMempool = mp_csMempool;
        hashnode_s** pp_stOldHashTab = mpp_stHashTab;

        /* update info of new hash table */
        m_ullHashTabSz = ullNewHashTabSz;
        mp_csMempool = new mempool_c( (ull_t)(m_ullHashTabSz * ms_ldThreshold * sizeof( hashnode_s )) );
        mpp_stHashTab = ( hashnode_s** ) mp_csMempool->m_alloc( m_ullHashTabSz * sizeof( hashnode_s* ) );
        mpp_vdKeyTab = ( void** ) mp_csMempool->m_alloc( m_ullHashTabSz * sizeof( void* ) );
        m_ullHashNdCnt = 0;
        m_ullKeyCnt = 0;
        /* transfer hash nodes in old hash table to new hash table */
        for( unsigned long long i=0; i < ullOldHashTabSz; i++ ) {
            hashnode_s* p_stHashNd = pp_stOldHashTab[i];
            while( p_stHashNd != NULL ) {
                hashnode_s* p_stHashNdLast = p_stHashNd;
                p_stHashNd = p_stHashNd->mp_stNextNd;
                m_addHashNode( p_stHashNdLast->mp_vdKey, p_stHashNdLast->mp_unValue );
            }
        }

        /* eliminate old hash table */
        delete p_csOldMempool;

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
    var_u* hashtab_c::m_getHashNode( const void* p_vdKey ) {

        unsigned long long ullHashCode = m_toHashCode( p_vdKey );
        hashnode_s* p_stHashNd = mpp_stHashTab[ullHashCode % m_ullHashTabSz];

        while( p_stHashNd != NULL ) {
            int r = mp_fnKeyCompFunc( p_stHashNd->mp_vdKey, p_vdKey );
            if( r < 0 ) {
                p_stHashNd = p_stHashNd->mp_stNextNd;
            }
            else if( r == 0 ) {
                return p_stHashNd->mp_unValue;
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
    hashnode_s* hashtab_c::m_newHashNode( const void* p_vdKey, var_u* p_unValue ) {

        /* new hash node */
        hashnode_s* p_stNewHashNd = ( hashnode_s* ) mp_csMempool->m_alloc( sizeof( hashnode_s ) );
        /* initialize "key" of new hash node */
        integer_t iByteNum = mp_fnKeyByteNumFunc( p_vdKey );
        p_stNewHashNd->mp_vdKey = ( void* ) mp_csMempool->m_alloc( iByteNum * sizeof( char ) );
        memcpy( p_stNewHashNd->mp_vdKey, p_vdKey, iByteNum );
        /* initialize "value" of new hash node */
        p_stNewHashNd->mp_unValue = ( var_u* ) mp_csMempool->m_alloc( sizeof( var_u ) );
        *p_stNewHashNd->mp_unValue = *p_unValue;
        /* initialize pointer to the next hash node */
        p_stNewHashNd->mp_stNextNd = NULL;

        return p_stNewHashNd;

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
    var_u* hashtab_c::m_addHashNode( const void* p_vdKey, var_u* p_unValue ) {

        /* if there are too many "key" conflicts */
        long double r = ( ( long double ) m_ullHashNdCnt / ( long double ) m_ullHashTabSz );
        if( r >= ms_ldThreshold ) {
            m_resize( m_ullHashTabSz * 2 + 1 );
        }

        /* search the hash node identified by "key" */
        unsigned long long ullHashCode = m_toHashCode( p_vdKey );
        hashnode_s* p_stHashNd = mpp_stHashTab[ullHashCode % m_ullHashTabSz];
        hashnode_s* p_stHashNdLast = p_stHashNd;

        while( p_stHashNd != NULL ) {
            int r = mp_fnKeyCompFunc( p_stHashNd->mp_vdKey, p_vdKey );
            if( r < 0 ) {
                p_stHashNdLast = p_stHashNd;
                p_stHashNd = p_stHashNd->mp_stNextNd;
            }
            else if( r == 0 ) {
                /* already existed in hash table */
                if( p_stHashNd->mp_unValue->mp_vdObject != NULL && mp_fnFreeFunc != NULL ) {
                    mp_fnFreeFunc( p_stHashNd->mp_unValue->mp_vdObject );
                }
                *p_stHashNd->mp_unValue = *p_unValue;
                return p_stHashNd->mp_unValue;
            }
            else {
                break;
            }
        }

        /* no such hash node */
        hashnode_s* p_stNewHashNd = m_newHashNode( p_vdKey, p_unValue );
        if( p_stHashNd == mpp_stHashTab[ullHashCode % m_ullHashTabSz] ) {
            p_stNewHashNd->mp_stNextNd = mpp_stHashTab[ullHashCode % m_ullHashTabSz];
            mpp_stHashTab[ullHashCode % m_ullHashTabSz] = p_stNewHashNd;
        }
        else {
            p_stNewHashNd->mp_stNextNd = p_stHashNdLast->mp_stNextNd;
            p_stHashNdLast->mp_stNextNd = p_stNewHashNd;
        }
        ++m_ullHashNdCnt;
        mpp_vdKeyTab[m_ullKeyCnt++] = p_stNewHashNd->mp_vdKey;
        return p_stNewHashNd->mp_unValue;

    }

    /*
     @ Function:
     * add a new ( key, value<Integer> ) pair into the hash table
     *
     @ Arguments:
     * 1) key (void*)
     * 2) value (unsigned long long)
     */
    void hashtab_c::m_addInt( const void* p_vdKey, integer_t ullValue ) {

        var_u unTmpVal = {0};
        unTmpVal.m_ullInteger = ullValue;
        m_addHashNode( p_vdKey, &unTmpVal );

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
    integer_t hashtab_c::m_getInt( const void* p_vdKey ) {

        var_u* p_unVal = m_getHashNode( p_vdKey );
        if( p_unVal != NULL ) {
            return p_unVal->m_ullInteger;
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
    void hashtab_c::m_addFloat( const void* p_vdKey, decimal_t ldValue ) {

        var_u unTmpVal = {0};
        unTmpVal.m_ldDecimal = ldValue;
        m_addHashNode( p_vdKey, &unTmpVal );

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
    decimal_t hashtab_c::m_getFloat( const void* p_vdKey ) {

        var_u* p_unVal = m_getHashNode( p_vdKey );
        if( p_unVal != NULL ) {
            return p_unVal->m_ldDecimal;
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
    void hashtab_c::m_addObject( const void* p_vdKey, void* p_vdValue ) {

        var_u unTmpVal = {0};
        unTmpVal.mp_vdObject = p_vdValue;
        m_addHashNode( p_vdKey, &unTmpVal );

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
    void* hashtab_c::m_getObject( const void* p_vdKey ) {

        var_u* p_unVal = m_getHashNode( p_vdKey );
        if( p_unVal != NULL ) {
            return p_unVal->mp_vdObject;
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
    unsigned long long hashtab_c::m_getKeyCnt() {

        return m_ullKeyCnt;

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
    void* hashtab_c::m_getKey( unsigned long long ullKeyIdx ) {

        if( ullKeyIdx < 0 || ullKeyIdx >= m_ullKeyCnt ) {
            return NULL;
        }
        return mpp_vdKeyTab[ullKeyIdx];

    }

}

