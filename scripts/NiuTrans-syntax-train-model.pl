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
#   Function         : Syntax Training
#   Author           : Qiang Li
#   Email            : liqiangneu@gmail.com
#   Date             : 2011-11-21
#   last Modified by :
#     
#############################################


use strict;
use File::Copy;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "# NiuTrans Syntax Training (version 1.0.0 Beta)   --www.nlplab.com #\n".
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

extractSyntaxRule();
close( $log );  


sub extractSyntaxRule
{
          ########
          #  STEP0
          #  syntax-based rule extraction
          #  input: source file
          #         target file
          #         alignment file
          #         source parse tree
          #         target parse tree
          # output: syntax-rule.step0
          $exec = "../bin/NiuTrans.SyntaxRuleEx";
          if( $param{ "-model" } eq "s2t" )
          {
                    $opt  =   " -model      $param{ \"-model\" }".
                              " -src        $param{  \"-src\" }".
                              " -tar        $param{  \"-tgt\" }".
                              " -tarparse   $param{  \"-ttree\" }".
                              " -align      $param{  \"-aln\" }".
                              " -output     $param{  \"-out\" }.step0";

                    if( ( exists $option{ "method" } ) and ( $option{ "method" } ne "default" ) )
                    {
                              if( ( $option{ "method" } eq "GHKM" ) or ( $option{ "method" } eq "SPMT" ) )
                              {
                                        $opt .=   " -method     $option{ \"method\" }";
                              }
                    }
                    
                    if( ( exists $option{ "compose" } ) and ( $option{ "compose" } ne "-1" ) )
                    {
                              $opt .=   " -compose $option{ \"compose\" }";
                    }

                    if( ( exists $option{ "unalign" } ) and ( $option{ "unalign" } ne "-1" ) )
                    {
                              $opt .=   " -unalign $option{ \"unalign\" }";
                    }

                    if( ( exists $option{ "varnum" } ) and ( $option{ "varnum" } ne "-1" ) )
                    {
                              $opt .=   " -varnum $option{ \"varnum\" }";
                    }

                    if( ( exists $option{ "wordnum" } ) and ( $option{ "wordnum" } ne "-1" ) )
                    {
                              $opt .=   " -wordnum $option{ \"wordnum\" }";
                    }

                    if( ( exists $option{ "depth" } ) and ( $option{ "depth" } ne "-1" ) )
                    {
                              $opt .=   " -depth $option{ \"depth\" }";
                    }

                    if( ( exists $option{ "uain" } ) and ( $option{ "uain" } ne "-1" ) )
                    {
                              $opt .=   " -uain $option{ \"uain\" }";
                    }

                    if( ( exists $option{ "uaout" } ) and ( $option{ "uaout" } ne "-1" ) )
                    {
                              $opt .=   " -uaout $option{ \"uaout\" }";
                    }

                    if( ( exists $option{ "oformat" } ) and ( $option{ "oformat" } ne "default" ) )
                    {
                              if( ( $option{ "oformat" } eq "oft" ) or ( $option{ "oformat" } eq "nft" ) )
                              {
                                        $opt .=   " -oformat $option{ \"oformat\" }";
                              }
                    }

                    $logContent  = "Running: Extracting String to Tree Syntax Rules\n";
          }
          elsif( $param{ "-model" } eq "t2t" )
          {
                    $opt =    " -model      $param{  \"-model\" }".
                              " -src        $param{  \"-src\" }".
                              " -srcparse   $param{  \"-stree\" }".
                              " -tar        $param{  \"-tgt\" }".
                              " -tarparse   $param{  \"-ttree\" }".
                              " -align      $param{  \"-aln\" }".
                              " -output     $param{  \"-out\" }.step0";

                    if( ( exists $option{ "method" } ) and ( $option{ "method" } ne "default" ) )
                    {
                              if( ( $option{ "method" } eq "GHKM" ) or ( $option{ "method" } eq "SPMT" ) )
                              {
                                        $opt .=   " -method     $option{ \"method\" }";
                              }
                    }
                    
                    if( ( exists $option{ "compose" } ) and ( $option{ "compose" } ne "-1" ) )
                    {
                              $opt .=   " -compose $option{ \"compose\" }";
                    }

                    if( ( exists $option{ "unalign" } ) and ( $option{ "unalign" } ne "-1" ) )
                    {
                              $opt .=   " -unalign $option{ \"unalign\" }";
                    }

                    if( ( exists $option{ "varnum" } ) and ( $option{ "varnum" } ne "-1" ) )
                    {
                              $opt .=   " -varnum $option{ \"varnum\" }";
                    }

                    if( ( exists $option{ "wordnum" } ) and ( $option{ "wordnum" } ne "-1" ) )
                    {
                              $opt .=   " -wordnum $option{ \"wordnum\" }";
                    }

                    if( ( exists $option{ "uaperm" } ) and ( $option{ "uaperm" } eq "1" ) )
                    {
                              $opt .=   " -uaperm";
                    }

                    if( ( exists $option{ "depth" } ) and ( $option{ "depth" } ne "-1" ) )
                    {
                              $opt .=   " -depth $option{ \"depth\" }";
                    }

                    if( ( exists $option{ "uain" } ) and ( $option{ "uain" } ne "-1" ) )
                    {
                              $opt .=   " -uain $option{ \"uain\" }";
                    }

                    if( ( exists $option{ "uaout" } ) and ( $option{ "uaout" } ne "-1" ) )
                    {
                              $opt .=   " -uaout $option{ \"uaout\" }";
                    }

                    if( ( exists $option{ "oformat" } ) and ( $option{ "oformat" } ne "default" ) )
                    {
                              if( ( $option{ "oformat" } eq "oft" ) or ( $option{ "oformat" } eq "nft" ) )
                              {
                                        $opt .=   " -oformat $option{ \"oformat\" }";
                              }
                    }

                    $logContent  = "Running: Extracting Tree to Tree Syntax Rules\n";
          }
          elsif( $param{ "-model" } eq "t2s" )
          {
                    $opt =    " -model      $param{  \"-model\" }".
                              " -src        $param{  \"-src\" }".
                              " -tar        $param{  \"-tgt\" }".
                              " -srcparse   $param{  \"-stree\" }".
                              " -align      $param{  \"-aln\" }".
                              " -output     $param{  \"-out\" }.step0";

                    if( ( exists $option{ "method" } ) and ( $option{ "method" } ne "default" ) )
                    {
                              if( ( $option{ "method" } eq "GHKM" ) or ( $option{ "method" } eq "SPMT" ) )
                              {
                                        $opt .=   " -method     $option{ \"method\" }";
                              }
                    }
                    
                    if( ( exists $option{ "compose" } ) and ( $option{ "compose" } ne "-1" ) )
                    {
                              $opt .=   " -compose $option{ \"compose\" }";
                    }

                    if( ( exists $option{ "unalign" } ) and ( $option{ "unalign" } ne "-1" ) )
                    {
                              $opt .=   " -unalign $option{ \"unalign\" }";
                    }

                    if( ( exists $option{ "varnum" } ) and ( $option{ "varnum" } ne "-1" ) )
                    {
                              $opt .=   " -varnum $option{ \"varnum\" }";
                    }

                    if( ( exists $option{ "wordnum" } ) and ( $option{ "wordnum" } ne "-1" ) )
                    {
                              $opt .=   " -wordnum $option{ \"wordnum\" }";
                    }

                    if( ( exists $option{ "depth" } ) and ( $option{ "depth" } ne "-1" ) )
                    {
                              $opt .=   " -depth $option{ \"depth\" }";
                    }

                    if( ( exists $option{ "uain" } ) and ( $option{ "uain" } ne "-1" ) )
                    {
                              $opt .=   " -uain $option{ \"uain\" }";
                    }

                    if( ( exists $option{ "uaout" } ) and ( $option{ "uaout" } ne "-1" ) )
                    {
                              $opt .=   " -uaout $option{ \"uaout\" }";
                    }

                    if( ( exists $option{ "oformat" } ) and ( $option{ "oformat" } ne "default" ) )
                    {
                              if( ( $option{ "oformat" } eq "oft" ) or ( $option{ "oformat" } eq "nft" ) )
                              {
                                        $opt .=   " -oformat $option{ \"oformat\" }";
                              }
                    }

                    $logContent  = "Running: Extracting Tree to String Syntax Rules\n";
          }

          ssystem( $exec." ".$opt );
          

          $logContent .=   "\n\t".$exec." ".$opt.
                           "\n\t\t ########".
                           "\n\t\t #  STEP0".
                           "\n\t\t #  syntax-based rule extraction".
                           "\n\t\t #  input: source file".
                           "\n\t\t #         target file".
                           "\n\t\t #         alignment file".
                           "\n\t\t #         source parse tree".
                           "\n\t\t #         target parse tree".
                           "\n\t\t # output: syntax-rule.step0".
                           "\n\t\t time:\t";
          generateLog( $log, $logContent );

          ########
          #  STEP1
          #  Scoring
          #  input: syntax-rule
          # output: syntax-rule.scored
          $exec = "../bin/NiuTrans.PhraseExtractor";

          $opt = " --LEX".
                 " -src $param{ \"-src\" }".
                 " -tgt $param{ \"-tgt\" }".
                 " -aln $param{ \"-aln\" }".
                 " -out $param{ \"-out\" }.lex";

         ssystem( $exec." ".$opt );

          $opt = " --SCORESYN".
                 " -model $param{ \"-model\" }".
                 " -rule $param{ \"-out\" }.step0".
                 " -ls2d $param{ \"-out\" }.lex.s2d.sorted".
                 " -ld2s $param{ \"-out\" }.lex.d2s.sorted".
                 " -out $param{ \"-out\" }.step1".
                 " -cutoff $option{ \"cutoff\" }";
                 " -lowerfreq $option{ \"lowerfreq\" }";
         ssystem( $exec." ".$opt );
          $logContent = "Running: Scoring for syntax-rule\n".
                        "\n\t".$exec." ".$opt.
                        "\n\t\t ########".
                        "\n\t\t #  STEP1".
                        "\n\t\t #  scoring for syntax-rule".
                        "\n\t\t #  input: syntax-rule".
                        "\n\t\t # output: syntax-rule.scored".
                        "\n\t\t ########".
                        "\n\t\t time:\t";
          generateLog( $log, $logContent );

#          rename $param{ "-out" }.".phrasescore.syntaxscore", $param{ "-out" }.".step1";

          unlink glob "$param{ \"-out\" }.lex.*" or die "Error: can not unlink $param{ \"-out\" }.lex.*!\n";
          unlink "$param{ \"-out\" }.step0" or die "Error: can not unlink $param{ \"-out\" }.step0!\n";

############################## add in 2012-0705 ############################################
          # Format Check
          $exec = "perl format-check.pl";
          $opt = "< $param{ \"-out\" }.step1 > $param{ \"-out\" }.step2";
          ssystem( $exec." ".$opt );
############################## add in 2012-0705 ############################################





          ########
          #  STEP2
          #  Filtering
          #  input: syntax-rule.score
          # output: syntax-rule.score.filter
          $exec = "../bin/NiuTrans.PhraseExtractor";
          $opt = " --FILTN".
                 " -in $param{ \"-out\"}.step2".
                 " -out $param{ \"-out\" }.step3".
                 " -strict $option{ \"filternum\" }".
                 " -tableFormat syntax";

          ssystem( $exec." ".$opt );

          $logContent = "Running: Filter syntax-rule\n".
                        "\n\t".$exec." ".$opt.
                        "\n\t\t ########".
                        "\n\t\t #  STEP2".
                        "\n\t\t #  Filter syntax-rule".
                        "\n\t\t #  input: syntax-rule.scored".
                        "\n\t\t # output: syntax-rule.scored.filterd".
                        "\n\t\t ########".
                        "\n\t\t time:\t";
          generateLog( $log, $logContent );

          unlink "$param{ \"-out\" }.step1" or die "Error: can not unlink file $param{ \"-out\" }.step1!\n";
          unlink "$param{ \"-out\" }.step2" or die "Error: can not unlink file $param{ \"-out\" }.step1!\n";



############################## add in 2012-0705 ############################################
          $exec = "perl filter.unary.rule.pl";
          $opt = "< $param{ \"-out\" }.step3 > $param{ \"-out\" }.step4";
          ssystem( $exec." ".$opt );
          $exec = "perl generate.bilex.s2t.pl";
          $opt = "< $param{ \"-out\" }.step4 > $param{ \"-out\" }.step5";
          ssystem( $exec." ".$opt );
          $exec = "perl add.null.algin.s2t.pl";
          $opt = "< $param{ \"-out\" }.step5 > $param{ \"-out\" }";
          ssystem( $exec." ".$opt );
############################## add in 2012-0705 ############################################

          unlink "$param{ \"-out\" }.step3" or die "Error: can not unlink file $param{ \"-out\" }.step1!\n";

          unlink "$param{ \"-out\" }.step4" or die "Error: can not unlink file $param{ \"-out\" }.step1!\n";

          unlink "$param{ \"-out\" }.step5" or die "Error: can not unlink file $param{ \"-out\" }.step1!\n";



          ssystem( "perl NiuTrans-change-syntaxrule-to-exp-format.pl <$param{ \"-out\" } >$param{ \"-out\" }.unbina" );

          $exec = "../bin/NiuTrans.RuleBinar";
          $opt  = " -input $param{ \"-out\" }".
                  " -output $param{ \"-out\" }.bina.step1";
          ssystem( $exec." ".$opt );

############################## add in 2012-0705 ############################################
          $exec = "perl trans.to.new.syntax.rule.pl";
          $opt = "< $param{ \"-out\" }.bina.step1 > $param{ \"-out\" }.bina.step2";
          ssystem( $exec." ".$opt );
          $exec = "perl trans.to.LNF.pl";
          $opt = "< $param{ \"-out\" }.bina.step2 > $param{ \"-out\" }.bina";
          ssystem( $exec." ".$opt );

          unlink "$param{ \"-out\" }.bina.step1" or die "Error: can not unlink file $param{ \"-out\" }.step1!\n";
          unlink "$param{ \"-out\" }.bina.step2" or die "Error: can not unlink file $param{ \"-out\" }.step1!\n";
          unlink "unary.rule.nopos.txt" or die "Error: can not unlink file \"unary.rule.nopos.txt\"\n";
          unlink "unary.rule.pos.txt" or die "Error: can not unlink file \"unary.rule.pos.txt\"\n";
############################## add in 2012-0705 ############################################


}

sub getParameter
{
          if( ( scalar( @_ ) < 8 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]        :\n".
                                 "    NiuTrans-syntax-train-model.pl                  [OPTIONS]\n".
                                 "[OPTION]       :\n".
                                 "    -model     :  The type of rules can be extracted.\n".
                                 "                    Its value can be \"s2t\", \"t2s\" or \"t2t\".\n".
                                 "    -src       :  Source Language File.\n".
                                 "    -stree     :  Source Parse Tree\n".
                                 "    -tgt       :  Target Language File.\n".
                                 "    -ttree     :  Target Parse Tree\n".
                                 "    -aln       :  Aligned File.\n".
                                 "    -out       :  Output Syntax Rule.\n".
                                 "    -config    :  Config File.                     [optional]\n".
                                 "                    Default copy our config-file to current dir.\n".
                                 "    -log       :  Log File.                        [optional]\n".
                                 "                    Default \"syntax-train-model.log\" on current dir.\n".
                                 "[EXAMPLE]   :\n".
                                 "    perl NiuTrans-syntax-train-model.pl   -model  [MODEL-VAL]\n". #-tmdir work-dir\n".
                                 "                                          -src    [SRC-FILE ]\n".
                                 "                                          -tgt    [TGT-FILE ]\n".
                                 "                                          -stree  [SRC-PTREE]\n".
                                 "                                          -ttree  [TGT-PTREE]\n".
                                 "                                          -aln    [ALN-FILE ]\n".
                                 "                                          -out    [MOD-FILE ]\n";
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

          if( !exists $param{ "-src" } || !exists $param{ "-tgt" } || !exists $param{ "-aln" } )
          {
                    print STDERR "Error: -src -tgt -aln must be assigned!\n";
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

          if( !exists $param{ "-model" } )
          {
                    print STDERR "Error: -model must be assigned!\n";
                    exit( 1 );
          }
          elsif( $param{ "-model" } eq "s2t" )
          {
                    if( !exists $param{ "-ttree" } )
                    {
                              print STDERR "Error: \"-ttree\" must be assigned!\n";
                              exit( 1 );
                    }
                    else
                    {
                              if( !( -e $param{ "-ttree" } ) ) 
                              {
                                        print STDERR "$param{ \"-ttree\" } does not exist!\n";
                                        exit( 1 );
                              }
                    }
          }
          elsif( $param{ "-model" } eq "t2t" )
          {
                    if( !exists $param{ "-stree" } or !exists $param{ "-ttree" } )
                    {
                              print STDERR "Error: \"-stree\" and \"-ttree\" must be assigned!\n";
                              exit( 1 );
                    }
                    else
                    {
                              if( !( -e $param{ "-stree" } ) or !( -e $param{ "-ttree" } ) )
                              {
                                        print STDERR "$param{ \"-stree\" } or $param{ \"-ttree\" } does not exist!\n";
                                        exit( 1 );
                              }
                    }
          }
          elsif( $param{ "-model" } eq "t2s" )
          {
                    if( !exists $param{ "-stree" } )
                    {
                              print STDERR "Error: \"-stree\" must be assigned!\n";
                              exit( 1 );
                    } 
                    else
                    {
                              if( !( -e $param{ "-stree" } ) )
                              {
                                        print STDERR "$param{ \"-stree\" } does not exist!\n";
                                        exit( 1 );
                              }
                    }
          }
          else
          {
                    print STDERR "Error: The value of parameter \"-model\" must be assigned to \"s2t\", \"t2t\" or \"t2s\"!\n";
                    exit( 1 );
          }

          if ( !exists $param{ "-log" } )
          {
                    $param{ "-log" } = "syntax-train-model.log";
          }
          if( !exists $param{ "-config" } )
          {
                    if( $param{ "-model" } eq "s2t" )
                    {
                              copy( "../config/NiuTrans.syntax.s2t.train.model.config", "NiuTrans.syntax.s2t.train.model.config" ) or die "Copy failed: $!";;
                              $param{ "-config" } = "NiuTrans.syntax.s2t.train.model.config";
                    }
                    elsif( $param{ "-model" } eq "t2t" )
                    {
                              copy( "../config/NiuTrans.syntax.t2t.train.model.config", "NiuTrans.syntax.t2t.train.model.config" ) or die "Copy failed: $!";;
                              $param{ "-config" } = "NiuTrans.syntax.t2t.train.model.config";
                    }
                    elsif( $param{ "-model" } eq "t2s" ) 
                    {
                              copy( "../config/NiuTrans.syntax.t2s.train.model.config", "NiuTrans.syntax.t2s.train.model.config" ) or die "Copy failed: $!";;
                              $param{ "-config" } = "NiuTrans.syntax.t2s.train.model.config";
                    }
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
  #                           if( ( $1 eq "Lexical-Table" ) || ( $1 eq "Extract-Hiero-Rule" ) || ( $1 eq "Hiero-Rule-Table" ) )
  #                           {
  #                                     $option{ $1 } = "$param{ \"-tmdir\" }"."$2";
  #                           }
  #                           else
  #                           {
                                       $option{ $1 } = $2;
  #                           }
                             print STDERR ".";
                    }
          }
          close( CONFIGFILE ); 

#          if( !exists $option{ "Lexical-Table" } || !exists $option{ "Extract-Hiero-Rule" } 
#           || !exists $option{ "Hiero-Rule-Table" } || !exists $option{ "cutoffInit" }
#           || !exists $option{ "cutoffHiero" } )
#          {
#                    print STDERR " Error.\n\n".
#                                 "Please validate your config file!\n".
#                                 "You must have the parameter in $param{\"-config\"} below:\n".
#                                 "\tLexical-Table\n".
#                                 "\tExtract-Hiero-Rule\n".
#                                 "\tHiero-Rule-Table\n".
#                                 "\tcutoffInit\n".
 #                                "\tcutoffHiero\n";
 #                   exit( 1 );
 #         }
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
