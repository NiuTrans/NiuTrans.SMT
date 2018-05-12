#!/usr/bin/perl

use strict;
use warnings;

sub usage() {
	print STDERR "Usage:\t $0 [Options]...\n";
	print STDERR "\t -flist             a list of files to be evaluated, seperated by mark colon (;)\n";
	print STDERR "\t -task              task name, e.g., \"zh_en_news_trans\"\n";
	print STDERR "\t -ci                case insensitive [optional]\n";
	print STDERR "\t -rlist             a list of references, seperated by mark colon (;) [optional]\n";
	print STDERR "\t -nref              number of references for each source sentence\n [optional]";
	print STDERR "\t -nlist             a list of numbers specifying the line each file has, seperated by mark colon (;) [optional]\n";
	print STDERR "\t NOTE: number of reference files and numbers should be the same!\n";
	print STDERR "\t -1best             specify the translation result is of 1-best format (default, n-best format) [optional]\n";
	print STDERR "\t -evaluate          evaluate the translation results [optional]\n";
	print STDERR "\t -showratio         show ratio of translation length [optional]\n";
	print STDERR "Example:\n";
	print STDERR "\t $0 -flist f0 -task zh_en_news_trans\n";
	print STDERR "\t $0 -flist f0;f1;f2 -rlist rfile -nref 4 -task zh_en_news_trans\n";
	print STDERR "\t $0 -flist f0;f1;f2 -rlist r0;r1;r2 -nlist 334;1082;998 -nref 4 -task zh_en_news_trans\n";
	print STDERR "\t $0 -flist f0;f1;f2 -rlist rfile -nref 4 -task zh_en_news_trans -1best\n";
}

if( @ARGV < 4 ) {
	usage();
	exit(1);
}

# read in parameters
my %params = ();
for( my $i = 0; $i < @ARGV; $i++ ) {
	if( $ARGV[$i] eq "-flist" ) {
		$params{"-flist"} = $ARGV[$i+1];
		++$i;
	}
	elsif( $ARGV[$i] eq "-rlist" ) {
		$params{"-rlist"} = $ARGV[$i+1];
		++$i;
	}
	elsif( $ARGV[$i] eq "-nlist" ) {
		$params{"-nlist"} = $ARGV[$i+1];
		++$i;
	}
	elsif( $ARGV[$i] eq "-nref" ) {
		$params{"-nref"} = $ARGV[$i+1];
		++$i;
	}
	elsif( $ARGV[$i] eq "-task" ) {
		$params{"-task"} = $ARGV[$i+1];
		++$i;
	}
	elsif( $ARGV[$i] eq "-1best" ) {
		$params{"-1best"} = 1;
	}
	elsif( $ARGV[$i] eq "-evaluate" ) {
		$params{"-evaluate"} = 1;
	}
	elsif( $ARGV[$i] eq "-showratio" ) {
		$params{"-showratio"} = 1;
	}
	elsif( $ARGV[$i] eq "-ci" ) {
		$params{"-ci"} = 1;
	}
}

if( !defined $params{"-flist"} || !defined $params{"-task"} || ( defined $params{"-evaluate"} && ( !defined $params{"-rlist"} || !defined $params{"-nref"} ) ) ) {
	usage();
	exit(1);
}

# generate all files to be evaluated
my @files = split(/;/, $params{"-flist"});
my @rfiles;
my @nums;
my $nref;
if( defined $params{"-evaluate"} ) {
	$nref = $params{"-nref"};
	@rfiles = split(/;/, $params{"-rlist"});
	@nums = split(/;/, $params{"-nlist"}) if defined $params{"-nlist"};
	if( defined $params{"-nlist"} and @rfiles != @nums ) {
		usage();
		exit(1);
	}
}
my @efiles = ();
foreach my $f (@files) {
	if( defined $params{"-nlist"} ) {
		open( IN, "<".$f ) or die "can not open file: ".$f."\n";
		foreach my $n (@nums) {
			open( OUT, ">$f.$n" ) or die "can not create file: $f.$n\n";
			my $nn = $n;
			while( $nn > 0 ) {
				my $in = <IN>;
				print OUT $in;
				--$nn if $in =~ m/^================$/;
			}
			close( OUT );
			push(@efiles, "$f.$n");
		}
		close( IN );
	}
	else {
		push(@efiles, $f);
	}
}

# evaluate all the generated files
my $task = $params{"-task"};
my $toZH = 0;
my @arr_task = split( /_/, $task );
if( ($arr_task[1] eq "zh") or ($arr_task[1] eq "ZH") ) {
	$toZH = 1;
}
my @scores = ();
for( my $i = 0; $i < @efiles; $i++ ) {
	my $ef = $efiles[$i];
	my $rf;
	if( defined $params{"-evaluate"} ) {
		if( !defined $params{"-nlist"} ) {
			$rf = $rfiles[0];
		}
		else {
			my $index = $i % @nums;
			$rf = $rfiles[$index];
		}
	}
	## list statistics of ratios of word number between source, target, reference
	if( defined $params{"-showratio"} ) {
		system("perl cal.len.ratio.pl $rf $nref $ef 1");
	}
	
	## post processing step ...
	print STDERR "Post-processing translation file $ef...\n";
	if( !defined $params{"-1best"} ) {
		# dev. format
		if( $toZH == 0 ) {
			# to English task
			system("perl extract.one.best.pl < $ef > $ef.1best");
			#system("perl slight_postprocess.pl < $ef.1best > $ef.post");
			#system("perl recase.pl -model recase-model/moses.ini -in $ef.post -moses moses-cmd.exe > $ef.rc");
			system("perl detokenizer.pl < $ef.1best > $ef.dtok");
			system("perl generate.nbest.pl < $ef.dtok > $ef.nbest");
		}
		else {
			# to Chinese task
			system("perl punc.postedit.pl < $ef > $ef.nbest");
		}
	}
	else {
		# test format
		if( $toZH == 0 ) {
			# to English task
			#system("perl slight_postprocess.pl < $ef > $ef.post");
			#system("perl recase.pl -model recase-model/moses.ini -in $ef.post -moses moses-cmd.exe > $ef.rc");
			system("perl detokenizer.pl < $ef > $ef.dtok");
			system("perl generate.nbest.pl < $ef.dtok > $ef.nbest");
		}
		else {
			# to Chinese task
			system("perl punc.postedit.pl < $ef > $ef.punc");
			system("perl generate.nbest.pl < $ef.punc > $ef.nbest");
		}
	}
	
	## generate xml files to be submitted
	print STDERR "Generate intermediate XML files...\n";
	my $cmd = "perl Post-gen-xml.pl -task $task -trans $ef.nbest";
	if( defined $params{"-evaluate"} ) {
		$cmd .= " -evaluate -dev $rf -nref $nref";
	}
	if( !defined $params{"-ci"} ) {
		$cmd .= " -c";
	}
	if( defined $params{"-showratio"} ) {
		$cmd .= " -sr";
	}
	system( $cmd );
	
	## remove temporary files
	if( $toZH == 0 ) {
		unlink "$ef.1best", "$ef.post", "$ef.rc", "$ef.dtok", "$ef.nbest", "lm.log";
	}
	else {
		unlink "$ef.punc", "$ef.nbest", "lm.log";
	}
}

