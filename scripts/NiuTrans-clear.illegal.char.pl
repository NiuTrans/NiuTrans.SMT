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
#   Function     : NiuTrans-clear.illegal.char
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
             "#  NiuTrans clear illegal char (version 1.1.0)   --www.nlplab.com  #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;

my %param;

getParameter( @ARGV );

open( SRCINFILE, "<", $param{ "-src" } ) or die "Error: can not read file $param{ \"-src\" }.\n";
open( TGTINFILE, "<", $param{ "-tgt" } ) or die "Error: can not read file $param{ \"-tgt\" }.\n";
open( OUTSRCFILE, ">", $param{ "-outSrc" } ) or die "Error: can not write file $param{ \"-outSrc\" }.\n";
open( OUTTGTFILE, ">", $param{ "-outTgt" } ) or die "Error: can not write file $param{ \"-outTgt\" }.\n";
open( OUTSRCFILEDISCARD, ">", $param{ "-outSrc" }.".discard" ) or die "Error: can not write file $param{ \"-outSrc\" }.illegalchar.\n";
open( OUTTGTFILEDISCARD, ">", $param{ "-outTgt" }.".discard" ) or die "Error: can not write file $param{ \"-outTgt\" }.illegalchar.\n";

print STDERR "Start filter with illegal char:\n";
my $lineNo = 0;
my $reservedNum = 0;
my $nullNum = 0;
my $illegalCharNum = 0;
my $srcInFile = "";
my $tgtInFile = "";
while( $srcInFile = <SRCINFILE> )
{
    ++$lineNo;
    $tgtInFile = <TGTINFILE>;

    $srcInFile =~ s/[\r\n]//g;
	$tgtInFile =~ s/[\r\n]//g;
	$srcInFile =~ s/\t/ /g;
	$tgtInFile =~ s/\t/ /g;
	$srcInFile =~ s/ +/ /g;
	$tgtInFile =~ s/ +/ /g;
	$srcInFile =~ s/^ +//;
	$tgtInFile =~ s/^ +//;
    $srcInFile =~ s/ +$//;
    $tgtInFile =~ s/ +$//;
	
	if( $srcInFile eq "" or $tgtInFile eq "" )
	{
	    ++$nullNum;
        print OUTSRCFILEDISCARD "[$lineNo NULL] $srcInFile\n";
		print OUTTGTFILEDISCARD "[$lineNo NULL] $tgtInFile\n";
		next;
	}
	elsif( !( $tgtInFile =~ /^[[:ascii:]]+$/ ) )
	{
	    ++$illegalCharNum;
		print OUTSRCFILEDISCARD "[$lineNo] $srcInFile\n";
		print OUTTGTFILEDISCARD "[$lineNo] $tgtInFile\n";
	}
	elsif( $srcInFile =~ /[[:cntrl:]]/ )
	{
	    ++$illegalCharNum;
		print OUTSRCFILEDISCARD "[$lineNo ControlC] $srcInFile\n";
		print OUTTGTFILEDISCARD "[$lineNo ControlC] $tgtInFile\n";	
	}
	elsif( $tgtInFile =~ /[[:cntrl:]]/ )
    {
		++$illegalCharNum;
		print OUTSRCFILEDISCARD "[$lineNo ControlE] $srcInFile\n";
		print OUTTGTFILEDISCARD "[$lineNo ControlE] $tgtInFile\n";	
	}
#	elsif( !( $srcInFile =~ /^[[:ascii:]\xA1-\xFE]+$/ ) )
#	{
#		++$illegalCharNum;
#		print OUTSRCFILEDISCARD "[$lineNo UNGBK] $srcInFile\n";
#		print OUTTGTFILEDISCARD "[$lineNo UNGBK] $tgtInFile\n";	
#	}
	else
	{
	    ++$reservedNum;
		print OUTSRCFILE "$srcInFile\n";
		print OUTTGTFILE "$tgtInFile\n";
	}
	
	print STDERR "\r    [LINENO=$lineNo RESERVED=$reservedNum DIS=$illegalCharNum NULL=$nullNum]" if $lineNo % 10000 == 0;
}
print STDERR "\r    [LINENO=$lineNo RESERVED=$reservedNum DIS=$illegalCharNum NULL=$nullNum]\n";

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
                                 "         NiuTrans-clear.illegal.char.pl       [OPTIONS]\n".
                                 "[OPTION]\n".
                                 "          -src    :  Source Input  File.\n".
                                 "          -tgt    :  Target Output File.\n".
                                 "          -outSrc :  Output Filtered Source File.\n".
                                 "          -outTgt :  Output Filtered Target File.\n".
                                 "[EXAMPLE]\n".
                                 "         perl NiuTrans-clear.illegal.char.pl  [-src    FILE]\n".
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
}
