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
             "#  NiuTrans Phrase GeneConf (version 1.0.0 Beta) --www.nlplab.com  #\n".
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
          open( ITGCONFIG, "<".$param{ "-c" } ) or die "Error: Can not open file $param{ \"-c\" }!\n";
          open( MERTCONFIG, ">".$param{ "-o" } ) or die "Error: Can not write file $param{ \"-o\" }!\n";
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
                                 "    NiuTrans-phrase-generate-mert-config.pl          [OPTIONS]\n".
                                 "[OPTION]        :\n".
                                 "    -tmdir      :  Translation Model Directory.\n".
                                 "    -phrasetab  :  Path for Phrase Table.            [optional]\n".
                                 "                     Default value is \"phrase.translation.table\".\n".
                                 "    -metab      :  FileName for Me-Reordering Table. [optional]\n".
                                 "                     Default value is \"me.reordering.table\".\n".
                                 "    -msdtab     :  FileName for MSD-Reordering Table.[optional]\n".
                                 "                     Default value is \"msd.reordering.table\".\n".
                                 "    -lmdir      :  Language Model Directory.\n".
                                 "    -vocab      :  FileName for Vocabulary File.     [optional]\n".
                                 "                     Default value is \"lm.vocab\".\n".
                                 "    -lmbin      :  FileName for Language Model File. [optional]\n".
                                 "                     Default value is \"lm.trie.data\".\n".
                                 "    -c          :  Config File.                      [optional]\n".
                                 "                     Default file is NiuTrans.phrase.config\n".
                                 "                     in /config dir.\n".
                                 "    -o          :  Output Mert Config File.          [optional]\n".
                                 "                     Default file is NiuTrans.phrase.user.config\n".
                                 "                     in current dir.\n".
                                 "    -nref       :  Dev Set reference number.         [optional]\n".
                                 "                     Default value is in your config file.\n".
                                 "    -ngram      :  N gram.                           [optional]\n".
                                 "                     Default value is 3.\n".
                                 "    -nthread    :  Number of threads.                [optional]\n".
                                 "                     Default value is 4.\n".
                                 "    -punctfile  :  Punctuations specified.           [optional]\n".
                                 "                     Default file is punct.utf8.txt in /resource dir.\n".
                                 "[EXAMPLE]       :\n".
                                 "  perl NiuTrans-phrase-gemerate-mert-config.pl -tmdir tm-dir -lmdir lm-dir\n";
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
          
          if( !exists $param{ "-phrasetab" } )
          {
                    $param{ "-phrasetab" } = "phrase.translation.table";
          }
          
          if( !exists $param{ "-msdtab" } )
          {
                    $param{ "-msdtab" } = "msd.reordering.table";
          }
          
          if( !exists $param{ "-metab" } )
          {
                    $param{ "-metab" } = "me.reordering.table";
          }
          
          if( !( -e $param{ "-tmdir" }.$param{ "-metab" } ) )
          {
                    print STDERR "Error: $param{ \"-tmdir\" }$param{ \"-metab\" } does not exist!\n";
                    exit( 1 );
          }
          else 
          {
                    $param{ "-ME-Reordering-Table" } = $param{ "-tmdir" }.$param{ "-metab" };
          }
          
          if( !( -e $param{ "-tmdir" }.$param{ "-msdtab" } ) )
          {
                    print STDERR "Error: $param{ \"-tmdir\" }$param{ \"-msdtab\" } does not exist!\n";
                    exit( 1 );          
          }
          else
          {
                    $param{ "-MSD-Reordering-Model" } = $param{ "-tmdir" }.$param{ "-msdtab" };
          }
          
          if( !( -e $param{ "-tmdir" }.$param{ "-phrasetab" } ) )
          {
                    print STDERR "Error: $param{ \"-tmdir\" }$param{ \"-phrasetab\" } does not exist!\n";
                    exit( 1 );
          }
          else
          {
                    $param{ "-Phrase-Table" } = $param{ "-tmdir" }.$param{ "-phrasetab" };
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
          
          if( !exists $param{ "-lmbin" } )
          {
                    $param{ "-lmbin" } = "lm.trie.data";
          }
          
          if( !exists $param{ "-vocab" } )
          {
                    $param{ "-vocab" } = "lm.vocab";
          }
          
          if( !( -e $param{ "-lmdir" }.$param{ "-lmbin" } ) )
          {
                    print STDERR "Error: $param{ \"-lmdir\" }$param{ \"-lmbin\" } does not exist!\n";
                    exit( 1 );
          }
          else
          {
                    $param{ "-Ngram-LanguageModel-File" } = $param{ "-lmdir" }.$param{ "-lmbin" }; 
          }
          
          if( !( -e $param{ "-lmdir" }.$param{ "-vocab" } ) )
          {
                    print STDERR "Error: $param{ \"-lmdir\"}$param{ \"-vocab\" } does not exist!\n";
                    exit( 1 );
          }
          else
          {
                    $param{ "-Target-Vocab-File" } = $param{ "-lmdir" }.$param{ "-vocab" };
          }
          
          if( !exists $param{ "-c" } )
          {
                    $param{ "-c" } = "../config/NiuTrans.phrase.config";
          }
          if( !( -e $param{ "-c" } ) )
          {
                    print STDERR "Error: Config file does not exist!\n";
                    exit( 1 );
          }

          if( !exists $param{ "-o" } )
          {
                    $param{ "-o" } = "NiuTrans.phrase.user.config";
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

