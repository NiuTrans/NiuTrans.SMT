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
* lock.h
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

#ifndef _LOCK_H_
#define _LOCK_H_

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

namespace lock
{

    class ILock
    {
    public:
        virtual ~ILock() {}

        virtual void Lock() const = 0;
        virtual void Unlock() const = 0;
    };

    class Mutex : public ILock
    {
    public:
        Mutex();
        ~Mutex();

        virtual void Lock() const;
        virtual void Unlock() const;

    private:

    #ifdef HAVE_PTHREAD_H
        mutable pthread_mutex_t m_mutex;
    #else
    #ifdef _WIN32
        HANDLE m_mutex;
    #endif
    #endif

    };

    class CLock
    {
    public:
        CLock(const ILock&);
        ~CLock();

    private:
        const ILock& m_lock;
    };

}

#endif
