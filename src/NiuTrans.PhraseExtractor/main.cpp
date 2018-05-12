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
 * phrase extraction and scorer; main.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 * 2011/07/14, add "-temp", rewrite the main function, modified by Qiang Li.
 * 2011/06/25, export sort dir, modified by Qiang Li.
 *
 */


#include "main.h"

int main( int argc, char * argv[] )
{
#ifdef __DEBUG__
    cerr<<"argc="<<argc<<"\n"<<flush;
#endif

    GetParameter getParameter;
    if( getParameter.allFunction( argc ) )                          
    {
#ifdef __DEBUG__
//      getchar(); 
#endif
        exit( 1 ); 
    }

    if( getParameter.allFunction( argc, string( argv[1] ) ) )       
    {
#ifdef __DEBUG__
//      getchar();
#endif
        exit( 1 );
    }
    if( getParameter.getParameters( argc, string( argv[1] ) ) )     
    {
#ifdef __DEBUG__
//      getchar();
#endif
        exit( 1 );
    }

    Dispatcher dispatcher;
    dispatcher.resolve_parameter( argc, argv );

    return 0;
}
