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
 * Main.cpp
 *
 * $Version:
 * 1.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); June 22th, 2011; add option "-decoding"
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn); June 15th, 2013; add option "-eval"
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Global.h"
#include "OurLM.h"
#include "OurDecoder_Phrase_ITG.h"
#include "OurDecoder_SCFG.h"
#include "OurTrainer.h"
#include "TrainingAndDecoding.h"
#include "Interface.h"

using namespace smt;

//#define DEBUG_XT

/* to activate debug mode in windows environment, please set macro "DEBUG_XT" */
#ifdef WIN32
    #ifdef DEBUG_XT
        #using <mscorlib.dll>
        #include <crtdbg.h>
    #endif
#endif

void Usage();
void Headline();

int main( int argc, const char ** argv )
{
    #ifdef WIN32
        #ifdef DEBUG_XT
            _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
        #endif
    #endif

    if( argc < 2 )
    {
        Usage();
        return 0;
    }

    /** machine translation (MT) system **/

    /* load language model's DLL */
#ifdef WIN32
    HINSTANCE hInsDll = GetLmFunc();
#else
    void* hInsDll = GetLmFunc();
#endif
    if( hInsDll == NULL )
    {
        fprintf( stderr, "[ERROR]: Language Model Library not existed!\n" );
        return 1;
    }

    /* what do you want to do? decoding, mert or something else? */
    if( strcmp( argv[1], "-decoding" ) == 0 && argc >= 3 )
    {
        Headline();
        ConfigManager.Create( argc, argv );
        const char * outfn = ConfigManager.GetString( "output", NULL );
        const char * logfn = ConfigManager.GetString( "log", NULL );

        SetUseF1Criterion( false );
        SetNormalizationLabel( true );
        OurSystem * s = new OurSystem();

		if(ConfigManager.GetBool("skeleton", false))
			s->Decoding(argv[2], outfn, logfn, SKELETON_BASED, false); 
        else if(ConfigManager.GetBool("scfg", false)){
            if(ConfigManager.GetBool("syntax", false))
                s->Decoding( argv[2], outfn, logfn, SYNTAX_BASED, false);
            else
                s->Decoding( argv[2], outfn, logfn, HIERO, false);
        }
        else
            s->Decoding( argv[2], outfn, logfn, PHRASE_BASED, false);

        delete s;

        ConfigManager.Destroy();
    }
    else if( strcmp( argv[1], "-mert" ) == 0 && argc >= 4 )
    {
        Headline();
        ConfigManager.Create( argc, argv );

        SetUseF1Criterion( false );
        SetNormalizationLabel( true );
        if(ConfigManager.GetBool("charmert", false) && ConfigManager.GetBool("utf8", false))
            SetInternationalTokenization( true );
        
        OurSystem * s = new OurSystem();

        if(ConfigManager.GetBool("skeleton", false))
            s->Training( argv[2], SKELETON_BASED, 0, ( BLEU_TYPE )atoi( argv[3] ) );
        else if(ConfigManager.GetBool("scfg", false)){
            if(ConfigManager.GetBool("syntax", false))
                s->Training( argv[2], SYNTAX_BASED, 0, ( BLEU_TYPE )atoi( argv[3] ) );
            else
                s->Training( argv[2], HIERO, 0, ( BLEU_TYPE )atoi( argv[3] ) );
        }
        else
            s->Training( argv[2], PHRASE_BASED, 0, ( BLEU_TYPE )atoi( argv[3] ) );

        delete s;

        ConfigManager.Destroy();
    }
    else if( strcmp( argv[1], "-mertonly" ) == 0 )
    {
        if( argc == 4 )
        {
            Headline();
            ConfigManager.Create( argc, argv );
            SetUseF1Criterion(true);
            MERTraining( NULL, argv[2], argv[3], 0, 3 );
            ConfigManager.Destroy();
        }
        else if( argc == 6)
        {
            Headline();
            ConfigManager.Create( argc, argv );
            MERTraining(argv[2], argv[3], argv[4], 4, atoi(argv[5]));
            ConfigManager.Destroy();
        }
        else
        { 
            Usage();
        }
    }
	else if( strcmp( argv[1], "-eval" ) == 0 ){
		if( argc > 5 ){
			Headline();
			ConfigManager.Create( argc, argv );
			SetUseF1Criterion( false );
			fprintf(stdout, "BLEU: %.4f (BP = %.4f)\n", 
                                CalculateBLEU(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6])),
                                CalculateBP(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6])));
			ConfigManager.Destroy();
		}
		else{
			Usage();
		}
	}
    else
    { 
        Usage();
    }

    UnloadMERTrainer();

#ifdef WIN32
    ::FreeLibrary( hInsDll );
#else
    dlclose( hInsDll );
#endif

    return 0;
}

void Usage()
{
    fprintf( stderr, "\nNiuTrans.Decoder, version 1.3.0\n" );
    fprintf( stderr, "\n" );
    fprintf( stderr, "Basic usage: NiuTrans.Decoder [params]...\n" );
    fprintf( stderr, "\n" );
    fprintf( stderr, "1. Decoding input sentences\n");
    fprintf( stderr, "   usage: -decoding test.txt [-output /path/to/output-file]\n" );
    fprintf( stderr, "2. MER training (decoding + optimization)\n" );
    fprintf( stderr, "   usage: -mert ref.txt bleu-type[0,1 or 2] [-scfg] [other opts]\n" );
    fprintf( stderr, "3. MER training from nbest lists with Bleu\n");
    fprintf( stderr, "   usage: -mertonly ref.txt nbest.txt config.txt bleu-type[0,1 or 2] [other opts]\n" );
    fprintf( stderr, "4. MER training from nbest lists with F1 score\n");
    fprintf( stderr, "   usage: -mertonly nbest.txt config.txt [other opts]\n" );
    fprintf( stderr, "5. Calculating BLEU\n");
    fprintf( stderr, "   usage: -eval ref.txt nbest.txt #-of-refs ngram bleu-type[0,1 or 2] [other opts]\n");
    fprintf( stderr, "\n" );
}

void Headline()
{
    fprintf( stderr, "\n" );
    fprintf( stderr, "#####################################################################\n" );
    fprintf( stderr, "#   Welcome to NiuTrans' world (version 1.3)  --   www.nlplab.com   #\n" );
    fprintf( stderr, "#####################################################################\n" );
    fprintf( stderr, "\n" );
}

