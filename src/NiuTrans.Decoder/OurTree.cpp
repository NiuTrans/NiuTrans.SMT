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
 * tree structure for tree-based decoding (or tree-parsing)
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Tong Xiao (email: xiaotong@mail.neu.edu.cn)
 *
 * $Last Modified by:
 *
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "OurTree.h"

namespace smt {

////////////////////////////////////////
// tree strcuture

Tree::Tree()
{
    Init();
}

Tree::~Tree()
{
    Clear();
    delete nodeBase;
    delete edgeBase;
    delete leaves;
}

void Tree::Init()
{
    nodeBase = new List(MAX_TREE_NODE_NUM);
    edgeBase = new List(MAX_TREE_EDGE_NUM);
    
    root     = NULL;
    leaves   = new List(MAX_WORD_NUM_IN_TREE);
}

void Tree::Clear()
{
    for(int i = 0; i < nodeBase->count; i++){
        TreeNode * node = (TreeNode *)nodeBase->GetItem(i);
        delete[] node->label;
        delete[] node->word;
        delete node->edges;

        if(node->treefrags != NULL){
            List * frags = node->treefrags;
            for(int j = 0; j < frags->count; j++){
                TreeFrag * frag = (TreeFrag *)frags->GetItem(j);
                delete frag;
            }
            delete node->treefrags;
            node->treefrags;
        }
        delete node;
    }
    nodeBase->Clear();

    for(int i = 0; i < edgeBase->count; i++){
        TreeEdge * edge = (TreeEdge *)edgeBase->GetItem(i);
        delete edge->children;
        delete edge;
    }
    edgeBase->Clear();

    leaves->Clear();
    root = NULL;
}

char leftBracket[2] = "(";
char rightBracket[2] = ")";

List * Tree::TokenizeString(const char * string)
{
    int length    = strlen(string);
    int pos       = 0;
    List * tokens = new List(length);
    TreeStringToken * token           = NULL;
    char tokenString[MAX_WORD_LENGTH] = "";
    
    while(pos < length)
    {
        char curChar = string[pos];
        if (curChar == '('){

            // specail case: "(abc)" is recongized as one word instead of three words "(", "abc", ")"
            bool spectalFlag = false;
            int tmpPos = pos + 1;
            while (tmpPos < length){
                if (string[tmpPos] == ')'){
                    spectalFlag = true;
                    break;
                }
                else if (string[tmpPos] == ' ')
                    break;

                tmpPos++;
            }

            if (spectalFlag && tmpPos > pos + 1){
                strncpy(tokenString, string + pos, tmpPos - pos + 1);
                tokenString[tmpPos - pos + 1] = '\0';
                token = GenerateToken(tokenString, tokens->count);
                tokens->Add((void*)token);
                tokenString[0] = '\0';
                pos = tmpPos;
            }
            else{
                if (strcmp(tokenString, "")){
                    token = GenerateToken(tokenString, tokens->count);
                    tokens->Add((void*)token);
                }

                tokenString[0] = '\0';

                token = GenerateToken(leftBracket, tokens->count);
                tokens->Add((void*)token);
            }
        }
        else if (curChar == ')'){
            if (strcmp(tokenString, "")){
                token = GenerateToken(tokenString, tokens->count);
                tokens->Add((void*)token);
            }

            tokenString[0] = '\0';

            token = GenerateToken(rightBracket, tokens->count);
            tokens->Add((void*)token);
        }
        else if (curChar == ' '){
            if (strcmp(tokenString, "")){
                token = GenerateToken(tokenString, tokens->count);
                tokens->Add((void*)token);
            }
            tokenString[0] = '\0';
        }
        else{
            int tLength = (int)strlen(tokenString);
            tokenString[tLength] = curChar;
            tokenString[tLength + 1] = '\0';
        }

        pos++;
    }

    return tokens;
}

bool Tree::CreateTree(const char * string)
{
    List * tokens = TokenizeString(string);
    int pos = 1;

    /*for(int i = 0; i < tokens->count; i++){
        TreeStringToken * token = (TreeStringToken *)tokens->GetItem(i);
        fprintf(stderr, "%d %s\n", i, token->token);
    }*/

    if (tokens->count == 0){
        DestroyTokenList(tokens);
        return false;
    }

    TreeStringToken * firstToken = (TreeStringToken *)tokens->GetItem(0);
    TreeStringToken * lastToken  = (TreeStringToken *)tokens->GetItem(tokens->count - 1);
    if(strcmp(firstToken->token, "(") || strcmp(lastToken->token, ")")){
        DestroyTokenList(tokens);
        return false;
    }
    
    leaves->Clear();
    leaves->Add(NULL);

    root = BuildSubTree(tokens, pos, NULL);

    if (pos != tokens->count - 1){
        root = NULL;
        DestroyTokenList(tokens);
        return false;
    }

    DestroyTokenList(tokens);

    return true;
}

TreeNode * Tree::BuildSubTree(List * tokens, int &pos, TreeNode * parentNode)
{
    TreeStringToken * token = (TreeStringToken *)tokens->GetItem(pos);

    if (pos >= tokens->count)
        return NULL;
    if (strcmp(token->token, "("))
        return NULL;

    pos++;     // skip '('
    token = (TreeStringToken *)tokens->GetItem(pos);

    TreeNode * curTreeNode = CreateTreeNode(token->token, NULL, nodeBase->count);

    pos++;
    token = (TreeStringToken *)tokens->GetItem(pos);

    bool isLeafNode = false;

    if (pos < tokens->count && strcmp(token->token, "("))
        isLeafNode = true;
    if (pos < tokens->count - 1){
        TreeStringToken * nextToken = (TreeStringToken *)tokens->GetItem(pos + 1);
        if(!strcmp(nextToken->token, ")"))
            isLeafNode = true;
    }

    if (isLeafNode){     // leaf node
        token = (TreeStringToken *)tokens->GetItem(pos);
        //StringUtil::ToLowercase(token->token);
        curTreeNode->word = StringUtil::Copy(token->token);
        curTreeNode->isLeaf = true;
        curTreeNode->beg = leaves->count;
        curTreeNode->end = curTreeNode->beg;

        leaves->Add((void*)curTreeNode);

        pos++;
    }
    else{
        int beg = MAX_WORD_NUM_IN_TREE, end = -1;

        TreeEdge * edge = CreateTreeEdge(curTreeNode, 3);

        while (pos < tokens->count){
            token = (TreeStringToken *)tokens->GetItem(pos);
            if(!strcmp(token->token, ")"))
                break;

            TreeNode * childNode = NULL;
            if(!strcmp(token->token, "("))
                childNode = BuildSubTree(tokens, pos, curTreeNode);

            if (childNode == NULL)
                return NULL;

            if(beg > childNode->beg)
                beg = childNode->beg;
            if(end < childNode->end)
                end = childNode->end;

            edge->children->Add(childNode);
        }

        curTreeNode->isLeaf = false;
        curTreeNode->edges = new List(1);
        curTreeNode->edges->Add((void*)edge);
        curTreeNode->beg = beg;
        curTreeNode->end = end;
    }

    if (pos >= tokens->count)
        return NULL;

    pos++; // skip ')';

    return curTreeNode;
}

bool Tree::CreateForest(const char * string)
{
    const char * blockSeg = strstr(string, " ||| "); // block1: hyper-nodes, block2: hyper-edges
    const char * beg = string;
    const char * end = NULL;
    char buf[MAX_WORD_NUM_IN_TREE * 10];
    int  ibuf[MAX_WORD_NUM_IN_TREE];

    if(blockSeg == NULL){
        fprintf(stderr, "invalid format in forest \"%s\"", string);
        return false;
    }

    // build nodes first
    while(1){
        end = strstr(beg, " || ");
        if(end != NULL){
            if(end < blockSeg){
                strncpy(buf, beg, end - beg);
                buf[end - beg] = '\0';
            }
            else{
                strncpy(buf, beg, blockSeg - beg);
                buf[blockSeg - beg] = '\0';
            }
        }
        else
            strcpy(buf, beg);

        int length = (int)strlen(buf);
        TreeNode * node = new TreeNode();
        node->isLeaf = false;
        node->label  = new char[length + 1];
        node->word   = new char[length + 1];

        // EXAMPLE: "6 2 2 NN report" means
        // node 6 is a (pre-)terminal node with lable "NN" and terminal "report". The corresponding span is [2, 2].
        int tn = sscanf(buf, "%d %d %d %s %s", &node->id, &node->beg, &node->end, node->label, node->word);

        if(tn == 5){
            node->isLeaf = true;
        }
        else if(tn == 4){
            delete[] node->word;
            node->word = NULL;
            node->edges = new List(1);
        }
        else{
            fprintf(stderr, "invalid format in hyper-node \"%s\"", buf);
            delete node;
            continue;
        }

        node->beg++;
        node->end++;

        nodeBase->Add(node);

        //fprintf(stderr, "node%d: %s\n", nodeBase->count, buf);

        if(end >= blockSeg || end == NULL)
            break;
        else
            beg = end + 4;
    }

    beg = blockSeg + 5;
    end = NULL;

    // then edges
    while(1){
        end = strstr(beg, " || ");
        if(end != NULL){
            strncpy(buf, beg, end - beg);
            buf[end - beg] = '\0';
        }
        else
            strcpy(buf, beg);

        int length = (int)strlen(buf);
        const char * ibeg = beg;
        int i = 0;

        while(ibeg != '\0'){
            while(*ibeg == ' ')
                ibeg++;

            if(*ibeg < '0' || *ibeg > '9')
                break;
           
            sscanf(ibeg, "%d", &ibuf[i++]);

            while(*ibeg != ' ' && *ibeg != '\0')
                ibeg++;
        }

        if(i < 2){
            fprintf(stderr, "invalid format in hyper-edge \"%s\"", buf);
            continue;
        }

        TreeEdge * edge = new TreeEdge();
        edge->parent = (TreeNode *)nodeBase->GetItem(ibuf[0]); // head of hyper-edge
        edge->children = new List(i - 1);
        for(int k = 1; k < i; k++){
            TreeNode * childNode = (TreeNode *)nodeBase->GetItem(ibuf[k]); // tail of hyper-edge
            edge->children->Add(childNode);
        }

        edge->parent->edges->Add(edge);
        edgeBase->Add(edge);

        //fprintf(stderr, "edge%d: %s\n", edgeBase->count, buf);

        if(end == NULL)
            break;
        else
            beg = end + 4;
    }

    nodeBase->Reverse();
    for(int k = 0; k < nodeBase->count; k++){
        TreeNode * node = (TreeNode *)nodeBase->GetItem(k);
        node->id = k;
    }

    root = (TreeNode *)nodeBase->GetItem(0);

    return true;
}

void Tree::DestroyTokenList(List * tokens)
{
    for(int i = 0; i < tokens->count; i++){
        TreeStringToken * token = (TreeStringToken *)tokens->GetItem(i);
        delete[] token->token;
        delete token;
    }

    delete tokens;
}

TreeStringToken * Tree::GenerateToken(char * string, int id)
{
    TreeStringToken * token = new TreeStringToken();
    token->token = new char[(int)strlen(string) + 1];
    strcpy(token->token, string);
    token->id = id;
    return token;
}

TreeNode * Tree::CreateTreeNode(char * label, char * word, int id)
{
    TreeNode * node = new TreeNode();
    memset(node, 0, sizeof(TreeNode));
    node->isLeaf = false;
    node->beg = MAX_WORD_NUM_IN_TREE;
    node->end = -1;

    if(label != NULL)
        node->label = StringUtil::Copy(label);

    if(word != NULL)
        node->word  = StringUtil::Copy(word);

    node->id = id;

    nodeBase->Add((void*)node);
    return node;
}

TreeEdge * Tree::CreateTreeEdge(TreeNode * parent, int childrenNum)
{
    TreeEdge * edge = new TreeEdge();
    edge->parent = parent;

    if(childrenNum >= 0)
        edge->children = new List(childrenNum);
    else
        edge->children = new List(3);

    edgeBase->Add((void*)edge);
    return edge;
}

void Tree::GenerateTreeFragments(bool generateTreeStructure)
{
    for(int i = nodeBase->count - 1; i >= 0; i--){
        TreeNode * node = (TreeNode *)nodeBase->GetItem(i);
        GenerateTreeFragments(node, generateTreeStructure);
    }
}

void Tree::GenerateTreeFragments(TreeNode * root, bool generateTreeStructure)
{
    int keyLength = 0;

    //fprintf(stderr, "%d: %s\n", root->id, root->label);
    root->treefrags = new List(2);

    // root
    TreeFrag * baseFrag = new TreeFrag(root);
    baseFrag->AddNT(root, true, !generateTreeStructure);
    if(generateTreeStructure)
        baseFrag->AttachBrackets();
    root->treefrags->Add((void*)baseFrag);

    if(root->isLeaf){ // word
        TreeFrag * leafFrag = new TreeFrag(root);
        leafFrag->AddWord(root, true);
        if(generateTreeStructure){
            leafFrag->AttachBrackets();
            leafFrag->AttachLabel(root);
        }
        root->treefrags->Add((void*)leafFrag);
    }
    else{ // build it recursively
        for(int e = 0; e < root->edges->count; e++){ // loop for each edge
            TreeEdge * edge = (TreeEdge *)root->edges->GetItem(e);
            List * fragList = NULL;
            int maxFragNum  = maxFragNumPerNode / root->edges->count + 1;

            for(int i = 0; i < edge->children->count; i++){ // loop for each child node (tail of edge)
                TreeNode * child = (TreeNode *)edge->children->GetItem(i);

                //if(child->treefrags == NULL)
                //    GenerateTreeFragments(child);

                List * childFrags = child->treefrags;

                if(i == 0){
                    fragList = new List(childFrags->count);
                    for(int j = 0; j < childFrags->count; j++){
                        TreeFrag * frag = (TreeFrag *)childFrags->GetItem(j);
                        TreeFrag * newFrag = frag->Copy();
                        newFrag->root = root;
                        fragList->Add((void*)newFrag);
                    }
                }
                else{
                    List * newFrags = new List(fragList->count);
                    for(int j = 0; j < childFrags->count; j++){
                        TreeFrag * childFrag = (TreeFrag *)childFrags->GetItem(j);
                        for(int k = 0; k < fragList->count; k++){
                            TreeFrag * curFrag = (TreeFrag *)fragList->GetItem(k);

                            if(!IsValidTreeJoin(curFrag, childFrag))
                                continue;

                            TreeFrag * newFrag = TreeFrag::Join(curFrag, childFrag);
                            newFrag->root = root;
                            newFrags->Add(newFrag);
                        }

                        if(newFrags->count >= maxFragNum) // too many fragments. so ...
                            break;
                    }

                    if(fragList != NULL){
                        for(int k = 0; k < fragList->count; k++){
                            TreeFrag * oldFrag = (TreeFrag *)fragList->GetItem(k);
                            delete oldFrag;
                        }
                        delete fragList;
                    }
                    fragList = newFrags;
                }
            }

            if(generateTreeStructure){
                bool unaryProduction = edge->children->count == 1 ? true : false;
                for(int j = 0; j < fragList->count; j++){
                    TreeFrag * frag = (TreeFrag *)fragList->GetItem(j);
                    //if(unaryProduction)
                    //    frag->AttachLabelOnly(root);
                    //else
                        frag->AttachLabel(root);
                }
            }

            root->treefrags->Add(fragList->items, fragList->count);
            delete fragList;
        }
    }
}

bool Tree::IsValidTreeFrag(TreeFrag * frag)
{
    if(frag->wordNum > maxWordNum)
        return false;
    if(frag->NTNum > maxNTNum)
        return false;
    if(frag->depth > maxDepth)
        return false;

    return true;
}

bool Tree::IsValidTreeJoin(TreeFrag * frag1, TreeFrag * frag2)
{
    if(frag1->wordNum + frag2->wordNum > maxWordNum)
        return false;
    if(frag1->NTNum + frag2->NTNum > maxNTNum)
        return false;
    if(frag1->depth > maxDepth || frag2->depth > maxDepth)
        return false;

    return true;
}

char * Tree::ToString(bool label, int * validWordIndicator)
{
	int bufLength = 0;
	char * buf = new char[MAX_LINE_LENGTH];
	char * treeString = NULL;
	
	if(root == NULL){
		treeString = new char[1];
		treeString[0] = '\0';
	}
	else{
		ToString(buf, bufLength, root, label, validWordIndicator);
		treeString = new char[(int)strlen(buf) + 3];
		sprintf(treeString, "(%s)", buf);
	}

	delete[] buf;

	return treeString;
}

void Tree::ToString(char * buf, int &bufLength, TreeNode * rootNode, bool label, int * validWordIndicator)
{
	bool nonSkeleton = true;
	for(int i = rootNode->beg; i <= rootNode->end; i++){
		if(validWordIndicator[i]){ // if the span contains a skeleton word
			nonSkeleton = false;
			break;
		}
	}

	if(nonSkeleton){
		sprintf(buf + bufLength, " %s", label ? rootNode->label : SKELETON_SLOT_SYMBOL);
		bufLength += 1 + label ? (int)strlen(rootNode->label) : (int)strlen(SKELETON_SLOT_SYMBOL);
	}
	else{
		if(rootNode->isLeaf){
			sprintf(buf + bufLength, " (%s %s)", rootNode->label, rootNode->word);
			bufLength += 4 + (int)strlen(rootNode->label) + (int)strlen(rootNode->word);
		}
		else{
			sprintf(buf + bufLength, " (%s", rootNode->label);
			bufLength += 2 + (int)strlen(rootNode->label);
			for(int e = 0; e < rootNode->edges->count; e++){
				TreeEdge * edge = (TreeEdge*)rootNode->edges->GetItem(e);
				for(int c = 0; c < edge->children->count; c++){
					ToString(buf, bufLength, (TreeNode*)edge->children->GetItem(c), label, validWordIndicator);
				}
				break; // TODO: forest
			}
			strcat(buf, ")");
			bufLength++;
		}
	}
}

////////////////////////////////////////
// tree fragment

TreeFrag::TreeFrag(TreeNode * root)
{
    memset(this, 0, sizeof(TreeFrag));
    this->root = root;
    frontierSequence = NULL;
    frontierNTs = new List(1);
}

TreeFrag::TreeFrag()
{
    memset(this, 0, sizeof(TreeFrag));
}

TreeFrag::~TreeFrag()
{
    delete[] frontierSequence;
    delete frontierNTs;
}

void TreeFrag::AddNT(TreeNode * NT, bool fromLeft, bool useNTSymbol)
{
    int baseLength = frontierSequence == NULL ? 0 : ((int)strlen(frontierSequence) + 1);
    int keyLength = (int)strlen(NT->label) + baseLength + 2;

    char * key = new char[keyLength];
    memset(key, 0, sizeof(char) * keyLength);

    // sequence of word and variables
    if(frontierSequence == NULL){
        if(useNTSymbol)
            sprintf(key, "#%s", NT->label);
        else
            sprintf(key, "%s", NT->label);
        frontierSequence = key;
    }
    else{
        if(useNTSymbol){
            if(fromLeft)
                sprintf(key, "#%s %s", NT->label, frontierSequence);
            else
                sprintf(key, "%s #%s", frontierSequence, NT->label);
        }
        else{
                if(fromLeft)
                sprintf(key, "%s %s", NT->label, frontierSequence);
            else
                sprintf(key, "%s %s", frontierSequence, NT->label);
        }

        delete[] frontierSequence;
        frontierSequence = key;
    }

    // variable list
    if(fromLeft)
        frontierNTs->Insert(0, (void*)NT);
    else
        frontierNTs->Add((void*)NT);

    NTNum++;
}

void TreeFrag::AddWord(TreeNode * NT, bool fromLeft)
{
    int baseLength = frontierSequence == NULL ? 0 : ((int)strlen(frontierSequence) + 1);
    int keyLength = (int)strlen(NT->word) + baseLength + 1;

    char * key = new char[keyLength];
    memset(key, 0, sizeof(char) * keyLength);

    // sequence of words and variables
    if(frontierSequence == NULL){
        sprintf(key, "%s", NT->word);
        frontierSequence = key;
    }
    else{
        if(fromLeft)
            sprintf(key, "%s %s", NT->word, frontierSequence);
        else
            sprintf(key, "%s %s", frontierSequence, NT->word);

        delete[] frontierSequence;
        frontierSequence = key;
    }

    wordNum++;
}

// to generate tree structure
void TreeFrag::AttachLabel(TreeNode * NT) 
{
    if(frontierSequence == NULL)
        return;

    int baseLength =  (int)strlen(frontierSequence) + 1;
    int keyLength = (int)strlen(NT->label) + baseLength + 3;

    char * key = new char[keyLength];
    memset(key, 0, sizeof(char) * keyLength);

    
    //if(NT->isLeaf)
    //    sprintf(key, "(%s %s)", NT->label, frontierSequence); // sample format: (NN car)
    //else
    //    sprintf(key, "%s (%s)", NT->label, frontierSequence); // sample format: NP(NP VP)

    sprintf(key, "(%s %s)", NT->label, frontierSequence); // sample format: NN (car)

    delete[] frontierSequence;
    frontierSequence = key;
}

void TreeFrag::AttachLabelOnly(TreeNode * NT)
{
    if(frontierSequence == NULL)
        return;

    int baseLength =  (int)strlen(frontierSequence) + 1;
    int keyLength = (int)strlen(NT->label) + baseLength + 1;

    char * key = new char[keyLength];
    memset(key, 0, sizeof(char) * keyLength);
    
    sprintf(key, "%s %s", NT->label, frontierSequence); // sample format: NP (NN)

    delete[] frontierSequence;
    frontierSequence = key;
}

void TreeFrag::AttachBrackets()
{
    if(frontierSequence == NULL)
        return;

    int baseLength =  (int)strlen(frontierSequence) + 1;
    int keyLength = baseLength + 3;

    char * key = new char[keyLength];
    memset(key, 0, sizeof(char) * keyLength);

    sprintf(key, "(%s)", frontierSequence); // sample format: (NP)

    delete[] frontierSequence;
    frontierSequence = key;
}

TreeFrag * TreeFrag::Copy()
{
    TreeFrag * newFrag = new TreeFrag();
    memcpy(newFrag, this, sizeof(TreeFrag));

    newFrag->frontierSequence = NULL;
    if(frontierSequence != NULL)
        newFrag->frontierSequence = StringUtil::Copy(frontierSequence);

    newFrag->frontierNTs = new List(frontierNTs->count);
    newFrag->frontierNTs->Add(frontierNTs->items, frontierNTs->count);

    return newFrag;
}

TreeFrag * TreeFrag::Join(TreeFrag * frag1, TreeFrag * frag2)
{
    TreeFrag * newFrag = new TreeFrag();

    newFrag->frontierSequence = new char[(int)strlen(frag1->frontierSequence) + 
                                         (int)strlen(frag2->frontierSequence) + 2];

    sprintf(newFrag->frontierSequence, "%s %s", frag1->frontierSequence, frag2->frontierSequence);

    newFrag->frontierNTs = new List(frag1->frontierNTs->count + frag2->frontierNTs->count);
    newFrag->frontierNTs->Add(frag1->frontierNTs->items, frag1->frontierNTs->count);
    newFrag->frontierNTs->Add(frag2->frontierNTs->items, frag2->frontierNTs->count);

    newFrag->wordNum = frag1->wordNum + frag2->wordNum;
    newFrag->NTNum   = frag1->NTNum + frag2->NTNum;
    newFrag->depth   = frag1->depth > frag2->depth ? frag1->depth : frag2->depth;

    return newFrag;
}

}

