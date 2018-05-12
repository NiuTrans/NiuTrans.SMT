$lineNo = 0;
if( scalar( @ARGV ) != 2 )
{
	print STDERR "Usage:\n".
		     "	perl convert.alignment.to.inv.pl Input-File Output-File!\n";
	exit( 1 );
}

open( INPUTFILE, "<", $ARGV[ 0 ] ) or die "Error: can not open file $ARGV[ 0 ] !\n";
open( OUTPUTFILE, ">", $ARGV[ 1 ] ) or die "Error: can not open file $ARGV[ 1 ]!\n";

while( <INPUTFILE> )
{
	++$lineNo;
	s/[\r\n]//g;
	@aligns = split / +/,$_;
	$first = 1;
	foreach $align ( @aligns )
	{
		if( $align =~ /(.*)-(.*)/ )
		{
			if( $first == 1 )
			{
				print OUTPUTFILE $2."-".$1;
				$first = 0;
			}
			else
			{
				print OUTPUTFILE " ".$2."-".$1;
			}
		}
	}
	print OUTPUTFILE "\n";
	print STDERR "\r$lineNo" if $lineNo % 10000 == 0;
}
print STDERR "\r$lineNo\n";

close( INPUTFILE );
close( OUTPUTFILE );
