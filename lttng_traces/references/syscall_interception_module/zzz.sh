geany /var/log/syslog newcall.c  & 
make -C /usr/src/linux-headers-`uname -r`/ SUBDIRS=$PWD modules;
# "insmod newcall.ko"      "rmmod newcall.ko"
