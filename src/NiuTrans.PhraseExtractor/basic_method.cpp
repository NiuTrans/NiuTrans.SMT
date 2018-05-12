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
 * phrase extraction and scorer; basic_method.cpp
 *
 * $Version:
 * 0.4.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2012/05/22, add rmEndSpace() and convertAlignmenttoInv() methods, modified by Qiang Li.
 *
 */


#include "basic_method.h"
using namespace basic_method;

string BasicMethod::mergeAlignedInfo( string &nowAlignedInfo, string &nextAlignedInfo )
{
    if( nowAlignedInfo == nextAlignedInfo )
        return nowAlignedInfo;

    vector< set< size_t > > alignedInfoV;
    string alignedInfo;
    typedef vector< set< size_t > >::size_type POS;

    POS nowAlignedInfoStart  = 0;
    POS nowAlignedInfoEnd    = 0 ;
    POS nextAlignedInfoStart = 0;
    POS nextAlignedInfoEnd   = 0;

    while( ( nowAlignedInfoStart  = nowAlignedInfo.find( "(" , nowAlignedInfoStart  ) ) != string::npos
        && ( nowAlignedInfoEnd    = nowAlignedInfo.find( ")" , nowAlignedInfoStart  ) ) != string::npos
        && ( nextAlignedInfoStart = nextAlignedInfo.find( "(", nextAlignedInfoStart ) ) != string::npos 
        && ( nextAlignedInfoEnd   = nextAlignedInfo.find( ")", nextAlignedInfoStart ) ) != string::npos )
    {
        set< size_t > tempSet;
        string tempNowAlignedInfo  = nowAlignedInfo.substr(  ++nowAlignedInfoStart , nowAlignedInfoEnd  - nowAlignedInfoStart  );
        string tempNextAlignedInfo = nextAlignedInfo.substr( ++nextAlignedInfoStart, nextAlignedInfoEnd - nextAlignedInfoStart ); 
        vector< string > dest;
        split( tempNowAlignedInfo, ',', dest );
        for( vector< string >::iterator iter = dest.begin(); iter != dest.end(); ++iter )
        {
            if( !iter->empty() )
                tempSet.insert( atoi( iter->c_str() ) );
        }
        dest.clear();
        split( tempNextAlignedInfo, ',', dest );
        for( vector< string >::iterator iter = dest.begin(); iter != dest.end(); ++iter )
        {
            if( !iter->empty() )
                tempSet.insert( atoi( iter->c_str() ) );
        }
        alignedInfoV.push_back( tempSet );
    }

    for( vector< set< size_t > >::iterator iter = alignedInfoV.begin(); iter != alignedInfoV.end(); ++iter )
    {
        alignedInfo += "(";
        for( set< size_t >::iterator iter_inner = iter->begin(); iter_inner != iter->end(); ++iter_inner )
        {
            if( iter_inner == iter->begin() )
            {
                size_t temp = *iter_inner;
                alignedInfo += size_tToString( temp );
            }
            else
            {
                size_t temp = *iter_inner;
                alignedInfo += ( "," + size_tToString( temp ) );
            }
        }
        alignedInfo += ") ";
    } 
    return alignedInfo;
}

bool BasicMethod::split( const string &phraseTable, const char &splitchar, vector< string > &dest )
{
    string::size_type splitPos = phraseTable.find( splitchar );
    string::size_type lastSplitPos = 0;
    string tempString;
    while( splitPos != string::npos )
    {
        tempString = phraseTable.substr( lastSplitPos, splitPos - lastSplitPos );
        if( !tempString.empty() )
        {
            dest.push_back( tempString );
        }
        lastSplitPos = splitPos + 1;
        splitPos = phraseTable.find( splitchar, lastSplitPos );
    }

    if( lastSplitPos < phraseTable.size() )
    {
        tempString = phraseTable.substr( lastSplitPos );
        dest.push_back( tempString );
    }

    if( !dest.empty() )
        return true;
    else
        return false;
}

bool BasicMethod::splitWithStr( const string &src, const string &separator, vector< string > &dest )
{
    string str = src;
    string substring;
    string::size_type start = 0, index = 0;
    string::size_type separator_len = separator.size();

    while( index != string::npos && start < src.size() )
    {
        index = src.find( separator, start );
        if( index == 0 )
        {
            start = start + separator_len;
            continue;
        }
        if( index == string::npos )
        {
            dest.push_back( src.substr( start ) );
            break;
        }
        dest.push_back( src.substr(start,index-start) );
        start = index + separator_len;
    }
    return true;
}


string BasicMethod::size_tToString( size_t &source )
{
    stringstream oss;
    oss << source;
    return oss.str();
}

bool BasicMethod::clearIllegalChar( string &str )
{
    string::size_type pos = 0;
    while( ( pos = str.find( "\r", pos ) ) != string::npos )
    {
        str.replace( pos, 1, "" );
    }
    pos = 0;
    while( ( pos = str.find( "\n", pos ) ) != string::npos )
    {
        str.replace( pos, 1, "" );
    }
    return true;
}

bool BasicMethod::toUpper( string &str )
{
    for( string::size_type i = 0; i < str.size(); ++i )
    {
        if( islower( str.at( i ) ) )
            str.at( i ) = toupper( str.at( i ) );
    }
    return true;
}

bool BasicMethod::toLower( string &str )
{
    for( string::size_type i = 0; i < str.size(); ++i )
    {
        if( isupper( str.at( i ) ) )
            str.at( i ) = tolower( str.at( i ) );
    }
    return true;
}

bool BasicMethod::rmEndSpace( string &str )
{
#ifdef __DEBUGBM__
    cerr<<"**str:"<<str<<"**\n"<<flush;
#endif

    while( str[ str.length() - 1 ] == ' ' )
    {
        str = str.substr( 0, str.length() - 1 );
#ifdef __DEBUGBM__
        cerr<<"**str:"<<str<<"**\n"<<flush;
#endif
    }
    return true;
}

bool BasicMethod::convertAlignmenttoInv( string &str )
{
#ifdef __DEBUGBM__
    cerr<<"alignment:"<<str<<"\n"<<flush;
#endif
    vector< string > alignmentVec;
    split( str, ' ', alignmentVec );
    str = "";
    for( vector< string >::iterator iter = alignmentVec.begin(); iter != alignmentVec.end(); ++iter )
    {
        vector< string > posVec;
        split( *iter, '-', posVec );
        str += posVec.at( 1 ) + "-" + posVec.at( 0 ) + " ";
    }
    str = str.substr( 0, str.length() - 1 );
#ifdef __DEBUGBM__
    cerr<<"alignmentInv:"<<str<<"\n"<<flush;
#endif

    return true;
}

bool BasicMethod::deleteFileList( vector<string> &fileList, SystemCommand &systemCommand )
{
    clock_t start,finish;
    string command;

    for( vector< string >::iterator iter = fileList.begin(); iter != fileList.end(); ++iter )
    {
        command = systemCommand.del + *iter;
        start = clock();
        cerr<<"Delete\n"
            <<"        command : "<<command<<"\n"
            <<"        input   : "<<*iter<<"\n"
            <<flush;
        system( command.c_str() );
        finish = clock();
        cerr<<"        time    : "<<(double)(finish-start)/CLOCKS_PER_SEC<<"s\n"
            <<flush;
    }

    fileList.clear();

    return true;
}
