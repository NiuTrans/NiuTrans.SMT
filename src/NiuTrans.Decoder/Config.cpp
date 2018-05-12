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
 * Config; Config.cpp
 *
 * $Version:
 * 1.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 */

#include "Config.h"

namespace smt {


/*
 @ Function:
 * free a string
 *
 @ Arguments:
 * pointer to the string (void*)
 */
void FreeAsciiStr( const void * var ) {

    char* str = ( char* ) var;
    delete[] str;

}

/*
 @ Function:
 * create and initialize a new configuration manager
 * load all system's settings
 *
 @ Arguments:
 * 1) number of command-line arguments (int)
 * 2) array of command-line arguments (char**)
 * 3) configuration file's path (char*)
 */
void Configer::Create( int argc, const char** argv ) {

    TimeUtil timer;
    hashTab = new HashTable( 1024, 1.0, FreeAsciiStr );
    char configFN[MAX_FILE_NAME_LENGTH] = CONFIG_FILE_NAME;

    for(int i = 0; i < argc - 1; i++){
        if(strcmp(argv[i], "-config") == 0){
            memset(configFN, 0, sizeof(char) * MAX_FILE_NAME_LENGTH);
            strcpy(configFN, argv[i + 1]);
        }
    }

    fprintf( stderr, "Loading system settings\n" );
    fprintf( stderr, "  >> From command-line ...\n" );
    fprintf( stderr, "  >> From File: %s ...\n", configFN );
    timer.StartTimer();
    if( configFN != NULL && strcmp( configFN, "" ) != 0 )
        LoadArgsFromConfig( configFN );
    LoadArgsFromCmdLine( argc, argv );
    timer.EndTimer();
    fprintf( stderr, "Done [%.2f sec(s)]\n", timer.GetTimerDiff() );

}

/*
 @ Function:
 * create and initialize a new configuration manager
 * load all system's settings
 *
 @ Arguments:
 * 1) name of config file
 */
void Configer::Create(const char * configFN) {

    TimeUtil timer;
    hashTab = new HashTable( 1024, 1.0, FreeAsciiStr );

    fprintf( stderr, "Loading system settings\n" );
    fprintf( stderr, "  >> From command-line ...\n" );
    fprintf( stderr, "  >> From File: %s ...\n", configFN );
    timer.StartTimer();
    if( configFN != NULL && strcmp( configFN, "" ) != 0 )
        LoadArgsFromConfig( configFN );
    timer.EndTimer();
    fprintf( stderr, "Done [%.2f sec(s)]\n", timer.GetTimerDiff() );

}

/*
 @ Function:
 * destroy the configuration manager
 */
void Configer::Destroy() {

    delete hashTab;

}

/*
 @ Function:
 * get the total parameter number
 *
 @ Return:
 * the total parameter number (unsigned long long)
 */
unsigned long long Configer::GetParaNum() {

    return hashTab->GetKeyCnt();

}

/*
 @ Function:
 * load system's settings from configuration file
 *
 @ Arguments:
 * configuration file's path (char*)
 */
void Configer::LoadArgsFromConfig( const char* configFile ) {

   /* open configuration file */
   FILE* fileHnd = fopen( configFile, "r" );
   if( fileHnd == NULL ) {
       fprintf( stderr, "[ERROR]: file %s not existed!\n", configFile );
       return;
   }
   /* record <key, value> pairs */
   char* line = NULL;
   while( line = StringUtil::ReadLine( fileHnd ) ) {
        char* keyPtr = strstr( line, "param=\"" );
        char* valPtr = strstr( line, "value=\"" );
        /* find the <key, value> pair */
        if( keyPtr && valPtr ) {
            int iLen = (int)strlen( line );
            char* key = new char[iLen + 1];
            char* val = new char[iLen + 1];
            keyPtr += strlen( "param=\"" );
            valPtr += strlen( "value=\"" );
            /* get key string */
            int i = 0;
            while( *keyPtr != '\"' ) {
                key[i++] = *keyPtr++;
            }
            key[i] = '\0';
            /* get value string */
            i = 0;
            while( *valPtr != '\"' ) {
                val[i++] = *valPtr++;
            }
            val[i] = '\0';
            hashTab->AddObject( StringUtil::ToLowercase( key ), val );
            delete[] key;
            /* Note: do not "delete val" because we use shallow copy for complex data types */
        }
        delete[] line;
   }
   /* close configuration file */
   fclose( fileHnd );

}

/*
 @ Function:
 * load system's settings from command-line
 *
 @ Arguments:
 * 1) number of command-line arguments (int)
 * 2) array of command-line arguments (char**)
 */
void Configer::LoadArgsFromCmdLine( int argc, const char** argv ) {

    char tmpArg[MAX_WORD_LENGTH];

    if( argc < 2 ) {
        return;
    }

    for( int i=2; i < argc; i++ ) {
        if( argv[i][0] == '-' ) {
            if( i == argc - 1 || argv[i+1][0] == '-' ){
                char * defaultSetting = new char[2];
                defaultSetting[0] = '1';
                defaultSetting[1] = '\0';
                memset(tmpArg, 0, sizeof(char) * MAX_WORD_LENGTH);
                strcpy(tmpArg, &argv[i][1]);
                hashTab->AddObject( StringUtil::ToLowercase( tmpArg ), defaultSetting );
            }
            else{
                int iValLen = (int)strlen( argv[i+1] );
                char* val = ( char* ) new char[iValLen+1];
                strcpy( val, argv[i+1] );
                memset(tmpArg, 0, sizeof(char) * MAX_WORD_LENGTH);
                strcpy(tmpArg, &argv[i][1]);
                hashTab->AddObject( StringUtil::ToLowercase( tmpArg ), val );
            }
        }
    }

}

/*
 @ Function:
 * get the string value of a given parameter
 *
 @ Arguments:
 * 1) parameter name (const char*)
 * 2) parameter default value (const char*)
 *
 @ Return:
 * parameter value (const char*)
 */
const char* Configer::GetString( const char* key, const char* defaultVal ) {

    char* keyLc = new char[strlen( key ) + 1];
    strcpy( keyLc, key );
    StringUtil::ToLowercase( keyLc );
    char* val = ( char* ) hashTab->GetObject( keyLc );
    delete[] keyLc;

    if( val == NULL ) {
        return defaultVal;
    }
    else {
        return val;
    }

}

/*
 @ Function:
 * get the boolean value of a given parameter
 *
 @ Arguments:
 * 1) parameter name (char*)
 * 2) parameter default value (bool)
 *
 @ Return:
 * parameter value (bool)
 */
bool Configer::GetBool( const char* key, bool defaultVal ) {

    char* val = ( char* ) GetString( key, NULL );
    if( val == NULL || strcmp( val, "" ) == 0 ) {
        return defaultVal;
    }
    else {
        //if( strcmp( val, "0" ) == 0 || strcmp( StringUtil::ToLowercase( val ), "false" ) == 0 ) {
		if(strcmp( val, "0" ) == 0) {
            return false;
        }
        else {
            return true;
        }
    }

}

/*
 @ Function:
 * get the integer value of a given parameter
 *
 @ Arguments:
 * 1) parameter name (char*)
 * 2) parameter default value (int)
 *
 @ Return:
 * parameter value (int)
 */
int Configer::GetInt( const char* key, int defaultVal ) {

    const char* val = GetString( key, NULL );
    if( val == NULL ) {
        return defaultVal;
    }
    else {
        return atoi( val );
    }

}

/*
 @ Function:
 * get the decimal value of a given parameter
 *
 @ Arguments:
 * 1) parameter name (char*)
 * 2) parameter default value (float)
 *
 @ Return:
 * parameter value (float)
 */
float Configer::GetFloat( const char* key, float defaultVal ) {

    const char* val = GetString( key, NULL );
    if( val == NULL ) {
        return defaultVal;
    }
    else {
        return ( float ) atof( val );
    }

}

/*
 @ Function:
 * get the decimal value of a given parameter
 *
 @ Arguments:
 * 1) parameter name (char*)
 * 2) parameter default value (double)
 *
 @ Return:
 * parameter value (double)
 */
double Configer::GetDouble( const char* key, double defaultVal ) {

    const char* val = GetString( key, NULL );
    if( val == NULL ) {
        return defaultVal;
    }
    else {
        return atof( val );
    }

}

Configer ConfigManager;

}

