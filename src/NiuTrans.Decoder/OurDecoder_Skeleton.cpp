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
 * implementation of a two-stage model which translates from 
 * skeleton to the whole sentence:; OurDecoder_Skeleton.cpp
 *
 * $Version:
 * 1.0.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 *
 * $Last Modified by:
 *
 */

#include "OurDecoder_Skeleton.h"

namespace smt {

Decoder_Skeleton::Decoder_Skeleton()
{
	skeletonDecoder       = NULL;
	assistantDecoder      = NULL;
	skeletonWordIndicator = new int[MAX_SENT_LENGTH_SHORT];
	memset(skeletonWordIndicator, 0, sizeof(int) * MAX_SENT_LENGTH_SHORT);
	skeletonWordOffset    = new int[MAX_SENT_LENGTH_SHORT];
	memset(skeletonWordOffset, 0, sizeof(int) * MAX_SENT_LENGTH_SHORT);
	skeletonWords         = new char*[MAX_SENT_LENGTH_SHORT];
	memset(skeletonWords, 0, sizeof(char*) * MAX_SENT_LENGTH_SHORT);
	skeletonWordCount     = 0;

	realWordOffsetForSkeleton = new int[MAX_SENT_LENGTH_SHORT];
	memset(realWordOffsetForSkeleton, 0, sizeof(int) * MAX_SENT_LENGTH_SHORT);
}

Decoder_Skeleton::~Decoder_Skeleton()
{
	delete skeletonDecoder;
	delete assistantDecoder;
	delete[] skeletonWordIndicator;
	delete[] skeletonWordOffset;
	delete[] skeletonWords;
	delete[] realWordOffsetForSkeleton;
}

void Decoder_Skeleton::Init(Model * m)
{
	Decoder_SCFG::Init(m);

	skeleton = true;

	modelType = (SKELETON_MODEL_TYPE)configer->GetInt("skeletonmodel", 0);
	if(configer->GetBool("stringskeleton", false))
		modelType = STRING_SKELTON;
	else if(configer->GetBool("hierarchicalskeleton", false))
		modelType = HIERARCHICAL_SKELTON;
	else if(configer->GetBool("syntaxskeleton", false))
		modelType = SYNTAX_SKELTON;


	for( int i = 0; i < MAX_WORD_NUM; i++ ){
        for( int j = i + 1; j < MAX_WORD_NUM; j++ ){
            cell[i][j].SynInit(i, j, beamSize, true, mem);
        }
    }

    int heapSize = 5 * beamSize * beamSize;
    if( heapSize < 50 )
        heapSize = 50;

    heapForSearch = new NodeHeapForSearch(heapSize, MAXHEAP);

	Model * m1 = (Model *)m->associatedModels->GetItem(0);
	Model * m2 = (Model *)m->associatedModels->GetItem(1);

	if(m1->type == PHRASE_BASED)
		skeletonDecoder = new Decoder_Phrase_ITG();
	else if(m1->type == HIERO)
		skeletonDecoder = new Decoder_SCFG(HIERO);
	else if(m1->type == SYNTAX_BASED)
		skeletonDecoder = new Decoder_SCFG(SYNTAX_BASED);

	if(m2->type == PHRASE_BASED)
		assistantDecoder = new Decoder_Phrase_ITG();
	else if(m2->type == HIERO)
		assistantDecoder = new Decoder_SCFG(HIERO);
	else if(m2->type == SYNTAX_BASED)
		assistantDecoder = new Decoder_SCFG(SYNTAX_BASED);

	skeletonDecoder->Init(m1);
	assistantDecoder->Init(m2);
	skeletonDecoder->decoderId = DECODER_ID_SKELETON;
	assistantDecoder->decoderId = DECODER_ID_ASSISTANT;

	skeletonModel = m1;
	assistantModel = m2;
}

void Decoder_Skeleton::ClearSkeletonInformation()
{
	for( int i = 0; i < MAX_SENT_LENGTH_SHORT; i++ ){
		if(skeletonWords[i] == NULL)
			break;
        delete[] skeletonWords[i];
	}
	memset(skeletonWords, 0, sizeof(char*) * MAX_SENT_LENGTH_SHORT);
    skeletonWordCount = 0;
}

int Decoder_Skeleton::DumpTransResult(int nbest, TransResult * result, DecodingLoger * log)
{
	CellHypo ** nlist = cell[0][srcLength].nList;
    int n = cell[0][srcLength].n;

    if( nbest > n )
        nbest = n;

    for( int i = 0; i < nbest; i++ ){
        CellHypo * ch = nlist[i];
        int transLength = (int)strlen(ch->translation);

        result[i].modelScore = ch->modelScore;            // model score
        if(normalizeOutput)
            result[i].translation = StringUtil::NormalizeText(ch->translation);
        else{
            result[i].translation = new char[transLength * 3 + 1];
            memset(result[i].translation, 0, sizeof(char)*(transLength + 1));
            strcpy(result[i].translation, ch->translation);   // translation
        }
        result[i].featValues = new float[model->featNum];
        memcpy(result[i].featValues, ch->featValues, sizeof(float)*model->featNum); // features
    }

    return nbest;
}

void Decoder_Skeleton::DumpLog(DecodingLoger * log)
{
}

bool Decoder_Skeleton::LoadSkeleton(char * input, int haveSlot)
{
	char ** terms = NULL;
    char ** words = NULL;
    int     termCount, wordCount, word;

	memset(skeletonWordIndicator, 0, sizeof(int) * MAX_SENT_LENGTH_SHORT);
	memset(skeletonWordOffset, 0, sizeof(int) * MAX_SENT_LENGTH_SHORT);
	memset(realWordOffsetForSkeleton, 0, sizeof(int) * MAX_SENT_LENGTH_SHORT);
	
    termCount = (int)StringUtil::Split(input, " |||| ", terms);

	if(termCount >= 4){
		skeletonWordCount = 0;
		wordCount = (int)StringUtil::Split(terms[3], " ", words);
		for(int i = 0; i < wordCount; i++){
			word = -1;
			char * p = strrchr(words[i], '[');

			if(p != NULL && words[i][(int)strlen(words[i])-1] == ']')
				sscanf(p + 1, "%d", &word);
			else
				sscanf(words[i], "%d", &word);

			if(word >= 0 && word < MAX_SENT_LENGTH_SHORT){
				skeletonWordIndicator[word + 1] = 1; // flag
				skeletonWords[skeletonWordCount++] = StringUtil::Copy(srcWords[word + 1]); // word used in the skeleton
			}
		}
	}

	skeletonWordIndicator[0] = 1;
	skeletonWordIndicator[srcLength - 1] = 1;

	int offset = 0;
	for(int i = 0; i < srcLength; i++){
		skeletonWordOffset[i] = offset;
		realWordOffsetForSkeleton[offset] = i;
		if(skeletonWordIndicator[i] == 1)
			offset++;
		else if(haveSlot > 0 && i > 0 && skeletonWordIndicator[i - 1] == 1)
			offset++;
	}
	skeletonWordOffset[srcLength] = skeletonWordOffset[srcLength - 1] + 1;

	/* span information */
	ResetSkeletonSpans();

	for( int i = 0; i < termCount; i++ )
        delete[] terms[i];
    delete[] terms;

	for( int i = 0; i < wordCount; i++ )
        delete[] words[i];
    delete[] words;

	return true;
}

void Decoder_Skeleton::ResetTransInfo()
{
	Decoder_SCFG::ResetTransInfo();
}

void Decoder_Skeleton::ResetSkeletonSpans()
{
	for( int i = 0; i <= srcLength; i++ ){
        SPAN_TYPE spanType = NORMAL_SPAN;
        for( int j = i + 1; j <= srcLength; j++ ){
			if(j == i + 1){
				if(i == 0 || i == srcLength - 1)
					spanType = SKELETON_SPAN;
				else if(skeletonWordIndicator[j - 1] == 1)
					spanType = SKELETON_SPAN;
				else
					spanType = OTHER_SPAN;
			}
			else{
				if(spanType == SKELETON_SPAN && skeletonWordIndicator[j - 1] == 0)
					spanType = NORMAL_SPAN;
				else if(spanType == OTHER_SPAN && skeletonWordIndicator[j - 1] == 1)
					spanType = NORMAL_SPAN;
			}

            cell[i][j].spanType = spanType;
        }
    }
}

void Decoder_Skeleton::DecodeInput(DecodingSentence * sentence)
{
	DECODER_TYPE t = skeletonDecoder->model->type;
	decodingFailure = false;

	if(sentence->id == 81){
		int nnn = 0;
	}
	
	ClearSkeletonInformation();

	/* load input sentence */
    if(!LoadInputSentence(sentence->string)){
        ResetTransInfo();
        fprintf(stderr, "\nFailure: %s\n", sentence->string);
        return;
    }

	/* initialization */
	ResetTransInfo();

	/* load skeleton information */
	LoadSkeleton(sentence->string, t != PHRASE_BASED);

	/* create a sentence for skeleton decoding */
	DecodingSentence * skeletonSentence = new DecodingSentence();
	memset(skeletonSentence, 0, sizeof(DecodingSentence));
	skeletonSentence->id = sentence->id;
	skeletonSentence->string = GenerateSkeletonString(sentence);

	/* decoding */
	tmpTrigger = 0;
	skeletonDecoder->DecodeInput(skeletonSentence);
	tmpTrigger = 0;
	assistantDecoder->DecodeInput(sentence);

	delete[] skeletonSentence->string;
	delete skeletonSentence;

	if(t == PHRASE_BASED){
		MatchSrcReorderFeat(skeletonModel);
		ReuseLongPhrasesInAssociatedModel(assistantDecoder);
	}

	tmpTrigger = 1;
	IntersectModels(sentence);

}

char * Decoder_Skeleton::GenerateSkeletonString(DecodingSentence * sentence)
{
	char * skeletonString = new char[(int)strlen(sentence->string) + 1];
	char * buf = new char[(int)strlen(sentence->string) + 1];
	char * buf2 = new char[(int)strlen(sentence->string) + 1];
	int bufLength = 0;

	skeletonString[0] = '\0';
	buf[0] = '\0';

	/* generate skeleton string */
	for(int i = 0; i < skeletonWordCount; i++){
		if(i == 0){
			strcpy(buf, skeletonWords[i]);
			bufLength = (int)strlen(skeletonWords[i]);
		}
		else{
			sprintf(buf + bufLength, " %s", skeletonWords[i]);
			bufLength += (int)strlen(skeletonWords[i]) + 1;
		}
	}
	buf[bufLength] = '\0';
	sprintf(skeletonString, "%s |||| ", buf); // sentence

	/* generate NE information used in skeleton */
	for(int i = 0; i < userTransCount; i++){
		UserTranslation * curTrans = userTrans + i;

		bool validNE = true;
		buf2[0] = '\0';
		for(int j = curTrans->beg; j < curTrans->end; j++){
			if(skeletonWordIndicator[j] == 0){
				validNE = false;
				break;
			}
			if(j == curTrans->beg)
				strcpy(buf2, srcWords[j]);
			else
				sprintf(buf2, "%s %s", buf2, srcWords[j]);
		}
		if(!validNE)
			continue;

		sprintf(buf, "{%d ||| %d ||| %s ||| %s ||| %s}", 
			    skeletonWordOffset[curTrans->beg] - 1, skeletonWordOffset[curTrans->end] - 2, curTrans->translation, curTrans->type, buf2);
		strcat(skeletonString, buf);

	}
	strcat(skeletonString, " |||| "); // append tree information (if tree is required in the input)

	/* generate tree */
	char * treeInfo = srcTree->ToString(modelType == SYNTAX_SKELTON ? true : false, skeletonWordIndicator);
	strcat(skeletonString, treeInfo);

	delete[] treeInfo;
	delete[] buf;
	delete[] buf2;
	return skeletonString;
}

/* intersection of the two models 
* (i.e., the skeleton translation model and the full translation model) 
*/
void Decoder_Skeleton::IntersectModels(DecodingSentence * sentence)
{
	IntersectModelsForPhraseBased(sentence);

	/*Cell * c = &cell[0][srcLength];
	Cell * sc = &skeletonDecoder->cell[0][skeletonDecoder->srcLength];
	c->nList = (CellHypo **)mem->Alloc(sizeof(CellHypo*) * sc->n);
	memcpy(c->nList, sc->nList, sizeof(CellHypo*) * sc->n);
	c->n = sc->n;*/
}

/* intersect the two models 
* where the skeleton translaiton model is phrase-based
*/
void Decoder_Skeleton::IntersectModelsForPhraseBased(DecodingSentence * sentence)
{
	if(cell[0][srcLength].spanType != NORMAL_SPAN){
		if(cell[0][srcLength].spanType == SKELETON_SPAN)
			CopyCellHypoList(&cell[0][srcLength], &skeletonDecoder->cell[0][skeletonDecoder->srcLength], true);
		else
			CopyCellHypoList(&cell[0][srcLength], &assistantDecoder->cell[0][assistantDecoder->srcLength], false);
	}
	else{
		for( int len = 1; len <= srcLength; len++ ){
			for( int beg = 0; beg + len <= srcLength; beg++ ){
				int end = beg + len;

				if(len > 1 && cell[beg][end].valid && cell[beg][end].spanType == NORMAL_SPAN)
					GenerateTrans(&cell[beg][end]);

				if(dumpCell){
					char fn[1024], rule[1024];
					sprintf(fn, "./cell/[%d]-%d-%d.txt", 100 + sentence->id, beg, end);
					FILE * f = fopen(fn, "w");
					for(int i = 0; i < cell[beg][end].n; i++){
						CellHypo * ch = cell[beg][end].nList[i];
						if(ch->ruleUsed != NULL){
							UnitRule * urule = (UnitRule *)ch->ruleUsed;
							SCFGRule * srule = urule->parentRule;
							sprintf(rule, "%s ||| %s -> %s", urule->src, srule->src, srule->tgt);
						}
						else{
							UnitRule * rule0 = (UnitRule*)ch->lc;
							UnitRule * rule1 = (UnitRule*)ch->rc;
							sprintf(rule, "<glue rule> : %s %s", ch->lc == NULL ? "<NULL>" : ch->lc->root, ch->rc == NULL ? "<NULL>" : ch->rc->root);
							sprintf(rule, "%s [%d, %d] [%d, %d] %d %d", rule, ch->lc == NULL ? -1 : ch->lc->cell->beg, ch->lc == NULL ? -1 : ch->lc->cell->end, 
																ch->rc == NULL ? -1 : ch->rc->cell->beg, ch->rc == NULL ? -1 : ch->rc->cell->end,
																ch->lc == NULL ? -1 : SynchronousGrammar::IsComplete(ch->lc->root), ch->rc == NULL ? -1 : SynchronousGrammar::IsComplete(ch->rc->root));
						}
						fprintf(f, "%2d[%.4f]: %s %s: %s ||| %s ||| %.4f |||", i, ch->modelScore, ch->root, ch->rootFineModel, ch->translation, rule, ch->modelScore);

						if(ch->LMScore != NULL){
							for(int i = 0; i < ch->wCount; i++){
								fprintf(f, " %.3f", ch->LMScore[i]);
							}

							fprintf(f, " |||");

							for(int i = 0; i < ch->wCount; i++){
								fprintf(f, " %d", ch->wid[i]);
							}
						}
						fprintf(f, "\n");
					}
					fclose(f);
				}

				/*if(beg == 8 && end == 12){
					CellHypo * ch = cell[beg][end].nList[0];
					float score = 0;
					for(int i = 0; i < model->featNum; i++){
						score += ch->featValues[i] * model->featWeight[i];
						if(i < model->associatedModelFeatBeg)
							fprintf(stderr, "%d %f %f\n", i, score, ch->featValues[i]);
						else
							fprintf(stderr, "%d %f %f %f\n", i, score, ch->featValues[i], ch->featValues[i] + ch->featValues[i - model->associatedModelFeatBeg]);
					}

					int nnn = 0;
				}*/
			}
		}
	}
}

void Decoder_Skeleton::GenerateTrans(Cell * c)
{
    if(doRecasing && c->beg > 0)
        return;

    int beg = c->beg, end = c->end;
	MemPool * tmpMem = smallMem ? new MemPool(M_BYTE * 32, 256) : mem;
    int nbestSize = (beg == 0 && end == srcLength) ? nbest : beamSize;
    bool lastUpdate = false;
	bool noPructPruning = CheckForPunctPruning(beg, end);

    if(!IsPrunedDecodingSpan(beg, end)){
        
        for( int mid = beg + 1; mid < end; mid++ ){  // partition of the span

			if(!noPructPruning && IsValidPartition(beg, mid, end))
				continue;

			//if(!CheckForPunctPruning(beg, mid) || !CheckForPunctPruning(mid, end))
			//	continue;

			/* if the two sub-spans are skipped in the assistant decoder (full translation model) */
			if(assistantDecoder->cell[beg][mid].n == 0 || assistantDecoder->cell[mid][end].n == 0)
				continue;

            Cell * c1 = GetCell(beg, mid);
			Cell * c2 = GetCell(mid, end);

			if(beg == 8 && mid == 11 && end == 12){
				int nnn = 0;
			}

            if(c1->n == 0 || c2->n == 0)
                continue;

            if(doRecasing && end - mid > maxWordNumInPhrase)
                continue;

            lastUpdate = false;
            ComposeTwoSpans(c, c1, c2, tmpMem);
        }
    }

    if( !lastUpdate )
		c->CompleteWithBeamPruning(nbestSize, this, false, false, true, false, mem);

	if(smallMem)
		delete tmpMem;
}

/* if the span is valid for decoding */
bool Decoder_Skeleton::IsPrunedDecodingSpan(int beg, int end)
{
	bool noPructPruning = CheckForPunctPruning(beg, end);
	bool validBoundary = IsSegmentBoundary(beg) && IsSegmentBoundary(end - 1);

	//return !(noPructPruning || validBoundary);
	return !(noPructPruning);
}

/* to overload the "CetCell" method which is used in "GenerateTrans(Cell * c)" */
Cell * Decoder_Skeleton::GetCell(int beg, int end)
{
	if(cell[beg][end].spanType == SKELETON_SPAN){
		int newBeg = skeletonWordOffset[beg];
		int newEnd = skeletonWordOffset[end];
		return &skeletonDecoder->cell[newBeg][newEnd];
	}
	else if(cell[beg][end].spanType == OTHER_SPAN)
		return &assistantDecoder->cell[beg][end];

	return &cell[beg][end];
}

/* get the offset of a skeleton word in the original sentence */
int Decoder_Skeleton::GetRealOffsetForSkeleton(int p)
{
	return skeletonWordOffset[p];
}

/* if the position is at the boundar of a segment */
bool Decoder_Skeleton::IsSegmentBoundary(int p)
{
	if(p < 0 || p > srcLength - 1)
		return false;
	if(p <= 1 || p >= srcLength - 2)
		return true;
	
	if(skeletonWordIndicator[p] != skeletonWordIndicator[p - 1])
		return true;
	if(skeletonWordIndicator[p] != skeletonWordIndicator[p + 1])
		return true;

	return false;
}

/* if the partition ia valid */
bool Decoder_Skeleton::IsValidPartition(int beg, int mid, int end)
{
	if(!IsSegmentBoundary(beg) || !IsSegmentBoundary(mid - 1) || !IsSegmentBoundary(end - 1))
		return false;
	return true;
}

/* composition of two hypotheses 
* NOTE that this method is defined in class "Decoder_Phrase_ITG" in "OurDecoder_Phrase_ITG.h"
*/
CellHypo * Decoder_Phrase_ITG::ComposeTwoHypothesesForSkeleton(Cell * c, CellHypo * ch1, CellHypo * ch2, DECODER_TYPE d, MemPool * tmpMem)
{
    ++totalComposeNum;
    if( !allowSequentialNULLTrans && ch1->wCount == 0 && ch2->wCount == 0)  // ??? TODO: remove it
        return NULL;

    int index = ch1->cell->end - ch1->cell->beg - 1;

    CellHypo * rch = (CellHypo*)tmpMem->Alloc(sizeof(CellHypo));
    rch->Init(c, tmpMem, model->featNum);

    // translation
    if( ch1->transLength == 0 || ch2->transLength == 0){
        rch->transLength = ch1->transLength + ch2->transLength;
        rch->translation = (char*)tmpMem->Alloc(sizeof(char) * (rch->transLength + 1));
        sprintf(rch->translation, "%s%s", ch1->translation, ch2->translation);
    }
    else{
        rch->transLength = ch1->transLength + ch2->transLength + 1;
        rch->translation = (char*)tmpMem->Alloc(sizeof(char) * (rch->transLength + 1));
        sprintf(rch->translation, "%s %s", ch1->translation, ch2->translation);
    }
    rch->translation[rch->transLength] = '\0';

    // left and right children
    rch->lc = ch1;
    rch->rc = ch2;

	BaseDecoder * d1 = ch1->cell->decoder;
	BaseDecoder * d2 = ch2->cell->decoder;
	Model * m1 = d1->model;
	Model * m2 = d2->model;
	int ngram1 = d1->ngram, ngram2 = d2->ngram;
	int ch1CountLM1 = 0, ch1CountLM2 = 0, ch2CountLM1 = 0, ch2CountLM2 = 0;
	int * ch1Id1 = NULL, * ch1Id2 = NULL, * ch2Id1 = NULL, * ch2Id2 = NULL;
	float * ch1Score1 = NULL, * ch1Score2 = NULL, * ch2Score1 = NULL, * ch2Score2 = NULL;

	if(d1->decoderId == DECODER_ID_NORMAL || d1->decoderId == DECODER_ID_SKELETON){
		ch1CountLM1 = ch1->wCount;
		ch1Id1 = ch1->wid;
		ch1Score1 = ch1->LMScore;
		if(d1->decoderId == DECODER_ID_NORMAL){
			ch1CountLM2 = ch1->wCount2;
			ch1Id2 = ch1->wid2;
			ch1Score2 = ch1->LMScore2;
		}
        else{
            ch1CountLM2 = ch1->wCount;
			ch1Id2 = ch1->wid;
			ch1Score2 = ch1->LMScore;
        }
	}
	else if(d1->decoderId == DECODER_ID_ASSISTANT){
		ch1CountLM2 = ch1->wCount;
		ch1Id2 = ch1->wid;
		ch1Score2 = ch1->LMScore;
	}

	if(d2->decoderId == DECODER_ID_NORMAL || d2->decoderId == DECODER_ID_SKELETON){
		ch2CountLM1 = ch2->wCount;
		ch2Id1 = ch2->wid;
		ch2Score1 = ch2->LMScore;
		if(d2->decoderId == DECODER_ID_NORMAL){
			ch2CountLM2 = ch2->wCount2;
			ch2Id2 = ch2->wid2;
			ch2Score2 = ch2->LMScore2;
		}
        else{
            ch2CountLM2 = ch2->wCount;
			ch2Id2 = ch2->wid;
			ch2Score2 = ch2->LMScore;
        }
	}
	else if(d2->decoderId == DECODER_ID_ASSISTANT){
		ch2CountLM2 = ch2->wCount;
		ch2Id2 = ch2->wid;
		ch2Score2 = ch2->LMScore;
	}
		
	/* computing LM score for the skeleton translaton model */
    float NGramLMScore1 = 0;
    int wc1 = ch1CountLM1 + ch2CountLM1;
    int * wid1 = (int*)tmpMem->Alloc(sizeof(int) * wc1);
    float * LM1 = (float*)tmpMem->Alloc(sizeof(float) * wc1);
    for( int i = 0; i < wc1; i++ ){
        if( i < ch1CountLM1 ){
            wid1[i] = ch1Id1[i];
            LM1[i] = ch1Score1[i];
        }
        else{
            wid1[i] = ch2Id1[i - ch1CountLM1];
            int e = i + 1;
            int b = e - ngram1 > 0 ? e - ngram1 : 0;
            LM1[i] = b < ch1CountLM1 ? skeletonModel->GetNGramLMProb(b, e, wid1) : ch2Score1[i - ch1CountLM1];
        }
        NGramLMScore1 += LM1[i];
    }

    rch->wid = wid1;
    rch->LMScore = LM1;
    rch->wCount = wc1;
    rch->featValues[Model::NGramLM] = NGramLMScore1;
	rch->featValues[Model::WordCount] = rch->wCount;

	/* computing LM score for the full translaton model */
    float NGramLMScore2 = 0;
    int wc2 = ch1CountLM2 + ch2CountLM2;
    int * wid2 = (int*)tmpMem->Alloc(sizeof(int) * wc2);
    float * LM2 = (float*)tmpMem->Alloc(sizeof(float) * wc2);
    for( int i = 0; i < wc2; i++ ){
        if( i < ch1CountLM2 ){
            wid2[i] = ch1Id2[i];
            LM2[i] = ch1Score2[i];
        }
        else{
            wid2[i] = ch2Id2[i - ch1CountLM2];
            int e = i + 1;
            int b = e - ngram2 > 0 ? e - ngram2 : 0;
            LM2[i] = b < ch1CountLM2 ? assistantModel->GetNGramLMProb(b, e, wid2) : ch2Score2[i - ch1CountLM2];
        }
        NGramLMScore2 += LM2[i];
    }

    rch->wid2 = wid2;
    rch->LMScore2 = LM2;
    rch->wCount2 = wc2;
	rch->featValues[Model::associatedModelFeatBeg + Model::NGramLM] = NGramLMScore2;
	rch->featValues[Model::associatedModelFeatBeg + Model::WordCount] = rch->wCount2;

    /* tgt boundary words */
	if(d1->decoderId != DECODER_ID_NORMAL && d1->model->type != PHRASE_BASED)
		SetBoundaryTgtWords(ch1, false, tmpMem);
	if(d2->decoderId != DECODER_ID_NORMAL && d2->model->type != PHRASE_BASED)
		SetBoundaryTgtWords(ch2, false, tmpMem);
    SetBoundaryTgtWords(rch, ch1, ch2, true, tmpMem);

    float overallScore = 0;
	float * ch1feat1 = NULL, * ch1feat2 = NULL, * ch2feat1 = NULL, * ch2feat2 = NULL;

	if(d1->decoderId == DECODER_ID_NORMAL || d1->decoderId == DECODER_ID_SKELETON){
		ch1feat1 = ch1->featValues;
		if(d1->decoderId == DECODER_ID_NORMAL)
			ch1feat2 = ch1->featValues + Model::associatedModelFeatBeg;
	}
    else if(d1->decoderId == DECODER_ID_ASSISTANT)
        ch1feat2 = ch1->featValues;

	if(d2->decoderId == DECODER_ID_NORMAL || d2->decoderId == DECODER_ID_SKELETON){
		ch2feat1 = ch2->featValues;
		if(d2->decoderId == DECODER_ID_NORMAL)
			ch2feat2 = ch2->featValues + Model::associatedModelFeatBeg;
	}
    else if(d2->decoderId == DECODER_ID_ASSISTANT)
        ch2feat2 = ch2->featValues;

	/* model score */
	for( int f = 0; f < model->associatedModelFeatBeg; f++ ){
		if( f != Model::NGramLM && f != Model::WordCount )
            rch->featValues[f] = (ch1feat1 == NULL ? 0 : ch1feat1[f]) + (ch2feat1 == NULL ? 0 : ch2feat1[f]);
        overallScore += rch->featValues[f] * model->featWeight[f];
    }

	for( int f = model->associatedModelFeatBeg; f < model->featNum; f++ ){
		if( f != Model::NGramLM + model->associatedModelFeatBeg && f != Model::WordCount + model->associatedModelFeatBeg)
            rch->featValues[f] = (ch1feat2 == NULL ? 0 : ch1feat2[f - model->associatedModelFeatBeg]) + 
                                 (ch2feat2 == NULL ? 0 : ch2feat2[f - model->associatedModelFeatBeg]);
        overallScore += rch->featValues[f] * model->featWeight[f];
    }

    rch->modelScore = overallScore;

	/*if(c->beg == 7 && c->end == 12){
		fprintf(tmpF, "%s ||| [%d,%d] [%d,%d] ||| %f ||| %s ||| %s\n", rch->translation, rch->lc->cell->beg, rch->lc->cell->end, rch->rc->cell->beg, rch->rc->cell->end, rch->modelScore, rch->lc->translation, rch->rc->translation);
	}*/

    return rch;
}

void Decoder_Skeleton::CopyCellHypoList(Cell * targetCell, Cell * sourceCell, bool inSkeleton)
{
	if(sourceCell == NULL){
		targetCell->n = 0;
		return;
	}

	Model * oldModel = sourceCell->decoder->model;
	int ngramUsed = sourceCell->decoder->ngram;

	targetCell->n = 0;
	targetCell->nList = (CellHypo **)mem->Alloc(sizeof(CellHypo*) * sourceCell->n);

	for(int i = 0; i < sourceCell->n; i++){
		CellHypo * oldch = sourceCell->nList[i];
		CellHypo * newch = CopyCellHypo(oldch, targetCell, sourceCell->decoder, inSkeleton);
		
		targetCell->nList[targetCell->n++] = newch;
	}
	
}

CellHypo * Decoder_Skeleton::CopyCellHypo(CellHypo * oldch, Cell * targetCell, BaseDecoder * d, bool inSkeleton)
{
	Model * oldModel = d->model;
	int ngramUsed = d->ngram;

	CellHypo * newch = CellHypo::Copy(oldch, oldModel, mem);

	newch->cell = targetCell;
	newch->featValues = (float*)mem->Alloc(sizeof(float) * model->featNum);
	memset(newch->featValues, 0, sizeof(float) * model->featNum);

	if(inSkeleton){ // the "source" hypothesis is from the skeleton translation model
		memcpy(newch->featValues, oldch->featValues, sizeof(float) * oldModel->featNum);

		newch->wid2 = oldch->wid;
		newch->wCount2 = oldch->wCount;
		newch->LMScore2 = new float[newch->wCount];

		/* computing n-gram language model score */
		float overallLMScore2 = 0;
		for( int i = 0; i < newch->wCount2; i++){
			int end = i + 1;
			int beg = end - ngramUsed > 0 ? end - ngramUsed : 0;
			float prob = oldModel->GetNGramLMProb(beg, end, newch->wid2);
			newch->LMScore2[i] = prob;
			overallLMScore2 += prob;
		}
		newch->featValues[Model::NGramLM + Model::associatedModelFeatBeg] = overallLMScore2;
		newch->featValues[Model::WordCount + Model::associatedModelFeatBeg] = newch->wCount2;
	}
	else{
		memcpy(newch->featValues + Model::associatedModelFeatBeg, oldch->featValues, sizeof(float) * oldModel->featNum);
		newch->wid2 = newch->wid;
		newch->wCount2 = newch->wCount;
		newch->LMScore2 = newch->LMScore;
		newch->wid = NULL;
		newch->wCount = 0;
		newch->LMScore = NULL;
	}

	float overallScore = 0;
	for( int f = 0; f < model->featNum; f++ )
		overallScore += newch->featValues[f] * model->featWeight[f];
	newch->modelScore = overallScore;

	return newch;
}

/* reuse the phrases (over length 1) that have been computed in "d" */
void Decoder_Skeleton::ReuseLongPhrasesInAssociatedModel(BaseDecoder * d)
{
	Decoder_Phrase_ITG * myd = (Decoder_Phrase_ITG *)d;
	int maxPhraseLength = myd->maxWordNumInPhrase;

    for( int beg = 0; beg < srcLength; beg++ ){
        for( int end = beg + 2 ; end - beg <= maxPhraseLength && end <= srcLength; end++){
			Cell * sourceCell = &myd->cell[beg][end];
			Cell * targetCell = &this->cell[beg][end];
			int reused = 0;

			for(int i = 0; i < sourceCell->n; i++){
				CellHypo * sourceHypo = sourceCell->nList[i];
				if(sourceHypo->rc == NULL && sourceHypo->lc == NULL){ // a hypothesis that is generated only using a entry in phrase table
					CellHypo * targetHypo = CopyCellHypo(sourceHypo, targetCell, d, false);
					targetCell->AddCellHypo(targetHypo);
					reused++;
				}
			}

			if(reused > 0)
				targetCell->CompleteWithBeamPruning(-1, this, false, false, true, false, mem);
        }

    }
}

}
