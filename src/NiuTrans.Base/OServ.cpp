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
 * OS features; OServ.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); June 16th, 2011
 *
 */


#include "OServ.h"
#include "stddef.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"


namespace oserv {

    /**************************************************************************
     *
     * implementations of class mempool_c's member functions
     *
    ***************************************************************************/

    /*
     @ Function:
     * get a new memory block from the OS
     */
    block_s* mempool_c::m_newMemBlk() {

        block_s* p_stMemBlk = new block_s();
        p_stMemBlk->mp_vdMem = new unsigned char[m_ullMemBlkMaxSz];
        p_stMemBlk->m_ullMemSize = m_ullMemBlkMaxSz;
        p_stMemBlk->m_ullMemUsed = 0;
        return p_stMemBlk;

    }

    /*
     @ Function:
     * initialize class "mempool_c"
     *
     @ Arguments:
     * 1) minimum block size in memory array (unsigned long long)
     * 2) minimum memory array size if argument 3) is true, else is maximum array size (unsigned long long)
     * 3) flag showing whether memory array is expandable or not (bool)
     *
     @ Return:
     * NULL
     *
     @ Basic Algorithm:
     * initialize class "mempool_c"'s member variables,
     * and only allocate one memory block for the memory array.
     * more memory blocks will be allocated if needed.
     */
    void mempool_c::m_initMempool( ull_t ullMemBlkSize, ull_t ullMemBlkNum, bool bMemSzExpandable ) {

        if( ullMemBlkSize <= 0 ) {
            fprintf( stderr, "%s:\t%s\n", "[ERROR]:", "Argument(s) Wrong Data Range" );
            exit( 1 );
        }
        /* memory chain */
        if( ullMemBlkNum <= 0 ) {
            ullMemBlkNum = 1;
        }
        this->m_ullMemBlkNum = ullMemBlkNum;
        this->mpp_stMemBlks = new block_s*[this->m_ullMemBlkNum];
        memset( this->mpp_stMemBlks, 0, this->m_ullMemBlkNum * sizeof( block_s* ) );
        /* memory status */
        this->m_ullMemBlkMaxSz = ( ullMemBlkSize >= ms_ullMemBlkMaxSz ? ms_ullMemBlkMaxSz : ullMemBlkSize );
        this->m_ullMemTotalSize = this->m_ullMemBlkMaxSz;
        this->m_ullMemUsedSize = 0;
        this->m_bMemSizeExpandable = bMemSzExpandable;
        /* info of current block */
        this->m_ullMemBlkLeftIdx = 0;
        this->m_ullMemBlkRightIdx = 1;
        block_s* p_stMemBlk = m_newMemBlk();
        this->mpp_stMemBlks[this->m_ullMemBlkLeftIdx] = p_stMemBlk;        

    }

    /*
     @ Function:
     * default constructor of class "mempool_c".
     * we defaultly construct a expandable 16 * 4K memory pool.
     */
    mempool_c::mempool_c() {

        /* 4K */
        ull_t ullMemBlkSz = 256;
        ull_t ullMemBlkNum = 16;
        bool bMemSzExpandable = true;
        m_initMempool( ullMemBlkSz, ullMemBlkNum, bMemSzExpandable );

    }

    /*
     @ Function:
     * constructor of class "mempool_c" with user specified arguments.
     */
    mempool_c::mempool_c( ull_t ullMemBlkSize, ull_t ullMemBlkNum, bool bMemSzExpandable ) {

        m_initMempool( ullMemBlkSize, ullMemBlkNum, bMemSzExpandable );

    }

    /*
     @ Function:
     * deconstructor of class "mempool_c"
     */
    mempool_c::~mempool_c() {

        for( int i=0; i < this->m_ullMemBlkRightIdx; i++ ) {
            delete[] ( unsigned char* ) ( this->mpp_stMemBlks[i]->mp_vdMem );
            delete this->mpp_stMemBlks[i];
        }
        delete[] this->mpp_stMemBlks;

    }

    /*
     @ Function:
     * find the memory block which can supplies the specified size memory for users,
     * by scanning over all available memory blocks.
     * if no such block exists, request a new memory block that fullfills the users from the OS.
     *
     @ Arguments:
     * size of wanted memory (unsigned long long)
     *
     @ Return:
     * pointer to the right memory block
     * NULL if failed
     *
     @ Basic Algorithm:
     * scan over available memory blocks (indicated by the two indices in the class) to find the right block
     * or request a new block from the OS
     */
    block_s* mempool_c::m_findMemBlk( ull_t ullMemSize ) {

        block_s* p_stMemBlk = NULL;

        /* scan over all available memory blocks */
        for( int i=(int)m_ullMemBlkLeftIdx; i < m_ullMemBlkRightIdx; i++ ) {
            p_stMemBlk = mpp_stMemBlks[i];
            /* there exists such memory block */
            if( p_stMemBlk->m_ullMemUsed + ullMemSize <= p_stMemBlk->m_ullMemSize ) {
                return p_stMemBlk;
            }
        }
        /* no such memory block */
        if( m_ullMemBlkRightIdx == m_ullMemBlkNum ) {
            /* the mempool is full and non-expandable */
            if( !m_bMemSizeExpandable )
                return NULL;
            else {
                block_s** pp_stNewMemBlks = new block_s*[m_ullMemBlkNum * 2 + 1];
                memset( pp_stNewMemBlks, 0, ( m_ullMemBlkNum * 2 + 1 ) * sizeof( block_s* ) );
                memcpy( pp_stNewMemBlks, mpp_stMemBlks, m_ullMemBlkNum * sizeof( block_s* ) );
                m_ullMemBlkNum = m_ullMemBlkNum * 2 + 1;
                delete[] mpp_stMemBlks;
                mpp_stMemBlks = pp_stNewMemBlks;
            }
        }
        /* allocate a new block from the OS */
        if( ullMemSize > m_ullMemBlkMaxSz && ullMemSize <= ms_ullMemBlkMaxSz ) {
            m_ullMemBlkMaxSz = ullMemSize;
        }
        p_stMemBlk = m_newMemBlk();
        mpp_stMemBlks[m_ullMemBlkRightIdx++] = p_stMemBlk;
        m_ullMemTotalSize += m_ullMemBlkMaxSz;
        return p_stMemBlk;

    }

    /*
     @ Function:
     * allocate memory of specified size from memory pool for users
     *
     @ Arguments:
     * size of wanted memory (unsigned long long)
     *
     @ Return:
     * pointer to the allocated memory for users (void*)
     * the memory has been initialized with value "0".
     *
     @ Basic Algorithm:
     * 1) make sure enough memory left in memory pool to satify the users,
     *    by sometimes allocating new memory blocks from the OS.
     * 2) allocate memory by changing used memory offset and updating memory pool's status.
     */
    void* mempool_c::m_alloc( ull_t ullMemSize ) {

        void* p_vdMem = NULL;

        /* wrong input data range, return null pointer */
        if( ullMemSize <= 0 )
            return NULL;

        /* find the block from which the memory will be allocated */
        block_s* p_stMemBlk = m_findMemBlk( ullMemSize );
        if( !p_stMemBlk ) {
            return NULL;
        }

        /* allocate the requested memory from the found block, and update mempool's status */
        p_vdMem = ( void* ) ( ( unsigned char* ) p_stMemBlk->mp_vdMem + p_stMemBlk->m_ullMemUsed );
        memset( p_vdMem, 0, sizeof( unsigned char ) * ullMemSize );
        p_stMemBlk->m_ullMemUsed += ullMemSize;
        m_ullMemUsedSize += ullMemSize;

        /* update maximum range of available memory blocks */
        while( m_ullMemBlkLeftIdx < m_ullMemBlkRightIdx ) {
            block_s* p_stTmpMemBlk = mpp_stMemBlks[m_ullMemBlkLeftIdx];
            ull_t ullAvailMemSz = p_stTmpMemBlk->m_ullMemSize - p_stTmpMemBlk->m_ullMemUsed;
            ull_t ullDynamicThresh = ( ull_t ) ( m_ullMemBlkMaxSz * 0.01 );
            ull_t ullStaticThresh = 256;
            if( ullAvailMemSz > ullDynamicThresh || ullAvailMemSz > ullStaticThresh ) {
                break;
            }
            ++m_ullMemBlkLeftIdx;
        }

        return p_vdMem;

    }

    /*
     @ Function:
     * clear one memory block for reusing
     *
     @ Arguments:
     * pointer to the memory block (block_s*)
     *
     @ Return:
     * NULL
     *
     @ Basic Algorithm:
     * clear the memory pointed by the block
     * and reset using status of the block
     */
    void mempool_c::m_clearMemBlk( block_s* p_stMemBlk ) {

        p_stMemBlk->m_ullMemUsed = 0;

    }

    /*
     @ Function:
     * clear the data beyond the specified memory bound of the memory pool
     *
     @ Agruments:
     * preserved size of memory (unsigned long long)
     *
     @ Return:
     * NULL
     *
     @ Basic Algorithm:
     * reset to be "0" within the specified bound of memory
     * free the memory blocks beyond the bound
     */
    void mempool_c::m_clear( ull_t ullPreservedSize ) {

        /* always preserve the first block in memory pool */
        ull_t ullCurSz = mpp_stMemBlks[0]->m_ullMemSize;
        m_clearMemBlk( mpp_stMemBlks[0] );
        ull_t ullIdx = 1;
        /* continue to clear memory block until the size of clean memory is not less than that requested */
        while( ullIdx < m_ullMemBlkRightIdx && ullCurSz < ullPreservedSize ) {
            block_s* p_stMemBlk = mpp_stMemBlks[ullIdx];
            ullCurSz += p_stMemBlk->m_ullMemSize;
            m_clearMemBlk( p_stMemBlk );
            ++ullIdx;
        }
        /* free all the left memory blocks */
        for( int i=(int)ullIdx; i < m_ullMemBlkRightIdx; i++ ) {
            delete[] (unsigned char*)(mpp_stMemBlks[i]->mp_vdMem);
            delete mpp_stMemBlks[i];
        }
        /* update using status of the memory pool */
        m_ullMemBlkNum = ullIdx;
        m_ullMemTotalSize = ullCurSz;
        m_ullMemUsedSize = 0;
        m_ullMemBlkLeftIdx = 0;
        m_ullMemBlkRightIdx = ullIdx;

    }

    /*
     @ Function:
     * get the size of total used memory
     *
     @ Return:
     * the used memory size (unsigned long long)
     */
    ull_t mempool_c::m_getUsedMemSz() {

        return m_ullMemUsedSize;

    }

    /*
     @ Function:
     * display current running status of the memory pool
     *
     @ Arguments:
     * file pointer to which the program writes information
     *
     @ Return:
     * NULL
     *
     */
    void mempool_c::m_showMempoolStatus( FILE* p_stFileHandle ) {

        fprintf( p_stFileHandle, "               Memory Pool Running Status\n" );
        fprintf( p_stFileHandle, "\n" );
        fprintf( p_stFileHandle, "-----------------------------------------------------------\n" );
        fprintf( p_stFileHandle, "\n" );
        fprintf( p_stFileHandle, ">> Total Size:                  %20llu bytes\n", m_ullMemTotalSize );
        fprintf( p_stFileHandle, ">> Used Size:                   %20llu bytes\n", m_ullMemUsedSize );
        fprintf( p_stFileHandle, ">> Available Size:              %20llu bytes\n", m_ullMemTotalSize - m_ullMemUsedSize );
        fprintf( p_stFileHandle, ">> Expandable:                  %20d\n", m_bMemSizeExpandable );
        fprintf( p_stFileHandle, ">> Memory Block Number:         %20llu\n", m_ullMemBlkRightIdx );
        fprintf( p_stFileHandle, "\n" );
        fprintf( p_stFileHandle, "-----------------------------------------------------------\n" );
        fprintf( p_stFileHandle, "\n" );
        fprintf( p_stFileHandle, ">> Memory Blocks\' Status:\n" );
        fprintf( p_stFileHandle, ">> Left Index:                  %20llu\n", m_ullMemBlkLeftIdx );
        fprintf( p_stFileHandle, "\n" );
        for( ull_t i=0; i < m_ullMemBlkRightIdx; i++ ) {
            fprintf( p_stFileHandle, ">>>> $ Block ID:                %20llu\n", i );
            fprintf( p_stFileHandle, "     $ Total Size:              %20llu bytes\n", mpp_stMemBlks[i]->m_ullMemSize );
            fprintf( p_stFileHandle, "     $ Used Size:               %20llu bytes\n", mpp_stMemBlks[i]->m_ullMemUsed );
            fprintf( p_stFileHandle, "     $ Available Size:          %20llu bytes\n", mpp_stMemBlks[i]->m_ullMemSize - \
                mpp_stMemBlks[i]->m_ullMemUsed );
            fprintf( p_stFileHandle, "\n" );
        }
        fprintf( p_stFileHandle, "-----------------------------------------------------------\n" );

    }

    /**************************************************************************
     *
     * operations on Critical Section for threads
     *
    ***************************************************************************/

    /*
     @ Function:
     * default constructor of class "mutex_c"
     */
    mutex_c::mutex_c() {

#if !defined( WIN32 ) && !defined( _WIN32 )
        pthread_mutex_init( &m_stMutex, NULL );
#else
        ::InitializeCriticalSection( &m_stMutex );
#endif

    }

    /*
     @ Function:
     * deconstructor of class "mutex_c"
     */
    mutex_c::~mutex_c() {

#if !defined( WIN32 ) && !defined( _WIN32 )
        pthread_mutex_destroy( &m_stMutex );
#else
        ::DeleteCriticalSection( &m_stMutex );
#endif

    }

    /*
     @ Function:
     * lock the critical section
     */
    void mutex_c::m_lock() {

#if !defined( WIN32 ) && !defined( _WIN32 )
        pthread_mutex_lock( &m_stMutex );
#else
        ::EnterCriticalSection( &m_stMutex );
#endif

    }

    /*
     @ Function:
     * unlock the critical section
     */
    void mutex_c::m_unlock() {

#if !defined( WIN32 ) && !defined( _WIN32 )
        pthread_mutex_unlock( &m_stMutex );
#else
        ::LeaveCriticalSection( &m_stMutex );
#endif

    }

}

