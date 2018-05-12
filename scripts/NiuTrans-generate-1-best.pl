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
#   Function  : generate-1-best
#   Author    : Qiang Li
#   Email     : liqiangneu@gmail.com
#   Date      : 06/16/2011
#######################################


use strict;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "#  NiuTrans generate 1-best (version 1.0.0 Beta) --www.nlplab.com  #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;

if( scalar( @ARGV ) != 2 )
{
    print STDERR "Usage: perl Niutrans-generate-1-best.pl n-best 1-best!\n";
    exit( 1 );
}

open( NBEST, "<".$ARGV[0] ) or die "Error: Can not open file $ARGV[0]\n";
open( ONEBEST, ">".$ARGV[1] ) or die "Error: Can not open file $ARGV[1]\n";

my $num = 0;
my $sentenceNum = 0;
while( <NBEST> )
{
    s/[\r\n]//g;
    next if /^$/;
    ++$num;
#    ++$sentenceNum,print ONEBEST $`."\n" if $num == 1 && / (\|\|\|\|) /;
    
    if( /^================$/ && $num > 1 )
    {
        $num = 0;
        next;
    }
    elsif( /^================$/ && $num == 1 )
    {
        print ONEBEST "\n";
        ++$sentenceNum;
        $num = 0;
        next;
    }
    
    if( $num == 1 )
    {
        if( / (\|\|\|\|) / )
        {
            print ONEBEST $`."\n";
            ++$sentenceNum;
        }
        else
        {
            print ONEBEST $_."\n";
            ++$sentenceNum;
        }
    }
    
    print STDERR "\rSentence Number:$sentenceNum" if $sentenceNum % 100 == 0;
}

print STDERR "\rSentence Number:$sentenceNum\n";

close( NBEST );
close( ONEBEST );
