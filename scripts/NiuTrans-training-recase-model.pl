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
#   version      : 1.2.0
#   Function     : training recasing model
#   Author       : Qiang Li
#   Email        : liqiangneu@gmail.com
#   Date         : 01/24/2013
#   Last Modified: 
#######################################


use strict;
use Encode;
use utf8;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "# NiuTrans training recase model (version 1.2.0)  --www.nlplab.com #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;

my %param;

getParameter( @ARGV );
recase();

sub recase
{
          trainingRecaseModel();
          trainingRecaseNgramLM();
          generateConfigFile();
}

sub trainingRecaseModel
{
          print STDERR "STEP1: Training Recase Model\n";
          my $modelDir = $param{ "-modelDir" };
          if( ! ( -e $modelDir ) )
          {
                    ssystem( "mkdir -p $modelDir" );
          }
          
          open( CORPUS, "<", $param{ "-corpus"  } ) or die "Error: can not open file $param{ \"-corpus\" }.\n";
          open( KEEPCASED, ">", $param{ "-modelDir" }."keepcased.txt" ) or die "Error: can not open file $param{ \"-modelDir\" }keepcased.txt.\n";
          open( LOWERCASED, ">", $param{ "-modelDir" }."lowercased.txt" ) or die "Error: can not open file $param{ \"-modelDir\" }lowercased.txt.\n";
          open( ALIGNMENT, ">", $param{ "-modelDir" }."k2l.alignment.txt" ) or die "Error: can not open file $param{ \"-modelDir\" }k2l.alignment.txt.\n";
          
          my $lineNo = 0;
          print STDERR "Change the data format of the inputted corpus.\n";
          while( <CORPUS> )
          {
                    ++$lineNo;
                    s/[\r\n]//g;
                    s/\|//g;
                    s/ +/ /g;
                    s/^ //;
                    s/ $//;
                    next if( /^$/ );
                    
                    print KEEPCASED $_."\n";
                    print LOWERCASED lc( $_ )."\n";
                    
                    my $num = 0;
                    my $alignment = "";
                    foreach( split )
                    {
                              $alignment .= $num."-".$num." ";
                              ++$num;
                    }
                    $alignment =~ s/ +$//;
                    print ALIGNMENT $alignment."\n";
                    
                    print STDERR "\r        processed $lineNo lines." if( $lineNo % 10000 == 0 );
          }
          print STDERR "\r        processed $lineNo lines.\n";
          
          
          close( KEEPCASED );
          close( LOWERCASED );
          close( ALIGNMENT );
          close( CORPUS );
          
          my $command = "";
          $command = "../bin/NiuTrans.PhraseExtractor --LEX ".
                     "-src $param{ \"-modelDir\" }lowercased.txt ".
                     "-tgt $param{ \"-modelDir\" }keepcased.txt ".
                     "-aln $param{ \"-modelDir\" }k2l.alignment.txt ".
                     "-out $param{ \"-modelDir\" }lex ";
          ssystem( $command );
          $command = "../bin/NiuTrans.PhraseExtractor --EXTP ".
                     "-src $param{ \"-modelDir\" }lowercased.txt ".
                     "-tgt $param{ \"-modelDir\" }keepcased.txt ".
                     "-aln $param{ \"-modelDir\" }k2l.alignment.txt ".
                     "-out $param{ \"-modelDir\" }extract ".
                     "-srclen 1 -tgtlen 1";
          ssystem( $command );
          $command = "../bin/NiuTrans.PhraseExtractor --SCORE ".
                     "-tab $param{ \"-modelDir\" }extract ".
                     "-tabinv $param{ \"-modelDir\" }extract.inv ".
                     "-ls2d $param{ \"-modelDir\" }lex.s2d.sorted ".
                     "-ld2s $param{ \"-modelDir\" }lex.d2s.sorted ".
                     "-out $param{ \"-modelDir\" }recased.phrase.translation.table";
          ssystem( $command );
          
          unlink #"$param{ \"-modelDir\" }lowercased.txt",
                 "$param{ \"-modelDir\" }k2l.alignment.txt",
                 "$param{ \"-modelDir\" }lex.s2d.sorted",
                 "$param{ \"-modelDir\" }lex.d2s.sorted",
                 "$param{ \"-modelDir\" }extract",
                 "$param{ \"-modelDir\" }extract.inv",
                 "$param{ \"-modelDir\" }extract.sorted",
                 "$param{ \"-modelDir\" }extract.inv.sorted";
}

sub trainingRecaseNgramLM
{
          print STDERR "STEP2: Training Recase N-gram LM\n";
          my $command = "";
          $command = "perl NiuTrans-training-ngram-LM.pl ".
                     "-corpus $param{ \"-modelDir\" }keepcased.txt ".
                     "-vocab $param{ \"-modelDir\" }recased.lm.vocab ".
                     "-lmbin $param{ \"-modelDir\" }recased.lm.trie.data";
          ssystem( $command );
}

sub generateConfigFile
{
          print STDERR "STEP3: Generating Recase Configuration File\n";
          
          open( RECASEDNULL, ">", $param{ "-modelDir" }."recased.null" ) or die "Error: can not open file $param{ \"-modelDir\" }recased.null.\n";
          close( RECASEDNULL );

          my $command = "";
          $command = "perl NiuTrans-phrase-generate-mert-config.pl ".
                     "-tmdir $param{ \"-modelDir\" } ".
                     "-phrasetab recased.phrase.translation.table ".
                     "-metab recased.null ".
                     "-msdtab recased.null ".
                     "-lmdir $param{ \"-modelDir\" } ".
                     "-vocab recased.lm.vocab ".
                     "-lmbin recased.lm.trie.data ".
                     "-o $param{ \"-modelDir\" }recased.config.file ".
                     "-ngram $param{ \"-ngram\" } ".
                     "-nthread $param{ \"-nthread\" } ".
                     "-use-me-reorder 0 ".
                     "-use-msd-reorder 0 ".
                     "-maxdd 1 ".
                     "-lowertext 0 ".
                     "-outputoov 1";
          ssystem( $command );
}

sub getParameter
{
          if( ( scalar( @_ ) < 2 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]\n".
                                 "     NiuTrans-training-recase-model.pl                    [OPTIONS]\n".
                                 "[OPTION]\n".
                                 "      -corpus      : Inputted Cased Corpus.\n".
                                 "      -modelDir    : Outputted recaser director.\n".
                                 "      -recasemodel : Outputted Recased Model File.       [optional]\n".
                                 "                      Default value is \"recased.phrase.translation.table\".\n".
                                 "      -ngram       : N-gram Language Model.              [optional]\n".
                                 "                      Default value is 5.\n".
                                 "      -vocab       : Outputted Cased Vocab File.         [optional]\n".
                                 "                      Default value is \"recased.lm.vocab\".\n".
                                 "      -lmbin       : Outputted N-gram Cased LM.          [optional]\n".
                                 "                      Default value is \"recased.lm.trie.data\".\n".
                                 "      -config      : Outputted Config File.              [optional]\n".
                                 "                      Default value is \"recased.NiuTrans.decoder.config\".\n".
                                 "[EXAMPLE]\n".
                                 "     perl NiuTrans-training-recase-model.pl     [-corpus      FILE]\n".
                                 "                                                [-modelDir    DIR ]\n".
                                 "                                                [-recasemodel FILE]\n".
                                 "                                                [-vocab       FILE]\n".
                                 "                                                [-lmbin       FILE]\n";
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

          
          if( !exists $param{ "-modelDir" } )
          {
                    print STDERR "Error: please assign -modelDir!\n";
                    exit( 1 );
          }
          else
          {
                    $param{ "-modelDir" } =~ s/\\/\//g;
                    if( !($param{ "-modelDir" } =~ /.*\/$/) )
                    {
                              $param{ "-modelDir" } = $param{ "-modelDir" }."/";
                    }
          }

          if( !exists $param{ "-corpus" } )
          {
                    print STDERR "Error: please assign \"-corpus\"!\n";
                    exit( 1 );
          }
          elsif( !( -e $param{ "-corpus" } ) )
          {
                    print STDERR "Error: $param{ \"-corpus\" } does not exist!\n";
                    exit( 1 );
          }
                    
          if( !exists $param{ "-recasemodel" } )
          {
                    $param{ "-recasemodel" } = "recased.phrase.translation.table";
          }

          if( !exists $param{ "-ngram" } )
          {
                    $param{ "-ngram" } = 5;
          }

          if( !exists $param{ "-nthread" } )
          {
                    $param{ "-nthread" } = 5;
          }

          if( !exists $param{ "-vocab" } )
          {
                    $param{ "-vocab" } = "recased.lm.vocab";
          }

          if( !exists $param{ "-lmbin" } )
          {
                    $param{ "-lmbin" } = "recased.lm.trie.data";
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

