#!/usr/bin/perl

use strict;
use warnings;

print STDERR "Usage: $0 <dev> <nref> <trans> <1 or 0>\n" unless @ARGV >= 4;

my $dev = shift;
my $nref = shift;
my $trans = shift;
my $onebest = shift;

open( DEV, "<$dev" ) or die "wrong: $!\n";
open( TS, "<$trans" ) or die "wrong: $!\n";

my $stoknum = 0;
my $sline = 0;
my $rtoknum = 0;
my $rline = 0;
my $ttoknum = 0;
my $tline = 0;

my $ts;

while( $ts = <TS> ) {
	$ts =~ s/[\r\n]//g;
	my @terms = split( / \|\|\|\| /, $ts );
	if( $onebest == 1 ) {
		my $tstr = $terms[0];
		my @toks = split( / /, $tstr );
		$ttoknum += @toks;
		++$tline;
		while( $ts = <TS> ) {
			$ts =~ s/[\r\n]//g;
			last if $ts =~ /^================$/;
		}
	}
	else {
		my $tstr = $terms[0];
		my @toks = split( / /, $tstr );
		$ttoknum += @toks;
		++$tline;
		while( $ts = <TS> ) {
			$ts =~ s/[\r\n]//g;
			last if $ts =~ /^================$/;
			@terms = split( / \|\|\|\| /, $ts );
			$tstr = $terms[0];
			my @toks = split( / /, $tstr );
			$ttoknum += @toks;
			++$tline;
		}
	}
}

while( $ts = <DEV> ) {
	$ts =~ s/[\r\n]//g;
	my @terms = split( / \|\|\|\| /, $ts );
	my @stoks = split( / /, $terms[0] );
	$stoknum += @stoks;
	++$sline;
	$ts = <DEV>;
	for( my $i = 0; $i < $nref; $i++ ) {
		$ts = <DEV>;
		$ts =~ s/[\r\n]//g;
		my @rtoks = split( / /, $ts );
		$rtoknum += @rtoks;
		++$rline;
	}
}

close( DEV );
close( TS );

my $tavg = $ttoknum / $tline;
my $savg = $stoknum / $sline;
my $ravg = $rtoknum / $rline;
my $t2s = $tavg / $savg;
my $t2r = $tavg / $ravg;
print STDERR "\n";
print STDERR ">> BEFORE DETOKENIZATION <<\n\n";
print STDERR "AVERAGE_TRANS_TOK_NUM\tAVERAGE_SRC_TOK_NUM\tAVERAGE_REF_TOK_NUM\n";
print STDERR "$tavg\t$savg\t$ravg\n\n";
print STDERR "T2S_RATIO\tT2R_RATIO\n";
print STDERR "$t2s\t$t2r\n";
print STDERR "\n";