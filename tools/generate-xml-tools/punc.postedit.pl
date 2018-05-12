#!/usr/bin/perl
# this script is used only for Chinese translations !!!

use strict;
use warnings;

my $line;
while( $line = <STDIN> ) {
	$line =~ s/[\r\n]//g;
	my @terms = split( / \|\|\|\| /, $line );
	
	### eliminate spaces between Chinese characters
	$terms[0] =~ s/ //g;
	
	### punctuation substitution
	
	## rules for period
	$terms[0] =~ s/\.\.\./…/g;
	# period (。) at sentence end
	$terms[0] =~ s/\.$/。/g;
	# period (。) not after number, alphabet
	$terms[0] =~ s/([^\da-zA-Z\.])\./$1。/g;
	
	## rules for 《》
	$terms[0] =~ s/[^<]<<[^<]/《/g;
	$terms[0] =~ s/[^>]>>[^>]/》/g;
	
	## no rules for 、
	
	## rules for “”
	$terms[0] =~ s/\"([^\" ]+)\"/“$1”/g;
	$terms[0] =~ s/\"//g;
	#print STDERR $terms[0]."\n";
	
	### dump transformed line
	print "$terms[0]";
	for( my $i=1; $i < @terms; $i++ ) {
		print " |||| $terms[$i]";
	}
	print "\n";
}