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
#   version      : 1.2.0 Beta
#   Function     : Running NiuSegmenter
#   Author       : Qiang Li
#   Email        : liqiangneu@gmail.com
#   Date         : 01/24/2013
#   Last Modified: 
#######################################

use strict;


my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "# NiuTrans Running NiuSeg (version 1.2.0 Beta)    --www.nlplab.com #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";


print STDERR $logo;

my %param;

getParameter( @ARGV );
runningNiuSegmenter();

sub runningNiuSegmenter
{
          my $encode = 0;
          if( $param{ "-encode" } eq "utf-8" )
          {
                   $encode = 1;
          }
          elsif( $param{ "-encode" } eq "gb2312" )
          {
                   $encode = 0;
          }
          else
          {
                   print STDERR "Error: Please check parameter \"-encode\", the optional value is \"utf-8\" or \"gb2312\".\n";
                   exit( 1 );
          }

          if( $param{ "-lang" } eq "ch" )
          {
                    open( OUTCONF, ">", "tmp.config.chi" ) or die "Error: can not open file \"tmp.config.chi\"!\n";
                    print OUTCONF $param{ "-input" }." ".$param{ "-output" }.".step1 ".$encode."\n";
                    close( OUTCONF );
                    ssystem( "../bin/NiuSegmenter_CN_x64 ../config/NiuTrans.NiuSeg.ch.config tmp.config.chi 111$param{ \"-method\" }" );

                    ssystem( "perl ../resource/remove.proper-noun.pl < $param{ \"-output\" }.step1 > $param{ \"-output\" }" );
                    unlink "tmp.config.chi";
          }
          elsif( $param{ "-lang" } eq "en" )
          {
                    open( OUTCONF, ">", "tmp.config.eng" ) or die "Error: can not open file \"tmp.config.eng\"!\n";
                    print OUTCONF $param{ "-input" }." ".$param{ "-output" }.".step1 ".$encode."\n";
                    close( OUTCONF );
                    ssystem( "../bin/NiuSegmenter_EN_x64 ../config/NiuTrans.NiuSeg.en.config tmp.config.eng 111$param{ \"-method\" }" );
          
                    ssystem( "perl ../resource/remove.proper-noun.pl < $param{ \"-output\" }.step1 > $param{ \"-output\" }" );
                    unlink "tmp.config.eng";
          }
          else
          {
                   print STDERR "Error: Please check parameter \"-lang\", the optional value is \"ch\" or \"en\".\n";
                   exit( 1 );
          }
          unlink $param{ "-output" }.".step1";
}


sub getParameter
{
#          print STDERR scalar( @_ )."\n";
          if( ( scalar( @_ ) < 2 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]       :\n".
                                 "      NiuTrans-running-segmenter.pl          [OPTIONS]\n".
                                 "[OPTION]      :\n".
                                 "    -lang     :  Selected Language.          [optional]\n".
                                 "                   Defaule value is \"en\".\n".
                                 "                   Optional value is \"ch\" or \"en\".\n".
                                 "    -input    :  Inputted File.\n".
                                 "    -output   :  Outputted File.\n".
                                 "    -encode   :  Character Encoding.         [optional]\n".
                                 "                   Default value is \"utf-8\".\n".
                                 "                   Optional value is \"utf-8\" or \"gb2312\".\n".
                                 "    -method   :  The value is \"00\", \"01\", or \"11\".\n".
                                 "                   Default value is \"01\".\n".
                                 "[EXAMPLE]     :\n".
                                 "       perl NiuTrans-running-segmenter.pl  [-lang   PARA]\n".
                                 "                                           [-input  FILE]\n".
                                 "                                           [-output FILE]\n".
                                 "                                           [-encode PARA]\n".
                                 "                                           [-method PARA]\n";
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


          if( !exists $param{ "-lang" } )
          {
                    $param{ "-lang" } = "en";
          }

          if( !exists $param{ "-input" } )
          {
                    print STDERR "Error: please assign \"-input\"!\n";
                    exit( 1 );
          }
          else
          {
                    if( !( -e $param{ "-input" } ) )
                    {
                              print STDERR "$param{ \"-input\" } does not exist!\n";
                              exit( 1 );
                    }
          }
          if( !exists $param{ "-output" } )
          {
                    print STDERR "Error: please assign \"-output\"!\n";
                    exit( 1 );
          }

          if( !exists $param{ "-encode" } )
          {
                    $param{ "-encode" } = "utf-8";
          }

          if( !exists $param{ "-method" } )
          {
                    $param{ "-method" } = "01";
          }
}

sub ssystem
{
          print STDERR "Running: @_\n";
          system( @_ );
          if( $? == -1 )
          {
                    print STDERR "Error: Failed to execute: @_\n  $!\n";
                    exit( 1 );
          }
          elsif( $? & 127 )
          {
                    printf STDERR "Error: Execution of: @_\n   die with signal %d, %s coredump\n",
                    ($? & 127 ), ( $? & 128 ) ? 'with' : 'without';
                    exit( 1 );
          }
          else
          {
                    my $exitcode = $? >> 8;
                    print STDERR "Exit code: $exitcode\n" if $exitcode;
                    return ! $exitcode;
          }         
}
