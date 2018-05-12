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
#   version   : 1.0.0 Beta
#   Function  : generate xml for mteval
#   Author    : Qiang Li
#   Email     : liqiangneu@gmail.com
#   Date      : 06/16/2011
#   last Modified by :
#     2011/7/16 add deal with OOV function by Qiang Li
#     2011/7/16 first change "&" with "&amp;" by Qiang Li
#######################################


#!/usr/bin/perl -w
use strict;
use Encode;
use Encode::CN;

my $logo =   "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n".
             "#                                                                  #\n".
             "#   NiuTrans Generate XML (version 1.0.0 Beta)  --www.nlplab.com   #\n".
             "#                                                                  #\n".
             "########### SCRIPT ########### SCRIPT ############ SCRIPT ##########\n";

print STDERR $logo;

my %param;

getParameter( @ARGV );
dealOOV( $param{ "-1f" }, $param{ "-1f" }.".temp" );
generateTst( $param{ "-1f" }.".temp", $param{ "-odir" }."tst.xml", 1 );
generateSrcAndRef( $param{ "-tf" }, $param{ "-odir" }."src.xml", $param{ "-odir" }."ref.xml", $param{ "-rnum" } );
#unlink $param{ "-1f" }.".temp";


######################################################
#get parameter
######################################################
sub getParameter
{
    if( ( scalar( @_ ) < 6 ) || ( scalar( @_ ) % 2 != 0 ) )
    {
        print STDERR "[USAGE]    :\n".
                     "        NiuTrans-generate-xml-for-mteval.pl [OPTIONS]\n".
                     "[OPTION]   :\n".
                     "    -1f    :  1best file. \n".
                     "    -tf    :  test file. \n".
                     "    -odir  :  output direction.           [optional]\n".
                     "                 default generate xml file in current dir!\n".
                     "    -rnum  :  reference number. \n".
                     "    -rmoov :  remove OOV items.(0 or 1)   [optional]\n".
                     "                 default value is 0 (not remove OOV).\n".
                     "    -ien   :  input file encoder.         [optional]\n".
                     "                 default is \"utf8\"!\n". 
                     "[EXAMPLE]  :\n".
                     "    perl NiuTrans-generate-xml-for-mteval.pl -1f 1best -tf test-file -odir xml -rnum 1\n";
        exit( 1 );
    }
 
    my $pos;
    for( $pos = 0; $pos < scalar( @_ ); ++$pos )
    {
        my $key = $ARGV[ $pos ];
        ++$pos;
        my $value = $ARGV[ $pos ];
        $param{ $key } = $value;
    }
    if( !exists $param{ "-1f" } )
    {
        print STDERR "Error: please assign -1f!\n";
        exit( 1 );
    }
    if( !exists $param{ "-tf" } )
    {
        print STDERR "Error: please assign -tf!\n";
        exit( 1 );
    }
    if( !exists $param{ "-odir" } )
    {
        $param{ "-odir" } = "./";
    }
    else
    {
        $param{ "-odir" } =~ s/\\/\//g;
        if( !($param{ "-odir" } =~ /.*\/$/) )
        {
            $param{ "-odir" } = $param{ "-odir" }."/";
        }
    }
    if( !exists $param{ "-rnum" } )
    {
        print STDERR "Error: please assign -rnum!\n";
        exit( 1 ); 
    }
    if( !exists $param{ "-ien" } )
    {
        $param{ "-ien" } = "utf8";
    }
    if( !exists $param{ "-rmoov" } )
    {
        $param{ "-rmoov" } = 0;
    }
}

######################################################
#deal with OOV and NULL
######################################################
sub dealOOV
{
    if( scalar( @_ ) != 2 )
    {
        print STDERR "Error: Parameters of dealOOV is not 2!\n";
        exit( 1 );
    }

    open( INPUTFILE, "<", $_[0] ) or die "Error: can not open file $_[0]\n";
    open( OUTPUTFILE, ">", $_[1] ) or die "Error: can not write file $_[1]\n";
    my $num = 0;
    while( <INPUTFILE> )
    {
        ++$num;
        s/[\r\n]//g;
#       print STDERR $_."\n";
        my @strTemp = split / +/, $_;
#       print STDERR @strTemp;
        #print STDERR "\n";

        my $res;

        foreach my $str ( @strTemp )
        {
            if( $str =~ /^<(.*)>$/ )
            {
                next if( $param{ "-rmoov" } == 1 );
                $res = $res.$1." ";
            }
            else
            {
                $res = $res.$str." ";
            }
        }
        $res = $1 if $res =~ /^(.*) $/;
        print OUTPUTFILE $res."\n";
        print STDERR "\rDeal with OOV: process $num lines.";
    }
    close( INPUTFILE );
    close( OUTPUTFILE );
    print STDERR "\rDeal with OOV: process $num lines.\n";
}

######################################################
#generate tst.xml file which is encoded by utf8.
######################################################
sub generateTst
{
    return if( scalar( @_ ) < 3 );
    
    my $srcFile = $_[0];
    my $tstFile = $_[1];
    my $transNum = $_[2];
    
    open ( SRCFILE, "<".$srcFile ) or die "ERROR: Can't open file $srcFile";
    open ( TSTFILE, ">".$tstFile ) or die "ERROR: Can't open file $tstFile";

    print TSTFILE "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".
                  "<!DOCTYPE mteval SYSTEM \"ftp://jaguar.ncsl.nist.gov/mt/resources/mteval-xml-v1.3.dtd\">\n".
                  "<mteval>\n".
                  "\t<tstset setid=\"NiuTrans_example_set\" srclang=\"Chinese\" trglang=\"English\" sysid=\"NiuTrans\">\n".
                  "\t\t<doc docid=\"doc1\" genre=\"NiuTrans\">\n";
                  
    my $lineNum = 0;
    my $num = 0;
    while( <SRCFILE> )
    {
        s/[\r\n]//g;

        s/&/&amp;/g;
        s/</&lt;/g;
        s/>/&gt;/g;
        s/'/&apos;/g;
        s/"/&quot;/g;

        ++$lineNum;
        ++$num;
#        my $temp = $_;
        my $temp = encode( "utf8", decode( $param{ "-ien" }, $_ ) );

        print TSTFILE "\t\t\t<p>\n".
                      "\t\t\t\t<seg id=\"$num\">".$temp."</seg>\n".
                      "\t\t\t</p>\n";
    }

    print TSTFILE "\t\t</doc>\n".
                  "\t</tstset>\n".
                  "</mteval>\n";

    close( SRCFILE );
    close( TSTFILE );
    print STDERR "TST: $num\n";
}

######################################################
#generate Src.xml and Ref.xml file, all this two file is encoded by utf8.
######################################################
sub generateSrcAndRef
{
    return if( scalar( @_ ) < 4 );

    my $file = $_[0];
    my $srcFile = $_[1];
    my $refFile = $_[2];
    my $splitLineNum = $_[3] + 2;
    my @ref;

    open ( NBESTFILE, "<".$file ) or die "ERROR: Can't open file $file";

    open ( SRCFILE, ">".$srcFile ) or die "ERROR: Can't open file $srcFile";
    print SRCFILE "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".
                  "<!DOCTYPE mteval SYSTEM \"ftp://jaguar.ncsl.nist.gov/mt/resources/mteval-xml-v1.3.dtd\">\n".
                  "<mteval>\n".
                  "\t<srcset setid=\"NiuTrans_example_set\" srclang=\"Chinese\">\n".
                  "\t\t<doc docid=\"doc1\" genre=\"NiuTrans\">\n";

    open ( REFFILE, ">".$refFile ) or die "ERROR: Can't open file $refFile";
    print REFFILE "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".
                  "<!DOCTYPE mteval SYSTEM \"ftp://jaguar.ncsl.nist.gov/mt/resources/mteval-xml-v1.3.dtd\">\n".
                  "<mteval>\n";

    my $lineNum = 0;
    my $num = 0;
    while( <NBESTFILE> )
    {
        s/[\r\n]//g;

        s/&/&amp;/g;
        s/</&lt;/g;
        s/>/&gt;/g;
        s/'/&apos;/g;
        s/"/&quot;/g;

        ++ $lineNum;
        my $temp = encode( "utf8", decode( $param{ "-ien" }, $_ ) );
        if( $lineNum % $splitLineNum == 1)
        {
            ++$num;

            if( $temp =~ /( \|\|\|\| )/ )
            {
                $temp = $`;
            }
            print SRCFILE "\t\t\t<p>\n"."\t\t\t\t<seg id=\"$num\">".$temp."</seg>\n\t\t\t</p>\n";
            next;
        }
        elsif( $lineNum % $splitLineNum != 2 )
        {
            push @ref, $temp;
            if( $lineNum == $param{ "-rnum" } + 2 )
            {
                $lineNum = 0;
            }
            next;
        }
    }

#    print "#######:".scalar( @ref )."\n";

    print SRCFILE "\t\t</doc>\n".
                  "\t</srcset>\n".
                  "</mteval>\n";

    my $numOuter;
    for( $numOuter = 1; $numOuter < $splitLineNum - 1; ++ $numOuter )
    {
        print REFFILE "\t<refset setid=\"NiuTrans_example_set\" srclang=\"Chinese\" trglang=\"English\" refid=\"ref$numOuter\">\n".
                      "\t\t<doc docid=\"doc1\" genre=\"NiuTrans\">\n";

        my $num = $numOuter - 1;
        my $numInner = 1;
        for( ; $num < scalar( @ref ); $num += ( $splitLineNum - 2 ) )
        {
            print REFFILE "\t\t\t<p>\n".
                          "\t\t\t\t<seg id=\"$numInner\">$ref[$num]</seg>\n".
                          "\t\t\t</p>\n";
            ++ $numInner;
        }
        print REFFILE "\t\t</doc>\n".
                      "\t</refset>\n";  
    }  
    print REFFILE "</mteval>\n";
    close( NBESTFILE );
    close( SRCFILE );
    close( REFFILE );
    my $number = scalar( @ref ) /( $splitLineNum - 2 );
    print STDERR "REF AND SRC: $number\n";
}

######################################################
# call system command
######################################################
sub ssystem 
{
    print STDERR "Executing: @_\n";
    system(@_);
    if ($? == -1) 
    {
        print STDERR "ERROR: Failed to execute: @_\n  $!\n";
        exit(1);
    }
    elsif ($? & 127) 
    {
      printf STDERR "ERROR: Execution of: @_\n  died with signal %d, %s coredump\n",
          ($? & 127),  ($? & 128) ? 'with' : 'without';
      exit(1);
  }
    else
    {
        my $exitcode = $? >> 8;
        print STDERR "Exit code: $exitcode\n" if $exitcode;
        return ! $exitcode;
    }
}
