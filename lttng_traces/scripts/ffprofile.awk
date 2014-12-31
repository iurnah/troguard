BEGIN {
    #migrate[]  
}
/firefox/{

    if( $3=="sys_execve:" ){
        print "Executed from: " $12
    }else if( $3=="sys_access:" ){    #{ filename = "/etc/ld.so.nohwcap", mode = 0 }
        print "File accessed: " $12
    }else if( $3=="sys_chmod:" ){     #{ filename = "/home/rui/.config/ibus/bus", mode = 448 }
        print "File accessed: " $12   
    }else if( $3=="sys_faccessat:" ){ #{ dfd = -100, filename = "/usr/bin/firefox", mode = 1 }
        print "File accessed: " $15
    }else if( $3=="sys_inotify_add_watch:" ){ #{ fd = 19, pathname = "/home/rui/.config/ibus/bus", mask = 16789454 }
        print "File accessed: " $15
    }else if( $3=="sys_mkdir:" ){ #{ pathname = "/home/rui/.mozilla/firefox/u1qjreel.default", mode = 493 }
        print "File accessed: " $12
    }else if( $3=="sys_newlstat:" ){ #{ filename = "/home/rui/.mozilla/...."", statbuf = 0x7FFF93BAF350 }
        print "File accessed: " $12
    }else if( $3=="sys_newstat:" ){ #{ filename = "/home/rui/.mozilla/...."", statbuf = 0x7FFF93BAF350 }
        print "File accessed: " $12
    }else if( $3=="sys_open:" ){ #{ filename = "/home/rui/.mozilla/firefox/extensions.sqlite", flags = 66, mode = 420 }
        print "File accessed: " $12
    }else if( $3=="sys_openat:" ){ # { dfd = -100, filename = "/usr/share/mozil
        print "File accessed: " $15
    }else if( $3=="sys_rmdir:" ){ #{ pathname = "/home/rui/.mozilla/firefox/u1qjreel.default/startupCache" }
        print "File accessed: " $12
    }else if( $3=="sys_statfs:" ){ #{ pathname = "/selinux", buf = 0x7FFFD635C320 }
        print "File accessed: " $12
    }else if( $3=="sys_unlink:" ){ #{ pathname = "/home/rui/.../startupCache.8.little" }
        print "File accessed: " $12
# The following is to abstrcat the dependency
    }else if( $3=="sys_symlink:" ){ #{ oldname = "...", newname = "/home/rui/.../lock" }
        print "Dependency11: " $12 " -> " $15
    }else if( $3=="sys_rename:" ){ #{ oldname = "/home/.../extensions.ini.tmp", newname = "/home/.../extensions.ini" }
        print "Dependency22: " $12 " -> " $15
    }else if( $3=="sched_process_fork:" ){
        print "Dependency33: " $12 " -> " $18 # $12 is parent_comm $18 is child_comm
    }else if ( $3=="sched_switch:" ){
        #if ( $12=="\"firefox\","){
        #    print "Dependency: " $12 " -> " $24
        #}else
        if ( $12=="\"dconf" && $13=="worker\","){
            print "sched_switch: " $12 "_" $13 " -> " $25
            #print $12 "_" $13 " -> " $25 ";"
        }else
            print "sched_switch: " $12 " -> " $24
            #print $12 " -> " $24 ";"
    }else if( $3=="sched_migrate_task:"){
#TODOs: add other syscalls for further info/profile.
    }
    #print $FFPID
}
END{
    #print $FFPID
}
