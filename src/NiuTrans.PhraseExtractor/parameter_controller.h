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
 * phrase extraction and scorer; get_parameter.h
 *
 * $Version:
 * 0.2.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2012/09/19 add "-addsent" parameter for phrase extractor
 * 2012/05/22, add getParamsOfScreSyn(), scoring for syntax-rule, modified by Qiang Li.
 * 2011/11/15, add this file.
 *
 */

#ifndef _PARAMETER_CONTROLLER_H_
#define _PARAMETER_CONTROLLER_H_

#include <iostream>
#include <string>

using namespace std;

namespace parameter_controller
{

    class GetParameter
    {
    public:
        void getParamsOfGeneLexiWei(       );
        void getParamsOfExtractPhrasePairs();
        void getParamsOfExtractHieroRules( );
        void getParamsOfScore(             );
        void getParamsOfScoreSyn(          );            // add in 2012-05-22, scoring for syntax-rule.
        void getParamsOfFilterWithNum(     );
        void getParamsOfFilterWithDev(     );

        bool getParameters( int argc, string argv );
        bool allFunction(   int argc              );
        bool allFunction(   int argc, string argv );
    };

}

#endif
