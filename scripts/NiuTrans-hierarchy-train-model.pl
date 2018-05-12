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
#   Function         : hierarchy-train-model
#   Author           : Qiang Li
#   Email            : liqiangneu@gmail.com
#   Date             : 2012-01-01
#   last Modified by : 2012-04-17
#     
#############################################


use strict;
use File::Copy;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "# NiuTrans Hierarchy Training(version 1.0.0 Beta) --www.nlplab.com #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;
my %param;
my %option;

getParameter( @ARGV );
readConfigFile( $param{ "-config" } );

my $log;
unlink $param{ "-log" };
open( $log, ">".$param{ "-log" } ) or die "Error: can not open $param{\"-log\"} file\n";
generateLog( $log, $logo );

my $exec;
my $opt;
my $logContent;

extractHieroRule();
close( $log );  


sub extractHieroRule
{
          $exec = "../bin/NiuTrans.PhraseExtractor";
          $opt = "--LEX ".
                    " -src $param{ \"-src\" }".
                    " -tgt $param{ \"-tgt\" }".
                    " -aln $param{ \"-aln\" }".
#                   " -out $option{ \"Lexical-Table\" }";
                    " -out $param{ \"-out\" }.lex";
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
                           "\n\t\t input:\t$param{ \"-src\" }".
                           "\n\t\t       \t$param{ \"-tgt\" }".
                           "\n\t\t       \t$param{ \"-aln\" }".
                           "\n\t\toutput:\t$param{ \"-out\"}.lex.s2d.sorted".
                           "\n\t\t       \t$param{ \"-out\"}.lex.d2s.sorted".
                           "\n\t\t  time:\t";
          generateLog( $log, $logContent );

          $opt = "--EXTH -src $param{ \"-src\" }".
                 " -tgt $param{ \"-tgt\" }".
                 " -aln $param{ \"-aln\" }".
                 " -out $param{ \"-out\" }.step1";
 
          if( exists $option{ "srclen" } && $option{ "srclen" } != 10 )
          {
                 $opt = $opt." -srclen ".$option{ "srclen" };
          }
          if( exists $option{ "tgtlen" } && $option{ "tgtlen" } != 10 )
          {
                 $opt = $opt." -tgtlen ".$option{ "tgtlen" };
          }
          if( exists $option{ "maxSrcI" } && $option{ "maxSrcI" } != 3 )
          {
                 $opt = $opt." -maxSrcI ".$option{ "maxSrcI" };
          }
          if( exists $option{ "maxTgtI" } && $option{ "maxTgtI" } != 5 )
          {
                 $opt = $opt." -maxTgtI ".$option{ "maxTgtI" };
          }
          if( exists $option{ "maxSrcH" } && $option{ "maxSrcH" } != 5 )
          {
                 $opt = $opt." -maxSrcH ".$option{ "maxSrcH" };
          }
          if( exists $option{ "maxTgtH" } && $option{ "maxTgtH" } != 10 )
          {
                 $opt = $opt." -maxTgtH ".$option{ "maxTgtH" };
          }
          if( exists $option{ "minSrcSubPhrase" } && $option{ "minSrcSubPhrase" } != 1 )
          {
                 $opt = $opt." -minSrcSubPhrase ".$option{ "minSrcSubPhrase" };
          }
          if( exists $option{ "minTgtSubPhrase" } && $option{ "minTgtSubPhrase" } != 1 )
          {
                 $opt = $opt." -minTgtSubPhrase ".$option{ "minTgtSubPhrase" };
          }
          if( exists $option{ "minLexiNum" } && $option{ "minLexiNum" } != 1 )
          {
                 $opt = $opt." -minLexiNum ".$option{ "minLexiNum" };
          }
          if( exists $option{ "maxnonterm" } && $option{ "maxnonterm" } != 2 )
          {
                 $opt = $opt." -maxnonterm ".$option{ "maxnonterm" };
          }
          
          if( exists $option{ "alignedLexiReq" } && $option{ "alignedLexiReq" } != 1 )
          {
                 $opt = $opt." -alignedLexiReq ".$option{ "alignedLexiReq" };
          }
          if( exists $option{ "srcNonTermAdjacent" } && $option{ "srcNonTermAdjacent" } != 0 )
          {
                 $opt = $opt." -srcNonTermAdjacent ".$option{ "srcNonTermAdjacent" };
          }
          if( exists $option{ "tgtNonTermAdjacent" } && $option{ "tgtNonTermAdjacent" } != 1 )
          {
                 $opt = $opt." -tgtNonTermAdjacent ".$option{ "tgtNonTermAdjacent" };
          }
          if( exists $option{ "duplicateHieroRule" } && $option{ "duplicateHieroRule" } != 0 )
          {
                 $opt = $opt." -duplicateHieroRule ".$option{ "duplicateHieroRule" };
          }
          if( exists $option{ "unalignedEdgeInit" } && $option{ "unalignedEdgeInit" } != 1 )
          {
                 $opt = $opt." -unalignedEdgeInit ".$option{ "unalignedEdgeInit" };
          }
          elsif( exists $option{ "unalignedEdgeInit" } && $option{ "unalignedEdgeInit" } == 1 )
          {
                 if( exists $option{ "maxNulExtSrcInitNum" } && $option{ "maxNulExtSrcInitNum" } >= 0 )
                 {
                         $opt = $opt." -maxNulExtSrcInitNum ".$option{ "maxNulExtSrcInitNum" };
                 }
                 if( exists $option{ "maxNulExtTgtInitNum" } && $option{ "maxNulExtTgtInitNum" } >= 0 )
                 {
                         $opt = $opt." -maxNulExtTgtInitNum ".$option{ "maxNulExtTgtInitNum" };
                 }
          }
          if( exists $option{ "unalignedEdgeHiero" } && $option{ "unalignedEdgeHiero" } != 0 )
          {
                 $opt = $opt." -unalignedEdgeHiero ".$option{ "unalignedEdgeHiero" };
                 
                 if( exists $option{ "maxNulExtSrcHieroNum" } && $option{ "maxNulExtSrcHieroNum" } >= 0 )
                 {
                         $opt = $opt." -maxNulExtSrcHieroNum ".$option{ "maxNulExtSrcHieroNum" };  
                 }
                 if( exists $option{ "maxNulExtTgtHieroNum" } && $option{ "maxNulExtTgtHieroNum" } >= 0 )
                 {
                         $opt = $opt." -maxNulExtTgtHieroNum ".$option{ "maxNulExtTgtHieroNum" };
                 }
                 
          }
          if( exists $option{ "headnonterm" } && $option{ "headnonterm" } != 1 )
          {
                 $opt = $opt." -headnonterm ".$option{ "headnonterm" };
          }
          if( exists $option{ "null" } && $option{ "null" } != 1 )
          {
                 $opt = $opt." -null ".$option{ "null" };
          }

          ssystem( $exec." ".$opt );
          $logContent = "Running: Extract Hiero Rule".
                        "\n\t".$exec." --EXTP".
                        "\n\t\t input:\t$param{ \"-src\" }".
                        "\n\t\t      :\t$param{ \"-tgt\" }".
                        "\n\t\t      :\t$param{ \"-aln\" }".
                        "\n\t\toutput:\t$param{ \"-out\" }.step1".
                        "\n\t\t       \t$param{ \"-out\"}.step1.inv".
                        "\n\t\t  time:\t";
          generateLog( $log, $logContent );

          $opt = "--SCORE -tab $param{ \"-out\" }.step1".
                 " -tabinv $param{ \"-out\" }.step1.inv".
                 " -ls2d $param{ \"-out\" }.lex.s2d.sorted".
                 " -ld2s $param{ \"-out\" }.lex.d2s.sorted".
#                " -out $option{ \"Hiero-Rule-Table\" }.step1".
                 " -out $param{ \"-out\" }.step2".
                 " -cutoffInit $option{ \"cutoffInit\" }".
                 " -cutoffHiero $option{ \"cutoffHiero\" }";
          if( exists $param{ "-sort" } )
          {
                    $opt = $opt." -sort ".$param{ "-sort" };
          }
          if( exists $param{ "-temp" } )
          {
                    $opt = $opt." -temp ".$param{ "-temp" };
          }
          
          if( exists $option{ "printFreq" } && $option{ "printFreq" } == 1 )
          {
                    $opt = $opt." -printFreq ".$option{ "printFreq" };
          }
          if( exists $option{ "printAlign" } && $option{ "printAlign" } == 1 )
          {
                    $opt = $opt." -printAlign ".$option{ "printAlign" };
          }

          ssystem( $exec." ".$opt );
          $logContent = "Running: Generate Hiero Rule Table".
                        "\n\t".$exec." --SCORE".
                        "\n\t\t      input:\t$param{ \"-out\" }.step1".
                        "\n\t\t            \t$param{ \"-out\" }.step1.inv".
                        "\n\t\t            \t$param{ \"-out\" }.lex.s2d.sorted".
                        "\n\t\t            \t$param{ \"-out\" }.lex.d2s.sorted".
#                       "\n\t\toutput:\t$option{ \"Hiero-Rule-Table\" }.step1".
                        "\n\t\t     output:\t$param{ \"-out\" }.step2".
                        "\n\t\t cutoffInit:\t$option{ \"cutoffInit\" }".
                        "\n\t\tcutoffHiero:\t$option{ \"cutoffHiero\" }".
                        "\n\t\t  time:\t";
          generateLog( $log, $logContent );

#         $opt = "--FILTN -in $option{ \"Hiero-Rule-Table\" }.step1 -out $option{ \"Hiero-Rule-Table\" } -strict $param{ \"-f\" }";
          $opt = "--FILTN -in $param{ \"-out\" }.step2 -out $param{ \"-out\" } -strict $param{ \"-filter\" }";
          ssystem( $exec." ".$opt );
          $logContent = "Running: Filter Hiero Rule Table.".
                        "\n\t".$exec." --FILTN".
                        "\n\t\t input:\t$param{ \"-out\" }.step2".
                        "\n\t\toutput:\t$param{ \"-out\" }".
                        "\n\t\tstrict:\t$param{ \"-filter\" }".
                        "\n\t\t  time:\t";
          generateLog( $log, $logContent );

          unlink "$param{ \"-out\"}.lex.s2d.sorted",
                 "$param{ \"-out\"}.lex.d2s.sorted",
                 "$param{ \"-out\" }.step1",
                 "$param{ \"-out\" }.step1.sorted",
                 "$param{ \"-out\" }.step1.inv",
                 "$param{ \"-out\" }.step1.inv.sorted",
                 "$param{ \"-out\" }.step2.half.sorted",
                 "$param{ \"-out\" }.step2.half.inv.sorted",
                 "$param{ \"-out\" }.step2.inv",
                 "$param{ \"-out\" }.step2";
}

sub getParameter
{
          if( ( scalar( @_ ) < 8 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]     :\n".
                                 "    NiuTrans-hierarchy-train-model.pl    [OPTIONS]\n".
                                 "[OPTION]    :\n".
#                                "    -tmdir  :  Working Dir.\n".
                                 "    -src    :  Source Language File.\n".
                                 "    -tgt    :  Target Language File.\n".
                                 "    -aln    :  Aligned File.\n".
                                 "    -out    :  Output Hierarchy Rule.\n".
                                 "    -sort   :  Cygwin Sort Path.         [optional]\n".
                                 "                   If you run the program in linux, ignore it!\n".
                                 "                   Default path is \"c:/cygwin/bin/sort.exe\".\n".
                                 "    -temp   :  Sort Temp Dir.            [optional]\n".
                                 "                   Default is sort temp dir.\n".
                                 "    -config :  Config File.              [optional]\n".
                                 "                   Default copy our config-file to current dir.\n".
                                 "    -filter :  filter number.            [optional]\n".
                                 "                   Default 30\n".
                                 "    -log    :  Log.                      [optional]\n".
                                 "                   Default file \"train-model.log\" on current dir.\n".
                                 "[EXAMPLE]   :\n".
 #                               "    perl NiuTrans-hierarchy-train-model.pl -tmdir working-dir\n".
                                 "    perl NiuTrans-hierarchy-train-model.pl -src  SOURCE-FILE -tgt TARGET-FILE \n".
                                 "                                           -aln ALIGNED-FILE -out OUTPUT-FILE \n";
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
#          if( !exists $param{ "-tmdir" } )
#          {
#                    print STDERR "Error: please assign -tmdir!\n";
#                    exit( 1 );
#          }
#          else
#          {
#                    $param{ "-tmdir" } =~ s/\\/\//g;
#                    if( !($param{ "-tmdir" } =~ /.*\/$/) )
#                    {
#                              $param{ "-tmdir" } = $param{ "-tmdir" }."/";
#                    }
#          }

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

          if( !exists $param{ "-filter" } )
          {
                    $param{ "-filter" } = 30;
          }
          if( !exists $param{ "-src" } || !exists $param{ "-tgt" } || !exists $param{ "-aln" } )
          {
                    print STDERR "Error: \"-src\", \"-tgt\" and \"-aln\" must be assigned!\n";
                    exit( 1 );
          }
          else
          {
                    if( !( -e $param{ "-src" } ) || !( -e $param{ "-tgt" } ) || !( -e $param{ "-aln" } ) )
                    {
                              print STDERR "$param{ \"-src\" }, $param{ \"-tgt\" } or $param{ \"-aln\" } does not exist!\n";
                              exit( 1 );
                    }
                    $param{ "-src" } =~ s/\\/\//g;
                    $param{ "-tgt" } =~ s/\\/\//g;
                    $param{ "-aln" } =~ s/\\/\//g;
          }

          if ( !exists $param{ "-log" } )
          {
#                    print STDERR $param{ "-tmdir" };
                    $param{ "-log" } = "train-model.log";
          }
          if( !exists $param{ "-config" } )
          {
                    copy( "../config/NiuTrans.hierarchy.train.model.config", "NiuTrans.hierarchy.train.model.config" ) or die "Copy failed: $!";;
                    $param{ "-config" } = "NiuTrans.hierarchy.train.model.config";
          }
}


sub readConfigFile
{
          print STDERR "Error: Config file does not exist!\n" if( scalar( @_ ) != 1 );
          $_[0] =~ s/\\/\//g;
          open( CONFIGFILE, "<".$_[0] ) or die "\nError: Can not read file $_[0] \n";
          print STDERR "Read $param{ \"-config\" } ";
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
#                             if( ( $1 eq "Lexical-Table" ) || ( $1 eq "Extract-Hiero-Rule" ) || ( $1 eq "Hiero-Rule-Table" ) )
#                             {
#                                       $option{ $1 } = "$param{ \"-tmdir\" }"."$2";
#                             }
#                             else
#                             {
#                                       $option{ $1 } = $2;
#                             }
                             $option{ $1 } = $2;
                             print STDERR ".";
                    }
          }
          close( CONFIGFILE ); 

#          if( !exists $option{ "Lexical-Table" } || !exists $option{ "Extract-Hiero-Rule" } 
#          || !exists $option{ "Hiero-Rule-Table" } || !exists $option{ "cutoffInit" }
#          if( !exists $option{ "Lexical-Table" } 
          if( !exists $option{ "cutoffInit" } || !exists $option{ "cutoffHiero" } )
          {
                    print STDERR " Error.\n\n".
                                 "Please validate your config file!\n".
                                 "You must have the parameter in $param{\"-c\"} below:\n".
#                                "\tLexical-Table\n".
#                                "\tExtract-Hiero-Rule\n".
#                                "\tHiero-Rule-Table\n".
                                 "\tcutoffInit\n".
                                 "\tcutoffHiero\n";
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
