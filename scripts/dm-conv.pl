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
#   Function  : dm-conv.pl
#   Author    : Tong Xiao
#   Email     : xiaotong@mail.neu.edu.cn
#   Date      : 07/06/2012
#######################################


open(FH, $ARGV[0]) || die $!;
while (<FH>)
{
	next if (/^#/);
	chomp;
	die if (!/^\d+/);
	$feat_count = $_;
	last;
}

print STDERR "Reading feature name...";
$fc = 0;
while (<FH>)
{
	chomp;
	push @feat, $_;
	last if (++$fc == $feat_count);
}

print STDERR "class name...";
$t = <FH> || die $!;
$class_count = $t;
for (1..$class_count)
{
	$t = <FH> || die $!;
}

print STDERR "feature-class mapping...";
@feat2 = ();
for $i(0..$feat_count-1)
{
	$t = <FH> || die $!;
	chomp $t;
	$t =~ s/\s+$//;
	($ncls, @cls) = split / /, $t;
	for (@cls)
	{
		push @feat2, "$_:$feat[$i]";
	}
}

print STDERR "feature weights...";
$t = <FH> || die $!;
$fv_count = $t;
die if ($fv_count != scalar @feat2);

$n = 0;
while (<FH>)
{
	chomp;
	$FEAT{$feat2[$n]} = $_ + 0;
	print $feat2[$n], "\t", $_ + 0, "\n";
	$n++;
}
print STDERR "\n";

$f1 = 'SLL=ÔÚ';
$f2 = 'SRL=¾ÙÐÐ';
$w1 = exp($FEAT{"0:$f1"} + $FEAT{"0:$f2"});
$w2 = exp($FEAT{"1:$f1"} + $FEAT{"1:$f2"});

$prob1 = $w1/($w1+$w2);
$prob2 = $w2/($w1+$w2);

print STDERR $prob1, "\t", $prob2;




