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
#   Function     : NiuTrans-monolingual.clear.illegal.char
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
             "#  NiuTrans Monolingual clear illegal char       --www.nlplab.com  #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;

my %param;

getParameter( @ARGV );

open( TGTINFILE, "<", $param{ "-tgt" } ) or die "Error: can not read file $param{ \"-tgt\" }.\n";
open( OUTTGTFILE, ">", $param{ "-outTgt" } ) or die "Error: can not write file $param{ \"-outTgt\" }.\n";
open( OUTTGTFILEDISCARD, ">", $param{ "-tgt" }.".discard" ) or die "Error: can not write file $param{ \"-outTgt\" }.illegalchar.\n";

print STDERR "Start filter with illegal char:\n";
my $lineNo = 0;
my $reservedNum = 0;
my $nullNum = 0;
my $illegalCharNum = 0;
my $tgtInFile = "";
while( $tgtInFile = <TGTINFILE> )
{
    ++$lineNo;

    $tgtInFile =~ s/[\r\n]//g;
    $tgtInFile =~ s/\t/ /g;
    $tgtInFile =~ s/ +/ /g;
    $tgtInFile =~ s/^ +//;
    $tgtInFile =~ s/ +$//;
    
    if( $tgtInFile eq "" )
    {
        ++$nullNum;
        print OUTTGTFILEDISCARD "[$lineNo NULL] $tgtInFile\n";
        next;
    }
    elsif( ( $param{ "-lang" } eq "en" ) && !( $tgtInFile =~ /^[[:ascii:]]+$/ ) )
    {
        ++$illegalCharNum;
        print OUTTGTFILEDISCARD "[$lineNo UNASCII] $tgtInFile\n";
    }
    elsif( $tgtInFile =~ /[[:cntrl:]]/ )
    {
        ++$illegalCharNum;
        print OUTTGTFILEDISCARD "[$lineNo CONTROLE] $tgtInFile\n";    
    }
    else
    {
        ++$reservedNum;
        print OUTTGTFILE "$tgtInFile\n";
    }
    
    print STDERR "\r    [LINENO=$lineNo RESERVED=$reservedNum DIS=$illegalCharNum NULL=$nullNum]" if $lineNo % 10000 == 0;
}
print STDERR "\r    [LINENO=$lineNo RESERVED=$reservedNum DIS=$illegalCharNum NULL=$nullNum]\n";

close( TGTINFILE );
close( OUTTGTFILE );
close( OUTTGTFILEDISCARD );


sub getParameter
{
          if( ( scalar( @_ ) < 4 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]\n".
                                 "         NiuTrans-monolingualclear.illegal.char.pl [OPTIONS]\n".
                                 "[OPTION]\n".
                                 "          -tgt    :  Target Output File.\n".
                                 "          -outTgt :  Output Filtered Target File.\n".
                                 "          -lang   :  Language, \"en\" or \"zh\"        [optional]\n".
                                 "                       Default value is \"en\".\n".
                                 "[EXAMPLE]\n".
                                 "         perl NiuTrans-clear.illegal.char.pl       [-tgt    FILE]\n".
                                 "                                                   [-outTgt FILE]\n".
                                 "                                                   [-lang   PARA]\n";
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
                    
          if( !exists $param{ "-tgt" } )
          {
                    print STDERR "Error: please assign \"-tgt\"!\n";
                    exit( 1 );
          }
                    
          if( !exists $param{ "-outTgt" } )
          {
                    print STDERR "Error: please assign \"-outTgt\"!\n";
                    exit( 1 );
          }
          
          if( !exists $param{ "-lang" } )
          {
                    $param{ "-lang" } = "en";
          }
}
