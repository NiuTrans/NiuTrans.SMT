/* NiuTrans - SMT platform
 * Copyright (C) 2013, NEU-NLPLab (http://www.nlplab.com/). All rights reserved.
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
 * DataCheck; DataCheck.h
 * This header file deinfes a module that check various
 * data files
 *
 * $Version:
 * 1.0.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 *
 */

#ifndef __DATACHECK_H__
#define __DATACHECK_H__

#include <stdio.h>
#include "Global.h"
#include "Config.h"
#include "Utilities.h"

using namespace utilities;

namespace smt
{

class DataChecker
{
public:
	DataChecker(){};
	~DataChecker(){};

public:
	static bool CheckInputSentences(const char * fn, bool forTuning, int nref);
	static bool CheckSentence(const char * sent, char * msg);
	static bool CheckModelFiles(Configer * configer);

};

}


#endif


