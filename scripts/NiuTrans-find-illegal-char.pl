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


my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "# NiuTrans Find Illegal Char(version 1.0.0 Beta)  --www.nlplab.com #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";


print STDERR $logo;

my %param;

getParameter( @ARGV );
findIllegalChar();


sub findIllegalChar
{

          open( INFILE, "<".$param{ "-src" } )  or die "Error: can not read $param{ \"-s\" }!\n";
          open( OUTFILE, ">".$param{ "-out" } ) or die "Error: can not write $param{ \"-o\" }!\n";

          my $lineNo = 0;
          my $illegalCharNo = 0;
          print STDERR "process";
          while( <INFILE> )
          {
                    ++$lineNo;
                    s/[\r\n]//g;
#                    if( $_ =~ /#/ || $_ =~ /\|\|\|/ )
                    if( $_ =~ /#/ && $_ =~ /\|\|\|/ )
                    {
                              ++$illegalCharNo;
                              print OUTFILE $lineNo.":Find # and \|\|\|\n";   
                    }
                    elsif( $_ =~ /\|\|\|/ )
                    {
                              ++$illegalCharNo;
                              print OUTFILE $lineNo.":Find \|\|\|\n";   
                    }                    
                    elsif( $_ =~ /#/ )
                    {
                              ++$illegalCharNo;
                              print OUTFILE $lineNo.":Find #\n";   
                    }

                    print STDERR "\rprocess $lineNo lines. Illegal Char No:$illegalCharNo" if( $lineNo % 10000 == 0 );
          }
          print STDERR "\rprocess $lineNo lines. Illegal Char No:$illegalCharNo\n";
          close( INFILE );
          close( OUTFILE );
}





sub getParameter
{
          if( ( scalar( @_ ) < 4 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]    :\n".
                                 "       NiuTrans-find-illegal-char.pl [OPTIONS]\n".
                                 "[OPTION]   :\n".
                                 "    -src   :  Source File.\n".
                                 "    -out   :  Output File.\n".
                                 "[EXAMPLE]  :\n".
                                 "    perl NiuTrans-convert-encode.pl -src source-file -out output-file\n";
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
          if( !exists $param{ "-out" } )
          {
                    print STDERR "Error: please assign \"-out\"!\n";
                    exit( 1 );
          }
}
