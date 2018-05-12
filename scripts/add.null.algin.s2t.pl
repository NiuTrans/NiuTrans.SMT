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

#######################################
#   version   : 1.0.0 Beta
#   Function  : add.null.align.s2t.pl
#   Author    : Tong Xiao
#   Email     : xiaotong@mail.neu.edu.cn
#   Date      : 07/06/2012
#######################################

while(<STDIN>){
	chomp();
	@terms = split( / \|\|\| /, $_ );
	if( $terms[1] eq '<NULL>' ){
		$terms[1] = 'NULL';
	}
	if( @terms < 5 ){
		push(@terms, "0-0 ||| NULL ");
	}
	print join( ' ||| ', @terms )."\n";
}