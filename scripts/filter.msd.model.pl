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
#   Function  : filter.msd.model.pl
#   Author    : Tong Xiao
#   Email     : xiaotong@mail.neu.edu.cn
#   Date      : 06/16/2011
#######################################


die "usage: phrase-table-file msd-model-file [-inversed]\n" if @ARGV < 2;

my %phraseEntry = ();
my ($pCount, $rCount) = (0, 0);
my $inversed = 0;
if( @ARGV > 2 && $ARGV[2] eq "-inversed"){
	$inversed = 1;
	print STDERR "setting: inversed!\n";
}

print STDERR "loading phrase table ...\n";
open($pf, "<".$ARGV[0]);
while(<$pf>){
	chomp();
	@terms = split( / \|\|\| /, $_ );
	$phraseEntry{$terms[0]}{$terms[1]} = 1;
	print STDERR "\r".$pCount if ++$pCount % 100000 == 0;
}
close($pf);
print STDERR "\r".$pCount."\n";

print STDERR "filtering ...\n";
open($msdf, "<".$ARGV[1]);
while(<$msdf>){
	chomp();
	@terms = split( / \|\|\| /, $_ );
	($key1, $key2) = ($terms[0], $terms[1]) if $inversed == 0;
	($key1, $key2) = ($terms[1], $terms[0]) if $inversed == 1;
	if(defined($phraseEntry{$key1}) && defined($phraseEntry{$key1}{$key2})){
		print $_."\n";
	}
	print STDERR "\r".$rCount if ++$rCount % 100000 == 0;
}
close($msdf);
print STDERR "\r".$rCount."\n";
