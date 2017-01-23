#$(CC)=arm-linux-gcc
MODULE_NAME = kudp
MODULE_NAME2= kudpb
KDIR = ./

obj-m	:= $(MODULE_NAME).o
obj-m   += $(MODULE_NAME2).o
all: 
	@echo "making $(MODULE_NAME)"
	@echo "PWD=$(PWD)"
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
