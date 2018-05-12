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
 * phrase extraction and scorer; LEX_generate_lexical_table.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 7/26/2011 add stemming method for generating lexical table( word translation table ) by Qiang Li. 
 *
 */

#include "lexicaltable_extractor.h"
using namespace lexicaltable_extractor;

// Initial for Multi Score Variable 
map< string, double > GenerateLexicalTable::globalTotalSourceLang = map< string, double >();
map< string, double > GenerateLexicalTable::globalTotalTargetLang = map< string, double >();
map< string, map< string, double > > GenerateLexicalTable::globalWordTranslation = map< string, map< string, double> >();

Mutex GenerateLexicalTable::g_Lock = Mutex();  

bool GenerateLexicalTable::generateLexicalTable( OptionsOfLexTable &options, string &stemmingFileName )
{
    ifstream infileStemming(    stemmingFileName.c_str()            );
    ifstream infileSourceLang(  options.sourceLangFileName.c_str()  );
    ifstream infileDestLang(    options.targetLangFileName.c_str()  );
    ifstream infileAlignedInfo( options.alignedInfoFileName.c_str() );

    if( !infileStemming || !infileSourceLang || !infileDestLang || !infileAlignedInfo )
    {
        cerr<<"        Error: Could not open file :\n"
            <<"        "<<options.sourceLangFileName <<" or\n"
            <<"        "<<options.targetLangFileName <<" or\n"
            <<"        "<<options.alignedInfoFileName<<" or\n"
            <<"        "<<stemmingFileName<<"\n";
        exit( 1 );
    }

    string lineOfStemming;
    size_t lineNo = 0;

    // create Stemming List( prototype -> stemmed -> probability ), probability inited by 1.
    cerr<<"Generate Stemming Table\n"<<flush;
    while( getline( infileStemming, lineOfStemming ) )
    {
        ++lineNo;
        stem.clearIllegalChar( lineOfStemming );
        stem.toLower( lineOfStemming );
        vector< string > tempStem;
        stem.splitWithStr( lineOfStemming, " ||| ", tempStem );
        if( tempStem.size() != 2 )
        {
            cerr<<"\n"
                <<"        WARNING: Format is not right in "<<stemmingFileName<<" in "<<lineNo<<" line!\n"<<flush;
            continue;
        }
        map< string, pair< string, double > >::iterator findStemList;
        
        if( ( findStemList = stem.stemmingList.find( tempStem.at( 0 ) ) ) != stem.stemmingList.end() )
        {
            ++findStemList->second.second;
            ++stem.stemmed[ tempStem.at( 1 ) ];
        }
        else
        {
            stem.stemmingList.insert( make_pair( tempStem.at( 0 ), make_pair( tempStem.at( 1 ), 1 ) ) );
            ++stem.stemmed[ tempStem.at( 1 ) ];
        }

        if( ( findStemList = stem.stemmingList.find( tempStem.at( 1 ) ) ) != stem.stemmingList.end() )
        {
            continue;
        }
        else
        {
            stem.stemmingList.insert( make_pair( tempStem.at( 1 ), make_pair( tempStem.at( 1 ), 1 ) ) );
            ++stem.stemmed[ tempStem.at( 1 ) ];
        }
        if( lineNo % 10000 == 0 )
            cerr<<"\r        processing "<<lineNo<<" lines!"<<flush;
    }
    cerr<<"\r        processing "<<lineNo<<" lines!\n"<<flush;

    lineNo = 0;
    string lineOfDestLang;
    cerr<<"Statistical Stemming Counts in "<<options.targetLangFileName<<"\n"<<flush;
    while( getline( infileDestLang, lineOfDestLang ) )
    {
        ++lineNo;
        stem.clearIllegalChar( lineOfDestLang );
        if( lineOfDestLang == "" )
        {
            cerr<<"\n"
                <<"        WARNING: null sentence in target file in line:"<<lineNo<<"\n"<<flush;
            continue;
        }
        vector< string > tempDestLang;
        stem.split( lineOfDestLang, ' ', tempDestLang );
        for( vector< string >::iterator iter = tempDestLang.begin(); iter != tempDestLang.end(); ++iter )
        {
            map< string, pair< string, double > >::iterator findStemList;
            if( ( findStemList = stem.stemmingList.find( *iter ) ) != stem.stemmingList.end() )
            {
                ++( findStemList->second.second );
                ++stem.stemmed[ findStemList->second.first ];
            }
        }
        if( lineNo % 10000 == 0 )
            cerr<<"\r        processing "<<lineNo<<" lines!"<<flush;
    }
    cerr<<"\r        processing "<<lineNo<<" lines!\n"<<flush;

    // calculate stemmed probablity
    lineNo = 0;
    cerr<<"Calculate Stemming Probability\n"<<flush;
    for( map< string, pair< string, double > >::iterator iter = stem.stemmingList.begin(); iter != stem.stemmingList.end(); ++iter )
    {
        ++lineNo;
//      cout<<iter->second.second<<" "<<stem.stemmed[iter->second.first]<<"\n"<<flush;
        iter->second.second = iter->second.second/stem.stemmed[ iter->second.first ];
        if( lineNo % 10000 == 0 )
            cerr<<"\r        processing "<<lineNo<<flush;
    }
    cerr<<"\r        processing "<<lineNo<<"\n"<<flush;

    infileDestLang.clear();
    infileDestLang.close();
    infileDestLang.open( options.targetLangFileName.c_str() );


    ofstream outfileLexS2D( options.lexTabS2TFileName.c_str() );
    ofstream outfileLexD2S( options.lexTabT2SFileName.c_str() );

    string lineOfSourceLang, lineOfAlignedInfo;
    lineNo = 0;

    cerr<<"Calculate Word Translation Table\n"<<flush;
    while( getline( infileSourceLang, lineOfSourceLang ) && getline( infileDestLang, lineOfDestLang ) && getline( infileAlignedInfo, lineOfAlignedInfo ) )
    {
        ++lineNo;
        BasicMethod be;
        be.clearIllegalChar( lineOfSourceLang );
        be.clearIllegalChar( lineOfDestLang );
        be.clearIllegalChar( lineOfAlignedInfo );

        if( lineOfSourceLang == "" )
        {
            cerr<<"\n"
                <<"        WARNING:null sentence in dest file in line:"<<lineNo<<"\n";
            continue;
        }
        if( lineOfDestLang == "" )
        {
            cerr<<"\n"
                <<"        WARNING:null sentence in source file in line:"<<lineNo<<"\n";
            continue;
        }
        if( lineOfAlignedInfo == "" )
        {
            cerr<<"\n"
                <<"        WARNING:null sentence in aligned file in line:"<<lineNo<<"\n";
            continue;
        }

        vector< string > sourceLang;
        vector< string > destLang;
        vector< string > alignmentTable;
        vector< size_t > alignedCountSourceLang;
        vector< size_t > alignedCountDestLang;

        split( lineOfSourceLang, ' ', sourceLang );
        split( lineOfDestLang, ' ', destLang );
        split( lineOfAlignedInfo, ' ', alignmentTable );

        alignedCountSourceLang.assign( sourceLang.size(), 0 );
        alignedCountDestLang.assign( destLang.size(), 0 );

        for( vector< string >::iterator iter_alignmentTable = alignmentTable.begin(); iter_alignmentTable != alignmentTable.end(); ++iter_alignmentTable )
        {
            unsigned int sourceWordNo, destWordNo;
#ifdef WIN32
            if( ! sscanf_s( iter_alignmentTable->c_str(), "%u-%u", &sourceWordNo, &destWordNo ) )
#else
            if( ! sscanf( iter_alignmentTable->c_str(), "%u-%u", &sourceWordNo, &destWordNo ) )
#endif
            {
                cerr<<"\n"
                    <<"        WARNING:"<<*iter_alignmentTable<<" is not num-num in line:"<<lineNo<<endl;
                break;
            }
            if( sourceWordNo >= sourceLang.size() || destWordNo >= destLang.size() )
            {
                cerr<<"\n"
                    <<"        WARNING:"        <<"out of bound in line:"    <<lineNo<<"\n"
                    <<"        sourceLang.size:"<<sourceLang.size()<<"\tnow position:" <<sourceWordNo
                    <<" (valid:0-"      <<sourceLang.size()-1        <<")\n"
                    <<"        destLang.size:"  <<destLang.size()<<"\tnow position:" <<destWordNo
                    <<" (valid:0-"      <<destLang.size()-1          <<")\n"
                    <<flush;
                break;
            }

            ++alignedCountSourceLang[ sourceWordNo ];
            ++alignedCountDestLang[ destWordNo ];
            ++totalSourceLang[ sourceLang.at( sourceWordNo ) ];
            ++totalDestLang[ destLang.at( destWordNo ) ];
            ++wordTranslation[ sourceLang.at( sourceWordNo ) ][ destLang.at( destWordNo ) ];

            map< string, pair< string, double > >::iterator findStemmingList;
            if( ( findStemmingList = stem.stemmingList.find( destLang.at( destWordNo ) ) ) != stem.stemmingList.end() )
            {
                ++stem.wordTranslationStem[ sourceLang.at( sourceWordNo ) ][ findStemmingList->second.first ];
                ++stem.totalDestLangStem[ findStemmingList->second.first ];
            }
        }

        map< string, map< string, double > >::size_type posOfSourceLang = 0;
        map< string, map< string, double > >::size_type posOfDestLang = 0;

        for( vector< size_t >::iterator iter = alignedCountSourceLang.begin(); iter != alignedCountSourceLang.end(); ++iter )
        {
            if( *iter == 0 )
            {
                ++wordTranslation[ sourceLang.at( posOfSourceLang ) ][ "NULL" ];
                ++totalSourceLang[ sourceLang.at( posOfSourceLang ) ];
                ++totalDestLang[ "NULL" ];

            }

            ++posOfSourceLang;
        }

        for( vector< size_t >::iterator iter = alignedCountDestLang.begin(); iter != alignedCountDestLang.end(); ++iter )
        {
            if( *iter == 0 )
            {
                ++wordTranslation[ "NULL" ][ destLang.at( posOfDestLang ) ];
                ++totalSourceLang[ "NULL" ];
                ++totalDestLang[ destLang.at( posOfDestLang ) ];
                map< string, pair< string, double > >::iterator findStemmingList;
                if( ( findStemmingList = stem.stemmingList.find( destLang.at( posOfDestLang ) ) ) != stem.stemmingList.end() )
                {
                    ++stem.wordTranslationStem[ "NULL" ][ findStemmingList->second.first ];
                    ++stem.totalDestLangStem[ findStemmingList->second.first ];
                }
            }
            ++posOfDestLang;
        }
        if( lineNo % 10000 == 0 )
            cout<<"\r        processed "<<lineNo<<" lines."<<flush;
    }
    cout<<"\r        processed "<<lineNo<<" lines."<<flush;

    for( map< string, map< string, double > >::iterator iter_outer = wordTranslation.begin(); iter_outer != wordTranslation.end(); ++iter_outer )
    {
        for( map< string, double >::iterator iter_inner = iter_outer->second.begin(); iter_inner != iter_outer->second.end(); ++iter_inner )
        {

            map< string, pair< string, double > >::iterator findStemList;
            if( ( findStemList = stem.stemmingList.find( iter_inner->first ) ) != stem.stemmingList.end() )
            {
                outfileLexS2D <<iter_inner->first<<" "<<iter_outer->first<<" "
                              <<findStemList->second.second*stem.wordTranslationStem[ iter_outer->first ][ findStemList->second.first ]/totalSourceLang[ iter_outer->first ]
//                <<" "<<findStemList->second.second<<" "<<stem.wordTranslationStem[iter_outer->first][ findStemList->second.first ]<<" "<<totalSourceLang[ iter_outer->first ]<<" "              
                <<"\n";
                outfileLexD2S <<iter_outer->first<<" "<<iter_inner->first<<" "
                              <<stem.wordTranslationStem[ iter_outer->first ][ findStemList->second.first ]/stem.totalDestLangStem[ findStemList->second.first ]
                              <<"\n";
            }
            else
            {
                outfileLexS2D <<iter_inner->first<<" "<<iter_outer->first<<" "
                              <<iter_inner->second/totalSourceLang[ iter_outer->first ]<<"\n";
                outfileLexD2S <<iter_outer->first<<" "<<iter_inner->first<<" "
                              <<iter_inner->second/totalDestLang[ iter_inner->first ]<<"\n";
            }
        }
    }

    infileStemming.close();
    infileSourceLang.close();
    infileDestLang.close();
    infileAlignedInfo.close();
    outfileLexS2D.close();
    outfileLexD2S.close();
    return true;
}

bool GenerateLexicalTable::generateLexicalTable( OptionsOfLexTable &options )
{
    ifstream infileSourceLang(  options.sourceLangFileName.c_str()  );
    ifstream infileDestLang(    options.targetLangFileName.c_str()  );
    ifstream infileAlignedInfo( options.alignedInfoFileName.c_str() );
    if( !infileSourceLang || !infileDestLang || !infileAlignedInfo )
    {
        cerr<<"        ERROR   : Could not open file"<<"\n"
            <<"                  \""<<options.sourceLangFileName <<"\" or"<<"\n"
            <<"                  \""<<options.targetLangFileName <<"\" or"<<"\n"
            <<"                  \""<<options.alignedInfoFileName<<"\"\n";
        exit( 1 );
    }

    ofstream outfileLexS2D( options.lexTabS2TFileName.c_str() );
    ofstream outfileLexD2S( options.lexTabT2SFileName.c_str() );

    if( options.nthread <= 1 )
        options.nthread = 0;

#ifdef __DEBUGLEXTAB__
    cerr<<"        nthread="<<options.nthread<<endl;
#endif

    if( options.nthread == 0 )
    {
        readLinesFromCorpusNormal( infileSourceLang, infileDestLang, infileAlignedInfo, options );
        outputLexicalTableNormal( outfileLexS2D, outfileLexD2S );
    }
    else
    {
        readLinesFromCorpusMultiThread( infileSourceLang, infileDestLang, infileAlignedInfo, options );
        outputLexicalTableMultiThread( outfileLexS2D, outfileLexD2S );
    }


    infileSourceLang.close();
    infileDestLang.close();
    infileAlignedInfo.close();
    outfileLexS2D.close();
    outfileLexD2S.close();
    return true;
}

bool GenerateLexicalTable::readLinesFromCorpusNormal( 
ifstream &infileSourceLang , 
ifstream &infileTargetLang , 
ifstream &infileAlignedInfo, 
OptionsOfLexTable &options   )
{
    string lineOfSourceLang, lineOfTargetLang, lineOfAlignedInfo;
    size_t lineNo = 0;

    while( getline( infileSourceLang, lineOfSourceLang ) && getline( infileTargetLang, lineOfTargetLang ) && getline( infileAlignedInfo, lineOfAlignedInfo ) )
    {
        ++lineNo;

        BasicMethod be;
        be.clearIllegalChar( lineOfSourceLang );
        be.clearIllegalChar( lineOfTargetLang );
        be.clearIllegalChar( lineOfAlignedInfo );

        if( lineOfSourceLang == "" )
        {
            cerr<<"\n"
                <<"        WARNING : NULL sentence in "<<lineNo<<" line in file \""<<options.sourceLangFileName<<"\"\n";
            continue;
        }
        if( lineOfTargetLang == "" )
        {
            cerr<<"\n"
                <<"        WARNING : NULL sentence in "<<lineNo<<" line in file \""<<options.targetLangFileName<<"\"\n";
            continue;
        }
        if( lineOfAlignedInfo == "" )
        {
            cerr<<"\n"
                <<"        WARNING : NULL sentence in "<<lineNo<<" line in file \""<<options.alignedInfoFileName<<"\"\n";
            continue;
        }

        statisticalWordCountNormal( lineOfSourceLang, lineOfTargetLang, lineOfAlignedInfo, lineNo );        

        if( lineNo % 10000 == 0 )
            cout<<"\r        processed "<<lineNo<<" lines."<<flush;

    }
    cout<<"\r        processed "<<lineNo<<" lines."<<flush;
    return true;
}

bool GenerateLexicalTable::statisticalWordCountNormal( string &lineOfSourceLang, string &lineOfTargetLang, string &lineOfAlignedInfo, size_t &lineNo )
{
    vector< string > sourceLang;
    vector< string > targetLang;
    vector< string > alignmentTable;
    vector< size_t > alignedCountSourceLang;
    vector< size_t > alignedCountDestLang;

    split( lineOfSourceLang , ' ', sourceLang     );
    split( lineOfTargetLang , ' ', targetLang     );
    split( lineOfAlignedInfo, ' ', alignmentTable );

    alignedCountSourceLang.assign( sourceLang.size(), 0 );
    alignedCountDestLang.assign(   targetLang.size(), 0 );

    for( vector< string >::iterator iter_alignmentTable = alignmentTable.begin(); iter_alignmentTable != alignmentTable.end(); ++iter_alignmentTable )
    {
        unsigned int sourceWordNo, destWordNo;
#ifdef WIN32
        if( ! sscanf_s( iter_alignmentTable->c_str(), "%u-%u", &sourceWordNo, &destWordNo ) )
#else
        if( ! sscanf( iter_alignmentTable->c_str(), "%u-%u", &sourceWordNo, &destWordNo ) )
#endif
        {
            cerr<<"\n"
                <<"        WARNING : "<<*iter_alignmentTable<<" is not num-num in line:"<<lineNo<<endl;
            break;
        }
        if( sourceWordNo >= sourceLang.size() || destWordNo >= targetLang.size() )
        {
            cerr<<"\n"
                <<"        WARNING : "        <<"Out of bound in "<<lineNo<<" line.\n"
                <<"                  srcLang.size:"<<sourceLang.size()<<"\tnow position:" <<sourceWordNo
                <<" (valid: 0-"      <<sourceLang.size()-1        <<")\n"
                <<"                  tgtLang.size:"  <<targetLang.size()<<"\tnow position:" <<destWordNo
                <<" (valid: 0-"      <<targetLang.size()-1          <<")\n"
                <<flush;
            break;
        }

        ++alignedCountSourceLang[ sourceWordNo ];
        ++alignedCountDestLang[ destWordNo ];
        ++totalSourceLang[ sourceLang.at( sourceWordNo ) ];
        ++totalDestLang[   targetLang.at( destWordNo ) ];
        ++wordTranslation[ sourceLang.at( sourceWordNo ) ][ targetLang.at( destWordNo ) ];
    }

    map< string, map< string, double > >::size_type posOfSourceLang = 0;
    map< string, map< string, double > >::size_type posOfDestLang = 0;

    for( vector< size_t >::iterator iter = alignedCountSourceLang.begin(); iter != alignedCountSourceLang.end(); ++iter )
    {
        if( *iter == 0 )
        {
            ++wordTranslation[ sourceLang.at( posOfSourceLang ) ][ "NULL" ];
            ++totalSourceLang[ sourceLang.at( posOfSourceLang ) ];
            ++totalDestLang[ "NULL" ];
        }

        ++posOfSourceLang;
    }

    for( vector< size_t >::iterator iter = alignedCountDestLang.begin(); iter != alignedCountDestLang.end(); ++iter )
    {
        if( *iter == 0 )
        {
            ++wordTranslation[ "NULL" ][ targetLang.at( posOfDestLang ) ];
            ++totalSourceLang[ "NULL" ];
            ++totalDestLang[ targetLang.at( posOfDestLang ) ];
        }
        ++posOfDestLang;
    }  
    return true;
}

bool GenerateLexicalTable::outputLexicalTableNormal( ofstream &outfileLexS2D, ofstream &outfileLexD2S )
{
	ofstream lex_log("lex.log");
    for( map< string, map< string, double > >::iterator iter_outer = wordTranslation.begin(); iter_outer != wordTranslation.end(); ++iter_outer )
    {
        for( map< string, double >::iterator iter_inner = iter_outer->second.begin(); iter_inner != iter_outer->second.end(); ++iter_inner )
        {
            outfileLexS2D   <<iter_inner->first<<" "<<iter_outer->first<<" "
                <<iter_inner->second/totalSourceLang[ iter_outer->first ]<<"\n";
			lex_log<<iter_inner->first<<" "<<iter_outer->first<<" "<<iter_inner->second<<" "<<totalSourceLang[iter_outer->first]<<endl;

            outfileLexD2S   <<iter_outer->first<<" "<<iter_inner->first<<" "
                <<iter_inner->second/totalDestLang[ iter_inner->first ]<<"\n";
		}
    }
	lex_log.close();
    return true;
}

bool GenerateLexicalTable::readLinesFromCorpusMultiThread( 
ifstream &infileSourceLang , 
ifstream &infileTargetLang , 
ifstream &infileAlignedInfo, 
OptionsOfLexTable &options   )
{
    string lineOfSourceLang;
    string lineOfTargetLang;
    string lineOfAlignedInfo;
    size_t lineNo = 0;
 
    while( true )
    {
        vector< GenerateLexicalTable > threads;
        size_t threadNum = 0;

        for( int t = 0; t < options.nthread; ++t )
        {
            if( getline( infileSourceLang , lineOfSourceLang  ) &&
                getline( infileTargetLang , lineOfTargetLang  ) &&
                getline( infileAlignedInfo, lineOfAlignedInfo )    )
            {   
                ++threadNum;
                ++lineNo;
                BasicMethod be;
                be.clearIllegalChar( lineOfSourceLang );
                be.clearIllegalChar( lineOfTargetLang );
                be.clearIllegalChar( lineOfAlignedInfo );

                if( lineOfSourceLang == "" )
                {
                    cerr<<"\n"
                        <<"        WARNING : NULL sentence in "<<lineNo<<" line in file \""<<options.sourceLangFileName<<"\"\n";
                    continue;
                }
                if( lineOfTargetLang == "" )
                {
                    cerr<<"\n"
                        <<"        WARNING : NULL sentence in "<<lineNo<<" line in file \""<<options.targetLangFileName<<"\"\n";
                    continue;
                }
                if( lineOfAlignedInfo == "" )
                {
                    cerr<<"\n"
                        <<"        WARNING : NULL sentence in "<<lineNo<<" line in file \""<<options.alignedInfoFileName<<"\"\n";
                    continue;
                }

                GenerateLexicalTable glt;
                glt.gltp.lineOfSourceLang = lineOfSourceLang;
                glt.gltp.lineOfTargetLang = lineOfTargetLang;
                glt.gltp.lineOfAlignedInfo = lineOfAlignedInfo;
                glt.gltp.lineNo = lineNo;
                glt.gltp.threadId = t;
                glt.gltp.nthread = options.nthread;

                threads.push_back( glt );
            }
            else
            {
                for( int t = 0; t < threadNum; ++t )
                    threads.at( t ).Start();
                for( int t = 0; t < threadNum; ++t )
                    threads.at( t ).Join();
                cout<<"\r        processed "<<lineNo<<" lines. ["<<options.nthread<<" threads]"<<flush;
                return true;
            }
        }

        for( int t = 0; t < options.nthread; ++t )
            threads.at( t ).Start();
        for( int t = 0; t < options.nthread; ++t )
            threads.at( t ).Join();
        
        if( lineNo % 10000 == 0 )
            cout<<"\r        processed "<<lineNo<<" lines. ["<<options.nthread<<" threads]"<<flush;
    }
    cout<<"\r        processed "<<lineNo<<" lines. ["<<options.nthread<<" threads]"<<flush;
    
    return true;
}

bool GenerateLexicalTable::statisticalWordCountMultiThread( GeneLexTabParam &gltp )
{
#ifdef __DEBUGLEXTAB__
    cerr<<"lineNo="<<gltp.lineNo<<" threadId="<<gltp.threadId<<" nthread="<<gltp.nthread<<endl
        <<"src="<<gltp.lineOfSourceLang<<"\ntgt="<<gltp.lineOfTargetLang<<"\n"<<endl;
#endif

    vector< string > sourceLang;
    vector< string > targetLang;
    vector< string > alignmentTable;
    vector< size_t > alignedCountSourceLang;
    vector< size_t > alignedCountDestLang;

    split( gltp.lineOfSourceLang , ' ', sourceLang     );
    split( gltp.lineOfTargetLang , ' ', targetLang     );
    split( gltp.lineOfAlignedInfo, ' ', alignmentTable );

    alignedCountSourceLang.assign( sourceLang.size(), 0 );
    alignedCountDestLang.assign(   targetLang.size(), 0 );

    for( vector< string >::iterator iter_alignmentTable = alignmentTable.begin(); iter_alignmentTable != alignmentTable.end(); ++iter_alignmentTable )
    {
        unsigned int sourceWordNo, destWordNo;
#ifdef WIN32
        if( ! sscanf_s( iter_alignmentTable->c_str(), "%u-%u", &sourceWordNo, &destWordNo ) )
#else
        if( ! sscanf( iter_alignmentTable->c_str(), "%u-%u", &sourceWordNo, &destWordNo ) )
#endif
        {
            cerr<<"\n"
                <<"        WARNING : "<<*iter_alignmentTable<<" is not num-num in line:"<<gltp.lineNo<<endl;
            break;
        }
        if( sourceWordNo >= sourceLang.size() || destWordNo >= targetLang.size() )
        {
            cerr<<"\n"
                <<"        WARNING : "        <<"Out of bound in "<<gltp.lineNo<<" line.\n"
                <<"                  srcLang.size:"<<sourceLang.size()<<"\tnow position:" <<sourceWordNo
                <<" (valid: 0-"      <<sourceLang.size()-1        <<")\n"
                <<"                  tgtLang.size:"  <<targetLang.size()<<"\tnow position:" <<destWordNo
                <<" (valid: 0-"      <<targetLang.size()-1          <<")\n"
                <<flush;
            break;
        }

        ++alignedCountSourceLang[ sourceWordNo ];
        ++alignedCountDestLang[ destWordNo ];
        CLock lock(g_Lock);  
        ++globalTotalSourceLang[ sourceLang.at( sourceWordNo ) ];
        ++globalTotalTargetLang[   targetLang.at( destWordNo ) ];
        ++globalWordTranslation[ sourceLang.at( sourceWordNo ) ][ targetLang.at( destWordNo ) ];
#ifdef _WIN32
        CLock unlock(g_Lock);  
#endif

    }

    map< string, map< string, double > >::size_type posOfSourceLang = 0;
    map< string, map< string, double > >::size_type posOfDestLang = 0;

    for( vector< size_t >::iterator iter = alignedCountSourceLang.begin(); iter != alignedCountSourceLang.end(); ++iter )
    {
        if( *iter == 0 )
        {
            CLock lock(g_Lock);  
            ++globalWordTranslation[ sourceLang.at( posOfSourceLang ) ][ "NULL" ];
            ++globalTotalSourceLang[ sourceLang.at( posOfSourceLang ) ];
            ++globalTotalTargetLang[ "NULL" ];
/*
#ifdef _WIN32
            CLock unlock(g_Lock);  
#endif
*/
        }

        ++posOfSourceLang;
    }

    for( vector< size_t >::iterator iter = alignedCountDestLang.begin(); iter != alignedCountDestLang.end(); ++iter )
    {
        if( *iter == 0 )
        {
            CLock lock(g_Lock);  
            ++globalWordTranslation[ "NULL" ][ targetLang.at( posOfDestLang ) ];
            ++globalTotalSourceLang[ "NULL" ];
            ++globalTotalTargetLang[ targetLang.at( posOfDestLang ) ];
/*
#ifdef _WIN32
            CLock unlock(g_Lock);  
#endif
*/
        }
        ++posOfDestLang;
    }  
    return true;
}

void GenerateLexicalTable::Run()
{
    statisticalWordCountMultiThread( gltp );
}

bool GenerateLexicalTable::outputLexicalTableMultiThread( ofstream &outfileLexS2D, ofstream &outfileLexD2S )
{
    for( map< string, map< string, double > >::iterator iter_outer = globalWordTranslation.begin(); iter_outer != globalWordTranslation.end(); ++iter_outer )
    {
        for( map< string, double >::iterator iter_inner = iter_outer->second.begin(); iter_inner != iter_outer->second.end(); ++iter_inner )
        {
            outfileLexS2D   <<iter_inner->first<<" "<<iter_outer->first<<" "
                <<iter_inner->second/globalTotalSourceLang[ iter_outer->first ]<<"\n";
            outfileLexD2S   <<iter_outer->first<<" "<<iter_inner->first<<" "
                <<iter_inner->second/globalTotalTargetLang[ iter_inner->first ]<<"\n";
        }
    }
    return true;
}
