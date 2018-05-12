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
* multi_thread.cpp
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

#include "multi_thread.h"
using namespace multi_thread;


void MultiThread::Start()
{
#ifdef HAVE_PTHREAD_H
    pthread_create( &hnd_, 0, &MultiThread::Wrapper, static_cast<void *>(this) );
#else
#ifdef _WIN32
    DWORD id;
    hnd_ = BEGINTHREAD(0, 0, &MultiThread::Wrapper, this, 0, &id );
#else
    Run();
#endif
#endif
}

void MultiThread::Join()
{
#ifdef HAVE_PTHREAD_H
    pthread_join( hnd_, 0 );
#else
#ifdef _WIN32
    WaitForSingleObject( hnd_, INFINITE );
    CloseHandle( hnd_ );
#endif
#endif
}
