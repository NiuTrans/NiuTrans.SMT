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
* multi_thread.h
*
* $Version:
* 1.1.0
*
* $Created by:
* Qiang Li (email: liqiangneu@gmail.com)
*
* $Last Modified by:
* 8/20/2012, add this file
*
*/

#ifndef _MULTI_THREAD_H_
#define _MULTI_THREAD_H_

#ifndef WIN32
#define HAVE_PTHREAD_H // for Linux
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#else
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif
#endif

#if defined HAVE_PTHREAD_H
#define _USE_NTHREAD 1
#endif

#if( defined( _WIN32 ) && !defined( __CYGWIN__ ) )
#define _USE_NTHREAD 1
#define BEGINTHREAD(src, stack, func, arg, flag, id) \
    (HANDLE)_beginthreadex((void *)(src), (unsigned)(stack), \
    (unsigned(_stdcall *)(void *))(func), (void *)(arg), \
    (unsigned)(flag),(unsigned *)(id))
#endif

namespace multi_thread
{

    class MultiThread
    {
    private:
    #ifdef HAVE_PTHREAD_H
        pthread_t hnd_;
    #else
    #ifdef _WIN32
        HANDLE hnd_;
    #endif
    #endif

    public:
        int id      ;
        int threadId;
        int nthread ;

        MultiThread(){}
        virtual ~MultiThread(){}

        static void* Wrapper( void *ptr )
        {
            MultiThread *p = ( MultiThread * )ptr;
            p->Run();
            return 0;
        }
     
        void Start();
        void Join( );
        virtual void Run(){}

    };

}

#endif
