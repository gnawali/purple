//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int soundfifo, pru_data, pru_clock; // file descriptors
    uint8_t sinebuf[490];
    ssize_t readpru, writepipe, prime_char, pru_clock_command;
    // Create a file
    soundfifo = open("msg.log", O_RDWR|O_CREAT, 0666);  
    if (soundfifo < 0)
      printf("Failed to open msg.log");
    printf("The node is currently listening on the light signals...\n");
	//  Open the character device to PRU0.
    pru_data = open("/dev/rpmsg_pru30", O_RDWR);
    if (pru_data < 0)
      printf("Failed to open pru character device rpmsg_pru30.");
    //  The character device must be "primed".
    prime_char = write(pru_data, "prime", 6);
    if (prime_char < 0)
      printf("Failed to prime the PRU0 char device.");
    //  This is the main data transfer loop.
    //  Note that the number of transfers is finite.
    //  This can be changed to a while(1) to run forever.
    for (int i = 0; i < 1; i++) {
      readpru = read(pru_data, sinebuf, 490);
      writepipe = write(soundfifo, sinebuf, 490);
      printf("%s\n", sinebuf);
    }
    //printf("The last value of readpru is %d and the last value of writepipe is "%d.\n", readpru, writepipe);
}
