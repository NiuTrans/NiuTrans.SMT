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
 * phrase extraction and scorer; basic_method.h
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


#ifndef _BASIC_METHOD_H_
#define _BASIC_METHOD_H_

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <ctime>
using namespace std;

//#define __DEBUGBM__

namespace basic_method
{

    class SystemCommand
    {
    public:
        string sortFile;
        string del;

        SystemCommand( string &newSortFile, string &newDel )
            : sortFile( newSortFile ), del( newDel ){}
    };

    class BasicMethod
    {
    public:
        typedef string::size_type STRPOS;

        bool   split(        const string &phraseTable, const char &splitchar  , vector< string > &dest );
        bool   splitWithStr( const string &src        , const string &separator, vector< string > &dest );

        string mergeAlignedInfo( string           &nowAlignedInfo, string        &nextAlignedInfo );
        bool   deleteFileList(   vector< string > &fileList      , SystemCommand &systemCommand   );  // add in 2012-05-28, delect file list.

        string size_tToString(        size_t &source );
        bool   clearIllegalChar(      string &str    );
        bool   toUpper(               string &str    );
        bool   toLower(               string &str    );
        bool   rmEndSpace(            string &str    );   // add in 2012-05-22, remove the end space char.
        bool   convertAlignmenttoInv( string &str    );   // add in 2012-05-22, convert word alignment to inverse format.
    };

}

#endif
