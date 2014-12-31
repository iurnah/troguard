
# builds the tracepoint example kernel modules;
# then to use one (as root):  insmod <module_name.ko>
CONFIG_SAMPLE_TRACEPOINTS=m
obj-$(CONFIG_SAMPLE_TRACEPOINTS) += tracepoint-sample.o
obj-$(CONFIG_SAMPLE_TRACEPOINTS) += tracepoint-probe-sample.o
obj-$(CONFIG_SAMPLE_TRACEPOINTS) += tracepoint-probe-sample2.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
