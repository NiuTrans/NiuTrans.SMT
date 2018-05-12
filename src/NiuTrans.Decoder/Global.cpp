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
 * Globals; Global.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn); June 19th, 2011; update function "Create()"
 * Hao Zhang (email: zhanghao1216@gmail.com); update class "ConfigManager"
 *
 */


#include "Global.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "assert.h"


namespace smt {

////////////////////////////////////////
// Global variables

char METHOD_NAME[4][20] = {"MERT", "PERCEPTRON", "MIRA", "RANKPAIRWISE"};
char BLEU_TYPE_NAME[4][20] = {"NIST_BLEU", "IBM_BLEU", "BLEU_SBP", "F1"};
char DECODER_NAME[4][20] = {"PHRASE", "HIERO", "SYNTAX", "SKELETON"};

bool GlobalVar::normalizeText  = true;
bool GlobalVar::allLowerCase   = true;
bool GlobalVar::useF1Criterion = false;
FILE * GlobalVar::lmlogFile    = NULL;
FILE * GlobalVar::mertlogFile  = NULL;
char GlobalVar::nullString[1]  = "";
char GlobalVar::oneString[2]   = "1";
bool GlobalVar::internationalTokenization = false;

void GlobalVar::Init()
{
}

void GlobalVar::SetNormalizeText(bool label)
{
    normalizeText = label;
}

void GlobalVar::SetUseF1Criterion(bool use)
{
    useF1Criterion = use;
}

void GlobalVar::SetInternationalTokenization(bool token)
{
    internationalTokenization = token;
}

}



