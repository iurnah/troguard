BEGIN{
	FS="\n"
	RS=""
	ORS=""
}
{
	x=1
	while( x<NF ){
		print $x "\t"
		x++
	}
	print $NF "\n"
}
