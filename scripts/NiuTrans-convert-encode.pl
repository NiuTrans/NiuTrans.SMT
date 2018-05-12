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
#   Function     : convert encode
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
             "#  NiuTrans convert encode (version 1.0.0 Beta)  --www.nlplab.com  #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;

my %param;

getParameter( @ARGV );
convertEncode();

sub convertEncode
{
          open( INFILE, "<".$param{ "-s" } ) or die "Error: can not read $param{ \"-s\" }!\n";
          open( OUTFILE, ">".$param{ "-o" } ) or die "Error: can not write $param{ \"-o\" }!\n";
          my $lineNo = 0;
          print STDERR "process";
          while( <INFILE> )
          {
                    ++$lineNo;
                    s/[\r\n]//g;
                    my $temp = encode( "$param{ \"-oe\" }", decode( "$param{ \"-se\" }", $_ ) );
                    print OUTFILE $temp."\n";
                    print STDERR "\rprocess $lineNo lines." if( $lineNo % 1000 == 0 );
          }
          print STDERR "\rprocess $lineNo lines.\n";
          close( INFILE );
          close( OUTFILE );
}

sub getParameter
{
          if( ( scalar( @_ ) < 4 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]    :\n".
                                 "       NiuTrans-convert-encode.pl [OPTIONS]\n".
                                 "[OPTION]   :\n".
                                 "    -s     :  Source File.\n".
                                 "    -o     :  Output File.\n".
                                 "    -se    :  Source File Encode.  [optional]\n".
                                 "                  Default \"gb2312\"\n".
                                 "    -oe    :  Output File Encode.  [optional]\n".
                                 "                  Default \"utf-8\"\n".
                                 "[EXAMPLE]  :\n".
                                 "    perl NiuTrans-convert-encode.pl -s sf -o of -se gb2312 -oe utf-8\n";
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
          if( !exists $param{ "-se" } )
          {
                    $param{ "-se" } = "gb2312";
          }
          if( !exists $param{ "-oe" } )
          {
                    $param{ "-oe" } = "utf-8";
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
