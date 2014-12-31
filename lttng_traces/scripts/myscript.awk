BEGIN {
    FS=":"
}
{
    if ( $1 ~ /rui/ )
{
    print $1
}
}

{
    if( $1 == "foo" ){
        if( $2 == "foo" )  {
            print "undo"
        }else if ( $1 == "foo" ){
            print "one"
        }
    }else if ( $1 == "bar" ){
            print "two"          
    }else {
            print "threre"
            
        }
        
}
