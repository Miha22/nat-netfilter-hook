MODULE_NAME := nat_module
BUILD_DIR := build

obj-m += $(MODULE_NAME).o
KERNEL_DIR := /lib/modules/$(shell uname -r)/build
CC ?= gcc

all: build_dir kernel_module user_app

build_dir:
	mkdir -p $(BUILD_DIR)

kernel_module:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
	rm -rf $(BUILD_DIR)
