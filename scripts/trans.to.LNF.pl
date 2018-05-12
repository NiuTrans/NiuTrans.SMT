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
#   Function  : filter.unary.rule.pl
#   Author    : Tong Xiao
#   Email     : xiaotong@mail.neu.edu.cn
#   Date      : 07/06/2012
#######################################

my $count = 0;
my $LNFCount = 0;

while(<STDIN>){
	chomp();
	my @terms = split(/ \|\|\| /, $_);
	my $IsLNF = LNF($terms[0]);
	
	print $_."\n";
	
	$_ = <STDIN>;
	chomp();
	while($_ ne ""){
		print $_."\n" if $IsLNF == 0;
		$_ = <STDIN>;
		chomp();
	}
	print "<null>\n" if $IsLNF == 1;
	print "\n";
	
	$count++;
	$LNFCount++ if $IsLNF == 1;
	#print STDERR $terms[0]."\n" if $IsLNF == 0;
	
	print STDERR "\r".$count if $count % 10000 == 0;
}

print STDERR "\r".$count."\n";
print STDERR "# of rules: ".$count."\n";
print STDERR "# of rules in LNF: ".$LNFCount."\n";

sub LNF
{
	my $shs = shift;
	my @words = split(/ /, $shs);
	
	foreach my $i(1..@words-1){
		return 0 if $words[$i - 1] =~ /^#.+/ && $words[$i] =~ /^#.+/ ;
	}
	return 1;
}