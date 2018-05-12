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
 * features; MemManager.h
 * this head file provides the utilities for memory management 
 *
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


#ifndef    __MEMMANAGER_H__
#define    __MEMMANAGER_H__


#include "stdio.h"
#if !defined( WIN32 ) && !defined( _WIN32 )
#include "pthread.h"
#else
#include "windows.h"
#endif


namespace memmanager {

    /*
     * define inner data types
     */
    typedef unsigned long long ull_t;

    /*
     * define the structure of a memory block
     */
    struct MemBlock {

        public:
            void *       vdMem;
            ull_t        ullMemSize;
            ull_t        ullMemUsed;

    };

    /*
     *
     * Memory Pool
     *
     */
    class MemPool {

    public:
        MemPool();
        MemPool( ull_t ullMemBlkSize, ull_t ullMemBlkNum = 256, bool bMemSzExpandable = true );
        ~MemPool();
        void * Alloc( ull_t ullMemSize );
        void Clear( ull_t ullPreservedSize = 0 );
        ull_t GetUsedMemSize();
        void ShowMemPoolStatus( FILE* p_stFileHandle = stderr );

    protected:
        MemBlock * CreateMemBlock();
        void InitMempool( ull_t ullMemBlkSize, ull_t ullMemBlkNum, bool bMemSzExpandable = true );
        MemBlock * FindMemBlock( ull_t ullMemSize );
        void ClearMemBlock( MemBlock * stMemBlock );

    private:
        /* chain of memory blocks */
        MemBlock**               mpstMemBlocks;
        ull_t                    ullMemBlockNum;
        /* running status of memory pool */
        ull_t                    ullMemTotalSize;
        ull_t                    ullMemUsedSize;
        ull_t                    ullMemBlockMaxSize;
        bool                     bMemSizeExpandable;
        /* maximum range of available memory blocks */
        ull_t                    ullMemBlockLeftIndex;
        ull_t                    ullMemBlockRightIndex;

    private:
        static const ull_t        staticMemBlockMaxSize = (ull_t)1024 * 1024 * 1024 * 16; //10737418240; //1024 * 1024 * 1024 * 16;

    };

    /*
     *
     * Critical Section (Mutex) Management Class
     *
     */
    class mutex_c {

    public:
        mutex_c();
        ~mutex_c();
        void m_lock();
        void m_unlock();

    private:
#if !defined( WIN32 ) && !defined( _WIN32 )
        pthread_mutex_t        m_stMutex;
#else
        CRITICAL_SECTION    m_stMutex;
#endif

    };

}


#endif

