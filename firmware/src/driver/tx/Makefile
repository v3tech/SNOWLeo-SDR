KERN_SRC=../../linux-kernel
obj-m := v3best-tx.o
 
all:
	make -C $(KERN_SRC) ARCH=arm M=`pwd` modules
clean:
	#make -C $(KERN_SRC) ARCH=arm M=`pwd=` clean
	rm modules.order Module.symvers *.o *.ko *.mod.c .*.cmd .tmp_versions -rf
