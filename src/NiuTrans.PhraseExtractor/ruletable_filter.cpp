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
 * phrase extraction and scorer; FIL_filter_final_table.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 
 *
 */

#include "ruletable_filter.h"
using namespace ruletable_filter;

bool FilterFinalTable::filterFinalTable( string &devSet, string &phraseTable, string &filterTable, int &maxlen, int &refnum )
{
    if( maxlen <= 0 )
    {
        maxlen = 10;
    }
    if( refnum <=0 )
    {
        refnum = 4;
    }

    ifstream devSetFile( devSet.c_str() );

    if( !devSetFile )
    {
        cerr<<"ERROR: Can not open file "<<devSet<<endl;
        exit( 1 );
    }

    string line;
    size_t lineNo = 0;
    while( getline( devSetFile, line ) )
    {
        ++lineNo;
        vector< string > sentence;
        clearIllegalChar( line );
        splitWithStr( line, " |||| ", sentence );
//      transform( sentence.at( 0 ).begin(), sentence.at( 0 ).end(), sentence.at( 0 ).begin(), tolower );
        toLower( sentence.at( 0 ) );
//      cout<<sentence.at( 0 )<<endl;
        for( int i = 0; i < refnum + 1; ++i )
        {
            getline( devSetFile, line );
        }
        if( sentence.at( 0 ) == "" )
        {
            cerr<<"ERROR: Dev Set have empty line in "<<lineNo<<endl;
            exit( 1 );
        }
        vector< string > word;
        split( sentence.at( 0 ), ' ', word );
        for( vector< string >::size_type i = 0; i < word.size(); i++ )
        {
            string phrase = word.at( i );
           // ++devTableLex[ phrase ];
			if(devTableLex.find(phrase) == devTableLex.end()){//change by yang
					devTableLex[phrase] =1;	
			}
            for( vector< string >::size_type j = i + 1; j < word.size() && j - i < vector< string >::size_type( maxlen ); ++j )
            {
                phrase += " " + word.at( j );
				if(devTableLex.find(phrase) == devTableLex.end()){// change by yang
					devTableLex[phrase] =1;	
				}
             //   ++devTableLex[ phrase ];
            }
        }
        if( ( lineNo % 1000 ) == 0 )
            cerr<<"\rDev Set has "<<lineNo<<" sentences"<<flush;
    }

    cerr<<"\rDev Set has "<<lineNo<<" sentences\n"<<flush;
    cerr<<"    Loading "<<devTableLex.size()<<" phrases!\n"<<flush;

    devSetFile.close();
    ofstream filterTableFile( filterTable.c_str() );
    if( !filterTableFile )
    {
        cerr<<"ERROR: Can not write file "<<filterTable<<endl;
        exit( 1 );
    }

    ifstream phraseTableFile( phraseTable.c_str() );
    
    lineNo = 0;
    size_t filterNum = 0;
    size_t findNum = 0;
    cerr<<"Filter phrase translation with dev set:\n"<<flush;
	vector< string > phrasePairs;
    while( getline( phraseTableFile, line ) )
    {
        ++lineNo;
        phrasePairs.clear();
        clearIllegalChar( line );
        splitWithStr( line, " ||| ", phrasePairs );

        replaceLabel( phrasePairs.at( 0 ) );

        //  toLower( phrasePairs.at( 0 ) );
        if( phrasePairs.at( 0 ).find( " #X " ) == string::npos && 
            phrasePairs.at( 0 ).find( "#X " ) == string::npos && 
            phrasePairs.at( 0 ).find( " #X" ) == string::npos )
        {
            string temp( phrasePairs.at( 0 ) );
            toLower( temp );
            //if( devTableLex[ temp ] != 0 )
			if(devTableLex.find(temp)!= devTableLex.end())// change by yang
            {
                ++findNum;
                filterTableFile<<line<<"\n";
            }
            else
            {
                ++filterNum; 
            }
        }
        else
        {
            vector< string > phraseFragment;
            if( phrasePairs.at( 0 ).find( "#X " ) == 0 )
                phrasePairs.at( 0 ) = " " + phrasePairs.at( 0 );
            if( phrasePairs.at( 0 ).rfind( " #X") == phrasePairs.at( 0 ).size() - 3 )
                phrasePairs.at( 0 ) += " ";
            splitWithStr( phrasePairs.at( 0 ), " #X ", phraseFragment );
            bool foundPhraseFlag = true;
            for( vector< string >::iterator iter = phraseFragment.begin(); iter != phraseFragment.end(); ++iter )
            {
                string temp( *iter );
                toLower( temp );
                if( devTableLex.find(temp)!= devTableLex.end() )// change by yang
                    continue;
                else
                {
                    foundPhraseFlag = false;
                    ++filterNum;
                    break;
                }
            }
            if( foundPhraseFlag )
            {
                ++findNum;
                filterTableFile<<line<<"\n";
            }
        }
    
        if( lineNo % 100000 == 0 )
        {
			//vector<string>().swap(phrasePairs);
			//cout<<"phrasePair.capacity:"<<phrasePairs.capacity()<<endl;
			//cout<<"devTableLex.size:"<<devTableLex.size()<<endl;
            cerr<<"\r    lineNo:"<<lineNo<<"  "<<"filterNum:"<<filterNum<<"  "<<"reserveNum:"<<findNum<<flush;
        }
    }
    cerr<<"\r    lineNo:"<<lineNo<<"  "<<"filterNum:"<<filterNum<<"  "<<"reserveNum:"<<findNum<<endl;
    

    phraseTableFile.close();
    filterTableFile.close();
    return true;
}

bool FilterFinalTable::replaceLabel( string &inputstr )
{
    if( inputstr.find( "#" ) == string::npos )
        return true;

    vector< string > temp;
    split( inputstr, ' ', temp );
    
    inputstr = "";

    for( vector< string >::iterator iter = temp.begin(); iter != temp.end(); ++iter )
    {
        if( iter->at( 0 ) == '#' )
            inputstr += "#X ";
        else
            inputstr += *iter + " ";
    }
    inputstr = inputstr.substr( 0, inputstr.length() - 1 );

    return true;
}

//bool FilterTableWithNum::filterTableWithNum( const string &phraseTransTableName, const string &destTransTableName, const int &outputMaxNumStrict )
bool FilterTableWithNum::filterTableWithNum( const OptionsOfFilterNum &options )
{
    //  ifstream infileTransTable( phraseTransTableName.c_str() );
    ifstream infileTransTable( options.sourceTransTableName.c_str() );

    if( !infileTransTable )
    {
        cerr<<"        Error: Can not read file "<<options.sourceTransTableName<<"\n"<<flush;
        exit( 0 );
    }

    //  ofstream outfileTransTable( destTransTableName.c_str() );
    ofstream outfileTransTable( options.targetTransTableName.c_str() );

    if( !outfileTransTable )
    {
        cerr<<"        Error: Can not write file "<<options.targetTransTableName<<"\n"<<flush;
        exit( 0 );
    }

    string line;
    size_t lineNo = 0;
    size_t reserved = 0;
    size_t domain = 0;
    while( getline( infileTransTable, line ) )
    {
        ++lineNo;
        clearIllegalChar( line );

        vector< string > dest;

        splitWithStr( line, " ||| ", dest );

        if( lineNo == 1 )
        {
            if( options.tableFormat == "syntax" || options.tableFormat == "hierarchy" || options.tableFormat == "SAMT" )
            {
                domain = 3;
            }
            else if( options.tableFormat == "phrase" )
            {
                domain = 2;            
            }
            else if( options.tableFormat == "auto" )
            {
                vector< string > tempDomain2;
                split( dest.at( 2 ), ' ', tempDomain2 );
                if( tempDomain2.size() == 1 )
                    domain = 3;
                else if( tempDomain2.size() >= 6 )
                    domain = 2;
            }
        }

        vector< string > destScore;
        split( dest.at( domain ), ' ', destScore );

        double score1 = atof( destScore.at( 0 ).c_str() );

        if( dest.at( 0 ) == lastSourceLang )
        {
            scoreItems.insert( make_pair( score1, line ) );
        }
        else
        {
            if( !scoreItems.empty() )
            {
                int outputMaxNum = 1;
                for( multimap< double, string >::reverse_iterator riterScoreItems = scoreItems.rbegin(); riterScoreItems != scoreItems.rend(); ++riterScoreItems )
                {
                    //                    if( outputMaxNum > outputMaxNumStrict )
                    if( outputMaxNum > options.outputMaxNumStrict )
                        break;
                    else
                    {
                        ++reserved;
                        outfileTransTable<<riterScoreItems->second<<"\n";
                    }
                    ++outputMaxNum;
                }
                scoreItems.clear();
            }
            scoreItems.insert( make_pair( score1, line ) );
            lastSourceLang = dest.at( 0 );
        } 
        if( lineNo % 100000 == 0 )
            cerr<<"\r        process "<<lineNo<<" lines."<<flush;
    }
    cerr<<"\r        process "<<lineNo<<" lines.\n"<<flush;

    if( !scoreItems.empty() )
    {
        int outputMaxNum = 1;
        for( multimap< double, string>::reverse_iterator riterScoreItems = scoreItems.rbegin(); riterScoreItems != scoreItems.rend(); ++riterScoreItems )
        {
            //            if( outputMaxNum > outputMaxNumStrict )
            if( outputMaxNum > options.outputMaxNumStrict )
            {
                break;
            }
            else
            {
                ++reserved;
                outfileTransTable<<riterScoreItems->second<<"\n";
            }
            ++outputMaxNum;
        }
        scoreItems.clear();
    }

    cerr<<"        reserved : "<<reserved<<"\n"<<flush;
    return true;
}

