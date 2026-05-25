
obj-m += my_sysfs.o 

all:
	make -C /lib/modules/$(shell uname -r)/build M=/home/murt/my_github/teonho/a/ma_nguon/my_sysfs modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=/home/murt/my_github/teonho/a/ma_nguon/my_sysfs clean

