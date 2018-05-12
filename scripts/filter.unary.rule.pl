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

my $mPOSRuleNum = 150;
my $mNoPOSRuleNum = 150;
foreach my $i(0..@ARGV-2){
	$mPOSRuleNum = $ARGV[$i+1] if $ARGV[$i] eq '-pos';
	$mNoPOSRuleNum = $ARGV[$i+1] if $ARGV[$i] eq '-nopos';
}

print STDERR "max unary rule (POS): ".$mPOSRuleNum."\n";
print STDERR "max unary rule (non-POS): ".$mNoPOSRuleNum."\n";

my %posHash = ();
open( $posFile, "<".'pos.txt' ) or die "cannot open pos-file\n";

while( <$posFile> ){
	chomp($_);
	$posHash{'#'.$_} = 1;
}

close( $posFile );

open( $posFile, "<".'pos.ch.txt' ) or die "cannot open pos-file\n";
while( <$posFile> ){
	chomp($_);
	$posHash{'#'.$_} = 1;
}
close( $posFile );

open( $unaryRFile_POS, ">".'unary.rule.pos.txt' ) or die "cannot open pos-unary-rule file\n";
open( $unaryRFile_NoPOS, ">".'unary.rule.nopos.txt' ) or die "cannot open nopos-unary-rule file\n";

my %unaryRuleHash_Pos = ();
my %unaryRuleHash_NoPOs = ();

while(<STDIN>){
	chomp($_);
	my $ruleString = $_;
	my @terms = split( / \|\|\| / , $ruleString );
	my @words = split( / +/, $terms[0]);
	if( @words == 1 && $words[0] =~ /#(.+)/ ){
		my @tags = split( /=/, $1);
		if( @tags > 1 ){
			if(defined($posHash{'#'.$tags[0]}) || defined($posHash{'#'.$tags[1]})){
				my @features = split( / +/, $terms[3] );
				$unaryRuleHash_POS{$ruleString} = $features[4];
				print $unaryRFile_POS $ruleString."\n";
			}
			else{
				my @features = split( / +/, $terms[3] );
				$unaryRuleHash_NoPOS{$ruleString} = $features[4];
				print $unaryRFile_NoPOS $ruleString."\n";
			}
		}
		else{
			if(defined($posHash{$words[0]})){
				my @features = split( / +/, $terms[3] );
				$unaryRuleHash_POS{$ruleString} = $features[4];
				print $unaryRFile_POS $ruleString."\n";
			}
			else{
				my @features = split( / +/, $terms[3] );
				$unaryRuleHash_NoPOS{$ruleString} = $features[4];
				print $unaryRFile_NoPOS $ruleString."\n";
			}
		}
	}
	else{
		print $ruleString."\n";
	}
}

close( $unaryRFile_POS );
close( $unaryRFile_NoPos );

my $count = 0;
foreach my $key( sort{ $unaryRuleHash_POS{$b} <=> $unaryRuleHash_POS{$a} } keys %unaryRuleHash_POS )
{
	last if $count++ > $mPOSRuleNum;
	print $key."\n";
}

$count = 0;
foreach my $key( sort{ $unaryRuleHash_NoPOS{$b} <=> $unaryRuleHash_NoPOS{$a} } keys %unaryRuleHash_NoPOS )
{
	last if $count++ > $mNoPOSRuleNum;
	print $key."\n";
}