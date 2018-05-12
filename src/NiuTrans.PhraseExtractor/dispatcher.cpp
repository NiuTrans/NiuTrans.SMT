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
 * phrase extraction and scorer; dispatcher.cpp
 *
 * $Version:
 * 1.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2012/05/22, add scoring for syntax-rule, modified by Qiang Li.
 * 2011/11/15, add this file, modified by Qiang Li.
 *
 */

#include "dispatcher.h"
using namespace dispatcher;

bool Dispatcher::resolve_parameter( int argc, char * argv[] )
{
    string           function( argv[ 1 ] );
    vector< string > argument             ;

#ifdef WIN32
    string sortFile = "c:\\cygwin\\bin\\sort.exe ";
    string del      = "del "                      ;
#else
    string sortFile = "LC_ALL=C sort ";
    string del      = "rm "           ;
#endif

    for( int i = 2; i != argc; ++i )
    {
#ifdef __DEBUG__
        cerr<<argv[i]<<"\n"<<flush;
#endif

        argument.push_back( argv[ i ] );
#ifdef WIN32
        string::size_type pos = 0;
        while( ( pos = argument.back().find( "/" ) ) != string::npos  )
        {
            argument.back().replace( pos, 1, "\\" );
        }
#endif
    }

    map< string, string > param;    
    for( vector< string >::iterator iter = argument.begin(); iter != argument.end(); ++iter )
    {
        string temp1( *iter   );
        string temp2( *++iter );

        param.insert( make_pair( temp1, temp2 ) );
    }

    if( param.find( "-sort" ) != param.end() )
    {
        sortFile = param[ "-sort" ];
        sortFile.push_back( ' ' );
    }

    if( param.find( "-temp" ) != param.end() )
    {
        sortFile += "-T " + param[ "-temp" ];
        sortFile.push_back( ' ' );
    }
    
#ifdef WIN32   
    string::size_type pos = 0;
    while( ( pos = sortFile.find( "/" ) ) != string::npos )
    {
        sortFile.replace( pos, 1, "\\" );
    }
#endif

#ifdef __DEBUG__
    cerr<<"Did I come here? Yes.\n"<<flush;
#endif

    return identify_function( function, param, sortFile, del );
}

bool Dispatcher::identify_function( 
string                &function, 
map< string, string > &param   ,
string                &sortFile,
string                &del       )
{
    if( function      == "--LEX"      )
        generateLexicalTable_function(   param, sortFile, del );
    else if( function == "--EXTP"     )
        phraseExtract_function(          param                );
    else if( function == "--EXTH"     )
        extractHieroRule_function(       param                );
    else if( function == "--SCORE"    )
        score_function(                  param, sortFile, del );
    else if( function == "--SCORESYN" )
        scoringforSynRule_function(      param, sortFile, del );
    else if( function == "--FILTN"    )
        filterTableWithNum_function(     param                );
    else if( function == "--FILPD"    )
        filterPhraseTabWithDev_function( param                );
    else
    {
        cerr<<"Error: Function error!\n";
        return false;
    }
    return true;
}

void Dispatcher::generateLexicalTable_function(
map< string, string > &param   ,
string                &sortFile,
string                &del       )
{
    if( param.find( "-nthread" ) == param.end() )
        param.insert( make_pair( "-nthread", "0" ) );

    if( param.find( "-src" ) != param.end() &&
        param.find( "-tgt" ) != param.end() && 
        param.find( "-aln" ) != param.end() && 
        param.find( "-out" ) != param.end() )
    {

        OptionsOfLexTable options;
        options.nthread = atoi( param[ "-nthread" ].c_str() );
        options.sourceLangFileName  = param[ "-src" ];
        options.targetLangFileName  = param[ "-tgt" ];
        options.alignedInfoFileName = param[ "-aln" ];
#ifdef __DEBUG__
        cerr<<"        nthread="<<options.nthread<<endl;
        cerr<<"        srcFileName="<<options.sourceLangFileName<<endl;
        cerr<<"        tgtFileName="<<options.targetLangFileName<<endl;
        cerr<<"        alnFileName="<<options.alignedInfoFileName<<endl;
#endif

        if( param.find( "-stem" ) == param.end() )
        {
            string temp = "null";            
            Dispatcher dispatcher;
            SystemCommand systemCommand( sortFile, del );
            dispatcher.call_generateLexicalTable( param["-out"], 
                                                  temp         ,
                                                  options      ,
                                                  systemCommand  );
        }
        else
        {
            Dispatcher dispatcher;
            SystemCommand systemCommand( sortFile, del );
            dispatcher.call_generateLexicalTable( param["-out"] , 
                                                  param["-stem"], 
                                                  options       ,
                                                  systemCommand   );
        }
    }
    else
    {
        cerr<<"ERROR: Please check your parameter!\n"<<flush;
        exit( 0 );
    }    
}

void Dispatcher::phraseExtract_function( map< string, string > &param )
{
    if( param.find( "-srclen" ) == param.end() )
        param.insert( make_pair( "-srclen", "3" ) );
    if( param.find( "-tgtlen" ) == param.end() )
        param.insert( make_pair( "-tgtlen", "5" ) );
	if( param.find( "-maxSrcNulExtNum" ) == param.end() )
		param.insert( make_pair( "-maxSrcNulExtNum", param[ "-srclen" ] ) );
	if( param.find( "-maxTgtNulExtNum" ) == param.end() )
		param.insert( make_pair( "-maxTgtNulExtNum", param[ "-tgtlen" ] ) );
    if( param.find( "-null" )   == param.end() )
        param.insert( make_pair( "-null",   "1" ) );
    if( param.find( "-allsent" )== param.end() )
        param.insert( make_pair( "-allsent","0" ) );

	if( param.find( "-method" ) == param.end() )
		param.insert( make_pair( "-method", "heuristic" ) );

    if( param[ "-method" ] != "heuristic" && param[ "-method" ] != "compose" )
		param[ "-method" ] = "heuristic";

	if( param[ "-method" ] == "compose" )
	{
		if( param.find( "-compose" ) == param.end() )
		{
			param.insert( make_pair( "-compose", "2" ) );
		}
	}

    if( param.find( "-src" )    != param.end() &&
        param.find( "-tgt" )    != param.end() &&
        param.find( "-aln" )    != param.end() &&
        param.find( "-out" )    != param.end() &&
        param.find( "-srclen" ) != param.end() &&
        param.find( "-tgtlen" ) != param.end()    )
    {
        OptionsOfPhraseTable options;
        options.srclen = atoi( param[ "-srclen" ].c_str() );
        options.tgtlen = atoi( param[ "-tgtlen" ].c_str() );

		options.maxSrcNulExtNum = atoi( param[ "-maxSrcNulExtNum" ].c_str() );
		options.maxTgtNulExtNum = atoi( param[ "-maxTgtNulExtNum" ].c_str() );

        options.srcFileName = param[ "-src" ];
        options.tgtFileName = param[ "-tgt" ];
        options.alnFileName = param[ "-aln" ];
        options.outFileName = param[ "-out" ];
        
		options.nullFlag    = ( param[ "-null" ]    == "0" ? false : true );
		options.allSentFlag = ( param[ "-allsent" ] == "0" ? false : true );

		options.method = param[ "-method" ];

		if( options.method == "compose" )
			options.compose = atoi( param[ "-compose" ].c_str() );
		else
			options.compose = 0;

        Dispatcher dispatcher;

        dispatcher.call_phraseExtract( options );
    }
    else
    {
        cerr<<"ERROR: Please check your parameter!\n"<<flush;
        exit( 0 );    
    }
}

void Dispatcher::extractHieroRule_function( map< string, string > &param )
{
    OptionsOfHiero options;
    if( param.find( "-srclen" )               == param.end()                   )
        param.insert( make_pair( "-srclen",               "10"               ) );
    if( param.find( "-tgtlen" )               == param.end()                   )
        param.insert( make_pair( "-tgtlen",               "10"               ) );
    if( param.find( "-maxSrcI" )              == param.end()                   )
        param.insert( make_pair( "-maxSrcI",              "3"                ) );
    if( param.find( "-maxTgtI" )              == param.end()                   )
        param.insert( make_pair( "-maxTgtI",              "5"                ) );
    if( param.find( "-maxSrcH" )              == param.end()                   )
        param.insert( make_pair( "-maxSrcH",              "5"                ) );
    if( param.find( "-maxTgtH" )              == param.end()                   )
        param.insert( make_pair( "-maxTgtH",              param[ "-tgtlen" ] ) );
    if( param.find( "-maxNulExtSrcHieroNum" ) == param.end()                   )
        param.insert( make_pair( "-maxNulExtSrcHieroNum", param[ "-srclen" ] ) );
    if( param.find( "-maxNulExtTgtHieroNum" ) == param.end()                   )
        param.insert( make_pair( "-maxNulExtTgtHieroNum", param[ "-tgtlen" ] ) );
    if( param.find( "-maxNulExtSrcInitNum" )  == param.end()                   )
        param.insert( make_pair( "-maxNulExtSrcInitNum",  param[ "-srclen" ] ) );
    if( param.find( "-maxNulExtTgtInitNum" )  == param.end()                   )
        param.insert( make_pair( "-maxNulExtTgtInitNum",  param[ "-tgtlen" ] ) );
    if( param.find( "-null" )                 == param.end()                   )
        param.insert( make_pair( "-null",                 "1"                ) );
    if( param.find( "-headnonterm" )          == param.end()                   )
        param.insert( make_pair( "-headnonterm",          "1"                ) );
    if( param.find( "-maxnonterm" )           == param.end()                   )
        param.insert( make_pair( "-maxnonterm",           "2"                ) );
    if( param.find( "-minSrcSubPhrase" )      == param.end()                   )
        param.insert( make_pair( "-minSrcSubPhrase",      "1"                ) );
    if( param.find( "-minTgtSubPhrase" )      == param.end()                   )
        param.insert( make_pair( "-minTgtSubPhrase",      "1"                ) );
    if( param.find( "-minLexiNum" )           == param.end()                   )
        param.insert( make_pair( "-minLexiNum",           "1"                ) );
    if( param.find( "-srcNonTermAdjacent" )   == param.end()                   )
        param.insert( make_pair( "-srcNonTermAdjacent",   "0"                ) );
    if( param.find( "-tgtNonTermAdjacent" )   == param.end()                   )
        param.insert( make_pair( "-tgtNonTermAdjacent",   "1"                ) );
    if( param.find( "-alignedLexiReq" )       == param.end()                   )
        param.insert( make_pair( "-alignedLexiReq",       "1"                ) );
    if( param.find( "-duplicateHieroRule" )   == param.end()                   )
        param.insert( make_pair( "-duplicateHieroRule",   "0"                ) );
    if( param.find( "-unalignedEdgeHiero" )   == param.end()                   )
        param.insert( make_pair( "-unalignedEdgeHiero",   "0"                ) );
    if( param.find( "-unalignedEdgeInit" )    == param.end()                   )
        param.insert( make_pair( "-unalignedEdgeInit",    "1"                ) );

    //add in 2012/03/29
    if( param.find( "-srcParseTreePath" ) == param.end() )
        param.insert( make_pair( "-srcParseTreeFlag", "0" ) );
    else
    {
        param.insert( make_pair( "-srcParseTreeFlag", "1" ) );
        param.insert( make_pair( "-srcParseTreePath", param[ "-srcParseTreePath" ] ) );
    }
    
    if( param.find( "-tgtParseTreePath" ) == param.end() )
        param.insert( make_pair( "-tgtParseTreeFlag", "0" ) );
    else
    {
        param.insert( make_pair( "-tgtParseTreeFlag", "1" ) );
        param.insert( make_pair( "-tgtParseTreePath", param[ "-tgtParseTreePath" ] ) );
    }
    //add in 2012/03/29

    if( param.find( "-src" )                != param.end() && param.find( "-tgt" )                != param.end() &&
        param.find( "-aln" )                != param.end() && param.find( "-out" )                != param.end() &&
        param.find( "-srclen" )             != param.end() && param.find( "-tgtlen" )             != param.end() &&
        param.find( "-maxSrcI" )            != param.end() && param.find( "-maxTgtI" )            != param.end() &&
        param.find( "-maxSrcH" )            != param.end() && param.find( "-maxTgtH" )            != param.end() &&
        param.find( "-null" )               != param.end() && param.find( "-headnonterm" )        != param.end() &&
        param.find( "-maxnonterm" )         != param.end() && param.find( "-minSrcSubPhrase" )    != param.end() &&
        param.find( "-minTgtSubPhrase" )    != param.end() && param.find( "-minLexiNum" )         != param.end() &&
        param.find( "-srcNonTermAdjacent" ) != param.end() && param.find( "-tgtNonTermAdjacent" ) != param.end() &&
        param.find( "-alignedLexiReq" )     != param.end() && param.find( "-duplicateHieroRule" ) != param.end() &&
        param.find( "-unalignedEdgeHiero" ) != param.end() && param.find( "-unalignedEdgeInit" )  != param.end()    )
    {
        options.maxSourcePhraseSize   = ( atoi( param[ "-srclen" ].c_str()          ) > 0 ? atoi( param[ "-srclen" ].c_str()          ) : 10                          );
        options.maxTargetPhraseSize   = ( atoi( param[ "-tgtlen" ].c_str()          ) > 0 ? atoi( param[ "-tgtlen" ].c_str()          ) : 10                          );
        options.maxSourceInitSize     = ( atoi( param[ "-maxSrcI" ].c_str()         ) > 0 ? atoi( param[ "-maxSrcI" ].c_str()         ) : 3                           );
        options.maxTargetInitSize     = ( atoi( param[ "-maxTgtI" ].c_str()         ) > 0 ? atoi( param[ "-maxTgtI" ].c_str()         ) : 5                           );
        options.maxSourceHieroSize    = ( atoi( param[ "-maxSrcH" ].c_str()         ) > 0 ? atoi( param[ "-maxSrcH" ].c_str()         ) : 5                           );
        options.maxTargetHieroSize    = ( atoi( param[ "-maxTgtH" ].c_str()         ) > 0 ? atoi( param[ "-maxTgtH" ].c_str()         ) : options.maxTargetPhraseSize );
        options.minSourceSubPhrase    = ( atoi( param[ "-minSrcSubPhrase" ].c_str() ) > 0 ? atoi( param[ "-minSrcSubPhrase" ].c_str() ) : 1                           );
        options.minTargetSubPhrase    = ( atoi( param[ "-minTgtSubPhrase" ].c_str() ) > 0 ? atoi( param[ "-minTgtSubPhrase" ].c_str() ) : 1                           );
        options.maxNonTermNum         = ( atoi( param[ "-maxnonterm" ].c_str()      ) > 0 ? atoi( param[ "-maxnonterm" ].c_str()      ) : 2                           );
        options.minLexiNumOfHieroRule = ( atoi( param[ "-minLexiNum" ].c_str()      ) > 0 ? atoi( param[ "-minLexiNum" ].c_str()      ) : 1                           );

        options.maxNulExtSrcHieroNum  = ( atoi( param[ "-maxNulExtSrcHieroNum" ].c_str() ) >= 0 ? atoi( param[ "-maxNulExtSrcHieroNum" ].c_str() ) : options.maxSourcePhraseSize );
        options.maxNulExtTgtHieroNum  = ( atoi( param[ "-maxNulExtTgtHieroNum" ].c_str() ) >= 0 ? atoi( param[ "-maxNulExtTgtHieroNum" ].c_str() ) : options.maxTargetPhraseSize );
        options.maxNulExtSrcInitNum   = ( atoi( param[ "-maxNulExtSrcInitNum" ].c_str()  ) >= 0 ? atoi( param[ "-maxNulExtSrcInitNum" ].c_str()  ) : options.maxSourcePhraseSize );
        options.maxNulExtTgtInitNum   = ( atoi( param[ "-maxNulExtTgtInitNum" ].c_str()  ) >= 0 ? atoi( param[ "-maxNulExtTgtInitNum" ].c_str()  ) : options.maxTargetPhraseSize );

        if( options.maxNulExtSrcHieroNum > options.maxNulExtSrcInitNum )
            options.maxNulExtSrcHieroNum = options.maxNulExtSrcInitNum;
        if( options.maxNulExtTgtHieroNum > options.maxNulExtTgtInitNum )
            options.maxNulExtTgtHieroNum = options.maxNulExtTgtInitNum;

        options.headNonTermFlag         = ( atoi( param[ "-headnonterm" ].c_str() )        == 0 ) ? false : true;
        options.nullFuncFlag            = ( atoi( param[ "-null" ].c_str() )               == 0 ) ? false : true;
        options.srcNonTermAdjacentFlag  = ( atoi( param[ "-srcNonTermAdjacent" ].c_str() ) == 0 ) ? false : true;
        options.tgtNonTermAdjacentFlag  = ( atoi( param[ "-tgtNonTermAdjacent" ].c_str() ) == 0 ) ? false : true;
        options.alignedLexiRequiredFlag = ( atoi( param[ "-alignedLexiReq" ].c_str() )     == 0 ) ? false : true;
        options.duplicateHieroRuleFlag  = ( atoi( param[ "-duplicateHieroRule" ].c_str() ) == 0 ) ? false : true;
        options.unalignedEdgeInitFlag   = ( atoi( param[ "-unalignedEdgeInit" ].c_str() )  == 0 ) ? false : true;
        options.unalignedEdgeHieroFlag  = ( atoi( param[ "-unalignedEdgeHiero" ].c_str() ) == 0 ) ? false : true;

        //add in 2012/03/29
        options.srcParseTreeFlag = ( atoi( param[ "-srcParseTreeFlag" ].c_str() ) == 0 ) ? false : true;
        options.tgtParseTreeFlag = ( atoi( param[ "-tgtParseTreeFlag" ].c_str() ) == 0 ) ? false : true;
        if( options.srcParseTreeFlag )
            options.srcParseTreePath = param[ "-srcParseTreePath" ];
        if( options.tgtParseTreeFlag )
            options.tgtParseTreePath = param[ "-tgtParseTreePath" ];
        //add in 2012/03/29

        if( !options.unalignedEdgeInitFlag )
            options.unalignedEdgeHieroFlag = false;
        Dispatcher dispatcher;
        dispatcher.call_extractHieroRule( param[ "-src" ], param[ "-tgt" ], param[ "-aln" ], param[ "-out" ], options );
    }
    else
    {
        cerr<<"ERROR: Please check your parameter!\n"<<flush;
        exit( 0 );    
    }
}

void Dispatcher::score_function( 
map< string, string > &param   ,
string                &sortFile,
string                &del       )
{
    if( param.find( "-cutoffInit"  ) == param.end() )
        param.insert( make_pair( "-cutoffInit"  , "0"    ) );
    if( param.find( "-cutoffHiero" ) == param.end() )
        param.insert( make_pair( "-cutoffHiero" , "1"    ) );
    if( param.find( "-method"      ) == param.end() )
        param.insert( make_pair( "-method"      , "lexw" ) );
    if( param.find( "-printAlign"  ) == param.end() )
        param.insert( make_pair( "-printAlign"  , "0"    ) );
    if( param.find( "-printFreq"   ) == param.end() )
        param.insert( make_pair( "-printFreq"   , "0"    ) );
    if( param.find( "-generInvFile" ) == param.end() )
        param.insert( make_pair( "-generInvFile", "0"    ) );
    if( param[ "-method" ] != "lexw"    && 
        param[ "-method" ] != "noisyor" && 
        param[ "-method" ] != "merge"      )
        param[ "-method" ]  = "lexw";

	// by wangqiang
	if( param.find( "-sortPhrase") == param.end() )
		param.insert( make_pair("-sortPhrase" , "1"));

#ifdef __DEBUG__
    cerr<<"**param -method:"<<param[ "-method" ]<<"**"<<endl;
#endif

    if( param.find( "-tab"          ) != param.end() &&
        param.find( "-ls2d"         ) != param.end() &&
        param.find( "-tabinv"       ) != param.end() &&
        param.find( "-ld2s"         ) != param.end() &&
        param.find( "-out"          ) != param.end() &&
        param.find( "-cutoffInit"   ) != param.end() &&
        param.find( "-cutoffHiero"  ) != param.end() &&
        param.find( "-printAlign"   ) != param.end() &&
        param.find( "-printFreq"    ) != param.end() &&
        param.find( "-generInvFile" ) != param.end() &&
        param.find( "-method"       ) != param.end() &&
		param.find( "-sortPhrase"         ) != param.end() )
    {
        Dispatcher dispatcher;
        SystemCommand systemCommand( sortFile, del );
        int  cutoffInit       = ( atoi( param[ "-cutoffInit"  ].c_str() ) >= 0 ? atoi( param[ "-cutoffInit"  ].c_str() ) : 0 );
        int  cutoffHiero      = ( atoi( param[ "-cutoffHiero" ].c_str() ) >= 0 ? atoi( param[ "-cutoffHiero" ].c_str() ) : 1 );
        bool printFreqFlag    = ( param[ "-printFreq"    ] == "1" ? true : false );
        bool printAlignFlag   = ( param[ "-printAlign"   ] == "1" ? true : false );
        bool generInvFileFlag = ( param[ "-generInvFile" ] == "1" ? true : false );
		bool is_sort_phrase_table = (param["-sortPhrase"] == "1" ? true : false);

        OptionsOfScore options( cutoffInit, cutoffHiero, printFreqFlag, printAlignFlag, generInvFileFlag , is_sort_phrase_table );

        string phraseTable = param[ "-out" ] + ".half";
        bool inverseFlag = false;
        dispatcher.call_score( param[ "-tab"    ], 
                               param[ "-ls2d"   ], 
                               phraseTable       , 
                               inverseFlag       , 
                               options           , 
                               param[ "-method" ], 
                               systemCommand       );

        inverseFlag = true;                           // scoring for inverse table
        dispatcher.call_score( param[ "-tabinv" ], 
                               param[ "-ld2s"   ], 
                               phraseTable       , 
                               inverseFlag       , 
                               options           , 
                               param[ "-method" ], 
                               systemCommand       );

        string phraseTableSorted    = ( phraseTable + ".sorted"     );
        string phraseTableInvSorted = ( phraseTable + ".inv.sorted" );

        dispatcher.call_consolidate( phraseTableSorted   , 
                                     phraseTableInvSorted, 
                                     param[ "-out" ]     , 
                                     options             , 
                                     systemCommand         );
    }
    else
    {
        cerr<<"ERROR: Please check your parameters!\n"<<flush;
        exit( 0 );                
    }
}

// add in 2012-05-22, scoring for syntax-rule.
void Dispatcher::scoringforSynRule_function( 
map< string, string > &param   ,
string                &sortFile,
string                &del       )
{
#ifdef __DEBUG__
    cerr<<"scoringforSynRule_function:step1\n"<<flush;
#endif

    if( param.find( "-model"  ) == param.end() )
        param.insert( make_pair( "-model"    , "t2s"    ) );

    if( param.find( "-cutoff" ) == param.end() )
        param.insert( make_pair( "-cutoff"   , "0"      ) );

    if( param.find( "-spormy" ) == param.end() )
        param.insert( make_pair( "-spormy"   , "memory" ) );

    if( param.find( "-lowerfreq" ) == param.end() )
        param.insert( make_pair( "-lowerfreq", "3"      ) );

#ifdef __DEBUG__
    cerr<<"-rule     :"<<param[ "-rule"      ]<<"\n"
        <<"-ls2d     :"<<param[ "-ls2d"      ]<<"\n"
        <<"-ld2s     :"<<param[ "-ld2s"      ]<<"\n"
        <<"-out      :"<<param[ "-out"       ]<<"\n"
        <<"-model    :"<<param[ "-model"     ]<<"\n"
        <<"-cutoff   :"<<param[ "-cutoff"    ]<<"\n"
        <<"-spormy   :"<<param[ "-spormy"    ]<<"\n"
        <<"-lowerfreq:"<<param[ "-lowerfreq" ]<<"\n"
        <<"-sort     :"<<sortFile             <<"\n"
        <<"delcomd   :"<<del                  <<"\n"
        <<flush;
#endif

    if( param.find( "-rule"      ) != param.end() &&
        param.find( "-ls2d"      ) != param.end() &&
        param.find( "-ld2s"      ) != param.end() &&
        param.find( "-out"       ) != param.end() &&
        param.find( "-model"     ) != param.end() &&
        param.find( "-cutoff"    ) != param.end() &&
        param.find( "-spormy"    ) != param.end() &&
        param.find( "-lowerfreq" ) != param.end()    )
    {
#ifdef __DEBUG__
        cerr<<"scoringforSynRule_function:step2\n"<<flush;
#endif

        SystemCommand systemCommand( sortFile, del );

#ifdef __DEBUG__
        cerr<<"SystemCommand.sortFile:"<<systemCommand.sortFile<<"\n"<<flush;
        cerr<<"SystemCommand.del     :"<<systemCommand.del     <<"\n"<<flush;
#endif
        int    cutoff        = ( atoi( param[ "-cutoff" ].c_str()    ) >= 0 ? atoi( param[ "-cutoff" ].c_str()    ) : 0 );
        int    lowerFreq     = ( atoi( param[ "-lowerfreq" ].c_str() ) >= 0 ? atoi( param[ "-lowerfreq" ].c_str() ) : 0 );
        string model         = param[ "-model" ];
        string speedormemory = param[ "-spormy" ];
        OptionsOfScoringforSynRule options( cutoff, model, speedormemory, lowerFreq );

#ifdef __DEBUG__
        cerr<<"options.cutoff   :"<<options.cutoff       <<"\n"<<flush;
        cerr<<"options.model    :"<<options.model        <<"\n"<<flush;
        cerr<<"options.spormy   :"<<options.speedorMemory<<"\n"<<flush;
        cerr<<"options.lowerFreq:"<<options.lowerFreq    <<"\n"<<flush;
#endif
        Dispatcher dispatcher;
        dispatcher.call_scoringforSynRule( param, options, systemCommand ); 

    }

    return;
}

void Dispatcher::filterTableWithNum_function( map< string, string > &param )
{
    if( param.find( "-strict"      ) == param.end() )
    {
        param.insert( make_pair( "-strict"     , "30"   ) );
    }
    if( param.find( "-tableFormat" ) == param.end() )
    {
        param.insert( make_pair( "-tableFormat", "auto" ) );
    }
    if( param.find( "-in"          ) != param.end() && 
        param.find( "-out"         ) != param.end() && 
        param.find( "-strict"      ) != param.end() &&
        param.find( "-tableFormat" ) != param.end()    )
    {
        param[ "-tableFormat" ] = ( param[ "-tableFormat" ] == "auto"      || 
                                    param[ "-tableFormat" ] == "phrase"    || 
                                    param[ "-tableFormat" ] == "hierarchy" || 
                                    param[ "-tableFormat" ] == "syntax"    || 
                                    param[ "-tableFormat" ] == "SAMT"         ) ? 
                                    param[ "-tableFormat" ] :  "auto"             ;

        int strictNum = atoi( param[ "-strict" ].c_str() );
        OptionsOfFilterNum options( param[ "-in"          ], 
                                    param[ "-out"         ], 
                                    strictNum              , 
                                    param[ "-tableFormat" ]  );
        Dispatcher dispatcher;
        dispatcher.call_filterTableWithNum( options );
    }
    else
    {
        cerr<<"Error: parameter is not right!\n";
        exit( 0 );
    }
}

void Dispatcher::filterPhraseTabWithDev_function( map<string,string> &param )
{
    if( param.find( "-maxlen" ) == param.end() )
        param.insert( make_pair( "-maxlen", "10" ) );
    if( param.find( "-rnum"   ) == param.end() )
        param.insert( make_pair( "-rnum"  , "4"  ) );

    int maxlen = atoi( param[ "-maxlen" ].c_str() );
    int rnum   = atoi( param[ "-rnum"   ].c_str() );

    if( param.find( "-dev" ) != param.end() && 
        param.find( "-in"  ) != param.end() && 
        param.find( "-out" ) != param.end()    )
    {
        Dispatcher dispatcher;
        dispatcher.call_filterPhraseTabWithDev( param[ "-dev" ], 
                                                param[ "-in"  ], 
                                                param[ "-out" ], 
                                                maxlen         , 
                                                rnum             );
    }
    else
    {
        cerr<<"ERROR: Please check your parameter!\n"<<flush;
        exit( 0 );
    }
}

bool Dispatcher::call_generateLexicalTable(
string        &lexFileName  , 
string        &stem         ,
OptionsOfLexTable &options  ,
SystemCommand &systemCommand  )
{
    cerr<<"\n####################################################################\n"
        <<  "#   Generate lexical table ( version 1.1.0 )  --  www.nlplab.com   #\n"
        <<  "####################################################################\n"
        <<flush;

    clock_t start,finish;

    options.lexTabS2TFileName = lexFileName + ".s2d";
    options.lexTabT2SFileName = lexFileName + ".d2s";

    start = clock();

#ifdef __DEBUG__
    cerr<<"        nthread="<<options.nthread<<endl;
    cerr<<"        srcFileName="<<options.sourceLangFileName<<endl;
    cerr<<"        tgtFileName="<<options.targetLangFileName<<endl;
    cerr<<"        alnFileName="<<options.alignedInfoFileName<<endl;
    cerr<<"        lexs2t="<<options.lexTabS2TFileName<<endl;
    cerr<<"        lext2s="<<options.lexTabT2SFileName<<endl;
#endif

    cerr<<"Generate lexical table\n"
        <<"        input   : "<<options.sourceLangFileName <<"\n"
        <<"                  "<<options.targetLangFileName <<"\n"
        <<"                  "<<options.alignedInfoFileName<<"\n"
        <<"        output  : "<<options.lexTabS2TFileName  <<"\n"
        <<"                  "<<options.lexTabT2SFileName  <<"\n"
        <<flush;

    GenerateLexicalTable glt;
    if( stem == "null" )
       glt.generateLexicalTable( options );
    else
       glt.generateLexicalTable( options, stem );

    finish = clock();
    cerr<<"\n"
        <<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;
    
    string commandSort1 = systemCommand.sortFile + options.lexTabS2TFileName + " > " + options.lexTabS2TFileName + ".sorted";
    string commandSort2 = systemCommand.sortFile + options.lexTabT2SFileName + " > " + options.lexTabT2SFileName + ".sorted";

    start = clock();
    cerr<<"Sort\n"
        <<"        command : "<<commandSort1<<"\n"
        <<"        input   : "<<options.lexTabS2TFileName <<"\n"
        <<"        output  : "<<options.lexTabT2SFileName <<".sorted\n"
        <<flush;

    system( commandSort1.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;
    start = clock();
    cerr<<"Sort\n"
        <<"        command : "<<commandSort2<<"\n"
        <<"        input   : "<<options.lexTabT2SFileName <<"\n"
        <<"        output  : "<<options.lexTabT2SFileName <<".sorted\n"
        <<flush;

    system( commandSort2.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;

    string commandDel1 = systemCommand.del + options.lexTabS2TFileName;
    string commandDel2 = systemCommand.del + options.lexTabT2SFileName;

    start = clock();
    cerr<<"Delete\n"
        <<"        command : "<<commandDel1<<"\n"
        <<"        input   : "<<options.lexTabS2TFileName<<"\n"
        <<flush;
    system( commandDel1.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;
    
    start = clock();    
    cerr<<"Delete\n"
        <<"        command : "<<commandDel2<<"\n"
        <<"        input   : "<<options.lexTabT2SFileName<<"\n"
        <<flush;
    system( commandDel2.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n\n"<<flush;
    return true;
}

bool Dispatcher::call_phraseExtract( OptionsOfPhraseTable &options )
{
    cerr<<"\n####################################################################\n"
        <<  "#   Extract phrase table (version 1.1.0)      --www.nlplab.com     #\n"
        <<  "####################################################################\n"
        <<flush;

    if( options.srclen <= 0 || options.tgtlen <= 0 )
    {
        cerr<<"\n"
            <<"ERROR: The max phrase size is too short, please reset it!\n"<<flush;
        exit( 1 );
    }

    ifstream infile_c(         options.srcFileName.c_str() );
    ifstream infile_e(         options.tgtFileName.c_str() );
    ifstream infile_alignment( options.alnFileName.c_str() );

    if( !infile_c || !infile_e || !infile_alignment )
    {
        cerr<<"\n"
            <<"ERROR: Please validate the input files that in your dir!\n"
            <<"  Src File Name: "  <<options.srcFileName<<"\n"
            <<"  Tgt File Name: "  <<options.tgtFileName<<"\n"
            <<"  Aln File Name: "  <<options.alnFileName<<"\n"
            <<flush;
        exit( 1 );
    }

    string line_c, line_e, line_alignment;
    size_t lineNo = 0;
/*
    string ofileExtractPhrasesPair(    extractResult );
    string ofileExtractPhrasesPairInv( extractResult );
*/
    string ofileExtractPhrasesPair(    options.outFileName );
    string ofileExtractPhrasesPairInv( options.outFileName );

    ofileExtractPhrasesPairInv += ".inv";

    ofstream outfileExtractPhrasesPair(    ofileExtractPhrasesPair.c_str()    );
    ofstream outfileExtractPhrasesPairInv( ofileExtractPhrasesPairInv.c_str() );
    if( !outfileExtractPhrasesPair || !outfileExtractPhrasesPair )
    {
        cerr<<"\n"
            <<"ERROR: Please validate the output files that in your dir!\n"
            <<"  Extract file: "      <<ofileExtractPhrasesPair   <<"\n"
            <<"Extract file inverse: "<<ofileExtractPhrasesPairInv<<"\n"
            <<flush;
        exit( 1 );
    }
    clock_t start,finish;
    start = clock();

    cerr<<"Parameters\n"
        <<"                   -srclen    :    "<<options.srclen         <<"\n"
        <<"                   -tgtlen    :    "<<options.tgtlen         <<"\n"
		<<"                     -null    :    "<<( options.nullFlag    == true ? "true" : "false" )<<"\n"
		<<"                  -allsent    :    "<<( options.allSentFlag == true ? "true" : "false" )<<"\n"
		<<"          -maxSrcNulExtNum    :    "<<options.maxSrcNulExtNum<<"\n"
		<<"          -maxTgtNulExtNum    :    "<<options.maxTgtNulExtNum<<"\n"
		<<"                   -method    :    "<<options.method         <<"\n";

	if( options.method == "compose" )
	{
		cerr<<"                  -compose    :    "<<options.compose<<"\n";
	}

	cerr<<"\n";
	


    cerr<<"Extract phrase table\n"
        <<"        input : "<<options.srcFileName       <<"\n"
        <<"                "<<options.tgtFileName       <<"\n"
        <<"                "<<options.alnFileName       <<"\n"
        <<"        output: "<<ofileExtractPhrasesPair   <<"\n"
        <<"                "<<ofileExtractPhrasesPairInv<<"\n"
        <<flush;

    RuleNum ruleNum;
    while( getline( infile_c        , line_c         ) && 
           getline( infile_e        , line_e         ) && 
           getline( infile_alignment, line_alignment )    )
    {
        BasicMethod be;
        be.clearIllegalChar( line_c         );
        be.clearIllegalChar( line_e         );
        be.clearIllegalChar( line_alignment );

        ++ lineNo;

        if( options.allSentFlag == true )
        {
            BasicMethod be;
            outfileExtractPhrasesPair<<line_c<<" ||| "<<line_e<<" ||| "<<line_alignment<<"\n";
            be.convertAlignmenttoInv( line_alignment );
            outfileExtractPhrasesPairInv<<line_e<<" ||| "<<line_c<<" ||| "<<line_alignment<<"\n";
        }
        else if( options.allSentFlag == false )
        {
            WordAlignment wordalignment;
            PhrasesPair phrasepair;
            bool return_value = wordalignment.createWordAlignment( line_e, line_c, line_alignment, lineNo );
            if( return_value )
			{
				phrasepair.extractPhrasesPair( wordalignment               , 
                                               outfileExtractPhrasesPair   , 
                                               outfileExtractPhrasesPairInv, 
                                               options                     ,
                                               ruleNum                       );
			}
            else
			{
                continue;
			}
        }
        if( lineNo % 10000 == 0 )
            cerr<<"\r        processed "<<lineNo<<" lines."<<flush;
    }
    cerr<<"\r        processed "<<lineNo<<" lines.\n"<<flush;

    finish = clock();
    cerr<<"        time  : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n\n"<<flush;

    if( options.allSentFlag == true )
    {
        cerr<<"Final Results\n"
            <<"        Extracted Rule Number: "<<lineNo<<"\n"<<flush;
    }
    else if( options.allSentFlag == false )
    {
        cerr<<"Final Results\n"
            <<"        Initial Rule Number   :   "<<ruleNum.initialRuleNum<<"\n"
            <<"        NULL    Rule Number   :   "<<ruleNum.nullRuleNum   <<"\n"
            <<"        Total   Rule Number   :   "<<( ruleNum.initialRuleNum + ruleNum.nullRuleNum )<<"\n"
            <<flush;
    }
    outfileExtractPhrasesPair.close();
    outfileExtractPhrasesPairInv.close();

    return true;
}

bool Dispatcher::call_extractHieroRule
( string         &sourceFile    , 
  string         &destFile      , 
  string         &alignedFile   , 
  string         &extractResult , 
  OptionsOfHiero &options        )
{
    if( options.srcParseTreeFlag || options.tgtParseTreeFlag )
        cerr<<"\n####################################################################\n"
            <<  "#    Extract SAMT Rules ( version 2.0 )     --  www.nlplab.com     #\n"
            <<  "####################################################################\n"
            <<flush;
    else
        cerr<<"\n####################################################################\n"
            <<  "#   Extract Hiero Rules ( version 2.0 )     --  www.nlplab.com     #\n"
            <<  "####################################################################\n"
            <<flush;

    cerr<<"Parameters\n"
        <<"        -srclen                   :     "<<options.maxSourcePhraseSize                            <<"\n"
        <<"        -tgtlen                   :     "<<options.maxTargetPhraseSize                            <<"\n"
        <<"        -maxSrcI                  :     "<<options.maxSourceInitSize                              <<"\n"
        <<"        -maxTgtI                  :     "<<options.maxTargetInitSize                              <<"\n"
        <<"        -maxSrcH                  :     "<<options.maxSourceHieroSize                             <<"\n"
        <<"        -maxTgtH                  :     "<<options.maxTargetHieroSize                             <<"\n"
        <<"        -minSrcSubPhrase          :     "<<options.minSourceSubPhrase                             <<"\n"
        <<"        -minTgtSubPhrase          :     "<<options.minTargetSubPhrase                             <<"\n"
        <<"        -minLexiNum               :     "<<options.minLexiNumOfHieroRule                          <<"\n"
        <<"        -maxnonterm               :     "<<options.maxNonTermNum                                  <<"\n"
        <<"        -alignedLexiReq           :     "<<( !options.alignedLexiRequiredFlag ? "false" : "true" )<<"\n"
        <<"        -srcNonTermAdjacent       :     "<<( !options.srcNonTermAdjacentFlag ?  "false" : "true" )<<"\n"
        <<"        -tgtNonTermAdjacent       :     "<<( !options.tgtNonTermAdjacentFlag ?  "false" : "true" )<<"\n"
        <<"        -duplicateHieroRule       :     "<<( !options.duplicateHieroRuleFlag ?  "false" : "true" )<<"\n"
        <<"        -unalignedEdgeInit        :     "<<( !options.unalignedEdgeInitFlag ?   "false" : "true" )<<"\n"
        <<"        -unalignedEdgeHiero       :     "<<( !options.unalignedEdgeHieroFlag ?  "false" : "true" )<<"\n"
        <<"        -headnonterm              :     "<<( !options.headNonTermFlag ?         "false" : "true" )<<"\n"
        <<"        -null                     :     "<<( !options.nullFuncFlag ?            "false" : "true" )<<"\n\n"
        <<flush;

    //add in 2012/03/29
    if( options.srcParseTreeFlag )
        cerr<<"        -srcParseTree             :     "<<"true"<<"\n"
            <<"        -srcParseTreePath         :     "<<options.srcParseTreePath<<"\n";
    else
        cerr<<"        -srcParseTree             :     "<<"false"<<"\n";
    if( options.tgtParseTreeFlag )
        cerr<<"        -tgtParseTree             :     "<<"true"<<"\n"
            <<"        -tgtParseTreePath         :     "<<options.tgtParseTreePath<<"\n\n";
    else
        cerr<<"        -tgtParseTree             :     "<<"false"<<"\n\n";
    //add in 2012/03/29

    if( options.unalignedEdgeInitFlag )
        cerr<<"        -maxNulExtSrcInitNum      :     "<<options.maxNulExtSrcInitNum<<"\n"
            <<"        -maxNulExtTgtInitNum      :     "<<options.maxNulExtTgtInitNum<<"\n"
            <<flush;

    if( options.unalignedEdgeHieroFlag )
        cerr<<"        -maxNulExtSrcHieroNum     :     "<<options.maxNulExtSrcHieroNum<<"\n"
            <<"        -maxNulExtTgtHieroNum     :     "<<options.maxNulExtTgtHieroNum<<"\n"
            <<flush;


    if( options.maxSourcePhraseSize <= 0 || options.maxTargetPhraseSize <= 0 || 
        options.maxSourceHieroSize  <= 0 || options.maxTargetHieroSize  <= 0 )
    {
        cerr<<"\n"
            <<"ERROR: The max phrase size is too short, please reset it!\n"<<flush;
        exit( 1 );
    }

    ifstream infile_c( sourceFile.c_str() );
    ifstream infile_e( destFile.c_str() );
    ifstream infile_alignment( alignedFile.c_str() );
    
    //add in 2012/03/29
    ifstream infile_srcParseTree;
    ifstream infile_tgtParseTree;

    if( options.srcParseTreeFlag )
    {
        infile_srcParseTree.open( options.srcParseTreePath.c_str() );
        if( !infile_srcParseTree )
        {
            cerr<<"\n"
                <<"ERROR: Please validate the input files that in your dir!\n"
                <<"  Source Parse Tree: "<<options.srcParseTreePath<<"\n"<<flush;
            exit( 1 );
        }
    }
    if( options.tgtParseTreeFlag )
    {
        infile_tgtParseTree.open( options.tgtParseTreePath.c_str() );
        if( !infile_tgtParseTree )
        {
            cerr<<"\n"
                <<"ERROR: Please validate the input files that in your dir!\n"
                <<"  Target Parse Tree: "<<options.tgtParseTreePath<<"\n"<<flush;
            exit( 1 );
        }
    }
    //add in 2012/03/29

    if( !infile_c || !infile_e || !infile_alignment )
    {
        cerr<<"\n"
            <<"ERROR: Please validate the input files that in your dir!\n"
            <<"  Source Language file: " <<sourceFile <<"\n"
            <<"  Target Language file: " <<destFile   <<"\n"
            <<"  Alignment       file: " <<alignedFile<<"\n"<<flush;
        exit( 1 );
    }

    //add in 2012/03/29
    string line_srcParseTree, line_tgtParseTree;
    //add in 2012/03/29

    string line_c, line_e, line_alignment;
    size_t lineNo                        = 0                                   ;
    size_t invalidTree                   = 0                                   ;
    size_t createWordAFA                 = 0                                   ;
    string ofileExtractPhrasesPair(        extractResult                      );
    string ofileExtractPhrasesPairInv(     extractResult                      );
    ofileExtractPhrasesPairInv          += ".inv"                              ;
    ofstream outfileExtractPhrasesPair(    ofileExtractPhrasesPair.c_str()    );
    ofstream outfileExtractPhrasesPairInv( ofileExtractPhrasesPairInv.c_str() );

#ifdef __DEBUG__
    string ofileErrorLog( "err.log" );
    ofstream outFileErrorLog( ofileErrorLog.c_str() );
#endif

    if( !outfileExtractPhrasesPair || !outfileExtractPhrasesPair )
    {
        cerr<<"\n"
            <<"ERROR: Please validate the output files that in your dir!\n"
            <<"  Extract file: "<<ofileExtractPhrasesPair            <<"\n"
            <<"  Extract file inverse: "<<ofileExtractPhrasesPairInv <<"\n"<<flush;
        exit( 1 );
    }

    clock_t start,finish;
    start = clock();
    if( options.srcParseTreeFlag || options.tgtParseTreeFlag )
        cerr<<"Extract SAMT Rule\n";
    else
        cerr<<"Extract Hiero Rule\n";

    cerr<<"        Input : "<<sourceFile                <<"\n"
        <<"                "<<destFile                  <<"\n"
        <<"                "<<alignedFile               <<"\n"
        <<"        Output: "<<ofileExtractPhrasesPair   <<"\n"
        <<"                "<<ofileExtractPhrasesPairInv<<"\n"<<flush;

    HieroRuleNum hieroRuleNum;

    while( getline( infile_c, line_c ) && getline( infile_e, line_e ) && getline( infile_alignment, line_alignment ) )
    {
        ++ lineNo;
        BasicMethod be;

        // add 2012/03/28
        if( options.srcParseTreeFlag )
        {
            getline( infile_srcParseTree, line_srcParseTree );
            be.clearIllegalChar( line_srcParseTree );
            if( line_srcParseTree == "(())" || line_srcParseTree == "( () )" ) // empty parse tree
            {
#ifdef __DEBUG__
                outFileErrorLog<<"[LINENO:"    <<lineNo           <<" TREE_F]"<<"\n";
                outFileErrorLog<<"[SRC_SENT:] "<<line_c           <<"\n"  ;
                outFileErrorLog<<"[SRC_PART:] "<<line_srcParseTree<<"\n\n";
#endif
                ++invalidTree;
                continue;
            }
        }
        // add 2012/03/28

        // add 2012/03/29
        if( options.tgtParseTreeFlag )
        {
            getline( infile_tgtParseTree, line_tgtParseTree );
            be.clearIllegalChar( line_tgtParseTree );
            if( line_tgtParseTree == "(())" || line_tgtParseTree == "( () )" ) // empty parse tree
            {
#ifdef __DEBUG__
                outFileErrorLog<<"[LINENO:"    <<lineNo           <<" TREE_F]"<<"\n";
                outFileErrorLog<<"[TGT_SENT:] "<<line_e           <<"\n"  ;
                outFileErrorLog<<"[TGT_PART:] "<<line_tgtParseTree<<"\n\n";
#endif
                ++invalidTree;
                continue;
            }
        }
        // add 2012/03/29 

        be.clearIllegalChar( line_c );
        be.clearIllegalChar( line_e );
        be.clearIllegalChar( line_alignment );

        WordAlignment wordalignment;
        PhrasesPairHiero phrasepairhiero;
        bool return_value;

        // add in 2012/03/29
        if( options.srcParseTreeFlag && options.tgtParseTreeFlag )
        {
            Tree treeofSrc;
            Tree treeofTgt;
            bool buildTreeofSrcFlag = treeofSrc.buildTree( line_srcParseTree );
            if( !buildTreeofSrcFlag )
            {
#ifdef __DEBUG__
                outFileErrorLog<<"[LINENO:"<<lineNo<<" TREE_F]"<<"\n";
                outFileErrorLog<<"[SRC_SENT:] "<<line_c<<"\n";
                outFileErrorLog<<"[SRC_PART:] "<<line_srcParseTree<<"\n\n";
#endif
                ++invalidTree;
                continue;
            }
            bool buildTreeofTgtFlag = treeofTgt.buildTree( line_tgtParseTree );
            if( !buildTreeofTgtFlag )
            {
#ifdef __DEBUG__
                outFileErrorLog<<"[LINENO:"<<lineNo<<" TREE_F]"<<"\n";
                outFileErrorLog<<"[TGT_SENT:] "<<line_e<<"\n";
                outFileErrorLog<<"[TGT_PART:] "<<line_tgtParseTree<<"\n\n";
#endif
                ++invalidTree;
                continue;
            }

            return_value = wordalignment.createWordAlignment( line_e, line_c, line_alignment, lineNo, treeofSrc, treeofTgt );

        }
        else if( options.tgtParseTreeFlag )
        {
            Tree tree;
            bool buildTreeFlag = tree.buildTree( line_tgtParseTree );
            if( !buildTreeFlag )
            {
#ifdef __DEBUG__
                outFileErrorLog<<"[LINENO:"<<lineNo<<" TREE_F]"<<"\n";
                outFileErrorLog<<"[TGT_SENT:] "<<line_e<<"\n";
                outFileErrorLog<<"[TGT_PART:] "<<line_tgtParseTree<<"\n\n";
#endif
                ++invalidTree;
                continue;
            }
            return_value = wordalignment.createWordAlignment( line_e, line_c, line_alignment, lineNo, tree, 1 );
        }
        else if( options.srcParseTreeFlag )
        {
            Tree tree;
            bool buildTreeFlag = tree.buildTree( line_srcParseTree );
            if( !buildTreeFlag )
            {
#ifdef __DEBUG__
                outFileErrorLog<<"[LINENO:"<<lineNo<<" TREE_F]"<<"\n";
                outFileErrorLog<<"[SRC_SENT:] "<<line_c<<"\n";
                outFileErrorLog<<"[SRC_PART:] "<<line_srcParseTree<<"\n\n";
#endif
                ++invalidTree;
                continue;
            }
            return_value = wordalignment.createWordAlignment( line_e, line_c, line_alignment, lineNo, tree, 0 );
        }
        // add in 2012/03/29
        else
        {
            return_value = wordalignment.createWordAlignment( line_e, line_c, line_alignment, lineNo );        
        }

        if( return_value )
            phrasepairhiero.extractPhrasesPair( wordalignment, outfileExtractPhrasesPair, outfileExtractPhrasesPairInv, hieroRuleNum, options );
        else
        {
            ++createWordAFA;
#ifdef __DEBUG__
            outFileErrorLog<<"[LINENO:"<<lineNo<<" WORD_A]"<<"\n";
            if( options.tgtParseTreeFlag )
            {
                outFileErrorLog<<"[TGT_SENT:] "<<line_e<<"\n";
                outFileErrorLog<<"[TGT_PART:] "<<line_tgtParseTree<<"\n\n";
            }
            if( options.srcParseTreeFlag )
            {
                outFileErrorLog<<"[SRC_SENT:] "<<line_c<<"\n";
                outFileErrorLog<<"[SRC_PART:] "<<line_srcParseTree<<"\n\n";            
            }
#endif
            continue;
        }
        if( lineNo % 100 == 0 )
        {
            if( options.srcParseTreeFlag || options.tgtParseTreeFlag )
                cerr<<"\r        processed "<<lineNo<<" lines. [TREE_F:"<<invalidTree<<" WORD_A:"<<createWordAFA<<"]  "<<flush;
            else
                cerr<<"\r        processed "<<lineNo<<" lines. "<<flush;
        }
    }
    if( options.srcParseTreeFlag || options.tgtParseTreeFlag )
        cerr<<"\r        processed "<<lineNo<<" lines. [TREE_F:"<<invalidTree<<" WORD_A:"<<createWordAFA<<"]  \n"<<flush;
    else
        cerr<<"\r        processed "<<lineNo<<" lines. \n"<<flush;

    finish = clock();
    cerr<<"        time: "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n\n"<<flush;
    cerr<<"Final Results\n"
        <<"        Initial Rule Number : "<<hieroRuleNum.initialRuleNum<<"\n";
    if( options.srcParseTreeFlag || options.tgtParseTreeFlag )
        cerr<<"        SAMT    Rule Number : "<<hieroRuleNum.hieroRuleNum<<"\n";
    else
        cerr<<"        HIERO   Rule Number : "<<hieroRuleNum.hieroRuleNum<<"\n";
    
    cerr<<"        NULL    Rule Number : "<<hieroRuleNum.nullRuleNum<<"\n"
        <<"        Total   Rule Number : "<<( hieroRuleNum.initialRuleNum + hieroRuleNum.hieroRuleNum + hieroRuleNum.nullRuleNum )<<"\n"<<flush;

    outfileExtractPhrasesPair.close();
    outfileExtractPhrasesPairInv.close();

#ifdef __DEBUG__
    outFileErrorLog.close();
#endif

    infile_c.close()           ;
    infile_e.close()           ;
    infile_alignment.close()   ;
    infile_srcParseTree.close();

    return true;
}

bool Dispatcher::call_score(
string         &extractFile    , 
string         &lexFile        , 
string         &phraseTableFile, 
bool           &inverseFlag    , 
OptionsOfScore &options        ,
string         &method         ,
SystemCommand  &systemCommand    )
{
    cerr<<"\n####################################################################\n"
        <<  "# Calculate Score of Rule Table (version 1.1.0)   --www.nlplab.com #\n"
        <<  "####################################################################\n"
        <<flush;

    cerr<<"Parameters\n"
        <<"        -cutoffInit             :     "<<options.cutoffInit    <<"\n"
        <<"        -cutoffHiero            :     "<<options.cutoffHiero   <<"\n"
        <<"        -method                 :     "<<method                <<"\n"
        <<"        -sort                   :     "<<systemCommand.sortFile<<"\n"
        <<"        -inverseFlag            :     "<<( inverseFlag            == 1    ? "true" : "false" )<<"\n"
        <<"        -printFreq              :     "<<( options.printFreqFlag  == true ? "true" : "false" )<<"\n"
        <<"        -printAlign             :     "<<( options.printAlignFlag == true ? "true" : "false" )<<"\n"
		<<"        -sortTable              :     "<<( options.sort_phrase_table == true ? "true" : "false" )<<"\n"
        <<flush;

    clock_t start, finish;
    string extractFileSorted = extractFile + ".sorted";
	
    string command("");
	if(options.sort_phrase_table)
	{
		command = systemCommand.sortFile + extractFile + " > " + extractFileSorted;

		start = clock();
		cerr<<"Sort\n"
			<<"        command : "<<command          <<"\n"    
			<<"        input   : "<<extractFile      <<"\n"
			<<"        output  : "<<extractFileSorted<<"\n"<<flush;
		system( command.c_str() );
		finish = clock();
		cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;
	}
	
    
    string phraseTable = phraseTableFile;

    if( inverseFlag )
        phraseTable += ".inv";

    start = clock();
    cerr<<"Loading lexical table\n"
        <<"        input   : "<<lexFile<<"\n"<<flush;
    LexicalTable lt;
    lt.loadLexicalTable( lexFile );
    finish = clock();
    cerr<<"\n"
        <<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;

    ifstream infileExtract( extractFileSorted.c_str() );
    if( !infileExtract )
    {
        cerr<<"\n"
            <<"ERROR: Could not open extract-file. ( file: "<<extractFile<<" )"<<endl;
        exit( 1 );
    }
    string line;
    size_t lineNo = 0;
    
    ofstream outfilePhraseTable( phraseTable.c_str() );
    if( !outfilePhraseTable )
    {
        cerr<<"\n"
            <<"ERROR: Could not open phrase-table while you want to generate. ( file: "<<phraseTable<<" )"<<endl;
        exit( 1 );
    }
    PhraseTable pt;
    
    start = clock();
    bool lastline = false;
    
    cerr<<"Caculate phrase table score\n"
        <<"        method  : "<<method           <<"\n"
        <<"        input   : "<<extractFileSorted<<"\n"
        <<"        output  : "<<phraseTable      <<"\n"<<flush;

    ScoreClassifyNum scoreClassifyNum;
    while( getline( infileExtract, line ) )
    {
        ++lineNo;
        PhraseAlignment pa;    
        pa.create( line, lineNo );
        LexicalWeight lw;
        lw.caculateLexicalWeight( pa, lt, method  );

        pt.generatePhraseTable( pa, lastline, outfilePhraseTable, inverseFlag, options, scoreClassifyNum );
        if( lineNo % 100000 == 0 )
            cerr<<"\r        processed "<<lineNo<<" lines."<<flush;
    }
    cerr<<"\r        processed "<<lineNo<<" lines.\n"<<flush;
    PhraseAlignment pa;
    pt.generatePhraseTable( pa, lastline = true, outfilePhraseTable, inverseFlag, options, scoreClassifyNum );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;
    infileExtract.close();
    outfilePhraseTable.close();

    cerr<<"Final Results\n"
        <<"        Initial Rule Number : "<<scoreClassifyNum.initialRuleNum<<"\n"
        <<"        Hiero   Rule Number : "<<scoreClassifyNum.hieroRuleNum  <<"\n"
        <<"        NULL    Rule Number : "<<scoreClassifyNum.nullRuleNum   <<"\n"
        <<"        Total   Rule Number : "<<( scoreClassifyNum.initialRuleNum + scoreClassifyNum.hieroRuleNum + scoreClassifyNum.nullRuleNum )<<"\n"
        <<flush;

    
    command = systemCommand.sortFile + phraseTable + " > " + phraseTable + ".sorted";

    start = clock();
    cerr<<"Sort\n"
        <<"        command : "<<command    <<"\n"
        <<"        input   : "<<phraseTable<<"\n"
        <<"        output  : "<<phraseTable<<".sorted\n"<<flush;
    system( command.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;
	
    command = systemCommand.del + phraseTable;
    start = clock();
    cerr<<"Delete\n"
        <<"        command : "<<command    <<"\n"
        <<"        input   : "<<phraseTable<<"\n"
        <<flush;
    system( command.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n\n"<<flush;
	
    return true;
}

// add in 2012-05-22, scoring for syntax-rule.
bool Dispatcher::call_scoringforSynRule( 
map< string, string >      &param        ,
OptionsOfScoringforSynRule &options      ,
SystemCommand              &systemCommand  )
{
#ifdef __DEBUG__
    cerr<<"Dispatcher::call_scoringforSynRule:step1\n"<<flush;
#endif

    cerr<<"\n####################################################################\n"
        <<  "#   Scoring for Syntax-Rule (version 4.0)       --www.nlplab.com   #\n"
        <<  "####################################################################\n"
        <<flush;

    cerr<<"Parameters\n"
        <<"        -cutoff        :        "<<options.cutoff        <<"\n"
        <<"        -lowerfreq     :        "<<options.lowerFreq     <<"\n"
        <<"        -model         :        "<<options.model         <<"\n"
        <<"        -sort          :        "<<systemCommand.sortFile<<"\n"
        <<flush;

    // change syntax-rule to phrase table format and we can calculate the phrase-table score for syntax-rule. 
    changeSynRuletoPhraseTabFormat( param[ "-rule" ], options );

    // generate parameters for phrase table format scoring.
    map< string, string > paramPhraseTab;
    generScoringForPhraseTabParam( param, paramPhraseTab );

    // scoring for syntax-rule of phrase table format, after this, we can get four scores for phrasetab format.
    score_function( paramPhraseTab, systemCommand.sortFile, systemCommand.del );

    // add in 2012-05-28, delete temp file.
    vector< string > delFileList;
    if( !delFileList.empty() )
    {
#ifdef __DEBUG__
        cerr<<"The vector of delFileList is not empty! Clear it now!\n"<<flush;
#endif
        delFileList.clear();
    }
    delFileList.push_back( paramPhraseTab[ "-tab" ] );
    delFileList.push_back( paramPhraseTab[ "-tabinv" ] );
    delFileList.push_back( paramPhraseTab[ "-tab" ] + ".sorted" );
    delFileList.push_back( paramPhraseTab[ "-tabinv" ] + ".sorted" );
    BasicMethod bm;
    bm.deleteFileList( delFileList, systemCommand );

    // calculate syntax scores
    ScoringforSynRule sfsr;
    if( options.speedorMemory == "speed" )
    {
        sfsr.calcuSrcandTgtScoreGivenRootSpeed(  param[ "-rule" ], param[ "-out" ], options, systemCommand );
    }
    else if( options.speedorMemory == "memory" )
    {
        sfsr.calcuSrcandTgtScoreGivenRootMemory( param[ "-rule" ], param[ "-out" ], options, systemCommand );
    }
    else
    {
        cerr<<"Error: the parameter of \"-spormy\" is wrong!\n"<<flush;
        exit( 1 );
    }

    // add in 2012-05-28, delete temp file.
    if( !delFileList.empty() )
    {
#ifdef __DEBUG__
        cerr<<"The vector of delFileList is not empty! Clear it now!\n"<<flush;
#endif
        delFileList.clear();
    }
    delFileList.push_back( param[ "-rule" ] + ".sorted" );
    bm.deleteFileList( delFileList, systemCommand );

    string phraseTab, syntaxRule;
    phraseTab = syntaxRule = param[ "-out" ];
    phraseTab.append( ".phrasescore"  );
    syntaxRule.append( ".syntaxscore" );
    sfsr.consolidatePhraseTabandSynRule( phraseTab, syntaxRule, systemCommand );
    
    // add in 2012-05-28, delete temp file.
    if( !delFileList.empty() )
    {
#ifdef __DEBUG__
        cerr<<"The vector of delFileList is not empty! Clear it now!\n"<<flush;
#endif
        delFileList.clear();
    }
    delFileList.push_back( phraseTab  + ".sorted" );
    delFileList.push_back( syntaxRule + ".sorted" );
    bm.deleteFileList( delFileList, systemCommand );

    // rename the final scored file.
#ifdef WIN32
    string command = "move ";
#else
    string command = "mv ";
#endif

    command += param[ "-out" ] + ".phrasescore.syntaxscore" + " " + param[ "-out" ];
    system( command.c_str() );

    return true;
}

bool Dispatcher::call_consolidate(
string         &infileS2D    , 
string         &infileD2S    , 
string         &outfile      ,
OptionsOfScore &options      ,
SystemCommand  &systemCommand  )
{
    cerr<<"\n####################################################################\n"
        <<  "#    Consolidate score file ( version 2.0 )    --  www.nlplab.com  #\n"
        <<  "####################################################################\n"
        <<flush;

    cerr<<"Parameters\n"
        <<"        -printAlign             :     "<<( options.printAlignFlag == true ? "true" : "false" )<<"\n"
        <<"        -printFreq              :     "<<( options.printFreqFlag  == true ? "true" : "false" )<<"\n";

    clock_t start,finish;
    start = clock();
    Consolidate cl;
    
    string outfileUnsorted = outfile + ".unsorted";

    cerr<<"Consolidate two half files\n"
        <<"        input   : "<<infileS2D<<"\n"
        <<"                  "<<infileD2S<<"\n"
        <<"        output  : "<<outfile<<"\n"<<flush;

    ConsolidateClassifyNum consolidateClassifyNum;

    cl.consolidate( infileS2D, infileD2S, outfileUnsorted, options, consolidateClassifyNum );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;

    cerr<<"Final Results\n"
        <<"        Initial Rule Number : "<<consolidateClassifyNum.initialRuleNum<<"\n"
        <<"        Hiero   Rule Number : "<<consolidateClassifyNum.hieroRuleNum<<"\n"
        <<"        NULL    Rule Number : "<<consolidateClassifyNum.nullRuleNum<<"\n"
        <<"        Total   Rule Number : "<<( consolidateClassifyNum.initialRuleNum + consolidateClassifyNum.hieroRuleNum + consolidateClassifyNum.nullRuleNum )<<"\n"
        <<flush;

    string commandSort1 = systemCommand.sortFile + outfileUnsorted + " > " + outfile;
    string commandSort2;
    string commandDel1 = systemCommand.del + outfileUnsorted;
    string commandDel2;
    // add in 2012/05/22
    if( options.generInvFileFlag )
    {
        commandSort2 = systemCommand.sortFile + outfileUnsorted + ".inv" + " > " + outfile + ".inv";
        commandDel2 = systemCommand.del + outfileUnsorted + ".inv";
    }

    start = clock();
    cerr<<"Sort\n"
        <<"        command : "<<commandSort1<<"\n"
        <<"        input   : "<<outfileUnsorted<<"\n"
        <<"        output  : "<<outfile<<"\n"
        <<flush;
    system( commandSort1.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;

    // add in 2012/05/22
    if( options.generInvFileFlag )
    {
        start = clock();
        cerr<<"Sort\n"
            <<"        command : "<<commandSort2<<"\n"
            <<"        input   : "<<outfileUnsorted + ".inv"<<"\n"
            <<"        output  : "<<outfile + ".inv"<<"\n"
            <<flush;
        system( commandSort2.c_str() );
        finish = clock();
        cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;
    }

    start = clock();
    cerr<<"Delete\n"
        <<"        command : "<<commandDel1<<"\n"
        <<"        input   : "<<outfileUnsorted<<"\n"
        <<flush;
    system( commandDel1.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"
        <<flush;
    
    // add in 2012/05/22
    if( options.generInvFileFlag )
    {
        start = clock();
        cerr<<"Delete\n"
            <<"        command : "<<commandDel2<<"\n"
            <<"        input   : "<<outfileUnsorted + ".inv"<<"\n"
            <<flush;
        system( commandDel2.c_str() );
        finish = clock();
        cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n\n"<<flush;
    }

	
    start = clock();
    string commandDel3 = systemCommand.del + infileS2D;
    cerr<<"Delete\n"
        <<"        command : "<<commandDel3<<"\n"
        <<"        input   : "<<infileS2D<<"\n"
        <<flush;
    system( commandDel3.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"
        <<flush;

    start = clock();
    string commandDel4 = systemCommand.del + infileD2S;
    cerr<<"Delete\n"
        <<"        command : "<<commandDel4<<"\n"
        <<"        input   : "<<infileD2S<<"\n"
        <<flush;
    system( commandDel4.c_str() );
    finish = clock();
    cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"
        <<flush;
	

    return true;
}


bool Dispatcher::call_filterTableWithNum( 
const OptionsOfFilterNum &options )
{
    cerr<<"\n####################################################################\n"
        <<  "#   Filter Rule with Number ( version 3.0 )    -- www.nlplab.com   #\n"
        <<  "####################################################################\n"
        <<flush;
    clock_t start,finish;
    start = clock();
    cerr<<"Generate translation score table with rule strict\n"
        <<"        input       : "<<options.sourceTransTableName<<"\n"
        <<"        output      : "<<options.targetTransTableName<<"\n"
        <<"        strict      : "<<options.outputMaxNumStrict<<"\n"
        <<"        tableFormat : "<<options.tableFormat<<"\n"<<flush;

    FilterTableWithNum ftwn;
    ftwn.filterTableWithNum( options );
    finish = clock();
    cerr<<"        time     : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"<<flush;
    return true;
}

bool Dispatcher::call_filterPhraseTabWithDev(
string &devSet, 
string &phraseTable, 
string &filterTable, 
int &maxlen, 
int &refnum )
{
    cerr<<"\n####################################################################\n"
        <<  "# Filter phrase table with dev set( version 2.0 ) --www.nlplab.com #\n"
        <<  "####################################################################\n"
        <<flush;

    FilterFinalTable fft;
    fft.filterFinalTable( devSet, phraseTable, filterTable, maxlen, refnum );
    return true;
}

// add in 2012-05-22, scoring for syntax-rule.
bool Dispatcher::changeSynRuletoPhraseTabFormat(
string                     &rule   ,
OptionsOfScoringforSynRule &options  )
{
#ifdef __DEBUG__
    cerr<<"Dispatcher::changeSynRuletoPhraseTabFormat:step1\n"<<flush;
#endif

    ifstream infile_rule( rule.c_str() );
    if( !infile_rule )
    {
        cerr<<"\n"
            <<"Error: Please validate the input files that in your dir!\n"
            <<"  -rule: "<<rule<<"\n"<<flush;
        exit( 1 );
    }
#ifdef __DEBUG__
    else
        cerr<<"Dispatcher::changeSynRuletoPhraseTabFormat:open "<<rule<<" success!\n"<<flush;
#endif

    string rulePhraseTabFormat    = rule                + ".phrasetabformat";
    string rulePhraseTabFormatInv = rulePhraseTabFormat + ".inv"            ;

#ifdef __DEBUG__
    cerr<<"rulePhraseTabFormat   :"<<rulePhraseTabFormat   <<"\n"<<flush;
    cerr<<"rulePhraseTabFormatInv:"<<rulePhraseTabFormatInv<<"\n"<<flush;
#endif

    ofstream outfile_rulePhraseTabFormat(    rulePhraseTabFormat.c_str()    );
    ofstream outfile_rulePhraseTabFormatInv( rulePhraseTabFormatInv.c_str() );

    if( !outfile_rulePhraseTabFormat || !outfile_rulePhraseTabFormatInv )
    {
        cerr<<"\n"
            <<"Error: can not write file\n"
            <<"         "<<rulePhraseTabFormat   <<"\n"
            <<"         "<<rulePhraseTabFormatInv<<"\n"
            <<flush;
        exit( 1 );
    }
#ifdef __DEBUG__
    else
        cerr<<"Dispatcher::changeSynRuletoPhraseTabFormat:open "<<rulePhraseTabFormat<<" "<<rulePhraseTabFormatInv<<" success!\n"<<flush;
#endif

    cerr<<"\nChange Syntax-rule to phrase-table format:\n"<<flush;

    string line_rule;
    size_t lineNo = 0;
    BasicMethod bm;
    if( options.model == "t2s" || options.model == "t2t" || options.model == "s2t" )
    {
        while( getline( infile_rule, line_rule ) )
        {
            ++lineNo;
            bm.rmEndSpace( line_rule );
            vector< string > rulevector;
            bm.splitWithStr( line_rule, " ||| ", rulevector );

            if( rulevector.at( 2 ) == "<NULL>" )
                rulevector.at( 2 ) = "NULL";

            outfile_rulePhraseTabFormat   <<rulevector.at( 1 )<<" ||| "<<rulevector.at( 2 )<<" ||| "<<rulevector.at( 5 )<<"\n";
            bm.convertAlignmenttoInv( rulevector.at( 5 ) );
            outfile_rulePhraseTabFormatInv<<rulevector.at( 2 )<<" ||| "<<rulevector.at( 1 )<<" ||| "<<rulevector.at( 5 )<<"\n";

            if( lineNo % 10000 == 0 )
                cerr<<"\r        processed "<<lineNo<<" lines."<<flush;
        }
        outfile_rulePhraseTabFormat<<flush;
        outfile_rulePhraseTabFormatInv<<flush;
    }
    cerr<<"\r        processed "<<lineNo<<" lines.\n"<<flush;

    infile_rule.close();
    outfile_rulePhraseTabFormat.close();
    outfile_rulePhraseTabFormatInv.close();
    return true;
}

bool Dispatcher::generScoringForPhraseTabParam( 
const map< string, string > &param         ,
      map< string, string > &paramPhraseTab  )
{
    for( map< string, string >::const_iterator iter = param.begin(); iter != param.end(); ++iter )
    {
        paramPhraseTab.insert( make_pair( iter->first, iter->second ) );
    }

    paramPhraseTab[ "-cutoffInit"  ] = paramPhraseTab[ "-cutoff" ]                     ;
    paramPhraseTab[ "-cutoffHiero" ] = paramPhraseTab[ "-cutoff" ]                     ;
    paramPhraseTab[ "-tab"         ] = paramPhraseTab[ "-rule"   ] + ".phrasetabformat";
    paramPhraseTab[ "-tabinv"      ] = paramPhraseTab[ "-tab"    ] + ".inv"            ;
    paramPhraseTab[ "-out"         ] = paramPhraseTab[ "-out"    ] + ".phrasescore"    ;
    return true;
}
