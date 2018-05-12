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
#   version      : 1.0
#   Function     : experiment-model
#   Author       : Qiang Li
#   Email        : liqiangneu@gmail.com
#   Date         : 06/09/2011
#   Last Modified: 
#        2011/7/1  does not generate mert config in this perl
#        2011/6/25 export nref in line 336 by Qiang Li.
#######################################


use strict;
use File::Copy;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "#  NiuTrans Hierarchy MERT (version 1.0.0 Beta)  --www.nlplab.com  #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;

my %param;
my %option;

getParameter( @ARGV );

#my $key;
#my $value;
#while( ( $key, $value ) = each %option )
#{
	#print STDERR "$key => $value\n";
#}
#while( ( $key, $value ) = each %param )
#{
#	print STDERR "$key => $value\n";
#}

my $log;
unlink $log;
open( $log, ">".$param{ "-log" } ) or die "Error: can not open file $log!\n";
print $log $logo;

experiment_avgWeight( $log ) if $param{ "-method" } eq "avg";
experiment_maxWeight( $log ) if $param{ "-method" } eq "max";

close( $log );

sub experiment_maxWeight
{
          if( scalar( @_ ) != 1 )
          {
                    print STDERR "Error: experiment parameter is wrong!\n";
                    exit( 1 );
          }

          my $log = shift @_;

          my $round = 0;
          my @DevMaxBleu;
          while( $round != $param{ "-round" } )
          {
                    ++$round;
#                    my $exec = "../bin/NiuTrans.Decoder -mert $param{ \"-dev\" } -config $param{ \"-c\" } 1 $param{ \"-nref\" } $param{ \"-ngram\" } $param{ \"-nthread\" }";
                    my $exec = "../bin/NiuTrans.Decoder -mert $param{ \"-dev\" } 1 -config $param{ \"-config\" } -scfg";
                    $exec = $exec." -nref $param{ \"-nref\" }" if exists $param{ "-nref" };
                    $exec = $exec." -ngram $param{ \"-ngram\" }" if exists $param{ "-ngram" };
                    $exec = $exec." -nthread $param{ \"-nthread\" }" if exists $param{ "-nthread" };
                    $exec = $exec." -nround $param{ \"-nround\" }" if exists $param{ "-nround" };

                    ssystem( $exec );
                    my $time = localtime( time );
                    print $log "Dev set experiment\tRound:$round\t$time\n";
                    my $paramOfMaxBleu = getParamOfMaxBleu( "MERT.xt.log.txt", $log, $round );
                    push @DevMaxBleu, $paramOfMaxBleu;
          }
          my $maxBleu = 0;
          my $paramOfMaxBleu;
          foreach( @DevMaxBleu )
          {
                    if( /(.*)\t(.*)/ )
                    {
                              if( $1 >= $maxBleu )
                              {
                                        $maxBleu = $1;
                                        $paramOfMaxBleu = $2;
                              }
                              print "\nBleu:$1\tparamOfThisBleu:$2\n";
                    }
          }
          print $log "\t\tBleu:$maxBleu\tparamOfMaxBleu:$paramOfMaxBleu\n";
          generateTestConfigFile( $paramOfMaxBleu );

          unlink "decoding.log.txt",
                 "lm.log.txt",
                 "MERT.xt.log.txt",
                 "mert.xt.result.txt",
                 "MERT.zh.log.txt",
                 "nbest.final.mert.txt",
                 "pattern.log",
                 "weights.txt";
}

sub experiment_avgWeight
{
          if( scalar( @_ ) != 1 )
          {
                    print STDERR "Error: experiment parameter is wrong!\n";
                    exit( 1 );
          }

          my $log = shift @_;

          my $round = 0;
          my @DevMaxBleu;
          while( $round != $param{ "-round" } )
          {
                    ++$round;
#                    my $exec = "../bin/NiuTrans.Decoder -mert $param{ \"-dev\" } -config $param{ \"-c\" } 1 $param{ \"-nref\" } $param{ \"-ngram\" } $param{ \"-nthread\" }";
                    my $exec = "../bin/NiuTrans.Decoder -mert $param{ \"-dev\" } 1 -config $param{ \"-config\" } -scfg";
                    $exec = $exec." -nref $param{ \"-nref\" }" if exists $param{ "-nref" };
                    $exec = $exec." -ngram $param{ \"-ngram\" }" if exists $param{ "-ngram" };
                    $exec = $exec." -nthread $param{ \"-nthread\" }" if exists $param{ "-nthread" };
                    $exec = $exec." -nround $param{ \"-nround\" }" if exists $param{ "-nround" };

                    ssystem( $exec );
                    my $time = localtime( time );
                    print $log "Dev set experiment\tRound:$round\t$time\n";
                    my $paramOfMaxBleu = getParamOfMaxBleu( "MERT.xt.log.txt", $log, $round );
                    push @DevMaxBleu, $2 if $paramOfMaxBleu =~ /(.*)\t(.*)/;
          }
          
          my @paramAverage;
          my $paramAverageStr;
          
          foreach( @DevMaxBleu )
          {
                    my @tempParam = split " ", $_;
                    print STDERR scalar( @tempParam )."\n";
                    my $pos;
                    for( $pos = 0; $pos < scalar( @tempParam); ++$pos )
                    {
                              $paramAverage[ $pos ] += $tempParam[ $pos ];
                    }
                    print STDERR $_."\n";
          }
          
          my $pos;
          for( $pos = 0; $pos < scalar( @paramAverage ); ++$pos )
          {
                    $paramAverage[$pos] /= scalar( @DevMaxBleu );
                    $paramAverageStr = $paramAverageStr.$paramAverage[$pos]." ";
          }

          $paramAverageStr = $1 if $paramAverageStr =~ /(.*) $/;
          generateTestConfigFile( $paramAverageStr );
          print $log "$paramAverageStr\n\n";          
          
          unlink "decoding.log.txt",
                 "lm.log.txt",
                 "MERT.xt.log.txt",
                 "mert.xt.result.txt",
                 "MERT.zh.log.txt",
                 "nbest.final.mert.txt",
                 "pattern.log",
                 "weights.txt";
}


sub getParameter
{
          if( ( scalar( @_ ) < 2 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]      :\n".
                                 "    NiuTrans-hierarchy-mert-model.pl                [OPTIONS]\n".
                                 "[OPTION]     :\n".
                                 "    -config  :  Mert Config File.\n".
                                 "    -dev     :  Development data set.               [optional]\n".
                                 "                    Default is Niu.dev.txt set.\n".
                                 "    -round   :  Round.                              [optional]\n".
                                 "                    Default round is 2.\n".
                                 "    -method  :  \"avg\" or \"max\".                     [optional]\n".
                                 "                    Default \"avg\".\n".
                                 "    -nref    :  Dev Set reference number.           [optional]\n".
                                 "                    Default value is in your config file!\n".
                                 "    -ngram   :  Parameter of N-gram LM.             [optional]\n".
                                 "                    Default value is in your config file!.\n".
                                 "    -nthread :  Number of threads.                  [optional]\n".
                                 "                    Default value is in your config file!.\n".
                                 "    -log     :  Log.                                [optional]\n".
                                 "                    Default file \"mert-model.log\" on current dir.\n".
                                 "[EXAMPLE]    :\n".
                                 "    perl NiuTrans-hierarchy-mert-model.pl -config CONFIG-FILE\n".
                                 "                                          -dev    DEV-SET\n";

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
          
          if( !exists $param{ "-config" } )
          {
                    print STDERR "Error: please assign \"-config\"!\n";
                    exit( 1 );
          }

          if( !exists $param{ "-log" } )
          {
                    $param{ "-log" } = "mert-model.log";
          }

          $param{ "-round" } = 2 if( !exists $param{ "-round" } );

          if( !exists $param{ "-dev" } )
          {
                    $param{ "-dev" } = "../sample-data/sample-submission-version/Dev-set/Niu.dev.txt";
                    if( !( -e $param{ "-dev" } ) )
                    {
                              print STDERR "$param{ \"-dev\"} does not exist!\n";
                              exit( 1 );
                    }
          }

#          if( !exists $param{ "-nref" } )
#          {
#                    $param{ "-nref" } = " ";
#          }
#          else
#          {
#                    $param{ "-nref" } = " -nref $param{ \"-nref\" } ";
#          }
          
#          if( !exists $param{ "-ngram" } )
#          {
#                    $param{ "-ngram" } = " ";
#          }
#          else
#          {
#                    $param{ "-ngram" } = " -ngram $param{ \"-ngram\" } ";
#          }
#          if( !exists $param{ "-nthread" } )
#          {         
#                    $param{ "-nthread" } = " ";
#          }
#          else
#          {
#                    $param{ "-nthread" } = " -nthread $param{ \"-nthread\" } ";
#          }
          
          $param{ "-method" } = "avg" if( !exists $param{ "-method" } );
}

sub generateTestConfigFile
{
          if( scalar( @_ ) != 1 )
          {
                    print "Error: generateTestConfigFile parameter is wrong!\n";
                    exit( 1 );
          }
          my $param = shift @_;
          
          my $testFile;
          open( $testFile, ">>".$param{ "-config"} );

          print $testFile "# optimized weights tuned by MERT training\n";
#                          "param=\"nbest\"                        value=\"1\"\n".
#                          "param=\"outputnull\"                   value=\"1\"\n";
                              
          if( $param ne "" )
          {
                          print $testFile "param=\"weights\"                      value=\"$param\"\n";
          }
          close( $testFile );
}


######################################################
# get parameter of maxBleu from MERT.xt.log.txt file
######################################################
sub getParamOfMaxBleu
{
          print "ERROR: NO PARAM OF getParamOfMaxBleu.\n", return if( scalar( @_ ) != 3 );

          my $file = $_[0];
          my $log = $_[1];
          my $round = $_[2];
          my $time;
          open( MERTFILE, "<".$file ) or die "ERROR: Can't read $file";
          my $maxBleu = 0;
          my $maxBleuFlag = 0;
          my $paramOfMaxBleu = "";
          while( <MERTFILE> )
          {
                    s/[\r\n]//g;
                    next if /^$/;
                    $time = localtime( time );
          
                    if( /(.*) BLEU(?: *\d* *)= (.*)/ )
                    {
                              print $log "\t".$1."\n\t\tBLEU:$2\t$time\n";
                    }
                    else
                    {
                              print $log "\t\tPARAM:".$1."\n" if /<(.*)>/;
                    }
                    $maxBleuFlag = 1, $maxBleu = $1 if /BLEU(?: *\d* *)= +(.+)$/ && $1 >= $maxBleu;
                    $paramOfMaxBleu = $1, $maxBleuFlag = 0 if /^<(.*)>$/ && $maxBleuFlag;
          }
          $time = localtime( time );
          print $log "\tParameters of MAX-BLEU from the MERT LOG:\n\t\tMAX-BLEU:$maxBleu\t$time\n\t\tparameter:$paramOfMaxBleu\n\n";
          close( MERTFILE );
          open( MERTFILE, ">".$file );
          close( MERTFILE );
          $paramOfMaxBleu = "$maxBleu\t$paramOfMaxBleu";
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
