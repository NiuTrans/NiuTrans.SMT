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
#   Function         : generate mert config file
#   Author           : Qiang Li
#   Email            : liqiangneu@gmail.com
#   Date             : 06/09/2011
#   last Modified by :
#############################################


use strict;
use File::Copy;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "# NiuTrans Syntax GeneConf (version 1.0.0 Beta)   --www.nlplab.com #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;
my %param;
my %option;

getParameter( @ARGV );

#print STDERR "$param{ \"-tmdir\" } $param{ \"-lmdir\" } \n";
#print STDERR "$param{ \"-c\" } $param{ \"-o\" } \n";

generateMertConfig();



#my $key;
#my $value;
#while( ( $key, $value ) = each %param )
#{
#	print "$key => $value\n";
#}
#while( ( $key, $value ) = each %param )
#{
#	print "$key => $value\n";
#}

sub generateMertConfig
{
          open( ITGCONFIG, "<".$param{ "-config" } ) or die "Error: Can not open file $param{ \"-config\" }!\n";
          open( MERTCONFIG, ">".$param{ "-out" } ) or die "Error: Can not write file $param{ \"-out\" }!\n";
          my $lineNo = 0;
          while( <ITGCONFIG> )
          {
                    ++$lineNo;
                    s/[\r\n]//g;
                    print MERTCONFIG "\n" if /^( |\t)*$/;
                    if( /param(?: |\t)*=(?: |\t)*"([\w\-]*)"((?: |\t)*)value="([\w\/\-. :]*)"(?: |\t)*/ )
                    {
                              if( exists $param{ "-$1" } )
                              {
                                        print MERTCONFIG "param=\"$1\""."$2"."value=\"$param{ \"-$1\" }\"\n";
                              }
                              else
                              {
                                        print MERTCONFIG $_."\n";
                              }
                    }
                    elsif( /^#/ )
                    {
                              print MERTCONFIG "$_\n";
                    }
                    print STDERR "\rprocess $lineNo lines!";
          }
          print STDERR "\rprocess $lineNo lines!\n";
          close( MERTCONFIG );
          close( ITGCONFIG );
}



sub getParameter
{
          if( ( scalar( @_ ) < 4 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]         :\n".
                                 "    NiuTrans-syntax-generate-mert-config.pl         [OPTIONS]\n".
                                 "[OPTION]        :\n".
                                 "    -syntaxrule :  Syntax Rule File.\n".
                                 "    -lmdir      :  Language Model Directory.\n".
                                 "    -model      :  The type of rules can be extracted.\n".
                                 "                     Its value can be \"s2t\", \"t2s\" or \"t2t\".\n".
                                 "    -config     :  Config File.                    [optional]\n".
                                 "                     Default copy our conf-file to current dir.\n".
                                 "    -out        :  Output Mert Config File.        [optional]\n".
                                 "                     Default is NiuTrans.syntax.\"model\".user.config\n".
                                 "                     on current dir!\n".
                                 "    -nref       :  Dev Set reference number.       [optional]\n".
                                 "                     Default value is in your config file!\n".
                                 "    -ngram      :  N-gram Language Model.          [optional]\n".
                                 "                     Default value is 3.\n".
                                 "    -nthread    :  Number of threads.              [optional]\n".
                                 "                     Default value is 4.\n".
                                 "    -punctfile  : Punctuations specified.          [optional]\n".
                                 "                     Default file is punct.utf8.txt in /resource.\n".
                                 "[EXAMPLE]  :\n".
                                 "  perl NiuTrans-syntax-gemerate-mert-config.pl\n".
                                 "                                      -model      [MODEL-VAL]\n".
                                 "                                      -syntaxrule [SYN-RULE ]\n".
                                 "                                      -lmdir      [LM-DIR   ]\n";
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

          if( !( -e $param{ "-syntaxrule" } ) )
          {
                    print STDERR "Error: $param{ \"-syntaxrule\" } does not exist!\n";
                    exit( 1 );
          }
          else 
          {
                    $param{ "-SCFG-Rule-Set" } = $param{ "-syntaxrule" };
          }
          
          if( !exists $param{ "-lmdir" } )
          {
                    print STDERR "Error: please assign -lmdir!\n";
                    exit( 1 );
          }
          else
          {
                    $param{ "-lmdir" } =~ s/\\/\//g;
                    if( !($param{ "-lmdir" } =~ /.*\/$/) )
                    {
                              $param{ "-lmdir" } = $param{ "-lmdir" }."/";
                    }
          }

          if( !exists $param{ "-model" } )
          {
                    print STDERR "Error: please assign -model!\n";
                    exit( 1 );
          }
          elsif( ( $param{ "-model" } ne "s2t" ) and ( $param{ "-model" } ne "t2t" ) and ( $param{ "-model" } ne "t2s" ) )
          {
                    print STDERR "Error: The value of \"-model\" must be assigned to \"s2t\", \"t2t\" or \"t2s\"!\n";
                    exit( 1 ); 
          }
          
          if( !( -e $param{ "-lmdir" }."lm.trie.data" ) )
          {
                    print STDERR "Error: $param{ \"-lmdir\" }lm.trie.data does not exist!\n";
                    exit( 1 );
          }
          else
          {
                    $param{ "-Ngram-LanguageModel-File" } = $param{ "-lmdir" }."lm.trie.data"; 
          }
          
          if( !( -e $param{ "-lmdir" }."lm.vocab" ) )
          {
                    print STDERR "Error: $param{ \"-lmdir\"}lm.vocab does not exist!\n";
                    exit( 1 );
          }
          else
          {
                    $param{ "-Target-Vocab-File" } = $param{ "-lmdir" }."lm.vocab";
          }
          
          if( !exists $param{ "-config" } )
          {
                    if( $param{ "-model" } eq "s2t" ) 
                    {
                              $param{ "-config" } = "../config/NiuTrans.syntax.s2t.config";
                    }
                    elsif( $param{ "-model" } eq "t2t" )
                    {
                              $param{ "-config" } = "../config/NiuTrans.syntax.t2t.config";
                    }
                    elsif( $param{ "-model" } eq "t2s" )
                    {
                              $param{ "-config" } = "../config/NiuTrans.syntax.t2s.config";
                    }
#                    $param{ "-config" } = "../config/NiuTrans.syntax.config";
          }
 
         if( !( -e $param{ "-config" } ) )
          {
                    print STDERR "Error: Config file does not exist!\n";
                    exit( 1 );
          }

          if( !exists $param{ "-out" } )
          {
                    if( $param{ "-model" } eq "s2t" )
                    {
                              $param{ "-out" } = "NiuTrans.syntax.s2t.user.config";
                    }
                    elsif( $param{ "-model" } eq "t2t" ) 
                    {
                              $param{ "-out" } = "NiuTrans.syntax.t2t.user.config";
                    }
                    elsif( $param{ "-model" } eq "t2s" )
                    {
                              $param{ "-out" } = "NiuTrans.syntax.t2s.user.config";
                    }
#                    $param{ "-out" } = "NiuTrans.syntax.user.config";
          }

          if( !exists $param{ "-punctfile" } )
          {
                    $param{ "-punctfile" } = "../resource/punct.utf8.txt";
          }
          if( !( -e $param{ "-punctfile" } ) )
          {
                    print STDERR "Error: Punct File does not exist!\n";
                    exit( 1 );
          }
          else
          {
                    $param{ "-Punct-Vocab-File" } = $param{ "-punctfile" };
          }
}

