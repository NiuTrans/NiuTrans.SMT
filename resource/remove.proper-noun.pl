$lineNo = 0;
while($temp=<>)
{
    ++$lineNo;
	chomp($temp);
	$temp=~s/\{[^{]+\$loc[^}]+\}//g;
	$temp=~s/\{[^{]+\$person[^}]+\}//g;
	$temp=~s/\{[^{]+\$literal[^}]+\}//g;
	$temp=~s/\|\|\|\| $//;
	print STDOUT $temp."\n";
	print STDERR "\r  Processed $lineNo lines" if( $lineNo % 1000 == 0 );
}
print STDERR "\r  Processed $lineNo lines!\n";
