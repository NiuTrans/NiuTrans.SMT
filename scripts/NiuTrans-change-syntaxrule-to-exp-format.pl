##################################################################################
#
# NiuTrans - SMT platform
# Copyright (C) 2011, NEU-NLPLab (http://www.nlplab.com/). All rights reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
#
##################################################################################

#############################################
#   version          : 1.0.0 Beta
#   Function         : train-model
#   Author           : Qiang Li
#   Email            : liqiangneu@gmail.com
#   Date             : 06/09/2011
#   last Modified by :
#     2011/6/26 user can select which program to execute in line 277-286 by Qiang Li
#     2011/6/26 remove MSD.s file in line 206-208 by Qiang Li
#############################################

$lineNo = 0;
while( <STDIN> )
{
	++$lineNo;
	s/[\r\n]//g;
	
	$line = $_;

#	@domain = split / \|\|\| /, $line;
#	@tgtPhr = split / +/, $domain[ 1 ];
#
#	$pos = 0;
#	$newtgtphr = "";
#	for( 0 .. scalar( @tgtPhr ) - 1 )
#	{
#		if( $tgtPhr[ $pos ] =~ /#(\d+)/ )
#		{
#			$num = $1 + 1;
#			$newtgtphr .= "#"."$num ";
#		}
#		else
#		{
#			$newtgtphr .= $tgtPhr[ $pos ]." ";
#		}
#		++$pos;
#	}
#
#	print $domain[ 0 ]." \|\|\| ".$newtgtphr."\|\|\| ".$domain[ 2 ]." \|\|\| ".$domain[ 3 ]." \|\|\| ".$domain[ 4 ]."\n"."<null>\n\n";
        print $_."\n"."<null>\n\n";
	print STDERR "\r        processed ".$lineNo." lines." if( $lineNo % 10000 == 0 );

}
print STDERR "\r        processed ".$lineNo." lines.\n";

