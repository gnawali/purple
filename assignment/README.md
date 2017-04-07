# OpenVLC 1.0 Get Started
This is to introduce the embedded visible light communication system based on BeagleBone Black (Rev. C) [BBB](http://beagleboard.org/project/OpenVLC/). 

## What do you need to get started
* 2 x [BBB Rev.C ](https://beagleboard.org/black) 
* 2 x [OpenVLC Cape](http://nsl.cs.uh.edu/~syin/openvlc/cape1.png)
* 2 x [mini USB cable](http://nsl.cs.uh.edu/~syin/openvlc/miniUSB.jpg)
* 2 x [LED (Type 1)](http://www.vishay.com/docs/83178/tlcr5800.pdf)
* 2 x [LED (Type 2)]
* 2 x [Power Supply](http://elinux.org/images/7/72/DC_Supply.jpg).

## Hardware Assembly
1. Attach the cape on top of BBB.
2. Attach LED on the pinout of the cape. See the [instructions](http://nsl.cs.uh.edu/~syin/openvlc/led_installation.pdf)
3. Power up the board.
4. Connect the mini USB cable to your PC/Desktop USB port.

## Software Preparation
1. Installing the image:
   * **OpenVLC image**: 
   * Download the [image](https://drive.google.com/file/d/0BwGT2J3dvAfNOEVibS1KQ2d5MGc/view)
   * Install the image on a SD card. You can following the [Raspberry Pi example](https://www.raspberrypi.org/documentation/installation/installing-images/)
   * Insert the SD card to BBB and boot the OS.
   * Install the [header file](https://drive.google.com/file/d/0BwGT2J3dvAfNcVljMTFUU2phQmc/view?usp=sharing) on BBB.
     For this, you can ssh to the board
`ssh machinekit@192.168.7.2` and type
`dpkg -i linux-headers-3.8.13xenomai-bone53_1.0cross_armhf.deb`
   * Download the openvlc source code and compile it.
2. Check on your PC/Desktop to see whether you have USB connections after you assembled all hardware.
   * **For Mac Users**: Type `option` + `space` key in your keyboard to open network preference, you will observe all operating connections. Click on the BeagleBone Black connection to see the IP address. Normally, this would be `192.168.7.1`. Once you confirmed with IP, you can connect to the board via SSH. 
   * **For Linux Users**: Open Terminal, and type `ifconfig` to see available network connections. You will observe the 
USB connection with IP address of `192.168.7.1`. Connect to the board via SSH.
   * **For Windows Users**:  First, you need to install the BeagleBone [USB drivers](http://beagleboard.org/static/Drivers/Windows/BONE_D64.exe) for windows. Before you do that, you have to disable driver signature verification on 64-Bit Windows 8 or 10 so that you can install the USB driver which is unsigned. To disable driver signature verification, please follow option 2 in this [link](http://www.howtogeek.com/167723/how-to-disable-driver-signature-verification-on-64-bit-windows-8.1-so-that-you-can-install-unsigned-drivers/). Once you install the USB driver successfully, you can open `Command Line Tools` and type `ipconfig`. You will notice a IP which is `192.168.7.1` from
the result. This shows the USB driver is successfully installed. 
Second, download [Putty](http://www.chiark.greenend.org.uk/~sgtatham/putty/download.html) and connect from Windows to the BeagleBone board using Putty. 

## Connecting to the BeagleBone Board (SSH)
   * **For Mac and Linux Users**: By default, the OS should already have SSH installed. So you can open terminal and type the following command  `ssh machinekit@192.168.7.2`. The default password is `machinekit`.
   * **For Windows Users**: You need to configure Putty using the IP address `192.168.7.2`. You can following this [tutorial](https://mediatemple.net/community/products/dv/204404604/using-ssh-in-putty-) to configure the SSH connection in Putty.

## Data Transmission Configuration.
Once you are able to access to the BBB via SSH.  

`sudo -s #change to root`   
`cd openvlc1.0 # go to the folder`  
`./bat_client.sh # insert the openvlc kernel module and configure it to be TX`  
`./ipert_client.sh # run iperf tool to start transmitting data throught the visible light channel`  

## Data Reception Configuration.
Once you are able to access to the BBB via SSH.  
`sudo -s # change to root`  
`cd openvlc1.0 # go to the folder`  
`./bat_server.sh # insert the openvlc kernel module and configure it to be RX`
`./iperf_server.sh # run iperf tool to start receiving data throught the visible light channel`

## Visible Light Communication Verification
From the iperf server terminal, you will observe the following measurement result.

As we can see from the iperf measurement result, the visible light communication link is established so that
we can observe the loss ratio and the round-trip time estimation (RTT).

Still so excited, let's start to work on some tasks that can help you learn how the system is implemented and what improvements we can help to contribute.

# Homework 1 (How to compile C file in Embedded Linux Environment)

- Objective:  
  - You will learn how to compile a file in Linux Environment.
  - You will learn how to use networking tools in Linux Environment, such as iperf, nc e.t.c.

- Instructions:  
   - In openVLC 1.0, we have provided an example file for you to get started, you can type the following command
to go to the example folder, which is `openvlc1.0`.  
`cd openvlc1.0`  
Here, you will find the source files that are used to create the kernel module. Normally, the kernel module has
a filetype that is `***.ko`. And you can load this module into the kernel to drive the VLC transceivers. 
It is similar when you plug in a WIFI USB dongle. Kernel driver for this particular WIFI USB dongle will load firmware 
into the MCU of the device and the USB device will start to transmit or receive data. Here, the VLC transceivers was 
managed by only the kernel module with no firmware running on external MCU.
To compile the source file into the kernel module, you just need to type  
`make`   
Linux will automatically compile the source file into the vlc.ko.  
Once you have the `vlc.ko` module, you need to load it into kernel. We wrote the script -`bat_client.sh` to help you load 
ther kernel with default configurations. But currently, the system is half-duplex meaning you have to configure the transceiver as either TX or RX. Let's assume we configure the first device as TX, we need to type  
`./bat_client.sh`.  
Now we are able to send data using the VLC transceiver that has been configured as the TX by typing  
`iperf_client.sh`  
You will observe the LED blinking as the indication for data transmission.  
   - In this homework, you need to repeat the procedure from the descriptions and submit a screenshot with the data analytics you received from the iperf server result.  

# Homework 2 (How to write a new modulation/demodulation for openVLC)
- Objective:
   - You will learn how data was transmitted from a human readable message to the binary code that are sent in physical layer. 
   - You will learn Linux Kernel Programming by modifying the kernel source file.
   - You will learn basic modulation and demodulation techniques for data communication.
- Instructions:  
   - We have provided you a C file and a Makefile to help you get started. Check this [repository](https://github.com/yinshengrong/vlc_lab1) to be prepared with the tasks. This lab will help you understand the basic 
kernel programming techniques. It is sending a fixed number of bytes from Kernel by controlling the blinking frequency of
the LED. It is modulating bit ONE and ZERO with different frequency. The frequency for bit ONE is 3 kHz, bit ZERO is 1.5 kHz. By blinking the LED using these two frequencies, the Binary Frequency Shift Keying Modulation is established.  
   - In this homework, you need to modify this BFSK modulation into an [On and Off Keying Modulation with Manchester Coding](https://en.wikipedia.org/wiki/On-off_keying). You should be able to modulate bit ZERO using the HIGH-LOW trigger and modulate bit ONE using the LOW-HIGH trigger or vice versa. 

# Homework 3 (Wireshark)

# Homework 4 (Socket programming on VLC)

# Homework 5 (ARM and PRU assembly)
 

