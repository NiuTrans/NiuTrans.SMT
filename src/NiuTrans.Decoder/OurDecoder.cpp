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
 * the base class of our decoder; OurDecoder.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (xiaotong@mail.neu.edu.cn); December 20th, 2010
 * Hao Zhang (email: zhanghao1216@gmail.com); January 21th, 2011
 *
 * $Last Modified by:
 * Tong Xiao (xiaotong@mail.neu.edu.cn); June 19th, 2011
 * Hao Zhang (email: zhanghao1216@gmail.com); May 9th, 2011
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "OurDecoder.h"
#include "MemManager.h"


namespace smt {

////////////////////////////////////////
// basic data structure for our decoders 

BaseDecoder::BaseDecoder()
{
	mem            = NULL;
    userTrans      = new UserTranslation[MAX_USER_TRANS_NUM];
    userTransCount = 0;
    srcSent        = NULL;
    srcWords       = NULL;
    srcLength      = 0;
    srcWordLength  = NULL;
    srcWordInfo    = NULL;
    srcTree        = NULL;
    model          = NULL;
	skeletonModel  = NULL;
	assistantModel = NULL;

    tgtSent        = NULL;
    tgtWords       = NULL;
    tgtLength      = 0;
    tgtWordLength  = NULL;
    tgtWid         = NULL;
	skeleton       = false;
	decoderId      = 0;
	ngram          = 0;
	realWordOffsetForSkeleton = NULL;
}

BaseDecoder::~BaseDecoder()
{
    ClearSentInfo();
    delete[] userTrans;
    delete[] srcSent;
    delete[] srcWords;
    delete[] srcWordLength;
    delete[] srcWordInfo;
    delete   srcTree;
    delete[] tgtSent;
    delete[] tgtWords;
    delete[] tgtWordLength;
    delete[] tgtWid;
}

void BaseDecoder::CreatBase(Model * m)
{
    model = m;
	configer = m->configer;
}

void BaseDecoder::ClearSentInfo()
{
    for( int i = 0; i < userTransCount; i++ ){
        delete[] userTrans[i].translation;
        delete[] userTrans[i].type;

    }
    memset(userTrans, 0, sizeof(UserTranslation) * userTransCount);
    userTransCount = 0;

    for( int i = 0; i < srcLength; i++ )
        delete[] srcWords[i];
    delete[] srcWords;
    srcWords  = NULL;
    srcLength = 0;
    delete[] srcWordLength;
    srcWordLength = NULL;

    delete[] srcSent;
    srcSent   = NULL;

    delete[] srcWordInfo;
    srcWordInfo = NULL;

    srcTree->Clear();
}

bool BaseDecoder::LoadInputSentence(char * input)
{
    char ** terms;
    char ** words;
    char ** t;
    int     termCount, wordCount;
    int     beg = -1, end = -1;
    char    transTmp[1024 * 4], srcTmp[1024 * 4];
    bool    successful = true;

    ClearSentInfo();

    termCount = (int)StringUtil::Split(input, " |||| ", terms );

    // parse user transaltions
    if( termCount > 1 ){
        char * trans = terms[1];
        int len = (int)strlen(trans);
        for( int i = 0; i < len; i++ ){
            if( trans[i] == '{' ){
                beg = i;
            }
            else if(trans[i] == '}' && beg >= 0){
                end = i;
                strncpy(transTmp, trans + beg + 1, end - beg - 1);
                transTmp[end - beg - 1] = '\0';
                beg = -1;

                //>>>int count = Util::Split(transTmp, " ||| ", t);
                int count = (int)StringUtil::Split( transTmp, " ||| ", t );

                if(count >= 5){
                    UserTranslation * curTrans = userTrans + userTransCount;
                    curTrans->beg = atoi(t[0]) + 1;       // beginnig position
                    curTrans->end = atoi(t[1]) + 2;       // ending position
                    curTrans->translation = new char[(int)strlen(t[2]) + 1];
                    strcpy(curTrans->translation, t[2]);  // translation
                    curTrans->type = new char[(int)strlen(t[3]) + 1];
                    strcpy(curTrans->type, t[3]);         // type of user translation,such as $time, $number, and $date

                    userTransCount++;
                }

                for( int j = 0; j < count; j++ )
                    delete[] t[j];
                delete[] t;
            }
        }
    }

    

    if(termCount > 0){
        srcSent   = new char[(int)strlen(terms[0]) + 1];
        strcpy(srcSent, terms[0]);
        StringUtil::RemoveRightSpaces(srcSent);

		if(strcmp(terms[0], "") != 0)
			sprintf(srcTmp, "<s> %s </s>", srcSent);
		else
			sprintf(srcTmp, "<s> </s>");
    }
    else // space line
        sprintf(srcTmp, "<s> </s>");

    //>>>wordCount = Util::SplitWithSpace(srcTmp, words);    // input source sentence
    wordCount = (int)StringUtil::Split( srcTmp, " ", words );

    if(MAX_WORD_NUM <= wordCount){
        fprintf(stderr, "WARNING: the input sentence is too long (%d < %d): \"%s\"\n", wordCount, MAX_WORD_NUM, srcTmp);
        wordCount = MAX_WORD_NUM;
    }

    srcWords  = words;
    srcLength = wordCount;

    srcWordInfo = new WordInfo[wordCount];
    memset(srcWordInfo, 0,sizeof(WordInfo) * wordCount);

    srcWordLength = new int[wordCount];
    for(int i = 0; i < wordCount; i++){
        srcWordLength[i] = (int)strlen(srcWords[i]);
        if( i > 0 && i < wordCount - 1)
            srcWordInfo[i].isIiteral = StringUtil::IsLiteral(srcWords[i]);
        else
            srcWordInfo[i].isIiteral = false;
    }

    // source tree (for syntax-based decoding)
    if(termCount > 2){
        if(model->useForest)
            srcTree->CreateForest(terms[2]);
        else
            srcTree->CreateTree(terms[2]);

        //if(srcTree->root == NULL)
        //    successful = false;
    }

    for( int i = 0; i < termCount; i++ )
        delete[] terms[i];
    delete[] terms;

    return successful;
}

bool BaseDecoder::IsByline(char * type)
{
    if(strcmp(type, "$byline") == 0)
        return true;
    return false;
}

bool BaseDecoder::IsFactoid(char * type)
{
    if(strcmp(type, "$time") == 0 || strcmp(type, "$number") == 0 || strcmp(type, "$date") == 0)
        return true;
    return false;
}

bool BaseDecoder::IsNE(char * type)
{
    if(strcmp(type, "$ne") == 0 || strcmp(type, "$forced") == 0)
        return true;

    if(strcmp(type, "$person") == 0 || strcmp(type, "$name") == 0 || strcmp(type, "$loc") == 0 )
        return true;
    if(strcmp(type, "$literal") == 0 || strcmp(type, "$url") == 0 )
        return true;

    //for patent MT (with replacement)
    if(strcmp(type, "$Quantifier") == 0 || strcmp(type, "$quantifier") == 0 || strcmp(type, "$patent") == 0 )
        return true;
    if(strcmp(type, "$formula") == 0)
        return true;

    //for patent MT (without replacement)
    if(strcmp(type, "$cterm") == 0)
        return true;

    return false;
}

bool BaseDecoder::CheckAvialability(char * input)
{
    char * src = new char[(int)strlen(input) + 1];
    strcpy(src, input);
    char * buf = new char[(int)strlen(input) + 1];

    int    len = (int)strlen(src);
    char * beg = strchr(src, NT_SYMBOL);
    char * end = beg + 1;
    char * last = src;

    while(beg != NULL){
        end = beg + 1;

        while(*end != ' ' && *end != '\0')
            end++;

        if(end != beg + 1){
            if(beg - last > 0){
                strncpy(buf, last, beg - last); // terminal sequence
                buf[beg - last] = '\0';

                if(strstr(srcSent, buf) == NULL){
                    delete[] src;
                    delete[] buf;
                    return false;
                }
            }

            last = end + 1;
        }

        beg = strchr(end, NT_SYMBOL);
    }

    beg = src + len;
    if(beg - last > 0){
        strncpy(buf, last, beg - last); // terminal sequence
        buf[beg - last] = '\0';

        if(strstr(srcSent, buf) == NULL){
            delete[] src;
            delete[] buf;
            return false;
        }
    }

    delete[] src;
    delete[] buf;

    return true;
}

char* BaseDecoder::GetSrcString(int begIndex, int endIndex, MemPool * myMem) 
{

    char str[MAX_LINE_LENGTH_SHORT];
    char * ret;

    strcpy( str, this->srcWords[begIndex] );
    for( int i = begIndex + 1; i <= endIndex; i++ ) {
        strcat( str, " " );
        strcat( str, srcWords[i] );
    }
	if(myMem != NULL)
		ret = (char*)myMem->Alloc(strlen( str ) + 1);
	else
		ret = new char[strlen( str ) + 1];

    strcpy( ret, str );

    return ret;

}

bool BaseDecoder::DoesDecodingFail()
{
    return decodingFailure;
}

//////////////////////////////////////////////////////////////
//// for "forced" decoding

void BaseDecoder::LoadRef(DecodingSentence * sentence)
{
    ClearRefInfo();

    if(sentence->refs == NULL)
        return;

    int     beg = -1, end = -1;
    char    transTmp[1024 * 4], tgtTmp[1024 * 4];
    bool    successful = true;

    tgtSent = StringUtil::Copy(sentence->refs->refs[0].str);

    StringUtil::RemoveRightSpaces(tgtSent);
    sprintf(tgtTmp, "<s> %s </s>", tgtSent);

    tgtLength = (int)StringUtil::Split( tgtTmp, " ", tgtWords );

    tgtWordLength = new int[tgtLength];
    for(int i = 0; i < tgtLength; i++){
        tgtWordLength[i] = (int)strlen(tgtWords[i]);
    }

    int wCount = 0;
    model->GetWId(tgtTmp, tgtWid, wCount, NULL);

    if(wCount != tgtLength){
        fprintf(stderr, "wCount != tgtLength\n");
        exit(0);
    }
}

void BaseDecoder::ClearRefInfo()
{
    for( int i = 0; i < tgtLength; i++ )
        delete[] tgtWords[i];
    delete[] tgtWords;
    tgtWords  = NULL;
    tgtLength = 0;
    delete[] tgtWordLength;
    tgtWordLength = NULL;

    delete[] tgtSent;
    tgtSent   = NULL;

    delete[] tgtWid;
    tgtWid    = NULL;
}

bool BaseDecoder::FindTransInRef(int * wid, int wCount, int &matchedCount, int * &matchedHeads, MemPool * mem)
{
    int matched[1024];
    int count = 0;
    int i, j;

    if(wid == NULL)
        return false;

    for(i = 0; i < tgtLength; i++){
        for(j = 0; j < wCount; j++){
            if(wid[j] != tgtWid[i + j])
                break;
        }

        if(j < wCount)
            continue;

        matched[count++] = i;
    }

    matchedHeads = (int*)mem->Alloc(sizeof(int) * count);
    memcpy(matchedHeads, matched, sizeof(int) * count);
    matchedCount = count;

    if(count > 0)
        return true;
    else
        return false;
}

//void BaseDecoder::FindTransInRef(CellHypo * ch)
//{
//    int matchedHead[1024];
//    int matchedCount = 0;
//    int i, j;
//
//    if(ch->wid == NULL)
//        return -1;
//
//    for(i = 0; i < tgtLength; i++){
//        for(j = 0; j < ch->wCount; j++){
//            if(ch->wid[j] != tgtWid[i + j])
//                break;
//        }
//
//        if(j < ch->wCount)
//            continue;
//
//        matchedHead[matchedCount++] = i;
//    }
//}

}

