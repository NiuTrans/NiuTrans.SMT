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
 * OS features; OServ.h
 * This header file provides some useful features common in the Operation System (OS).
 * 1) It defines the data structures and manipulations of a self-management memory pool.
 * It simulates some basic memory operations, which greatly reduces the times of allocating memory requests
 * from the OS, by employing a chain of large pre-allocated memory arrays.
 * 2) It also defines a mutex class for synchronization of threads when operating on global shared variables.
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


#ifndef    __OSERVICE_H__
#define    __OSERVICE_H__


#include "stdio.h"
#if !defined( WIN32 ) && !defined( _WIN32 )
#include "pthread.h"
#else
#include "windows.h"
#endif


namespace oserv {

    /*
     * define inner data types
     */
    typedef unsigned long long ull_t;

    /*
     * define the structure of a memory block
     */
    typedef struct __block_s {

        public:
            void*        mp_vdMem;
            ull_t        m_ullMemSize;
            ull_t        m_ullMemUsed;

    }block_s;

    /*
     *
     * Memory Pool
     *
     */
    class mempool_c {

    public:
        mempool_c();
        mempool_c( ull_t ullMemBlkSize, ull_t ullMemBlkNum = 256, bool bMemSzExpandable = true );
        ~mempool_c();
        void* m_alloc( ull_t ullMemSize );
        void m_clear( ull_t ullPreservedSize = 0 );
        ull_t m_getUsedMemSz();
        void m_showMempoolStatus( FILE* p_stFileHandle = stderr );

    protected:
        block_s* m_newMemBlk();
        void m_initMempool( ull_t ullMemBlkSize, ull_t ullMemBlkNum, bool bMemSzExpandable = true );
        block_s* m_findMemBlk( ull_t ullMemSize );
        void m_clearMemBlk( block_s* p_stMemBlk );

    private:
        /* chain of memory blocks */
        block_s**                mpp_stMemBlks;
        ull_t                    m_ullMemBlkNum;
        /* running status of memory pool */
        ull_t                    m_ullMemTotalSize;
        ull_t                    m_ullMemUsedSize;
        ull_t                    m_ullMemBlkMaxSz;
        bool                    m_bMemSizeExpandable;
        /* maximum range of available memory blocks */
        ull_t                    m_ullMemBlkLeftIdx;
        ull_t                    m_ullMemBlkRightIdx;

    private:
        static const ull_t        ms_ullMemBlkMaxSz = 1024*1024*1024;

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

