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
 * OS features; MemManager.cpp
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


#include "MemManager.h"
#include "stddef.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"


namespace memmanager {

/**************************************************************************
 *
 * implementations of class MemPool's member functions
 *
***************************************************************************/

/*
 @ Function:
 * get a new memory block from the OS
 */
MemBlock* MemPool::CreateMemBlock() {

    MemBlock* stMemBlock = new MemBlock();
    stMemBlock->vdMem = new unsigned char[ullMemBlockMaxSize];
    stMemBlock->ullMemSize = ullMemBlockMaxSize;
    stMemBlock->ullMemUsed = 0;
    return stMemBlock;

}

/*
 @ Function:
 * initialize class "MemPool"
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
 * initialize class "MemPool"'s member variables,
 * and only allocate one memory block for the memory array.
 * more memory blocks will be allocated if needed.
 */
void MemPool::InitMempool( ull_t ullMemBlkSize, ull_t ullMemBlkNum, bool bMemSzExpandable ) {

    if( ullMemBlkSize <= 0 ) {
        fprintf( stderr, "%s:\t%s\n", "[ERROR]:", "Argument(s) Wrong Data Range" );
        exit( 1 );
    }
    /* memory chain */
    if( ullMemBlkNum <= 0 ) {
        ullMemBlkNum = 1;
    }
    this->ullMemBlockNum = ullMemBlkNum;
    this->mpstMemBlocks = new MemBlock*[this->ullMemBlockNum];
    memset( this->mpstMemBlocks, 0, this->ullMemBlockNum * sizeof( MemBlock* ) );
    /* memory status */
    //fprintf(stderr, "%lld %lld", ullMemBlkSize, staticMemBlockMaxSize);
    this->ullMemBlockMaxSize = ( ullMemBlkSize >= staticMemBlockMaxSize ? staticMemBlockMaxSize : ullMemBlkSize );
    this->ullMemTotalSize = this->ullMemBlockMaxSize;
    this->ullMemUsedSize = 0;
    this->bMemSizeExpandable = bMemSzExpandable;
    /* info of current block */
    this->ullMemBlockLeftIndex = 0;
    this->ullMemBlockRightIndex = 1;
    MemBlock* stMemBlock = CreateMemBlock();
    this->mpstMemBlocks[this->ullMemBlockLeftIndex] = stMemBlock;        

}

/*
 @ Function:
 * default constructor of class "MemPool".
 * we defaultly construct a expandable 16 * 4K memory pool.
 */
MemPool::MemPool() {

    /* 4K */
    ull_t ullMemBlkSz = 256;
    ull_t ullMemBlkNum = 16;
    bool bMemSzExpandable = true;
    InitMempool( ullMemBlkSz, ullMemBlkNum, bMemSzExpandable );

}

/*
 @ Function:
 * constructor of class "MemPool" with user specified arguments.
 */
MemPool::MemPool( ull_t ullMemBlkSize, ull_t ullMemBlkNum, bool bMemSzExpandable ) {

    InitMempool( ullMemBlkSize, ullMemBlkNum, bMemSzExpandable );

}

/*
 @ Function:
 * deconstructor of class "MemPool"
 */
MemPool::~MemPool() {

    for( int i=0; i < this->ullMemBlockRightIndex; i++ ) {
        delete[] ( unsigned char* ) ( this->mpstMemBlocks[i]->vdMem );
        delete this->mpstMemBlocks[i];
    }
    delete[] this->mpstMemBlocks;

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
MemBlock* MemPool::FindMemBlock( ull_t ullMemSize ) {

    MemBlock* stMemBlock = NULL;

    /* scan over all available memory blocks */
    for( int i=(int)ullMemBlockLeftIndex; i < ullMemBlockRightIndex; i++ ) {
        stMemBlock = mpstMemBlocks[i];
        /* there exists such memory block */
        if( stMemBlock->ullMemUsed + ullMemSize <= stMemBlock->ullMemSize ) {
            return stMemBlock;
        }
    }
    /* no such memory block */
    if( ullMemBlockRightIndex == ullMemBlockNum ) {
        /* the mempool is full and non-expandable */
        if( !bMemSizeExpandable )
            return NULL;
        else {
            MemBlock** pp_stCreateMemBlocks = new MemBlock*[ullMemBlockNum * 2 + 1];
            memset( pp_stCreateMemBlocks, 0, ( ullMemBlockNum * 2 + 1 ) * sizeof( MemBlock* ) );
            memcpy( pp_stCreateMemBlocks, mpstMemBlocks, ullMemBlockNum * sizeof( MemBlock* ) );
            ullMemBlockNum = ullMemBlockNum * 2 + 1;
            delete[] mpstMemBlocks;
            mpstMemBlocks = pp_stCreateMemBlocks;
        }
    }
    /* allocate a new block from the OS */
    if( ullMemSize > ullMemBlockMaxSize && ullMemSize <= staticMemBlockMaxSize ) {
        ullMemBlockMaxSize = ullMemSize;
    }
    stMemBlock = CreateMemBlock();
    mpstMemBlocks[ullMemBlockRightIndex++] = stMemBlock;
    ullMemTotalSize += ullMemBlockMaxSize;
    return stMemBlock;

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
void* MemPool::Alloc( ull_t ullMemSize ) {

    void* p_vdMem = NULL;

    /* wrong input data range, return null pointer */
    if( ullMemSize <= 0 )
        return NULL;

    /* find the block from which the memory will be allocated */
    MemBlock* stMemBlock = FindMemBlock( ullMemSize );
    if( !stMemBlock ) {
        return NULL;
    }

    /* allocate the requested memory from the found block, and update mempool's status */
    p_vdMem = ( void* ) ( ( unsigned char* ) stMemBlock->vdMem + stMemBlock->ullMemUsed );
    memset( p_vdMem, 0, sizeof( unsigned char ) * ullMemSize );
    stMemBlock->ullMemUsed += ullMemSize;
    ullMemUsedSize += ullMemSize;

    /* update maximum range of available memory blocks */
    while( ullMemBlockLeftIndex < ullMemBlockRightIndex ) {
        MemBlock* p_stTmpMemBlk = mpstMemBlocks[ullMemBlockLeftIndex];
        ull_t ullAvailMemSz = p_stTmpMemBlk->ullMemSize - p_stTmpMemBlk->ullMemUsed;
        ull_t ullDynamicThresh = ( ull_t ) ( ullMemBlockMaxSize * 0.01 );
        ull_t ullStaticThresh = 256;
        if( ullAvailMemSz > ullDynamicThresh || ullAvailMemSz > ullStaticThresh ) {
            break;
        }
        ++ullMemBlockLeftIndex;
    }

    return p_vdMem;

}

/*
 @ Function:
 * clear one memory block for reusing
 *
 @ Arguments:
 * pointer to the memory block (MemBlock*)
 *
 @ Return:
 * NULL
 *
 @ Basic Algorithm:
 * clear the memory pointed by the block
 * and reset using status of the block
 */
void MemPool::ClearMemBlock( MemBlock* stMemBlock ) {

    stMemBlock->ullMemUsed = 0;

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
void MemPool::Clear( ull_t ullPreservedSize ) {

    /* always preserve the first block in memory pool */
    ull_t ullCurSz = mpstMemBlocks[0]->ullMemSize;
    ClearMemBlock( mpstMemBlocks[0] );
    ull_t ullIdx = 1;
    /* continue to clear memory block until the size of clean memory is not less than that requested */
    while( ullIdx < ullMemBlockRightIndex && ullCurSz < ullPreservedSize ) {
        MemBlock* stMemBlock = mpstMemBlocks[ullIdx];
        ullCurSz += stMemBlock->ullMemSize;
        ClearMemBlock( stMemBlock );
        ++ullIdx;
    }
    /* free all the left memory blocks */
    for( int i=(int)ullIdx; i < ullMemBlockRightIndex; i++ ) {
        delete[] (unsigned char*)(mpstMemBlocks[i]->vdMem);
        delete mpstMemBlocks[i];
    }
    /* update using status of the memory pool */
    ullMemBlockNum = ullIdx;
    ullMemTotalSize = ullCurSz;
    ullMemUsedSize = 0;
    ullMemBlockLeftIndex = 0;
    ullMemBlockRightIndex = ullIdx;

}

/*
 @ Function:
 * get the size of total used memory
 *
 @ Return:
 * the used memory size (unsigned long long)
 */
ull_t MemPool::GetUsedMemSize() {

    return ullMemUsedSize;

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
void MemPool::ShowMemPoolStatus( FILE* p_stFileHandle ) {

    fprintf( p_stFileHandle, "               Memory Pool Running Status\n" );
    fprintf( p_stFileHandle, "\n" );
    fprintf( p_stFileHandle, "-----------------------------------------------------------\n" );
    fprintf( p_stFileHandle, "\n" );
    fprintf( p_stFileHandle, ">> Total Size:                  %20llu bytes\n", ullMemTotalSize );
    fprintf( p_stFileHandle, ">> Used Size:                   %20llu bytes\n", ullMemUsedSize );
    fprintf( p_stFileHandle, ">> Available Size:              %20llu bytes\n", ullMemTotalSize - ullMemUsedSize );
    fprintf( p_stFileHandle, ">> Expandable:                  %20d\n", bMemSizeExpandable );
    fprintf( p_stFileHandle, ">> Memory Block Number:         %20llu\n", ullMemBlockRightIndex );
    fprintf( p_stFileHandle, "\n" );
    fprintf( p_stFileHandle, "-----------------------------------------------------------\n" );
    fprintf( p_stFileHandle, "\n" );
    fprintf( p_stFileHandle, ">> Memory Blocks\' Status:\n" );
    fprintf( p_stFileHandle, ">> Left Index:                  %20llu\n", ullMemBlockLeftIndex );
    fprintf( p_stFileHandle, "\n" );
    for( ull_t i=0; i < ullMemBlockRightIndex; i++ ) {
        fprintf( p_stFileHandle, ">>>> $ Block ID:                %20llu\n", i );
        fprintf( p_stFileHandle, "     $ Total Size:              %20llu bytes\n", mpstMemBlocks[i]->ullMemSize );
        fprintf( p_stFileHandle, "     $ Used Size:               %20llu bytes\n", mpstMemBlocks[i]->ullMemUsed );
        fprintf( p_stFileHandle, "     $ Available Size:          %20llu bytes\n", mpstMemBlocks[i]->ullMemSize - \
            mpstMemBlocks[i]->ullMemUsed );
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

