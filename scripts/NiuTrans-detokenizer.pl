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
#   version      : 1.1.0
#   Function     : detokenizer
#   Author       : Qiang Li
#   Email        : liqiangneu@gmail.com
#   Date         : 08/06/2012
#   Last Modified: 
#######################################


use strict;
use Encode;
use utf8;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "#     NiuTrans  detokenizer  (version 1.1.0)  --www.nlplab.com     #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;

my %param;

getParameter( @ARGV );
detokenize();

sub detokenize
{
          open( INFILE, "<", $param{ "-in"  } ) or die "Error: can not open file $param{ \"-in\" }.\n";
          open( OUTPUT, ">", $param{ "-out" } ) or die "Error: can not open file $param{ \"-out\" }.\n";

          my $sentNo = 0;
          my $inputFileSent;
          while( $inputFileSent = <INFILE> )
          {
                    ++$sentNo;
                    $inputFileSent =~ s/[\r\n]//g;
                    if( $inputFileSent =~ /^<.+>$/ || $inputFileSent =~ /^\s*$/ )
                    {
                              print OUTPUT $inputFileSent."\n";
                    }
                    else
                    {
                              my $detokenizeRes = startDetokenize( $inputFileSent );
                              print OUTPUT $detokenizeRes."\n";
                    }
                    print STDERR "\rProcessed $sentNo lines." if( $sentNo % 100 == 0 );
          }
          print STDERR "\rProcessed $sentNo lines.\n";
          
          
          close( INFILE );
          close( OUTFILE );
}

sub startDetokenize
{
          my $sentence = $_[ 0 ];

          $sentence =~ s/ \@\-\@ /-/g;  # de-escape special chars
          $sentence =~ s/\&bar;/\|/g;   # factor separator
          $sentence =~ s/\&lt;/\</g;    # xml
          $sentence =~ s/\&gt;/\>/g;    # xml
          $sentence =~ s/\&bra;/\[/g;   # syntax non-terminal (legacy)
          $sentence =~ s/\&ket;/\]/g;   # syntax non-terminal (legacy)
          $sentence =~ s/\&quot;/\"/g;  # xml
          $sentence =~ s/\&apos;/\'/g;  # xml
          $sentence =~ s/\&#91;/\[/g;   # syntax non-terminal
          $sentence =~ s/\&#93;/\]/g;   # syntax non-terminal
          $sentence =~ s/\&amp;/\&/g;   # escape escape

          my @words = split / +/,$sentence;
          my $sentenceDetoken = "";
          my %quoteCount = ( "\'" => 0, "\"" => 0 );
          my $connector = " ";
          
          my $wordCnt = 0;
          my $preWord = "";
          foreach my $word ( @words )
          {
                    if( $word =~ /^[\p{IsSc}\(\[\{]+$/ )
                    {
                              $sentenceDetoken = $sentenceDetoken.$connector.$word;
                              $connector = "";
                    }
                    elsif( $word =~ /^[\,\.\?\!\:\;\\\%\}\]\)]+$/ )
                    {
                              $sentenceDetoken = $sentenceDetoken.$word;
                              $connector = " ";
                    }
                    elsif( ( $wordCnt > 0 ) && ( $word =~ /^[\'][\p{IsAlpha}]/ ) && ( $preWord =~ /[\p{IsAlnum}]$/ ) )
                    {
                              $sentenceDetoken = $sentenceDetoken.$word;
                              $connector = " ";
                    }
                    elsif( $word =~ /^[\'\"]+$/ )
                    {
                              if( !exists $quoteCount{ $word } )
                              {
                                        $quoteCount{ $word } = 0;
                              }
                              
                              if( ( $quoteCount{ $word } % 2 ) eq 0 )
                              {
                                        if( ( $word eq "'" ) && ( $wordCnt > 0 ) && ( $preWord =~ /[s]$/ ) )
                                        {
                                                  $sentenceDetoken = $sentenceDetoken.$word;
                                                  $connector = " ";
                                        }
                                        else
                                        {
                                                  $sentenceDetoken = $sentenceDetoken.$connector.$word;
                                                  $connector = "";
                                                  ++$quoteCount{ $word };
                                        }
                              }
                              else
                              {
                                        $sentenceDetoken = $sentenceDetoken.$word;
                                        $connector = " ";
                                        ++$quoteCount{ $word };
                              }
                    }
                    else
                    {
                              $sentenceDetoken = $sentenceDetoken.$connector.$word;
                              $connector = " ";
                    }
                    
                    $preWord = $word;
                    ++$wordCnt;
          }

          $sentenceDetoken =~ s/ +/ /g;
          $sentenceDetoken =~ s/^ //g;
          $sentenceDetoken =~ s/ $//g;
          
          $sentenceDetoken =~ s/^([[:punct:]\s]*)([[:alpha:]])(.*)$/$1\U$2\E$3/ if( $param{ "-upcase" } eq 1);
          
          return $sentenceDetoken;
}

sub getParameter
{
          if( ( scalar( @_ ) < 4 ) || ( scalar( @_ ) % 2 != 0 ) )
          {
                    print STDERR "[USAGE]\n".
                                 "         NiuTrans-detokenizer.pl               [OPTIONS]\n".
                                 "[OPTION]\n".
                                 "          -in     :  Input  File.\n".
                                 "          -out    :  Output File.\n".
                                 "          -upcase :  Uppercase the first char  [optional]\n".
                                 "                     Default value is 1.\n".
                                 "[EXAMPLE]\n".
                                 "         perl NiuTrans-detokenizer.pl          [-in  FILE]\n".
                                 "                                               [-out FILE]\n";
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
          
          if( !exists $param{ "-in" } )
          {
                    print STDERR "Error: please assign \"-in\"!\n";
                    exit( 1 );
          }
          
          if( !exists $param{ "-out" } )
          {
                    print STDERR "Error: please assign \"-out\"!\n";
                    exit( 1 );
          }
          
          if( !exists $param{ "-upcase" } )
          {
                    $param{ "-upcase" } = 1;
          }
          elsif( $param{ "-upcase" } ne 1 )
          {
                    $param{ "-upcase" } = 0;
          }
}
