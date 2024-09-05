obj-m += UniversalIO.o
UniversalIO-objs += ./src/main.o ./src/i2c_engine.o ./src/spi_engine.o ./src/uart_engine.o
CONFIG_MODULE_SIG=n
EXTRA_CFLAGS := -I$(src)/include
I2C_BUS = 4

reload: build
	@-sudo rmmod UniversalIO.ko 2>/dev/null
	@sudo modprobe ./UniversalIO.ko


build:
	@make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules -j$(shell nproc)

clean:
	@make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

sendTest:
	sudo i2ctransfer -y $(I2C_BUS) w6500@0x50 0x64-

style:
	@/lib/modules/$(shell uname -r)/build/scripts/checkpatch.pl -q --no-tree -f src/*.c include/*.h

dmesg:
	@sudo dmesg