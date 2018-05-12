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
 * tree-parsing (or tree-based decoding); OurDecoder_SCFG_Tree.cpp
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn) 2012/12/11
 *
 * $Last Modified by:
 *
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Model.h"
#include "OurDecoder_SCFG.h"

namespace smt {

////////////////////////////////////////////////////////
// for tree-parsing

void Decoder_SCFG::SetTreeNode(Tree * tree)
{
    SetTreeNode(tree->root);
}

void Decoder_SCFG::SetTreeNode(TreeNode * root)
{
    if(root == NULL)
        return;

    SetTreeNodeNoRecursion(root);

    if(root->edges != NULL){
        // for each hyper-edge
        for(int i = 0; i < root->edges->count; i++){
            TreeEdge * edge = (TreeEdge *)root->edges->GetItem(i);

            if(edge->children == NULL)
                continue;

            for(int j = 0; j < edge->children->count; j++){
                TreeNode * childNode = (TreeNode *)edge->children->GetItem(j);
                SetTreeNode(childNode);
            }
        }
    }
}

void Decoder_SCFG::SetTreeNodeNoRecursion(TreeNode * root)
{
    if(root == NULL)
        return;

    Cell * c = &cell[root->beg][root->end + 1];

    bool exist = false;
    for(int i = 0; i < c->treeNodes->count; i++){
        TreeNode * node = (TreeNode *)c->treeNodes->GetItem(i);
        if(root == node){
            exist = true;
            break;
        }
    }

    // node has not been calculated
    if(!exist){
        c->treeNodes->Add(root);
        c->cellNodes->Add(cellNodes + root->id);
    }
}

void Decoder_SCFG::MatchRulesForTreeParsing(MemPool * myMem)
{
    int maxNodeNum = srcTree->nodeBase->count;
    cellNodeNum    = 0;

    cellNodes = new Cell[maxNodeNum];
    for(int i = 0; i < maxNodeNum; i++){ // in topological order
        TreeNode * treeNode = (TreeNode *)srcTree->nodeBase->GetItem(i);
        cellNodes[i].Init(treeNode->beg, treeNode->end + 1, this, beamSize, mem);
        cellNodes[i].SynInit(treeNode->beg, treeNode->end + 1, beamSize, withNTSymbol, mem);

        if(cellNodes[i].treeNodes != NULL)
            cellNodes[i].treeNodes->Clear();
        else
            cellNodes[i].treeNodes = new List(1);
            
        cellNodes[i].treeNodes->Add(treeNode);

        SetTreeNodeNoRecursion(treeNode);

        cellNodeNum++;
    }

    srcTree->GenerateTreeFragments(generateTreeStructure);  // to generate tree-fragments for each node

    SetUserTransForTreeParsing();

    for(int i = 0; i < maxNodeNum; i++) // in topological order
        MatchRulesForTreeParsing(cellNodes + i, myMem);
}

void Decoder_SCFG::MatchRulesForTreeParsing(Cell * c, MemPool * myMem)
{
    bool noMatched = true;
    List * treeNodes = c->treeNodes;

    for(int i = 0; i < treeNodes->count; i++){
        TreeNode * node = (TreeNode *)treeNodes->GetItem(i);
        List * fragList = node->treefrags;

        if(fragList == NULL)
            continue;

        for(int j = 1; j < fragList->count; j++){
            TreeFrag * frag = (TreeFrag *)fragList->GetItem(j);
            if(AddMatchedPatternUsingTreeFragment(c, frag, myMem)) // add the pattern
                noMatched = false;
        }
    }

    if(noMatched && c->tList->count == 0){
        TreeNode * curNode = (TreeNode *)treeNodes->GetItem(0);
        if(curNode->edges == NULL || curNode->edges->count == 0){ // unknown word
            List * ruleList = model->FindRuleList("<unk>");
            for(int i = 0; i < ruleList->count; i++){
                UnitRule * rule = (UnitRule *)ruleList->GetItem(i);
                AddCellTrans(c, rule, outputOOV, GetDefaultRootLabel(c, "NP", mem));
            }
        }
    }

    c->matchedPatternCount = c->matchedPatternList->count;
}

// added a tree-frag into the pattern-list of a node/span
bool Decoder_SCFG::AddMatchedPatternUsingTreeFragment(Cell * c, TreeFrag * frag, MemPool * myMem)
{
    List * ruleList = NULL;
    char key[MAX_RULE_LENGTH] = "";

    if(generateTreeStructure){
        ruleList = model->FindRuleList(frag->frontierSequence);
        //fprintf(tmpF, "[%d, %d]: %s ||| %d\n", c->beg, c->end, frag->frontierSequence, ruleList == NULL ? 0 : ruleList->count);
    }
    else{
        sprintf(key, "#%s ( %s )", frag->root->label, frag->frontierSequence); // key for indexing
        ruleList = model->FindRuleList(key);                                   // search for corresponding rules
    }

    if(ruleList == NULL)
        return false;
    
    MatchedPattern * pattern  = (MatchedPattern *)myMem->Alloc(sizeof(MatchedPattern));
    memset(pattern, 0, sizeof(MatchedPattern));

    pattern->key = StringUtil::Copy(frag->frontierSequence, myMem);
    pattern->matchingNum = frag->NTNum;
    
    NTBoundary * NTs = (NTBoundary *)myMem->Alloc(sizeof(NTBoundary) * frag->NTNum);
    for(int i = 0; i < frag->NTNum; i++){
        TreeNode * childNode = (TreeNode *)frag->frontierNTs->GetItem(i);
        NTs[i].left     = childNode->beg;
        NTs[i].right    = childNode->end;
        NTs[i].cellNode = &cellNodes[childNode->id];
    }
    pattern->matching = NTs;
    pattern->ruleList = ruleList;

    c->AddMatchedPattern(pattern); // add this pattern

    return true;
}

void Decoder_SCFG::TreeParsing()
{
    for(int i = cellNodeNum - 1; i >= 0; i--){ // in topological order
        GenerateTrans(cellNodes + i);          // for each tree-node

        /*char fn[1024], rule[1024];
        Cell * c = cellNodes + i;
        int beg = c->beg, end = c->end;
        sprintf(fn, ".\\cell\\%d-%d-%d.txt", beg, end, i);
        FILE * f = fopen(fn, "w");
        for(int i = 0; i < c->n; i++){
            CellHypo * ch = c->nList[i];
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
            fprintf(f, "%2d[%.4f]: %s %s: %s ||| %s ||| All:%.4f TM:%.4f LM:%.4f |||",
                    i, ch->modelScore, ch->root, ch->rootFineModel, ch->translation, rule, 
                    ch->modelScore, GetTranslationModelScore(ch), GetNGramLMModelScore(ch));

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
        fclose(f);*/
    }

    // glue n-best candididates with <s> and </s>
    for(int i = 0; i < cellNodeNum; i++){
        Cell * cellNode = cellNodes + i;

        if(cellNode->beg > 1 || cellNode->end < srcLength - 1)
            break;

        List * glueItems = new List(3);
        glueItems->Add((void*)&cell[0][1]);  // <s>
        glueItems->Add((void*)cellNode);     // n-best
        glueItems->Add((void*)&cell[srcLength - 1][srcLength]); // </s>

        GlueMultipleNodeTrans(&cell[0][srcLength], glueItems, mem); // glue them
		cell[0][srcLength].CompleteWithBeamPruning(nbest, this, false, false, // reranking
                                                   true, false, mem);

        delete glueItems;
    }

    if(lossAugumentedDecoding)
        RerankingWithLossAugumentedScore(&cell[0][srcLength]);

    /*for(int i = 0; i < cell[0][srcLength].n; i++){
        CellHypo * ch = cell[0][srcLength].nList[i];
        fprintf(stderr, "%d %f %f %f\n", i, ch->modelScore, ch->beamLoss, ch->bleuLoss);
    }*/
}

// glue translations for a given sequence of tree-nodes (for tree-parsing)
void Decoder_SCFG::GlueMultipleNodeTrans(Cell * target, List * cNodes, MemPool * myMem)
{
    int  beg = target->beg, end = target->end;
    bool entireSpan = (beg <= 1 && end >= srcLength - 1) ? true : false;
    int  nbestSize = (beg == 0 && end == srcLength) ? nbest : beamSize;
    bool completedWithNTSymbol = withNTSymbol && !entireSpan;         // distinguish different Non-terminal symbols for beam search
    bool completedWithFineNTSymbol = withFineNTSymbol && !entireSpan; // distinguish different (fine-grained) Non-terminal symbols for beam search

    if(cNodes->count < 2)
        return;

    // tmp structures
    SlotInformation * slotInfo = (SlotInformation *)myMem->Alloc(sizeof(SlotInformation));
    slotInfo->Init(2, myMem);

    // tmp structures (cont')
    Cell * tmpC = new Cell();
    tmpC->Init(target->beg, target->end, this, beamSize, myMem);
    tmpC->SynInit(target->beg, target->end, beamSize, withNTSymbol, myMem);

    // tmp structures (cont' 2)
    Cell * curCellNode    = (Cell *)cNodes->GetItem(0);
    Cell * nextCellNode   = (Cell *)cNodes->GetItem(1);

    // glue translation for the first two tree-nodes
    GlueTwoCells(tmpC, curCellNode, nextCellNode, slotInfo, myMem);              // glue the first two nodes
	tmpC->CompleteWithBeamPruning(nbestSize, this, completedWithNTSymbol,        // beam pruning
                                  completedWithFineNTSymbol, true, false, myMem);
    tmpC->ReassignCellToHyposInBeam(&cell[curCellNode->beg][nextCellNode->end]); // post-processing
    tmpC->tList->Clear();
    tmpC->beg = curCellNode->beg;
    tmpC->end = nextCellNode->end;

    // then incrementally glue translations for other nodes
    for(int i = 2; i < cNodes->count; i++){
        Cell * targetCellNode = i == cNodes->count - 1 ? target : tmpC;
        curCellNode = (Cell *)cNodes->GetItem(i);

        GlueTwoCells(targetCellNode, tmpC, curCellNode, slotInfo, myMem);        // glue translations for a node pair

        if(i < cNodes->count - 1){
			targetCellNode->CompleteWithBeamPruning(nbestSize, this, completedWithNTSymbol, // beam pruning
                                                    completedWithFineNTSymbol, true, 
                                                    false, myMem);
            targetCellNode->ReassignCellToHyposInBeam(&cell[tmpC->beg][curCellNode->end]);    // post-processing
            targetCellNode->tList->Clear();
        }
    }

    delete tmpC;
}

// get the default syntactic label for a tree-node
char * Decoder_SCFG::GetDefaultRootLabel(Cell * c, const char * tgtSymbol, MemPool * myMem)
{
    if(c->end - c->beg == 1 && IsSentEnd(c)){
        char * label = (char *)myMem->Alloc(sizeof(char) * 5);

        if(c->beg == 0)
            strcpy(label, "<s>");
        else
            strcpy(label, "</s>");

        return label;
    }

    TreeNode * treeNode = (c == NULL || c->treeNodes == NULL || c->treeNodes->count == 0) ? 
                           NULL :
                          (TreeNode *)c->treeNodes->GetItem(0);
    return GetDefaultRootLabel(treeNode, tgtSymbol, myMem);
}

char * Decoder_SCFG::GetDefaultRootLabel(TreeNode * treeNode, const char * tgtSymbol, MemPool * myMem)
{
    if(treeNode == NULL)
        return defaultSymbol;

    if(model->treeBasedModel == 1){
        char * label = (char *)myMem->Alloc(sizeof(char) * (strlen(tgtSymbol) + 1));
        strcpy(label, tgtSymbol);
        return label;
    }
    else if(model->treeBasedModel !=  3){ // tree-to-string
        return treeNode->label;
    }
    else{ // activated for tree-to-tree model
        int length = (int)(strlen(treeNode->label) + strlen(tgtSymbol) + 2);
        char * label = (char*)myMem->Alloc(sizeof(char) * length);
        memset(label, 0, sizeof(char) * length);
        sprintf(label, "%s=%s", tgtSymbol, treeNode->label);
        return label;
    }
}

void Decoder_SCFG::SetUserTransForTreeParsing()
{
    // copy user-defined translations from chart-cells
    for( int len = 1; len <= srcLength; len++ ){
        for( int beg = 0; beg + len <= srcLength; beg++ ){
            int end = beg + len;

            Cell * c = &cell[beg][end]; // span

            if(c->tList->count == 0)
                continue;

            for(int k = 0; k < c->treeNodes->count; k++){ // nodes yielded by the span
                TreeNode * treeNode = (TreeNode *)c->treeNodes->GetItem(k);
                Cell * cellNode = &cellNodes[treeNode->id];

                for(int i = 0; i < c->tList->count; i++){ // partial translations
                    CellHypo * oldC = (CellHypo *)c->tList->GetItem(i);
                    CellHypo * newC = CellHypo::Copy(oldC, model, mem);
                    newC->root = GetDefaultRootLabel(treeNode, "NP", mem);
                    cellNode->AddCellHypo(newC);
                }
            }
        }
    }
}


}

