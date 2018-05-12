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
 * Using language models for our SMT system; OurLM.h
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


#ifndef _LANGUAGEMODEL_H_
#define _LANGUAGEMODEL_H_

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "Utilities.h"

using namespace utilities;

#define MAX_NGRAM_ORDER 10

#define CHECK_NEW_FAILED( p )\
	if( p == NULL )\
{\
	printf("Error: Out of memory\n%s:%d\n",__FILE__,__LINE__);\
	exit(0);\
}

namespace smt {

    /* --------------------------------------------------------------------------- */
    /*                              language model                                 */
    /* --------------------------------------------------------------------------- */

typedef int ( *INITLM_FUNC )(const char*, const char*, int, int);
typedef void ( *UNLOADLM_FUNC )(int);
typedef float ( *GET_NGRAM_PROB_FUNC )(int, int*, int, int, int);
    
#ifdef WIN32
    HINSTANCE GetLmFunc();
    #ifdef __MACHINE_TYPE_32__
        #define LM_LIB TEXT("..\\lib\\NiuTrans.LanguageModel.i386.dll")
    #else
        #define LM_LIB TEXT("..\\lib\\NiuTrans.LanguageModel.dll")
    #endif
#else
    void* GetLmFunc();
    #ifdef __MACHINE_TYPE_32__
        #define LM_LIB "libNiuTrans.LanguageModel.i386.so"
    #else
        #define LM_LIB "libNiuTrans.LanguageModel.so"
    #endif
#endif

    class OurLM
    {
    public:
        OurLM(){};
        virtual ~OurLM(){};

        virtual void LoadModel(const char *lmfn, const char *vfn, unsigned order, int nthrd, int is_mmap, bool limitVocab = false) = 0;
        virtual void UnloadModel() = 0;

        virtual float GetProb(int* wid, int beg, int end) = 0; // get n-gram probability, i.e. Pr(w_n|w_1...w_n-1)

        // other methods to create LM and vocab files
    };

    // for the n-gram LM included in NiuTrans
    class NiuTransLM : public OurLM
    {
    private:
        int lmIndex;

    public:
        NiuTransLM();
        ~NiuTransLM();

        void LoadModel(const char *lmfn, const char *vfn, unsigned order, int nthrd, int is_mmap, bool limitVocab = false);
        void UnloadModel();

        float GetProb(int* wid, int beg, int end); // get n-gram probability, i.e. Pr(w_n|w_1...w_n-1)
    };

}

#endif

