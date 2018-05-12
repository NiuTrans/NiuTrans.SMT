while(<STDIN>){
	chomp();
	@terms = split(/ \|\|\|\| /, $_);
	if($terms[0] =~ /=====/){
		print "\n";
	}
	else{
		$terms[0] =~ s/\|//g;
		print $terms[0]."\n";
	}
	while($_ !~ /=======/){
		$_ = <STDIN>;
	}
}