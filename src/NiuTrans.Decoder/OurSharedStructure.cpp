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
 * this head file provides implementations of the basic structures shared by the decoding and training modules
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email:xiaotong@mail.neu.edu.cn) 2012/12/11
 *
 */

#include <stdio.h>
#include "OurSharedStructure.h"


namespace smt{

////////////////////////////////////////
// structs and classes shared globally

TransResult::TransResult()
{
    translation = NULL;
    modelScore  = 0;
    featValues  = NULL;
    log         = NULL;
    wordTranslation = NULL;
    anchorLoss  = 0;
}

TransResult::~TransResult()
{
    delete[] translation;
    delete[] featValues;
    delete[] log;
    //delete wordTranslation;
}

DecodingLoger::DecodingLoger()
{
    defeatedViterbiLog = NULL;
    usedRuleLog        = NULL;
    others             = NULL;
}

DecodingLoger::~DecodingLoger()
{
    delete[] defeatedViterbiLog;
    delete[] usedRuleLog;
    delete[] others;
}

}

