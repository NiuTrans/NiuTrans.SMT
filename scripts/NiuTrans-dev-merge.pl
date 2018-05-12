#!/usr/bin/perl
if( @ARGV < 2)
{
	print STDERR "usage : perl merge.pl src tgt1 tgt2 ···\n";
	exit(0);
}
open C , $ARGV[0] or die "can not open $ARGV[0]\n";
for(my $i = 1 ; $i < @ARGV ; $i++)
{
	open ( $i , "<" , $ARGV[$i] ) or die "can not open $ARGV[$i]\n";
}
while( <C> )
{
	s/[\r\n]//g;
	print STDOUT "$_\n\n";
	for($i = 1; $i < @ARGV; $i++)
	{
		my $str = <$i>;
		$str =~ s/[\r\n]//g;
		print STDOUT "$str\n";
	}
}
close(C);
for(my $i = 1; $i < @ARGV; $i++)
{
	close($i);
}