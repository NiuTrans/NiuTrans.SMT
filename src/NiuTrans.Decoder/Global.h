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
 * Globals; Global.h
 * This header file defines a configuration manager to manage
 * all settings for our SMT translation system.
 * It also defines some globals for the whole project.
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 * Hao Zhang (email: zhanghao1216@gmail.com)
 *
 * $Last Modified by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn); June 19th, 2011; add some macros
 * Hao Zhang (email: zhanghao1216@gmail.com); update class "ConfigManager"
 *
 */


#ifndef __GLOBALS_H__
#define __GLOBALS_H__


#include  <limits.h>
#include "Utilities.h"
#include "MemManager.h"

using namespace utilities;
using namespace memmanager;

namespace smt {

#define INVALID_ID           -1
#define NGRAM_MIN_NUM        1000
#define MAX_LINE_LENGTH      1024 * 100
#define MAX_LINE_LENGTH_SHORT 1024 * 2
#define MAX_WORD_LENGTH      1024
#define MAX_SENT_LENGTH      MAX_LINE_LENGTH
#define MAX_SENT_LENGTH_SHORT 1024
#define MAX_SENT_NUM         500000
#define MIN_float_VALUE      -1000000
#define M_BYTE               1024 * 1024
#define MILLION              1000000
#define MAX_USER_TRANS_NUM   128
#define FLETTEN_DIS_FACTOR   1
#define MAX_RULE_SIZE        1024 * 2
#define MAX_NT_NUM           128
#define MAX_PHRASE_CHAR_NUM  128
#define MIN_OPTION_NUM       5
#define MIN_HYPO_NODE_NUM    1
#define MAX_PATTERN_LENGTH   128 
#define MAX_FILE_NAME_LENGTH 1024
#define MAX_TMP_TRANS_LENGTH 256
#define MAX_UNIT_RULE_NUM_PER_SCFG_RULE 128
#define MODEL_SCORE_MIN      -1.0E30
#define FLOAT_MIN            -1.0E38
#define MAX_HYPO_NUM_IN_CELL 100000
#define MAX_NUM_OF_NT_SYMBOL 10000
#define MAX_RULE_LENGTH      1024 * 2
#define LOWEST_LM_SCORE      -20
#define MAX_NGRAM            10

#define NT_SYMBOL            '#'
#define NT_SYMBOL_STRING     "#"
#define NT_SYMBOL_STRING1    " #"
#define NT_SYMBOL_STRING2    "# "
#define NT_SYMBOL_STRING3    " # "
#define VIRUTAL_NT_SYMBOL    ':'
#define SYMBOL_SEPARATOR     '='
#define SKELETON_SLOT_SYMBOL "SSX"
#define CONFIG_FILE_NAME     "NiuTrans.itg.config"


#define MTEVAL_NORMALIZATION
#define USE_OLD_TRICK
//#define USE_TRANS_HEAP

//#define MTEVAL_NORMALIZATION 1

#define SLOW_WINDOWS_PRINTF 1

// decoders:
// 0: phrase-based CKY-style decoder (XT-version) - default
enum DECODER_TYPE {NULL_TYPE = -1, PHRASE_BASED = 0, HIERO = 1, SYNTAX_BASED = 2, SKELETON_BASED = 3};
extern char DECODER_NAME[4][20];

// skeleton-based translation model
#define DECODER_ID_NORMAL    0
#define DECODER_ID_SKELETON  1
#define DECODER_ID_ASSISTANT 2
#define DECODER_ID_OTHER     3

extern FILE * tmpF;   // for test
extern int tmpTrigger;// for test

extern int MAX_WORD_NUM;

////////////////////////////////////////
// Global variables
class GlobalVar
{
public:
    static bool     normalizeText;
    static bool     allLowerCase;
    static bool     useF1Criterion;
    static FILE *   lmlogFile;
    static FILE *   mertlogFile;
    static char     nullString[1];
    static char     oneString[2];
    static bool     internationalTokenization;

public:
    static void Init();
    static void SetNormalizeText(bool label);
    static void SetUseF1Criterion(bool use);
    static void SetInternationalTokenization(bool token);
};

}


#endif

