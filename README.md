# What is this repository for #

This repository contains hardware and software for building practical Visible Light Communication system. The PURPLE board is a cape that works with BeagleBone Black embedded computer. With PURPLE, one can send and receive information using visible light.

![blink](/docs/assignment/images/blink.png)

The hardware design is available in the eagle folder.

### What's included ###
* Firmware running on PRU including both transmitter and receiver
* User-space program to run on embedded Linux environment 
* Hardware schematic
* Partlist to assemble your own cape
* PCB ordering

### Contents ###

* config/

 This directory contains all configuration files used before make.

* docs/

 This directory contains all documents and notes used as instructions and tutorials deployed in class.

* example/

 This directory contains the userspace program to send and receive msgs through light.

* firmware/

 This directory contains the binary files that's going to run on PRU

* pcb/

 This directory contains the hardware schematic and the PCB layout as well as the partlists.

* src/

 This directory contains the source file that needs to be compiled into the binary file under firmware folder.

* test/

 This directory contains the test and debugging file in user space for the PRU source code.

### How do I get set up? ###

- Summary of set up
    1. Hardware
    	- Beaglebone Black
    	- Purple Cape
    	- SD card
    	- Micro USB cable
    2. Software
	    - BBB Driver[[Windows]](user_spac://learn.adafruit.com/ssh-to-beaglebone-black-over-usb/installing-drivers-windows)[[Mac]](https://learn.adafruit.com/ssh-to-beaglebone-black-over-usb/installing-drivers-mac)[[Linux]](https://learn.adafruit.com/ssh-to-beaglebone-black-over-usb/installing-drivers-linux)
	    - [Ethcher](https://etcher.io/)
	    - SSH[[Windows]](https://learn.adafruit.com/ssh-to-beaglebone-black-over-usb/ssh-with-windows-and-putty)[[Mac/Linux]](https://learn.adafruit.com/ssh-to-beaglebone-black-over-usb/ssh-on-mac-and-linux) 
    3. Internet Connection (optional)
	    - Ethernet Cable (Required physical access to the LAN port of your router/access point)
	    - [Wireless Dongle](https://learn.adafruit.com/setting-up-wifi-with-beaglebone-black/overview)
    
* Configuration
* Dependencies
* Deployment instructions
  1. LED installation.
  2. Photodiode installation.

### How to get started ###

Step 1: Insert SD card to BBB board. Make sure SD card has been flashed with the purple image.

Step 2: Boot Up the BBB board and connect the board to your PC/Laptop. Make sure you have successfully installed the drivers for BBB.

Step 3: SSH to the board. ssh debian@192.168.7.2

```
ssh debian@192.168.7.2
```

Step 4: Reload the pru remoteproc kernel module. 
```

rmmod pru_rproc

modprobe pru_rproc

```

Step 5: Configure GPIOs.
```
./config/pru_gpio_configure
```

Step 6: Compile the code and deploy from the repository.
```
make clean
make
```

Step 7: Start listening on incoming messages.
``` 
./example/receive
```

Step 8: Open another SSH instance.
```
./example/send
```

Step 9: Modifying the data rate. 
You can specify the VALUE in makefile. Repeat Step 6-8 to see how it changes the light.

Step 10: Congratulations. You've completed the VLC tutorial.

### Who do I talk to? ###

* Shengrong Yin (syin@uh.edu) 
* Omprakash Gnawali (gnawali@cs.uh.edu) 


### References ###

[Exploring Beaglebone](http://exploringbeaglebone.com/chapter13/)

[PRU-ICSS](http://processors.wiki.ti.com/index.php/PRU-ICSS)

[PRU-software-support-package](https://git.ti.com/pru-software-support-package)

[spruhv7a](http://www.ti.com/lit/ug/spruhv7a/spruhv7a.pdf)

[arduino VLC](https://github.com/jpiat/arduino)

[PRU mailing list](https://groups.google.com/forum/#!forum/beagleboard)

[BeagleScope](https://github.com/ZeekHuge/BeagleScope)

[PRUDAQ](https://github.com/google/prudaq/wiki)



