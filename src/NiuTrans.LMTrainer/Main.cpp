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
 * language model trainer; Main.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Rushan Chen (email: chenrsster@gmail.com)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); July 25th, 2012; split language model functions
 * Hao Zhang (email: zhanghao1216@gmail.com); Oct. 23th, 2011; support 32bit runtime environment
 * Hao Zhang (email: zhanghao1216@gmail.com); July 5th, 2011; explicitly use "dlopen" to load dynamic library
 * Hao Zhang (email: zhanghao1216@gmail.com); June 19th, 2011; add function "usage()"
 *
 */


#include "stdio.h"
#include "string.h"

#ifdef WIN32
    #include <windows.h>
    #ifdef __MACHINE_TYPE_32__
        #define LM_LIB TEXT("..\\lib\\NiuTrans.LanguageModel.i386.dll")
    #else
        #define LM_LIB TEXT("..\\lib\\NiuTrans.LanguageModel.dll")
    #endif
#else
    #include <dlfcn.h>
    #ifdef __MACHINE_TYPE_32__
        #define LM_LIB "libNiuTrans.LanguageModel.i386.so"
    #else
        #define LM_LIB "libNiuTrans.LanguageModel.so"
    #endif
#endif
typedef int (*LM_MAIN_FUNC)(int, const char**);

void usage( const char *cmd ) {
        int i = 0, cnt = 80;
        fprintf( stderr, "NiuTrans.LMTrainer, version 1.0.0 Beta\n" );
        for (i = 0; i < cnt; i++)
            fprintf( stderr, "-" );
        fprintf( stderr, "\n" );
        fprintf( stderr, "[USAGE]:\n" );
        fprintf( stderr, "\t%s <action> [OPTIONS]\n", cmd );
        fprintf( stderr, "[ACTION]:\n" );
        fprintf( stderr, "\tmkvocab: making the vocabulary, with words occurring less then\n"
             "\t         some given times cut off\n" );
        fprintf( stderr, "\tcntngram:counting the ngrams, with ngrams occurring less than\n"
             "\t         some given times cut off\n" );
        fprintf( stderr, "\tmkmodel: making the language model\n" );
        fprintf( stderr, "[OPTION]:\n" );
        fprintf( stderr, "\t-c <num> (default: 1)\n" );
        fprintf( stderr, "\t\tthis option specifies the cutoff value, used in action mkvocab\n"
             "\t\tand cntngram\n" );
        fprintf( stderr, "\t-t <corpus-file> (default: stdin)\n" );
        fprintf( stderr, "\t\tthis option specifies the input corpus file\n" );
        fprintf( stderr, "\t-o <output>\n" );
        fprintf( stderr, "\t\tfor mkvocab action, this option specifies the output vocabulary\n"
             "\t\t\tdefault: vocab\n"
             "\t\tfor cntngram action, it specifies the output ngram counts\n"
             "\t\t\tdefault: count\n"
             "\t\tfor mkmodel action, it specifies the output language model\n"
             "\t\t\tdefault: lm.bin\n" );
        fprintf( stderr, "\t-v <vocab>\n" );
        fprintf( stderr, "\t\tload vocabulary from <vocab>\n" );
        fprintf( stderr, "\t-n <ngram>\n" );
        fprintf( stderr, "\t-m <memory-size> (default 128)\n" );
        fprintf( stderr, "\t\tused for cntngram action, specifying the size of memory\n"
             "\t\tavailable in unit of Megabyte\n" );
        fprintf( stderr, "\t-u <ngram-count-file>\n" );
        fprintf( stderr, "\t\tthis is the file output by cntngram action\n" );
        fprintf( stderr, "\t-C <1-gram_cutoff,2-gram_cutoff,...,N-gram_cutoff>\n" );
        fprintf( stderr, "\t\t(default: 0,0,0,1,1,2,2,2...)\n" );
        fprintf( stderr, "\t\tthe argument of this option is a comma-sperated list of numbers,\n"
             "\t\twith each speifying the cutoff value for 1 to N gram\n" );
        fprintf( stderr, "\t-h\n" );
        fprintf( stderr, "\t\toutput this help info\n" );
        fprintf( stderr, "[EXAMPLE]:\n" );
        fprintf( stderr, "\tTo make vocabulary\n" );
        fprintf( stderr, "\t   %s mkvocab -n 5 -t corpus -o vocab -c 0\n", cmd );
        fprintf( stderr, "\tTo create ngram count file:\n" );
        fprintf( stderr, "\t   %s cntngram -n 5 -v vocab -o count -m 2048 -t "
                                                              "corpus -c 1\n", cmd );
        fprintf( stderr, "\tTo make language model\n" );
        fprintf( stderr, "\t   %s mkmodel -n 5 -v vocab -o lm.bin -u count\n", cmd );
        for (i = 0; i < cnt; i++)
            fprintf( stderr, "-" );
        fprintf( stderr, "\n" );
}

int main( int argc, const char ** argv )
{

    if( argc < 3 ) {
        usage( argv[0] );
        return 0;
    }

    /* load dll */
#ifdef WIN32
    HINSTANCE hInsDll = ::LoadLibrary( LM_LIB );
    if( hInsDll == NULL ) {
        ::FreeLibrary( hInsDll );
        return 1;
    }
    LM_MAIN_FUNC lmmain = (LM_MAIN_FUNC)::GetProcAddress( hInsDll, "lmmain" );
    if( lmmain == NULL ) {
        ::FreeLibrary( hInsDll );
        return 1;
    }
#else
    void* handle = dlopen(LM_LIB, RTLD_LAZY);
    if( handle == NULL ) {
        fprintf( stderr, "[ERROR]: Can not load dynamic library: %s\n", dlerror() );
        return 1;
    }
    /* reset errors */
    dlerror();
    LM_MAIN_FUNC lmmain = (LM_MAIN_FUNC) dlsym( handle, "lmmain" );
    const char* dlsym_error = dlerror();
    if( dlsym_error ) {
        fprintf( stderr, "[ERROR]: Cant not load symbol \"lmmain\": %s\n", dlsym_error );
        return 1;
    }
#endif

    /* train lm model */
    const char* cmd[1024];
		cmd[0] = "exe";
		cmd[1] = "-lm";
		for( int i=1; i < argc; i++  ) {
        cmd[1+i] = argv[i];
		}
		lmmain( argc+1, cmd );

    /* release dll */
#ifdef WIN32
    ::FreeLibrary( hInsDll );
#else
    dlclose(handle);
#endif

    return 0;

}
