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
#   Function  : trans.to.new.syntax.rule.pl
#   Author    : Hao Zhang
#   Email     : zhanghao1216@gmail.com
#   Date      : 07/06/2012
#######################################

while(<STDIN>){
	chomp();
	my @terms = split(/ \|\|\| /, $_);
	
	$terms[1] = AdjustNTId($terms[1]);
	my @feats = split(/ /, $terms[3]);
	foreach $i(0..5){
		$feats[$i] = sprintf("%.8f", log($feats[$i]));
	}
	$terms[3] = join(' ', @feats[0..3]).' 1 '.join(' ', @feats[4..@feats-1]);
	
	print join(' ||| ', @terms)."\n";
	
	$_ = <STDIN>;
	chomp();
	while($_ ne ''){
		@terms = split(/ \|\|\| /, $_);
		my @tWords = split(/ +/, $terms[1]);
		foreach my $i(0..@tWords-1){
			if($tWords[$i] =~ /\#([0-9]+)/){
				$newId = $1 + 0;
				$tWords[$i] = '#'.$newId;
			}
		}
		$terms[1] = join(' ', @tWords);
		
		$terms[0] =~ s/^#//g;
		print $terms[1].' ||| '.AdjustNTId($terms[2]).' ||| '.$terms[0]."\n" if $terms[0] ne '<null>';
		print "<null>\n" if $terms[0] eq '<null>';
		
		$_ = <STDIN>;
		chomp();
	}
	print "\n";
	$count++;
}

print STDERR "# of rules: ".$count."\n";

sub AdjustNTId
{
	my $term = shift;
	my @tWords = split(/ +/, $term);
	foreach my $i(0..@tWords-1){
		if($tWords[$i] =~ /\#([0-9]+)/){
			$newId = $1 + 0;
			$tWords[$i] = '#'.$newId;
		}
	}
	return join(' ', @tWords);
}