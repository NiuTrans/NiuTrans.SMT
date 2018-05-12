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
 * phrase extraction and scorer; FIL_filter_final_table.h
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

#ifndef _RULETABLE_FILTER_H_
#define _RULETABLE_FILTER_H_

#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include "basic_method.h"

using namespace std;
using namespace basic_method;

namespace ruletable_filter
{

    class FilterFinalTable:public BasicMethod
    {
    public:
        map< string, int > devTableLex;
        bool filterFinalTable( string &devSet     , 
                               string &phraseTable, 
                               string &filterTable, 
                               int    &maxlen     , 
                               int    &refnum       );
        
    private:
        bool replaceLabel( string &inputstr );
        
    };


    class OptionsOfFilterNum
    {
    public:
        string sourceTransTableName;
        string targetTransTableName;
        int    outputMaxNumStrict  ;
        string tableFormat         ;

        OptionsOfFilterNum( const string &srcTransTableName, 
                            const string &tgtTransTableName, 
                            const int    &maxNumStrict     , 
                            const string &tabFor             )
            : sourceTransTableName( srcTransTableName ), 
              targetTransTableName( tgtTransTableName ),
              outputMaxNumStrict(   maxNumStrict      ),
              tableFormat(          tabFor            ){}
    };

    class FilterTableWithNum:public BasicMethod
    {
    public:
        bool filterTableWithNum( const OptionsOfFilterNum &options );

    private:
        multimap< double, string > scoreItems    ;
        string                     lastSourceLang;
    };

}

#endif
