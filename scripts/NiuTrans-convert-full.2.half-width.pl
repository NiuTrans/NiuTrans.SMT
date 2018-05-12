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
#   version      : 1.0.0 Beta
#   Function     : convert full-width char to half-width
#   Author       : Qiang Li
#   Email        : liqiangneu@gmail.com
#   Date         : 06/26/2011
#   Last Modified: 
#######################################


use strict;
use Encode;
use utf8;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "#  Convert full-width char (version 1.0.0 Beta)  --www.nlplab.com  #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;

my %param;
my %dict;

getParameter( @ARGV );
constructDict();
generateRes();

sub generateRes
{
          open( INFILE, "<".$param{ "-s" } ) or die "Error: can not read file $param{ \"-s\" }\n";
          open( OUTFILE, ">".$param{ "-o" } ) or die "Error: can not write file $param{ \"-o\" }\n";
          my $lineNo = 0;
          print STDERR "$param{ \"-s\"}:";
          while( <INFILE> )
          {
                    ++$lineNo;
                    s/[\r\n]//g;
                    my $returnValue;
                    my $returnValue = convertChar( $_ );
                    print OUTFILE $returnValue."\n";
                    print STDERR "\r$param{ \"-s\"}: process $lineNo lines!" if( $lineNo % 1000 == 0 );
          }
          print STDERR "\r$param{ \"-s\"}: process $lineNo lines!\n";

          close( INFILE );
          close( OUTFILE );
}

sub convertChar
{
          if( scalar( @_ ) != 1 )
          {
                    print STDERR "Error: parameter in convertChar is wrong!\n";
                    exit( 1 );
          }
          my $src = shift @_;
          my $key;
          my $value;
          foreach $key ( keys %dict )
          {
                    $value = $dict{ $key };
                    $src =~ s/$key/$value/g;
          }
          return $src;
}

sub constructDict
{
          open( DICT, "<".$param{ "-dict" } ) or die "Error: can not open file $param{ \"-dict\" }\n";
          
          my $lineNo = 0;
          print STDERR "Loading dict...";
          while( <DICT> )
          {
                    ++$lineNo;
                    s/[\r\n]//g;
                    if( /^(.*)\t(.*)$/ )
                    {
                              $dict{ $1 } = $2;
                              print STDERR "\rLoading dict... $lineNo";
                    }
                    else
                    {
                              print STDERR "Error: dict format is not right!\n";
                    }
          }
          print STDERR "\rLoading dict... $lineNo\n";
          close( DICT );
}

sub getParameter
{
          if( ( scalar( @_ ) < 4 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]    :\n".
                                 "       NiuTrans-convert-full.2.half-width.pl [OPTIONS]\n".
                                 "[OPTION]   :\n".
                                 "    -s     :  Source File.\n".
                                 "    -o     :  Output File.\n".
                                 "    -dict  :  Used dictionary to convert.    [optional]\n".
                                 "                Default file is fullwidth.2.halfwidth.utf8.dict\n".
                                 "                in /resource dir.\n".
                                 "[EXAMPLE]  :\n".
                                 "    perl NiuTrans-convert-full-width-char.pl [-dict DICT]\n".
                                 "                                             [-s    FILE]\n".
                                 "                                             [-o    FILE]\n";
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
          if( !exists $param{ "-dict" } )
          {
                    $param{ "-dict" } = "../resource/fullwidth.2.halfwidth.utf8.dict";
          }
          if( !exists $param{ "-s" } )
          {
                    print STDERR "Error: please assign \"-s\"!\n";
                    exit( 1 );
          }
          if( !exists $param{ "-o" } )
          {
                    print STDERR "Error: please assign \"-o\"!\n";
                    exit( 1 );
          }
}
