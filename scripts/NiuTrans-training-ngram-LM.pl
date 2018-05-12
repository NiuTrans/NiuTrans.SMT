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
#   version      : 1.1.0 Beta
#   Function     : Training n-gram Language-Model
#   Author       : Qiang Li
#   Email        : liqiangneu@gmail.com
#   Date         : 07/26/2011
#   Last Modified: 
#######################################

use strict;


my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "# NiuTrans training ngram LM (version 1.1.0 Beta) --www.nlplab.com #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";


print STDERR $logo;

my %param;

getParameter( @ARGV );
trainLM();


sub trainLM
{
          my $exec = "../bin/NiuTrans.LMTrainer ";

          
          #step1 mkvocab
          my $opt  = "mkvocab ".
                     "-n $param{ \"-ngram\" } ".
                     "-t $param{ \"-corpus\" } ".
                     "-o $param{ \"-vocab\" } ".
                     "-c $param{ \"-cutoff\" }";
          print STDERR "STEP1: mkvocab ...\n";
          ssystem( "$exec $opt\n" );

          #step2 cntngram
          my $opt  = "cntngram ".
                     "-n  $param{ \"-ngram\" } ".
                     "-m  $param{ \"-memsz\" } ".
                     "-v  $param{ \"-vocab\" } ".
                     "-t  $param{ \"-corpus\" } ".
                     "-o  $param{ \"-corpus\" }.step1 ";
          print STDERR "STEP2: count ngram ...\n";
          ssystem( "$exec $opt" );

          #step3 mkmodel
          my $opt  = "mkmodel ".
                     "-n  $param{ \"-ngram\" } ".
                     "-v  $param{ \"-vocab\" } ".
                     "-u  $param{ \"-corpus\" }.step1 ".
                     "-o  $param{ \"-lmbin\" } ";
          print STDERR "STEP3: make model ...\n";
          ssystem( "$exec $opt" );

          print STDERR "unlink $param{\"-corpus\"}.step1\n";
          unlink "$param{ \"-corpus\" }.step1" or die "Error: $param{ \"-corpus\" }.step1 does not exist!\n";
}


sub getParameter
{
          if( ( scalar( @_ ) < 4 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]       :\n".
                                 "       NiuTrans-training-ngram-lm.pl             [OPTIONS]\n".
                                 "[OPTION]      :\n".
                                 "    -ngram    :  Specifying the order of n-gram. [optional]\n".
                                 "                 Default is 5.\n".
                                 "    -memsz    :  Specifying the size of memory.  [optional]\n".
                                 "                 Default is 2048.\n".
                                 "    -cutoff   :  Cutoff value for unit vocab.    [optional]\n".
                                 "                 Default is 0.\n".
                                 "    -corpus   :  Input LM training corpus.\n".
                                 "    -vocab    :  Output vocab file.\n".
                                 "    -lmbin    :  Output binary LM file.\n".
                                 "[EXAMPLE]     :\n".
                                 "       perl NiuTrans-training-ngram-lm.pl -corpus CORPUS\n".
                                 "                                          -vocab  lm.vocab\n".
                                 "                                          -lmbin  lm.trie.data\n";
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

          if( !exists $param{ "-ngram" } )
          {
                    $param{ "-ngram" } = 5;
          }
          if( !exists $param{ "-memsz" } )
          {
                    $param{ "-memsz" } = 2048;
          }
          if( !exists $param{ "-cutoff" } )
          {
                    $param{ "-cutoff" } = 0;
          }

          if( !exists $param{ "-vocab" } )
          {
                    print STDERR "Error: please assign \"-vocab\"!\n";
                    exit( 1 );
          }
          if( !exists $param{ "-lmbin" } )
          {
                    print STDERR "Error: please assign \"-lmbin\"!\n";
                    exit( 1 );
          }
          if( !exists $param{ "-corpus" } )
          {
                    print STDERR "Error: please assign \"-corpus\"!\n";
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
