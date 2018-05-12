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
 * HE_parsetree.cpp
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

#include "n_ary_tree.h"
using namespace n_ary_tree;

bool Tree::buildTree( string &parseTree )
{
//  cerr<<"parseTree:"<<parseTree<<endl;
    int parent = -1;
//  int parentNodePos = -1;
    int nodePos = -1;
    int currentNodePos = -1;
    int level = 0;
    size_t srcSentPos = 0;
    bool leafFlag = false;

    bool headLabel = true;
    for( string::iterator iter = parseTree.begin(); iter != parseTree.end(); ++iter )
    {
        if( *iter == '(' || *iter == ' ' )
        {
            if( headLabel )
            {
                if( *iter == '(' && iter != parseTree.begin() )
                {
                    headLabel = false;
                }
                continue;
            }
            else if( *iter == ' ' )
            {
                parent = currentNodePos - 1;
                continue;
            }
            else
            {
                if( iter != parseTree.end() && *( iter + 1 ) == ')' )
                {
                    string lable( "(" );
                    ++nodePos;
                    currentNodePos = nodePos;
                    ++level;
                    ++parent;
                    leafFlag = true;
                    TreeNode treeNode( lable, parent, level, nodePos );
                    //if( leafFlag )
                    //{
                        leaf.insert( make_pair( srcSentPos, treeNode ) );
                        ++srcSentPos;
                    //}
                    if( parent != -1 )
                    {
                        if( parent == nodePos )
                        {
//                          cerr<<"parent:"<<parent<<" "<<"nodePos:"<<nodePos<<endl;
                            return false;
                        }
                        tree[ parent ].son.insert( nodePos );
                    }

                    tree.push_back( treeNode );

                }
                else
                {
                    ++level;
                    parent = currentNodePos;
                }
            }
        }
        else if( *iter != ')' )
        {
            string lable;
            lable += *iter;

            while( iter!= parseTree.end() && *( iter + 1 ) !=  ' ' && *( iter + 1 ) != ')' )
            {
                if( *iter == '(' )
                    return false;
                ++iter;
                lable += *iter;
            }
//          parent = nodePos;
            ++nodePos;
            currentNodePos = nodePos;
            if( iter != parseTree.end() && *( iter + 1 ) == ')' )
            {
                ++level;
                ++parent;
                leafFlag = true;
            }

            TreeNode treeNode( lable, parent, level, nodePos );
            if( leafFlag )
            {
                leaf.insert( make_pair( srcSentPos, treeNode ) );
                ++srcSentPos;
            }
            if( parent != -1 )
            {
                if( parent == nodePos )
                {
//                  cerr<<"parent:"<<parent<<" "<<"nodePos:"<<nodePos<<endl;
                    return false;
                }
                tree[ parent ].son.insert( nodePos );
            //  tree[ parent ].son.push_back( nodePos );
            //  tree[parent].son.push_back(nodePos);
            }
            tree.push_back( treeNode );

        }
        else
        {
            if( iter != ( parseTree.end() - 1 ) && *( iter - 1 ) == ' ' )
//          if(  *( iter - 1 ) == ' ' )
            {
//              cerr<<"parseTree.end()"<<*(parseTree.end()-1)<<endl;
                string lable( ")" );
                ++nodePos;
                currentNodePos = nodePos;
                ++level;
                ++parent;
                leafFlag = true;
                TreeNode treeNode( lable, parent, level, nodePos );
                //if( leafFlag )
                //{
                    leaf.insert( make_pair( srcSentPos, treeNode ) );
                    ++srcSentPos;
                //}
                if( parent != -1 )
                {
                    if( parent == nodePos )
                    {
//                      cerr<<"parent:"<<parent<<" "<<"nodePos:"<<nodePos<<endl;
                        return false;
                    }
                    tree[ parent ].son.insert( nodePos );
                }
                tree.push_back( treeNode );
            }
            else if( leafFlag )
            {
                level -= 2;
//              currentNodePos -= 2;
                leafFlag = false;
                --currentNodePos;
                currentNodePos = tree[ currentNodePos ].parent;
            }
            else
            {
                if( currentNodePos == -1 )
                    return true;
                currentNodePos = tree[ currentNodePos ].parent;
                --level;
            }

        }
    }
    return true;
}

string Tree::validateSpanOfParseTree( size_t &beginPos, size_t &endPos )
{
//  cerr<<"beginPos:"<<beginPos<<" "<<"endPos:"<<endPos<<endl;
    if( endPos < beginPos )
    {    
        cerr<<"EndPos is bigger than beginPos!\n";
        return "ERROR";
    }
    else if( endPos == beginPos )
    {
        map< size_t, TreeNode>::iterator iter = leaf.find( beginPos );
        if( iter != leaf.end() )
        {
            return tree[ iter->second.parent ].lable;
        }
    }
    else
    {

        beginPos = leaf.find( beginPos )->second.nodePos;
        endPos = leaf.find( endPos )->second.nodePos;

        while( tree[ beginPos ].parent != tree[ endPos ].parent && ( tree[ beginPos ].parent != -1 || tree[ endPos ].parent != -1 ) )
        {
            while( tree[ beginPos ].level > tree[ endPos ].level )
            {
                if( *( tree[ tree[ beginPos ].parent ].son.begin() ) < int( beginPos ) )
                    return "UNDEF";
                beginPos = tree[ beginPos ].parent;
            }
            while( tree[ beginPos ].level < tree[ endPos ].level )
            {
//              leaf.find( endPos )->second.nodePos;
//              cerr<<tree[ leaf.find( endPos )->second.nodePos ].parent<<endl;
//              cerr<<( tree[ tree[ leaf.find( endPos )->second.nodePos ].parent ].lable )<<endl;
//              cerr<<*( tree[ tree[ leaf.find( endPos )->second.nodePos ].parent ].son.rbegin() )<<endl;
                if( *( tree[ tree[ endPos ].parent ].son.rbegin() ) > int( endPos ) )
                    return "UNDEF";
                endPos = tree[ endPos ].parent;
            }
            while( tree[ beginPos ].level == tree[ endPos ].level )
            {
                if( *tree[ tree[ beginPos ].parent ].son.begin() < int( beginPos ) )
                    return "UNDEF";
                if( *tree[ tree[ endPos ].parent ].son.rbegin() > int( endPos ) )
                    return "UNDEF";
                if( tree[ beginPos ].parent == tree[ endPos ].parent )
                    return tree[ tree[ beginPos ].parent ].lable;
                beginPos = tree[ beginPos ].parent;
                endPos = tree[ endPos ].parent;

            }
        }

    }
    return "UNDEF";
}
