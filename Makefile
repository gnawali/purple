#    Makefile for Beaglebone PRU VLC Project
#    Copyright (C) 2017  
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.



INCLUDE_PATHS=-I$(PRU_CGT)/includeSupportPackage -I$(PRU_CGT)/include -I$(PRU_SUPPORT)/include -I$(PRU_SUPPORT)/lib -I$(PRU_CGT)/lib
CFLAGS=$(INCLUDE_PATHS) --hardware_mac=on --c99
LNK_CMDS1=-z -i$(PRU_CGT)/lib -i$(PRU_CGT)/include -i$(PRU_CGT)/includeSupportPackage
LNK_CMDS2=-i$(PRU_CGT)/includeSupportPackage/am335x --reread_libs --stack_size=0x100
LNK_CMDS3=--heap_size=0x100 --library=$(PRU_SUPPORT)/lib/rpmsg_lib.lib

all: $(SOURCES)
	./config/pru_gpio_config >> /dev/null
	clpru $(CFLAGS) src/pru/pru_rx.c $(LNK_CMDS1) $(LNK_CMDS2) $(LNK_CMDS3) ./src/pru/AM335x_PRU.cmd -o ./firmware/am335x-pru0-fw --library=libc.a 
	clpru $(CFLAGS) src/pru/pru_tx.c $(LNK_CMDS1) $(LNK_CMDS2) $(LNK_CMDS3) ./src/pru/AM335x_PRU.cmd -o ./firmware/am335x-pru1-fw --library=libc.a 
	gcc -std=c99 ./example/receive.c -o ./example/receive
	gcc -std=c99 ./example/send.c    -o ./example/send
	cp ./firmware/am335x-pru0-fw /lib/firmware
	cp ./firmware/am335x-pru1-fw /lib/firmware
	./config/prumodout
	./config/prumodin

clean:
	rm ./example/receive
	rm ./example/send
	rm *.obj
	rm ./example/msg.log

