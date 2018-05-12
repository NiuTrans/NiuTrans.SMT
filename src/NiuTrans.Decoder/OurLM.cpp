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
 * Using language models for our SMT system; OurLM.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email:xiaotong@mail.neu.edu.cn)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); June 9th, 2011; load language model from dynamic library
 *
 */

#include "OurLM.h"

namespace smt{

#define MAX_NGRAM_ORDER 10

INITLM_FUNC g_initlm = NULL;
UNLOADLM_FUNC g_unloadlm = NULL;
GET_NGRAM_PROB_FUNC g_get_ngram_prob2 = NULL;

#ifdef WIN32
    HINSTANCE GetLmFunc()
    {
        HINSTANCE hInsDll = ::LoadLibrary( LM_LIB );
        if( hInsDll == NULL ) {
            ::FreeLibrary( hInsDll );
            return NULL;
        }
        g_initlm = (INITLM_FUNC)::GetProcAddress( hInsDll, "g_initlm" );
        g_unloadlm = (UNLOADLM_FUNC)::GetProcAddress( hInsDll, "g_unloadlm" );
        g_get_ngram_prob2 = (GET_NGRAM_PROB_FUNC)::GetProcAddress( hInsDll, "g_get_ngram_prob2" );
        if( !g_initlm || !g_unloadlm || !g_get_ngram_prob2 ) {
            ::FreeLibrary( hInsDll );
            return NULL;
        }
        return hInsDll;
    }
#else
    void* GetLmFunc()
    {
        void* handle = dlopen(LM_LIB, RTLD_LAZY);
        if( handle == NULL ) {
            fprintf( stderr, "[ERROR]: Can not load dynamic library: %s\n", dlerror() );
            return NULL;
        }
        const char* dlsym_error;
        /* load "g_initlm" */
        dlerror();  /* reset errors */
        g_initlm = (INITLM_FUNC) dlsym( handle, "g_initlm" );
        dlsym_error = dlerror();
        if( dlsym_error ) {
            fprintf( stderr, "[ERROR]: Cant not load symbol \"g_initlm\": %s\n", dlsym_error );
            return NULL;
        }
        /* load "g_unloadlm" */
        dlerror();
        g_unloadlm = (UNLOADLM_FUNC) dlsym( handle, "g_unloadlm" );
        dlsym_error = dlerror();
        if( dlsym_error ) {
            fprintf( stderr, "[ERROR]: Cant not load symbol \"g_unloadlm\": %s\n", dlsym_error );
            return NULL;
        }
        /* load "g_get_ngram_prob2" */
        dlerror();
        g_get_ngram_prob2 = (GET_NGRAM_PROB_FUNC) dlsym( handle, "g_get_ngram_prob2" );
        dlsym_error = dlerror();
        if( dlsym_error ) {
            fprintf( stderr, "[ERROR]: Cant not load symbol \"g_get_ngram_prob2\": %s\n", dlsym_error );
            return NULL;
        }
        return handle;        
    }
#endif

    /*************************************
    for the n-gram LM included in NiuTrans
    **************************************/

    NiuTransLM::NiuTransLM(){}
    NiuTransLM::~NiuTransLM(){UnloadModel();}

    void NiuTransLM::LoadModel(const char *lmfn, const char *vfn, unsigned order, int nthrd, int is_mmap, bool limitVocab)
    {
        TimeUtil timer;
        timer.StartTimer();
        fprintf( stderr, "Loading %s\n", "Ngram-LanguageModel-File" );
        fprintf( stderr, "  >> From File: %s ...\n", lmfn );
        fprintf( stderr, "  >> " );
        lmIndex = g_initlm(lmfn, vfn, nthrd, is_mmap);
        timer.EndTimer();
        double time = timer.GetTimerDiff();
        fprintf( stderr, "\nDone [%.3f sec(s)]\n", time );
    }

    void NiuTransLM::UnloadModel()
    {
        g_unloadlm(lmIndex);
    }

    float NiuTransLM::GetProb(int* wid, int beg, int end )
    {
        return g_get_ngram_prob2(lmIndex, wid, beg, end, -1 );
    }

}
