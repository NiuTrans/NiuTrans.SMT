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
 * Config; Conifg.h
 * This header file defines a manager that make other compenents can
 * access all settings of the translation system.
 *
 * $Version:
 * 1.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "Global.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


namespace smt {

/**************************************************************************
*
* configuration manager for managing all settings
*
***************************************************************************/

class Configer {

public:
    void Create( int argc, const char** argv );
	void Create( const char * configFN );
    void Destroy();
    
public:
    unsigned long long GetParaNum();

public:
    const char* GetString( const char * key, const char * defaultVal );
    bool GetBool( const char * key, bool defaultVal );
    int GetInt( const char * key, int defaultVal );
    float GetFloat( const char * key, float defaultVal );
    double GetDouble( const char * key, double defaultVal );

public:
    void LoadArgsFromConfig( const char* configFile );
    void LoadArgsFromCmdLine( int argc, const char** argv );

private:
    HashTable*   hashTab;

};

extern Configer ConfigManager;

}

#endif

