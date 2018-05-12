#!/usr/bin/perl

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
#   Function  : format-check.pl
#   Author    : Hao Zhang
#   Email     : zhanghao1216@gmail.com
#   Date      : 07/06/2012
#######################################

# ||| armed services ||| NR ||| 0 -1.38629 -2.70805 -7.3885 1 -13.5257 -13.5257 1 0 1 0 ||| 0-0 0-1

while(<STDIN>) {
	$_ =~ s/[\r\n]//g;
	@tms = split( / \|\|\| /, $_ );
	print STDERR "format error!\n" if @tms != 5;
	#$tms[1] = AdjustNTId($tms[1]);
	@fs = split( / /, $tms[3] );
	print STDERR "format error!\n" if @fs != 11;
	$f0 = exp( $fs[0] );
	$f1 = exp( $fs[1] );
	$f2 = exp( $fs[2] );
	$f3 = exp( $fs[3] );
	$f5 = exp( $fs[5] );
	$f6 = exp( $fs[6] );
	print "$tms[0] ||| $tms[1] ||| $tms[2] ||| $f0 $f1 $f2 $f3 $f5 $f6 $fs[7] $fs[8] $fs[9] ||| $tms[4]\n";
}

sub AdjustNTId
{
	my $term = shift;
	my @tWords = split(/ +/, $term);
	foreach my $i(0..@tWords-1){
		if($tWords[$i] =~ /\#([0-9]+)/){
			$newId = $1 - 1;
			$tWords[$i] = '#'.$newId;
		}
	}
	return join(' ', @tWords);
}

