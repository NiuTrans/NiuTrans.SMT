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
* lock.cpp
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

#include "lock.h"
using namespace lock;


Mutex::Mutex()
{

#ifdef HAVE_PTHREAD_H
    pthread_mutex_init( &m_mutex, NULL );
#else
#ifdef _WIN32
    m_mutex = ::CreateMutex(NULL, FALSE, NULL);
#endif
#endif

}

Mutex::~Mutex()
{
#ifdef HAVE_PTHREAD_H
    pthread_mutex_destroy( &m_mutex );
#else
#ifdef _WIN32
    ::CloseHandle(m_mutex);
#endif
#endif
}

void Mutex::Lock() const
{

#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock( &m_mutex );
#else
#ifdef _WIN32
    DWORD d = WaitForSingleObject(m_mutex, INFINITE);
#endif
#endif

}

void Mutex::Unlock() const
{

#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock( &m_mutex );
#else
#ifdef _WIN32
    ::ReleaseMutex(m_mutex);
#endif
#endif

}

CLock::CLock(const ILock& m) : m_lock(m)
{
    m_lock.Lock();
}

CLock::~CLock()
{
    m_lock.Unlock();
}
