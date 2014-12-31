BEGIN{
#    print "dump firefox pid from lttng_statedump_process_state"
}
/firefox/ {
    if ( $3=="lttng_statedump_process_state:" || $30=="\"firefox\","){
        print $18
    }
}
