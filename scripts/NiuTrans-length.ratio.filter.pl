#!/usr/bin/perl -w
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
#   version      : 1.1.0
#   Function     : NiuTrans-length.ratio.filter
#   Author       : Qiang Li
#   Email        : liqiangneu@gmail.com
#   Date         : 08/21/2012
#   Last Modified: 
#######################################


use strict;
use Encode;
use utf8;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "#  NiuTrans length ratio filter (version 1.1.0)  --www.nlplab.com  #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;

my %param;

getParameter( @ARGV );

#print STDERR "src=".$param{ "-src" }."\n";
#print STDERR "tgt=".$param{ "-tgt" }."\n";
#print STDERR "outSrc=".$param{ "-outSrc" }."\n";
#print STDERR "outTgt=".$param{ "-outTgt" }."\n";

open( SRCINFILE, "<", $param{ "-src" } ) or die "Error: can not read file $param{ \"-src\" }.\n";
open( TGTINFILE, "<", $param{ "-tgt" } ) or die "Error: can not read file $param{ \"-tgt\" }.\n";

my $srcInFile;
my $tgtInFile;
my $srcWordsTotalNum = 0;
my $tgtWordsTotalNum = 0;
my $lineNo = 0;
my $discardLengthNum = 0;
print STDERR "Statistical Length Ratio in corpus:\n";
print STDERR "    -lengthRestrict=$param{ \"-lengthRestrict\" }\n";
while( $srcInFile = <SRCINFILE> )
{
    ++$lineNo;
    $tgtInFile = <TGTINFILE>;

    $srcInFile =~ s/[\r\n]//g;
	$tgtInFile =~ s/[\r\n]//g;
	$srcInFile =~ s/ +/ /g;
	$tgtInFile =~ s/ +/ /g;
	$srcInFile =~ s/^ +//;
	$tgtInFile =~ s/^ +//;
    $srcInFile =~ s/ +$//;
    $tgtInFile =~ s/ +$//;
	
	next if( $srcInFile eq "" or $tgtInFile eq "" );
	
	
    my @srcWords = split / /, $srcInFile;
	my @tgtWords = split / /, $tgtInFile;

	if( scalar( @srcWords ) > $param{ "-lengthRestrict" } )
	{
	    ++$discardLengthNum;
		next;
	}
		
	$srcWordsTotalNum += scalar( @srcWords );
	$tgtWordsTotalNum += scalar( @tgtWords );
	
	print STDERR "\r    processed $lineNo lines. [DISCARDLENGTH=$discardLengthNum]" if $lineNo % 10000 == 0;
}
print STDERR "\r    processed $lineNo lines. [DISCARDLENGTH=$discardLengthNum]\n";

my $ratio = $srcWordsTotalNum/$tgtWordsTotalNum;

print STDERR  "    srcWordsTotalNum=".$srcWordsTotalNum."\n";
print STDERR  "    tgtWordsTotalNum=".$tgtWordsTotalNum."\n";
printf STDERR "    ratio           =%.6f\n", $ratio;

close( SRCINFILE );
close( TGTINFILE );


open( SRCINFILE, "<", $param{ "-src" } ) or die "Error: can not read file $param{ \"-src\" }.\n";
open( TGTINFILE, "<", $param{ "-tgt" } ) or die "Error: can not read file $param{ \"-tgt\" }.\n";

open( OUTSRCFILE, ">", $param{ "-outSrc" } ) or die "Error: can not write file $param{ \"-outSrc\" }.\n";
open( OUTTGTFILE, ">", $param{ "-outTgt" } ) or die "Error: can not write file $param{ \"-outTgt\" }.\n";
open( OUTSRCFILEDISCARD, ">", $param{ "-src" }.".discard" ) or die "Error: can not write file $param{ \"-outSrc\" }.discard.\n";
open( OUTTGTFILEDISCARD, ">", $param{ "-tgt" }.".discard" ) or die "Error: can not write file $param{ \"-outTgt\" }.discard.\n";

$lineNo = 0;

my $ratioLowerBound = $ratio*$param{ "-lowerBoundCoef" };
my $ratioUpperBound = $ratio*$param{ "-upperBoundCoef" };

printf STDERR "    ratioLowerBound =%.6f [%.6f*%.2f]\n", $ratioLowerBound, $ratio, $param{ "-lowerBoundCoef" };
printf STDERR "    ratioUpperBound =%.6f [%.6f*%.2f]\n", $ratioUpperBound, $ratio, $param{ "-upperBoundCoef" };

my $reservedNum = 0;
my $discardNum = 0;
my $discardNullNum = 0;
$discardLengthNum = 0;
print STDERR "Start filter using length ratio:\n";
print STDERR "    -lengthRestrict=$param{ \"-lengthRestrict\" }\n";
while( $srcInFile = <SRCINFILE> )
{
   ++$lineNo;
    $tgtInFile = <TGTINFILE>;

    $srcInFile =~ s/[\r\n]//g;
	$tgtInFile =~ s/[\r\n]//g;
	$srcInFile =~ s/ +/ /g;
	$tgtInFile =~ s/ +/ /g;
	$srcInFile =~ s/^ +//;
	$tgtInFile =~ s/^ +//;
    $srcInFile =~ s/ +$//;
    $tgtInFile =~ s/ +$//;
	
	if( $srcInFile eq "" or $tgtInFile eq "" )
	{
	    ++$discardNullNum;
		print OUTSRCFILEDISCARD "[$lineNo NULL] $srcInFile"."\n";
		print OUTTGTFILEDISCARD "[$lineNo NULL] $tgtInFile"."\n";
		next;
	}
	
    my @srcWords = split / /, $srcInFile;
	my @tgtWords = split / /, $tgtInFile;
	
	if( scalar( @srcWords ) > $param{ "-lengthRestrict" } )
	{
	    ++$discardLengthNum;
		print OUTSRCFILEDISCARD "[$lineNo UPPER$param{ \"-lengthRestrict\" }] $srcInFile"."\n";
		print OUTTGTFILEDISCARD "[$lineNo UPPER$param{ \"-lengthRestrict\" }] $tgtInFile"."\n";
		next;
	
	}
		
	my $srcWordsLength = scalar( @srcWords );
	my $tgtWordsLength = scalar( @tgtWords );
	
	my $sentenceRatio = $srcWordsLength/$tgtWordsLength;
	if( ( $sentenceRatio > $ratioLowerBound ) && ( $sentenceRatio < $ratioUpperBound ) )
	{
	     ++$reservedNum;
	     print OUTSRCFILE $srcInFile."\n";
		 print OUTTGTFILE $tgtInFile."\n";
	}
	else
	{
	     ++$discardNum;
	     print OUTSRCFILEDISCARD "[$lineNo] $srcInFile"."\n";
		 print OUTTGTFILEDISCARD "[$lineNo] $tgtInFile"."\n";
	}
	
	print STDERR "\r    [LINENO=$lineNo RESERVED=$reservedNum DISCARD=$discardNum DISCARDNULL=$discardNullNum DISCARDLENG=$discardLengthNum]" if $lineNo % 10000 == 0;
}
print STDERR "\r    [LINENO=$lineNo RESERVED=$reservedNum DISCARD=$discardNum DISCARDNULL=$discardNullNum DISCARDLENG=$discardLengthNum]\n";

close( SRCINFILE );
close( TGTINFILE );
close( OUTSRCFILE );
close( OUTTGTFILE );
close( OUTSRCFILEDISCARD );
close( OUTTGTFILEDISCARD );


sub getParameter
{
          if( ( scalar( @_ ) < 4 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]\n".
                                 "         NiuTrans-length.ratio.filter.pl      [OPTIONS]\n".
                                 "[OPTION]\n".
                                 "          -src    :  Source Input  File.\n".
                                 "          -tgt    :  Target Output File.\n".
                                 "          -outSrc :  Output Filtered Source File.\n".
								 "          -outTgt :  Output Filtered Target File.\n".
								 "  -lowerBoundCoef :  Lower Bound Coefficient. [optional]\n".
								 "                       Default value is 0.4.\n".
								 "  -upperBoundCoef :  Upper Bound Coefficient. [optional]\n".
								 "                       Default value is 3.\n".
								 "  -lengthRestrict :  Length Restrict.\n".
								 "                       Default value is 100.\n".
                                 "[EXAMPLE]\n".
                                 "         perl NiuTrans-length.ratiofilter.pl  [-src    FILE]\n".
                                 "                                              [-tgt    FILE]\n".
								 "                                              [-outSrc FILE]\n".
								 "                                              [-outTgt FILE]\n";
                    exit( 0 );
          }
          
          my $pos;
          for( $pos = 0; $pos < scalar( @_ ); ++$pos )
          {
                    my $key = $ARGV[ $pos ];
                    ++$pos;
                    my $value = $ARGV[ $pos ];
                    $param{ $key } = $value;
          }
          
          if( !exists $param{ "-src" } )
          {
                    print STDERR "Error: please assign \"-src\"!\n";
                    exit( 1 );
          }
          
          if( !exists $param{ "-tgt" } )
          {
                    print STDERR "Error: please assign \"-tgt\"!\n";
                    exit( 1 );
          }
          
          if( !exists $param{ "-outSrc" } )
          {
                    print STDERR "Error: please assign \"-outSrc\"!\n";
                    exit( 1 );
          }
          
          if( !exists $param{ "-outTgt" } )
          {
                    print STDERR "Error: please assign \"-outTgt\"!\n";
                    exit( 1 );
          }

          if( !exists $param{ "-lowerBoundCoef" } )
          {
                    $param{ "-lowerBoundCoef" } = 0.4;
          }
		  
          if( !exists $param{ "-upperBoundCoef" } )
          {
                    $param{ "-upperBoundCoef" } = 3;
          }  

          if( !exists $param{ "-lengthRestrict" } )
          {
                    $param{ "-lengthRestrict" } = 100;
          }  
}
