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
 * HE_parsetree.h
 *
 * $Version:
 * 0.4.0
 *
 * $Created by:
 * Qiang Li (email: liqiangneu@gmail.com)
 *
 * $Last Modified by:
 *
 */

#ifndef _N_ARY_TREE_H_
#define _N_ARY_TREE_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
using namespace std;

namespace n_ary_tree
{

    class TreeNode
    {
    public:
        string   lable;
        int      parent;
        int      level;
        int      nodePos;
        set<int> son;

        TreeNode( string newlable , 
                  int    newparent, 
                  int    newlevel , 
                  int    newNodePos )
                  :lable(   newlable   ), 
                   parent(  newparent  ), 
                   level(   newlevel   ), 
                   nodePos( newNodePos ){};
};

class Tree
{
public:
    vector< TreeNode >      tree;
    map< size_t, TreeNode > leaf;

    bool   buildTree( string &parseTree );
    string validateSpanOfParseTree( size_t &beginPos, size_t &endPos );

};

}

#endif
