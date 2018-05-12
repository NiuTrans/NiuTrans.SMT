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
#   Function     : Running GIZA++
#   Author       : Qiang Li
#   Email        : liqiangneu@gmail.com
#   Date         : 01/23/2013
#   Last Modified: 
#######################################

use strict;


my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "# NiuTrans Running GIZA++ (version 1.2.0 Beta)    --www.nlplab.com #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";


print STDERR $logo;

my %param;

getParameter( @ARGV );
runningGIZAPP();

sub runningGIZAPP
{
          ssystem( "cp $param{ \"-src\" } $param{ \"-tmpdir\" }src" );
          ssystem( "cp $param{ \"-tgt\" } $param{ \"-tmpdir\" }tgt" );

          #step1 plain2snt
          my $exec = "../bin/plain2snt.out ";
          my $opt  = "$param{ \"-tmpdir\" }src ".
                     "$param{ \"-tmpdir\" }tgt ";

          print STDERR "STEP1: Exec plain2snt ...\n";
          ssystem( "$exec $opt\n" );

          #step2 snt2cooc.out
          $exec = "../bin/snt2cooc.out ";
          my $opt  = "$param{ \"-tmpdir\" }src.vcb ".
                     "$param{ \"-tmpdir\" }tgt.vcb ".
                     "$param{ \"-tmpdir\" }src_tgt.snt ".
                     "> $param{ \"-tmpdir\" }cooc.src.tgt";
          print STDERR "STEP2: Exec snt2cooc.out ...\n";
          ssystem( "$exec $opt" );

          #step3 snt2cooc.out
          my $opt  = "$param{ \"-tmpdir\" }tgt.vcb ".
                     "$param{ \"-tmpdir\" }src.vcb ".
                     "$param{ \"-tmpdir\" }tgt_src.snt ".
                     "> $param{ \"-tmpdir\" }cooc.tgt.src";
          print STDERR "STEP3: Exec snt2cooc.out ...\n";
          ssystem( "$exec $opt" );

          #step4 mkcls
          $exec = "../bin/mkcls ";
          my $opt  = "-m2 ".
                     "-c80 ".
                     "-n10 ".
                     "-p$param{ \"-tmpdir\" }src ".
                     "-V$param{ \"-tmpdir\" }src.vcb.classes ".
                     "opt";
          print STDERR "STEP4: mkcls ...\n";
          ssystem( "$exec $opt" );

          #step5 mkcls
          my $opt  = "-m2 ".
                     "-c80 ".
                     "-n10 ".
                     "-p$param{ \"-tmpdir\" }tgt ".
                     "-V$param{ \"-tmpdir\" }tgt.vcb.classes ".
                     "opt";
          print STDERR "STEP5: mkcls ...\n";
          ssystem( "$exec $opt" );

          #step6 GIZA++
          $exec = "../bin/GIZA++ ";
          my $opt  = "-S $param{ \"-tmpdir\" }src.vcb ".
                     "-T $param{ \"-tmpdir\" }tgt.vcb ".
                     "-C $param{ \"-tmpdir\" }src_tgt.snt ".
                     "-CoocurrenceFile $param{ \"-tmpdir\" }cooc.src.tgt ".
                     "-O $param{ \"-tmpdir\" }src2tgt";
          print STDERR "STEP6: Running GIZA++ ...\n";
          ssystem( "$exec $opt" );

          #step7 GIZA++
          $exec = "../bin/GIZA++ ";
          my $opt  = "-S $param{ \"-tmpdir\" }tgt.vcb ".
                     "-T $param{ \"-tmpdir\" }src.vcb ".
                     "-C $param{ \"-tmpdir\" }tgt_src.snt ".
                     "-CoocurrenceFile $param{ \"-tmpdir\" }cooc.tgt.src ".
                     "-O $param{ \"-tmpdir\" }tgt2src";
          print STDERR "STEP7: Running GIZA++ ...\n";
          ssystem( "$exec $opt" );
         
          #step8 Sym Alignment
          $exec = "../bin/NiuTrans.SymAlignment ";
          my $opt  = "$param{ \"-tmpdir\" }tgt2src.A3.final ".
                     "$param{ \"-tmpdir\" }src2tgt.A3.final ".
                     "$param{ \"-out\" }";
          print STDERR "STEP8: Sym Alignment ...\n";
          ssystem( "$exec $opt" );


 
          unlink glob "$param{ \"-tmpdir\" }cooc.*";
          unlink      "$param{ \"-tmpdir\" }src";
          unlink      "$param{ \"-tmpdir\" }tgt";
          unlink      "$param{ \"-tmpdir\" }src2tgt.Decoder.config";
          unlink      "$param{ \"-tmpdir\" }src2tgt.gizacfg";
          unlink      "$param{ \"-tmpdir\" }src2tgt.n3.final";
          unlink      "$param{ \"-tmpdir\" }src2tgt.p0_3.final";
          unlink      "$param{ \"-tmpdir\" }src2tgt.perp";
          unlink      "$param{ \"-tmpdir\" }src2tgt.t3.final";
          unlink glob "$param{ \"-tmpdir\" }src2tgt.*.vcb";
          unlink      "$param{ \"-tmpdir\" }src_tgt.snt";
          unlink glob "$param{ \"-tmpdir\" }src.vcb*";
          unlink      "$param{ \"-tmpdir\" }tgt2src.Decoder.config";
          unlink      "$param{ \"-tmpdir\" }tgt2src.gizacfg";
          unlink      "$param{ \"-tmpdir\" }tgt2src.n3.final";
          unlink      "$param{ \"-tmpdir\" }tgt2src.p0_3.final";
          unlink      "$param{ \"-tmpdir\" }tgt2src.perp";
          unlink      "$param{ \"-tmpdir\" }tgt2src.t3.final";
          unlink glob "$param{ \"-tmpdir\" }tgt2src.*.vcb";
          unlink      "$param{ \"-tmpdir\" }tgt_src.snt";
          unlink glob "$param{ \"-tmpdir\" }tgt.vcb*";

          unlink      "$param{ \"-tmpdir\" }src2tgt.a3.final";
          unlink      "$param{ \"-tmpdir\" }src2tgt.d3.final";
          unlink      "$param{ \"-tmpdir\" }src2tgt.d4.final";
          unlink      "$param{ \"-tmpdir\" }src2tgt.D4.final";
          unlink      "$param{ \"-tmpdir\" }tgt2src.a3.final";
          unlink      "$param{ \"-tmpdir\" }tgt2src.d3.final";
          unlink      "$param{ \"-tmpdir\" }tgt2src.d4.final";
          unlink      "$param{ \"-tmpdir\" }tgt2src.D4.final";
}


sub getParameter
{
          if( ( scalar( @_ ) < 4 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]       :\n".
                                 "       NiuTrans-running-GIZA++.pl                [OPTIONS]\n".
                                 "[OPTION]      :\n".
                                 "    -src      :  Source Inputted File.\n".
                                 "    -tgt      :  Target Inputted File.\n".
                                 "    -out      :  Outputted Alignment File.\n".
                                 "    -tmpdir   :  Tmp Directory.                  [optional]\n".
                                 "                   Default value is \"./\".\n".
                                 "[EXAMPLE]     :\n".
                                 "       perl NiuTrans-running-GIZA++.pl  -src    FILE\n".
                                 "                                        -tgt    FILE\n".
                                 "                                        -out    FILE\n".
                                 "                                        -tmpdir DIR \n";
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

          if( !exists $param{ "-tmpdir" } )
          {
                    print STDERR "Error: please assign \"-tmpdir\"!\n";
                    exit( 1 );
          }
          else
          {
                    $param{ "-tmpdir" } =~ s/\\/\//g;
                    if( !( $param{ "-tmpdir" } =~ /.*\/$/ ) )
                    {
                             $param{ "-tmpdir" } = $param{ "-tmpdir" }."/";
                    }
          }

          if( !exists $param{ "-src" } )
          {
                    print STDERR "Error: please assign \"-src\"!\n";
                    exit( 1 );
          }
          else
          {
                    if( !( -e $param{ "-src" } ) )
                    {
                              print STDERR "$param{ \"-src\" } does not exist!\n";
                              exit( 1 );
                    }
          }
          if( !exists $param{ "-tgt" } )
          {
                    print STDERR "Error: please assign \"-tgt\"!\n";
                    exit( 1 );
          }
          else
          {
                    if( !( -e $param{ "-tgt" } ) )
                    {
                              print STDERR "$param{ \"-tgt\" } does not exist!\n";
                              exit( 1 );
                    }
          }
          if( !exists $param{ "-out" } )
          {
                    print STDERR "Error: please assign \"-out\"!\n";
                    exit( 1 );
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
