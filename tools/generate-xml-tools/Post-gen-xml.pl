#!/usr/bin/perl

use strict;
use warnings;

sub usage {
	print STDERR "Usage:\t $0 [Options]...\n";
	print STDERR "\t -task                name of the task, e.g., \"zh_en_news_trans\"\n";
	print STDERR "\t                      NOTE: the task name should in format: <srclang>_<trglang>_<docid>_trans\n";
	print STDERR "\t -trans               translation results of the source sentences\n";
	print STDERR "\t -evaluate            evaluate the translation results [optional]\n";
	print STDERR "\t -dev                 development dataset in format: <src><CRLF><empty-line><CRLF><REFs> [optional]\n";
	print STDERR "\t -nref                number of references for every source sentence [optional]\n";
	print STDERR "\t -n                   generate nbest instead of 1best xml translation file [optional]\n";
	print STDERR "\t -c                   preserve case [optional]\n";
	print STDERR "\t -sr                  show length ratio [optional]\n";
}

# read in parameters
my %param = ();
if( @ARGV < 4 ) {
	usage();
	exit(1);
}
for( my $i=0; $i < @ARGV; $i++ ) {
	if( $ARGV[$i] eq "-task" ) {
		$param{"-task"} = $ARGV[$i+1];
		++$i;
	}
	elsif( $ARGV[$i] eq "-trans" ) {
		$param{"-trans"} = $ARGV[$i+1];
		++$i;
	}
	elsif( $ARGV[$i] eq "-evaluate" ) {
		$param{"-evaluate"} = "1";
	}
	elsif( $ARGV[$i] eq "-dev" ) {
		$param{"-dev"} = $ARGV[$i+1];
		++$i;
	}
	elsif( $ARGV[$i] eq "-nref" ) {
		$param{"-nref"} = $ARGV[$i+1];
		++$i;
	}
	elsif( $ARGV[$i] eq "-n" ) {
		$param{"-n"} = "1";
	}
	elsif( $ARGV[$i] eq "-c" ) {
		$param{"-c"} = "1";
	}
	elsif( $ARGV[$i] eq "-sr" ) {
		$param{"-sr"} = "1";
	}
}
if( !defined $param{"-task"} || !defined $param{"-trans"}
		|| ( defined $param{"-evaluate"} && ( !defined $param{"-dev"} || !defined $param{"-nref"} ) ) ) {
	usage();
	exit(1);
}

# open input and output files
my $trans = $param{"-trans"};
my $trgxml;
my $dev;
my $srcxml;
my $refxml;
if( defined $param{"-evaluate"} ) {
	$dev = $param{"-dev"};
	$srcxml = "src.xml";
	$refxml = "ref.xml";
	open( DEV, "<".$dev ) or die "can not open file ".$dev."\n";
	open( SRC, ">".$srcxml ) or die "can not create file ".$srcxml."\n";
	open( REF, ">".$refxml ) or die "can not create file ".$srcxml."\n";
}
if( defined $param{"-n"} ) {
	$trgxml = "nbest.xml";
}
else {
	$trgxml = "1best.xml";
}
open( TR, "<".$trans ) or die "can not open file ".$trans."\n";
open( TRG, ">".$trgxml ) or die "can not create file ".$srcxml."\n";

# generate xml files
my @tms = split( /_/, $param{"-task"} );
my $srclang = $tms[0];
my $trglang = $tms[1];
my $type = $tms[2];

my $stoknum = 0;
my $sline = 0;
my $rtoknum = 0;
my $rline = 0;
my $ttoknum = 0;
my $tline = 0;

if( defined $param{"-evaluate"} ) {
	# SRC
	# <?xml version="1.0" encoding="UTF-8"?>
	# <srcset setid="zh_en_news_trans" srclang="zh" trglang="en">
	# <DOC docid="news">
	print SRC "\xEF\xBB\xBF";
	print SRC "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	print SRC "<srcset setid=\"".$param{"-task"}."\" srclang=\"".$srclang."\" trglang=\"".$trglang."\">\n";
	print SRC "<DOC docid=\"".$type."\">\n";
	
	# REF
	# <?xml version="1.0" encoding="UTF-8"?>
	# <refset setid="zh_en_news_trans" srclang="zh" trglang="en">
	# <DOC docid="news" site="transorg1">
	print REF "\xEF\xBB\xBF";
	print REF "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	print REF "<refset setid=\"".$param{"-task"}."\" srclang=\"".$srclang."\" trglang=\"".$trglang."\">\n";
}

# TRANS
# <?xml version="1.0" encoding="UTF-8"?>
# <tstset setid="zh_en_news_trans" srclang="zh" trglang="en">
# <system site="system_nbest" sysid="Unlimited4Sys">
# NEU-NLPLab
# </system>
# <DOC docid="news">
print TRG "\xEF\xBB\xBF";
print TRG "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
print TRG "<tstset setid=\"".$param{"-task"}."\" srclang=\"".$srclang."\" trglang=\"".$trglang."\">\n";
print TRG "<system site=\"NEU-NLPLab\" sysid=\"NiuTrans\">\n";
print TRG "system description\n".
					"TO BE ADDED: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx".
					"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx".
					"xxxxxxxxxxxxxxxxxxxxxxxxxx\n";
print TRG "</system>\n";
print TRG "<DOC docid=\"".$type."\">\n";

# TRANS
my $id = 1;
my $ts;
while( $ts = <TR> ) {
	$ts =~ s/[\r\n]//g;
	if( $ts !~ m/^================$/ ) {
		## not an empty translation
		my @terms = split( / \|\|\|\| /, $ts );
		# 1best
		if( !defined $param{"-n"} ) {
			my $tstr = $terms[0];
			$tstr = NormalizeText($tstr);
			print TRG "<seg id=\"".$id."\">".$tstr."</seg>\n";
			my @toks = split( / /, $tstr );
			$ttoknum += @toks;
			++$tline;
			while( $ts = <TR> ) {
				$ts =~ s/[\r\n]//g;
				last if $ts =~ /^================$/;
			}
		}
		# nbest
		else {
			my $tstr = $terms[0];
			my $tscore = $terms[2];
			$tstr = NormalizeText($tstr);
			print TRG "<seg id=\"".$id."\">\n";
			print TRG "<cand score=\"".$tscore."\"> ".$tstr."</cand>\n";
			my @toks = split( / /, $tstr );
			$ttoknum += @toks;
			++$tline;
			while( $ts = <TR> ) {
				$ts =~ s/[\r\n]//g;
				last if $ts =~ /^================$/;
				@terms = split( / \|\|\|\| /, $ts );
				$tstr = $terms[0];
				$tscore = $terms[2];
				$tstr = NormalizeText($tstr);
				print TRG "<cand score=\"".$tscore."\"> ".$tstr."</cand>\n";
				my @toks = split( / /, $tstr );
				$ttoknum += @toks;
				++$tline;
			}
			print TRG "</seg>\n";
		}
	}
	else {
		## empty translation
		if( !defined $param{"-n"} ) {
			print TRG "<seg id=\"".$id."\"></seg>\n";
			++$tline;
		}
		else {
			print TRG "<seg id=\"".$id."\">\n";
			print TRG "<cand score=\"-INFI\"></cand>\n";
			print TRG "</seg>\n";
			++$tline;
		}
	}
	++$id;
}

if( defined $param{"-evaluate"} ) {
	# SRC and REF
	my @refs = ();
	my $nref = $param{"-nref"};
	for( my $i=0; $i < $nref; $i++ ) {
		$refs[$i] = "";
	}
	$id = 1;
	while( $ts = <DEV> ) {
		$ts =~ s/[\r\n]//g;
		my @terms = split( / \|\|\|\| /, $ts );
		$terms[0] = NormalizeText($terms[0]);
		print SRC "<seg id=\"".$id."\">".$terms[0]." </seg>\n";
		my @stoks = split( / /, $terms[0] );
		$stoknum += @stoks;
		++$sline;
		$ts = <DEV>;
		for( my $i = 0; $i < $nref; $i++ ) {
			$ts = <DEV>;
			$ts =~ s/[\r\n]//g;
			$ts = NormalizeText($ts);
			$ts = "<seg id=\"".$id."\">".$ts."</seg>\n";
			$refs[$i] .= $ts;
			my @rtoks = split( / /, $ts );
			$rtoknum += @rtoks;
			++$rline;
		}
		++$id;
	}
	for( my $refid = 0; $refid < $nref; $refid++ ) {
		print REF "<DOC docid=\"".$type."\" site=\"transorg".$refid."\">\n";
		print REF $refs[$refid];
		print REF "</DOC>\n";
	}
	print SRC "</DOC>\n</srcset>\n";
	print REF "</refset>\n";
	close( DEV );
	close( SRC );
	close( REF );
}

print TRG "</DOC>\n</tstset>\n";
close( TR );
close( TRG );

if( defined $param{"-sr"} ) {
	if( defined $param{"-evaluate"} ) {
		my $tavg = $ttoknum / $tline;
		my $savg = $stoknum / $sline;
		my $ravg = $rtoknum / $rline;
		my $t2s = $tavg / $savg;
		my $t2r = $tavg / $ravg;
		print STDERR "\n";
		print STDERR "AVERAGE_TRANS_TOK_NUM\tAVERAGE_SRC_TOK_NUM\tAVERAGE_REF_TOK_NUM\n";
		print STDERR "$tavg\t$savg\t$ravg\n";
		print STDERR "T2S_RATIO\tT2R_RATIO\n";
		print STDERR "$t2s\t$t2r\n";
	}
	else {
		my $tavg = $ttoknum / $tline;
		print STDERR "AVERAGE_TRANS_TOK_NUM\n";
		print STDERR "$tavg\n";
	}
}

sub NormalizeText {
    my $norm_text = shift;
    #return $norm_text;

    # language-dependent part (assuming Western languages):
    $norm_text = " $norm_text ";
    $norm_text =~ tr/[A-Z]/[a-z]/ unless defined $param{"-c"};
    $norm_text =~ s/([\{-\~\[-\` -\&\(-\+\:-\@\/])/ $1 /g;   # tokenize punctuation
    $norm_text =~ s/([^0-9])([\.,])/$1 $2 /g; # tokenize period and comma unless preceded by a digit
    $norm_text =~ s/([\.,])([^0-9])/ $1 $2/g; # tokenize period and comma unless followed by a digit
    $norm_text =~ s/([0-9])(-)/$1 $2 /g; # tokenize dash when preceded by a digit
    $norm_text =~ s/\s+/ /g; # one space only between words
    $norm_text =~ s/^\s+//;  # no leading space
    $norm_text =~ s/\s+$//;  # no trailing space
    
    # transformations of special characters
		$norm_text =~ s/&/&amp;/g;
    $norm_text =~ s/</&lt;/g;
    $norm_text =~ s/>/&gt;/g;
    $norm_text =~ s/\"/&quot;/g;
    $norm_text =~ s/\'/&apos;/g;

    return $norm_text;
}