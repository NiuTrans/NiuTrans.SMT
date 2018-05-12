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

#############################################
#   version          : 1.0.0 Beta
#   Function         : train-model
#   Author           : Qiang Li
#   Email            : liqiangneu@gmail.com
#   Date             : 06/09/2011
#   last Modified by :
#     2011/6/26 user can select which program to execute in line 277-286 by Qiang Li
#     2011/6/26 remove MSD.s file in line 206-208 by Qiang Li
#############################################


use strict;
use File::Copy;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "#  NiuTrans Phrase Training (version 1.0.0 Beta) --www.nlplab.com  #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;
my %param;
my %option;

getParameter( @ARGV );
readConfigFile( $param{ "-c" } );

my $log;
unlink $param{ "-l" };
open( $log, ">".$param{ "-l" } ) or die "Error: can not open $param{\"-l\"} file\n";
generateLog( $log, $logo );

my $exec;
my $opt;
my $logContent;

if( ( substr $param{ "-select" }, 0, 1 ) == 1 )
{
          $exec = "../bin/NiuTrans.PhraseExtractor";
          $opt = "--LEX -src $param{ \"-s\" }".
                    " -tgt $param{ \"-t\" }".
                    " -aln $param{ \"-a\" }".
                    " -out $option{ \"Lexical-Table\" }";
          if( exists $param{ "-sort" } )
          {
                    $opt = $opt." -sort ".$param{ "-sort" };
          }
          if( exists $param{ "-temp" } )
          {
                    $opt = $opt." -temp ".$param{ "-temp" };
          }
          ssystem( $exec." ".$opt );
          $logContent = "Running: Generate Lexical Table".
                           "\n\t".$exec." --LEX".
                           "\n\t\t input:\t$param{ \"-s\" }".
                           "\n\t\t       \t$param{ \"-t\" }".
                           "\n\t\t       \t$param{ \"-a\" }".
                           "\n\t\toutput:\t$option{\"Lexical-Table\"}.s2d.sorted".
                           "\n\t\t       \t$option{\"Lexical-Table\"}.d2s.sorted".
                           "\n\t\t  time:\t";
          generateLog( $log, $logContent );

          $opt = "--EXTP -src $param{ \"-s\" }".
                 " -tgt $param{ \"-t\" }".
                 " -aln $param{ \"-a\" }".
                 " -out $option{ \"Extract-Phrase-Pairs\" }".
                 " -srclen $option{ \"Max-Source-Phrase-Size\" }".
                 " -tgtlen $option{ \"Max-Target-Phrase-Size\" }";
          ssystem( $exec." ".$opt );
          $logContent = "Running: Extract Phrase Pairs".
                        "\n\t".$exec." --EXTP".
                        "\n\t\t input:\t$param{ \"-s\" }".
                        "\n\t\t      :\t$param{ \"-t\" }".
                        "\n\t\t      :\t$param{ \"-a\" }".
                        "\n\t\toutput:\t$option{ \"Extract-Phrase-Pairs\" }".
                        "\n\t\t       \t$option{\"Extract-Phrase-Pairs\"}.inv".
                        "\n\t\tsrclen:\t$option{ \"Max-Source-Phrase-Size\" }".
                        "\n\t\ttgtlen:\t$option{ \"Max-Target-Phrase-Size\" }".
                        "\n\t\t  time:\t";
          generateLog( $log, $logContent );

          $opt = "--SCORE -tab $option{ \"Extract-Phrase-Pairs\" }".
                 " -tabinv $option{ \"Extract-Phrase-Pairs\" }.inv".
                 " -ls2d $option{ \"Lexical-Table\" }.s2d.sorted".
                 " -ld2s $option{ \"Lexical-Table\" }.d2s.sorted".
                 " -out $option{ \"Phrase-Table\" }.step1".
                 " -cutoffInit $option{ \"Phrase-Cut-Off\" }";
          if( exists $param{ "-sort" } )
          {
                    $opt = $opt." -sort ".$param{ "-sort" };
          }
          if( exists $param{ "-temp" } )
          {
                    $opt = $opt." -temp ".$param{ "-temp" };
          }
          ssystem( $exec." ".$opt );
          $logContent = "Running: Generate Phrase Translation Table".
                        "\n\t".$exec." --SCORE".
                        "\n\t\t input:\t$option{ \"Extract-Phrase-Pairs\" }".
                        "\n\t\t       \t$option{ \"Extract-Phrase-Pairs\" }.inv".
                        "\n\t\t       \t$option{ \"Lexical-Table\" }.s2d.sorted".
                        "\n\t\t       \t$option{ \"Lexical-Table\" }.d2s.sorted".
                        "\n\t\toutput:\t$option{ \"Phrase-Table\" }.step1".
                        "\n\t\tcutoff:\t$option{ \"Phrase-Cut-Off\" }".
                        "\n\t\t  time:\t";
          generateLog( $log, $logContent );

          $opt = "--FILTN -in $option{ \"Phrase-Table\" }.step1 -out $option{ \"Phrase-Table\" } -strict $param{ \"-f\" }";
          ssystem( $exec." ".$opt );
          $logContent = "Running: Filter phrase translation table.".
                        "\n\t".$exec." --FILTN".
                        "\n\t\t input:\t$option{ \"Phrase-Table\" }.step1".
                        "\n\t\toutput:\t$option{ \"Phrase-Table\" }".
                        "\n\t\tstrict:\t$param{ \"-f\" }".
                        "\n\t\t  time:\t";
          generateLog( $log, $logContent );

          unlink "$option{ \"Lexical-Table\"}.s2d.sorted",
                 "$option{ \"Lexical-Table\"}.d2s.sorted",
                 "$option{ \"Extract-Phrase-Pairs\" }",
                 "$option{ \"Extract-Phrase-Pairs\" }.sorted",
                 "$option{ \"Extract-Phrase-Pairs\" }.inv",
                 "$option{ \"Extract-Phrase-Pairs\" }.inv.sorted",
                 "$option{ \"Phrase-Table\" }.step1.half.sorted",
                 "$option{ \"Phrase-Table\" }.step1.half.inv.sorted",
                 "$option{ \"Phrase-Table\" }.step1.inv",
                 "$option{ \"Phrase-Table\" }.step1";
}

if( ( substr $param{ "-select" }, 1, 1 ) == 1 )
{
          $exec = "../bin/NiuTrans.MEReorder";

#          $option{ "ME-max-src-phrase-len" } = 3 if $option{ "ME-max-src-phrase-len" } <= 0;
#          $option{ "ME-max-tar-phrase-len" } = 5 if $option{ "ME-max-tar-phrase-len" } <= 0;

          $opt = "-src $param{ \"-s\" }".
                 " -tgt $param{ \"-t\" }".
                 " -algn $param{ \"-a\" }".
                 " -output $option{ \"ME-Reordering-Table\" }.step1";
 
          $opt = $opt." -maxSrcPhrWdNum $option{ \"ME-max-src-phrase-len\" }" if( $option{ "ME-max-src-phrase-len" } != 0 && $option{ "ME-max-src-phrase-len" } != -1 );
          $opt = $opt." -maxTgtPhrWdNum $option{ \"ME-max-tar-phrase-len\" }" if( $option{ "ME-max-tar-phrase-len" } != 0 && $option{ "ME-max-tar-phrase-len" } != -1 );
          $opt = $opt." -maxTgtGapWdNum $option{ \"ME-null-algn-word-num\" }" if( $option{ "ME-null-algn-word-num" } != -1 );
          $opt = $opt." -maxSampleNum $option{ \"ME-max-sample-num\" }" if( $option{ "ME-max-sample-num" } != -1 );
          print STDERR "\n";
          
          if( ( $option{ "ME-use-src-parse-pruning" } == 1 ) && ( -e $option{ "ME-src-parse-path" } ) )
          {
                    $opt = $opt." -srcParsePruning";
                    $opt = $opt." -srcParse $option{ \"ME-src-parse-path\" }";
          }
          
          ssystem( $exec." ".$opt );

          $exec = "../bin/maxent";
          $opt = "-i 200".
                 " -g 1".
                 " -m $option{ \"ME-Reordering-Table\" }.step2".
                 " $option{ \"ME-Reordering-Table\" }.step1".
                 " --lbfgs";
          print STDERR "\nStart training ME reordering model by maxent classfier...\nThis will take a while, please be patient...\n";  
          ssystem( $exec." ".$opt );

          $exec = "perl ../scripts/dm-conv.pl";
          $opt = "$option{ \"ME-Reordering-Table\" }.step2".
                 " > ".
                 "$option{ \"ME-Reordering-Table\" }";
          print STDERR "\n";
          ssystem( $exec." ".$opt );

          $logContent = "\nRunning: Generate ME Reordering Table".
                        "\n\t../bin/NiuTrans.MEReorder".
                        "\n\t../bin/maxent".
                        "\n\tperl ../scripts/dm-conv.pl".
                        "\n\t\t input:\t$param{ \"-s\" }".
                        "\n\t\t       \t$param{ \"-t\" }".
                        "\n\t\t       \t$param{ \"-a\" }".
                        "\n\t\toutput:\t$option{ \"ME-Reordering-Table\" }".
                        "\n\t\t    sz:\t$option{ \"Max-Source-Phrase-Size\" }".
                        "\n\t\t    dz:\t$option{ \"Max-Target-Phrase-Size\" }".
                        "\n\t\t GapWd:\t1".
                        "\n\t\t  time:\t";
          generateLog( $log, $logContent );

          unlink "$option{ \"ME-Reordering-Table\" }.step1",
                 "$option{ \"ME-Reordering-Table\" }.step2";
}

if( ( substr $param{ "-select" }, 2, 1 ) == 1 )
{
          $exec = "../bin/NiuTrans.MSDReorder";
          $opt = "-f $param{ \"-s\" }".
                 " -e $param{ \"-t\" }".
                 " -a $param{ \"-a\" }".
                 " -m $option{ \"MSD-model-type\" }".
                 " -o $option{ \"MSD-Reordering-Model\" }.step1";
          print STDERR "\n\n";
          ssystem( $exec." ".$opt );

          if( !( -e $option{ "Phrase-Table" } ) )
          {
                  print STDERR "\n$option{ \"Phrase-Table\" } does not exist!\nThe msd reordering table can not be filtered!\n";
                  rename "$option{ \"MSD-Reordering-Model\" }.step1",$option{ "MSD-Reordering-Model" };
                  exit( 1 );
          }
          $exec = "perl ../scripts/filter.msd.model.pl";
          $opt = " $option{ \"Phrase-Table\" }".
                 " $option{ \"MSD-Reordering-Model\" }.step1".
                 " > $option{ \"MSD-Reordering-Model\" }";
          print STDERR "\n";
          ssystem( $exec." ".$opt );
          $logContent = "Running: Generate MSD Reordering Model".
                        "\n\t../bin/NiuTrans.MSDReorder".
                        "\n\tperl ../scripts/filter.msd.model.pl".
                        "\n\t\t input:\t$param{ \"-s\" }".
                        "\n\t\t       \t$param{ \"-t\" }".
                        "\n\t\t       \t$param{ \"-a\" }".
                        "\n\t\t       \t$option{ \"Phrase-Table\"}".
                        "\n\t\toutput:\t$option{ \"MSD-Reordering-Model\" }".
                        "\n\t\t  time:\t";
          generateLog( $log, $logContent );

          unlink "$option{ \"MSD-Reordering-Model\" }.step1"; 
          unlink glob "smt_tmp_phrase_table.*";
}

#my $msdReorderingModelS;
#open ( $msdReorderingModelS, ">".$option{ "MSD-Reordering-Model" }.".s" ) or die "Error: can not open file $option{ \"MSD-Reordering-Model\" }"; 
#close( $msdReorderingModelS );

close( $log );  

#my $key;
#my $value;
#while( ( $key, $value ) = each %option )
#{
#	print "$key => $value\n";
#}
#while( ( $key, $value ) = each %param )
#{
#	print "$key => $value\n";
#}

sub getParameter
{
          if( ( scalar( @_ ) < 8 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]     :\n".
                                 "        NiuTrans-phrase-train-model.pl   [OPTIONS]\n".
                                 "[OPTION]    :\n".
                                 "    -tmdir  :  Working Dir.\n".
                                 "    -s      :  Source Language File.\n".
                                 "    -t      :  Target Language File.\n".
                                 "    -a      :  Aligned File.\n".
                                 "    -select :  Select program (3 bits).  [optional]\n".
                                 "                   Default the value is \"111\".\n".
                                 "                   first bit represent Extractor.\n".
                                 "                   second bit represent MEReorder.\n".
                                 "                   third bit represent MSDReorder.\n".
                                 "    -sort   :  Cygwin Sort Path.         [optional]\n".
                                 "                   If you run the program in linux, ignore it!\n".
                                 "                   Default path is \"c:/cygwin/bin/sort.exe\".\n".
                                 "    -temp   :  Sort Temp Dir.            [optional]\n".
                                 "                   Default is sort temp dir.\n".
                                 "    -c      :  Config File.              [optional]\n".
                                 "                   Default copy our config-file to your working dir.\n".
                                 "    -f      :  filter number.            [optional]\n".
                                 "                   Default 30\n".
                                 "    -l      :  Log.                      [optional]\n".
                                 "                   Default file \"train-model.log\" on working dir.\n".
                                 "[EXAMPLE]   :\n".
                                 "    perl NiuTrans-phrase-train-model.pl -tmdir working-dir -s sf -t tf -a af -select 111\n";
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
          if( !exists $param{ "-tmdir" } )
          {
                    print STDERR "Error: please assign -tmdir!\n";
                    exit( 1 );
          }
          else
          {
                    $param{ "-tmdir" } =~ s/\\/\//g;
                    if( !($param{ "-tmdir" } =~ /.*\/$/) )
                    {
                              $param{ "-tmdir" } = $param{ "-tmdir" }."/";
                    }
          }
          if( exists $param{ "-temp" } )
          {
                    $param{ "-temp" } =~ s/\\/\//g;
                    if( $param{ "-temp" } =~ /(.*)\/$/ )
                    {
                              $param{ "-temp" } = $1;
                    }
                    if( !( -e $param{ "-temp" } ) )
                    {
                              print STDERR "Error: The temp dir does not exist!\n";
                              exit( 1 );
                    }
          }
          if( !exists $param{ "-select" } )
          {
                    $param{ "-select" } = "111";
          }
          elsif( length( $param{ "-select" } ) != 3 )
          {
#                   print STDERR length( $param{ "-select" } )."\n";
                    print STDERR "Error: the \"-select\" parameter must be 3 bits.\n";
                    exit( 1 );
          }
          if( !exists $param{ "-f" } )
          {
                    $param{ "-f" } = 30;
          }
          if( !exists $param{ "-s" } || !exists $param{ "-t" } || !exists $param{ "-a" } )
          {
                    print STDERR "Error: -s -t -a must be assigned!\n";
                    exit( 1 );
          }
          else
          {
                    if( !( -e $param{ "-s" } ) || !( -e $param{ "-t" } ) || !( -e $param{ "-a" } ) )
                    {
                              print STDERR "$param{ \"-s\" }, $param{ \"-t\" } or $param{ \"-a\" } does not exist!\n";
                              exit( 1 );
                    }
                    $param{ "-s" } =~ s/\\/\//g;
                    $param{ "-t" } =~ s/\\/\//g;
                    $param{ "-a" } =~ s/\\/\//g;
          }
          if ( !exists $param{ "-l" } )
          {
                    print STDERR $param{ "-tmdir" };
                    $param{ "-l" } = $param{ "-tmdir" }."train-model.log";
          }
          if( !exists $param{ "-c" } )
          {
                    copy( "../config/NiuTrans.phrase.train.model.config", $param{ "-tmdir"}."NiuTrans.phrase.train.model.config" ) or die "Copy failed: $!";;
                    $param{ "-c" } = $param{ "-tmdir" }."NiuTrans.phrase.train.model.config";
                    #exit( 1 );
          }
}


sub readConfigFile
{
          print STDERR "Error: Config file does not exist!\n" if( scalar( @_ ) != 1 );
          $_[0] =~ s/\\/\//g;
          open( CONFIGFILE, "<".$_[0] ) or die "\nError: Can not read file $_[0] \n";
          print STDERR "Read $param{ \"-c\" } ";
          my $configFlag = 0;
          my $appFlag = 0;
          my $lineNo = 0;
          while( <CONFIGFILE> )
          {
                    s/[\r\n]//g;
                    next if /^( |\t)*$/;
                    if( /param(?: |\t)*=(?: |\t)*"([\w\-]*)"(?: |\t)*value="([\w\/\-. :]*)"(?: |\t)*/ )
                    {
                             ++$lineNo;
#                             if( ( $1 ne "Max-Source-Phrase-Size" ) && ( $1 ne "Max-Target-Phrase-Size" ) && ( $1 ne "Phrase-Cut-Off" ) )
                             if( ( $1 eq "Lexical-Table" ) || ( $1 eq "Extract-Phrase-Pairs" ) || ( $1 eq "Phrase-Table" )
                              || ( $1 eq "ME-Reordering-Table" ) || ( $1 eq "MSD-Reordering-Model" ) )
                             {
                                       $option{ $1 } = "$param{ \"-tmdir\" }"."$2";
                             }
                             else
                             {
                                       $option{ $1 } = $2;
                             }
                             print STDERR ".";
                    }
          }
          close( CONFIGFILE ); 

          if( !exists $option{ "Lexical-Table" } || !exists $option{ "MSD-Reordering-Model" } ||
              !exists $option{ "Extract-Phrase-Pairs" } || !exists $option{ "Max-Source-Phrase-Size" } ||
              !exists $option{ "Max-Target-Phrase-Size" } || !exists $option{ "Phrase-Table" } ||
              !exists $option{ "Phrase-Cut-Off" } || !exists $option{ "ME-Reordering-Table" } )
          {
                    print STDERR " Error.\n\n".
                                 "Please validate your config file!\n".
                                 "You must have the parameter in $param{\"-c\"} below:\n".
                                 "\tLexical-Table\n".
                                 "\tExtract-Phrase-Pairs\n".
                                 "\tMax-Source-Phrase-Size\n".
                                 "\tMax-Target-Phrase-Size\n".
                                 "\tPhrase-Table\n".
                                 "\tPhrase-Cut-Off\n".
                                 "\tME-Reordering-Table\n".
                                 "\tMSD-Reordering-Model\n";
                    exit( 1 );
          }
          print STDERR " Over.\n\n";
}


sub generateLog
{
          if( scalar( @_ ) != 2 )
          {
                    print STDERR "Error: please check your parameter in generateLog!\n"; 
                    exit( 1 ); 
          }
          my $log = $_[0];
          my $time = localtime( time );
          print $log $_[1]."".$time."\n\n";  
         
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

