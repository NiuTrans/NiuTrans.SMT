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
 * head file of the implementation of a two-stage model which translates 
 * from skeleton to the entire sentence:; OurDecoder_Skeleton.h
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

#ifndef _OURDECODER_SKELETON_H_
#define _OURDECODER_SKELETON_H_

#include "Model.h"
#include "OurDecoder_SCFG.h"

namespace smt {

enum SKELETON_MODEL_TYPE {STRING_SKELTON = 0, HIERARCHICAL_SKELTON = 1, SYNTAX_SKELTON = 2};

class Decoder_Skeleton : public Decoder_SCFG
{
private:
	Decoder_Phrase_ITG * skeletonDecoder;
	Decoder_Phrase_ITG * assistantDecoder;
	
	int *                skeletonWordIndicator;                // an array indidating which words are contained in the skeleton
	int *                skeletonWordOffset;
	char **              skeletonWords;
	int                  skeletonWordCount;
	
	/* settings */
private:
	SKELETON_MODEL_TYPE modelType;

public:
	Decoder_Skeleton();
	~Decoder_Skeleton();

	void Init(Model * m);
	void ClearSkeletonInformation();
	int DumpTransResult(int nbest, TransResult * result, DecodingLoger * log);
    void DumpLog(DecodingLoger * log);
	bool LoadSkeleton(char * input, int haveSlot);
	void ResetTransInfo();
	void ResetSkeletonSpans();
	void DecodeInput(DecodingSentence * sentence);
	char * GenerateSkeletonString(DecodingSentence * sentence);
	void IntersectModels(DecodingSentence * sentence);
	void IntersectModelsForPhraseBased(DecodingSentence * sentence);
	void GenerateTrans(Cell * c);
	bool IsPrunedDecodingSpan(int beg, int end);
	Cell * GetCell(int beg, int end); // overload the "CetCell" method which is used in "GenerateTrans(Cell * c)"
	int GetRealOffsetForSkeleton(int p);
	bool IsSegmentBoundary(int p);
	bool IsValidPartition(int beg, int mid, int end);
	CellHypo * ComposeTwoHypotheses(Cell * c, CellHypo * ch1, CellHypo * ch2, DECODER_TYPE d, MemPool * tmpMem);
	void CopyCellHypoList(Cell * targetCell, Cell * sourceCell, bool inSkeleton);
	CellHypo * CopyCellHypo(CellHypo * oldch, Cell * targetCell, BaseDecoder * d, bool inSkeleton);
	void ReuseLongPhrasesInAssociatedModel(BaseDecoder * d);
};

}

#endif

