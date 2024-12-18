MODULE_NAME := nat_module
SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIR := include
KERNEL_DIR := /lib/modules/$(shell uname -r)/build
CC ?= gcc

obj-m := $(SRC_DIR)/$(MODULE_NAME).o

EXTRA_CFLAGS := -I$(PWD)/$(INCLUDE_DIR)

all: build_dir kernel_module

build_dir:
	mkdir -p $(BUILD_DIR)

kernel_module:
	make -C $(KERNEL_DIR) M=$(PWD)/$(BUILD_DIR) SRC_DIR=$(PWD)/$(SRC_DIR) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD)/$(BUILD_DIR) clean
	rm -rf $(BUILD_DIR)