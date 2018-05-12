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


#ifndef _OURTREE_H_
#define _OURTREE_H_

#include <string.h>
#include <stdlib.h>
#include "Global.h"

namespace smt {

#define MAX_TREE_NODE_NUM    100000
#define MAX_TREE_EDGE_NUM    100000
#define MAX_WORD_NUM_IN_TREE 256

struct TreeEdge;

// hyper-node
typedef struct TreeNode 
{
    char *          label;
    char *          word;
    bool            isLeaf;
    List *          edges;
    int             id;       // node id
    int             beg;      // beginning of corresponding span
    int             end;      // end of corresponding span
    List *          treefrags;// tree fragments rooting at the node
}* pTreeNode;

// hyper-edge
typedef struct TreeEdge 
{
    TreeNode *      parent;      // parent node (head)
    List *          children;    // child nodes (tails)
}* pTreeEdge;

typedef struct TreeStringToken
{
    char *          token;
    int             id;
}* pTreeStringToken;

class TreeFrag;

// tree or forest
class Tree
{
public:
    List *          nodeBase;
    List *          edgeBase;

public:
    TreeNode *      root;
    List *          leaves;

    short           maxWordNum;
    short           maxNTNum;
    short           maxDepth;
    int             maxEdgeNumPerNode;
    int             maxFragNumPerNode;

public:
    Tree();
    ~Tree();
    void Init();
    void Clear();
    List * TokenizeString(const char * string);
    bool CreateTree(const char * string);
    TreeNode * BuildSubTree(List * tokens, int &pos, TreeNode * parentNode);
    bool CreateForest(const char * string);
    void DestroyTokenList(List * tokens);
    TreeStringToken * GenerateToken(char * string, int id);
    TreeNode * CreateTreeNode(char * label, char * word, int id);
    TreeEdge * CreateTreeEdge(TreeNode * parent, int childrenNum);
    void GenerateTreeFragments(bool generateTreeStructure);
    void GenerateTreeFragments(TreeNode * root, bool generateTreeStructure);
    bool IsValidTreeFrag(TreeFrag * frag);
    bool IsValidTreeJoin(TreeFrag * frag1, TreeFrag * frag2);
	char * ToString(bool label = false, int * validWordIndicator = NULL);
	void ToString(char * buf, int &bufLength, TreeNode * rootNode, bool label = false, int * validWordIndicator = NULL);
};

class TreeFrag
{
public:
    TreeNode *   root;
    char *       frontierSequence; // sequence of terminals and frontier non-terminals
    List *       frontierNTs;      // frontier non-terminals

    short        wordNum;
    short        NTNum;
    short        depth;

public:
    TreeFrag(TreeNode * root);
    TreeFrag();
    ~TreeFrag();
    void AddNT(TreeNode * NT, bool fromLeft, bool useNTSymbol);
    void AddWord(TreeNode * NT, bool fromLeft);
    void AttachLabel(TreeNode * NT);
    void AttachLabelOnly(TreeNode * NT);
    void AttachBrackets();
    TreeFrag * Copy();
    static TreeFrag * Join(TreeFrag * frag1, TreeFrag * frag2);
};

}

#endif

