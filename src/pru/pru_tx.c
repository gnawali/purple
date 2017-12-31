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

#include "resource_table_1.h"
#include <am335x/pru_cfg.h>
#include <am335x/pru_intc.h>
#include <am335x/pru_ctrl.h>
#include <pru_rpmsg.h>
#include <rsc_types.h>
#include <stdint.h>
#include <stdio.h>

volatile register uint32_t __R30;
volatile register uint32_t __R31;



#define ZERO_MASK 0xFFFFFF0F
#define ONE_MASK 0x000000F0

#define PACKET_CNT  100000

#define DELAY 4000 //25kHZ
//#define DELAY 100000  //1kHz
//#define PACKET_TIME 2*10*10*DELAY
//#define INTERVAL  (PACKET_TIME + 10*10*DELAY)
long packet_time = 0 ;
long packet_interval = 0 ;




// Host-0 Interrupt sets bit 30 in register R31
#define HOST_INT ((uint32_t)1 << 30)

// The PRU-ICSS system events used for RPMsg are defined in the Linux device
// tree
// PRU0 uses system event 16 (To ARM) and 17 (From ARM)
// PRU1 uses system event 18 (To ARM) and 19 (From ARM)
#define TO_ARM_HOST 18
#define FROM_ARM_HOST 19

// Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
// at linux-x.y.z/drivers/rpmsg/rpmsg_pru.c
// Use Channel 31 for the device driver.
#define CHAN_NAME "rpmsg-pru"
#define CHAN_DESC "Channel 31"
#define CHAN_PORT 31

// Used to make sure the Linux drivers are ready for RPMsg communication
// Found at linux-x.y.z/include/uapi/linux/virtio_config.h
#define VIRTIO_CONFIG_S_DRIVER_OK 4

uint8_t payload[RPMSG_BUF_SIZE];

#define PRU_SHAREDMEM 0x00010000


void modulate_manchester(uint8_t byte)
{
	uint8_t bitcount = 8;

	// send 10 as start bit
	__R30 |= ONE_MASK;
	__delay_cycles(DELAY);
	__R30 &= ZERO_MASK;
        __delay_cycles(DELAY);

	// send the byte
	while (bitcount--)
	{
		if (byte & 0x01) {
			__R30 &= ZERO_MASK;
			__delay_cycles(DELAY);
			__R30 |= ONE_MASK;
			__delay_cycles(DELAY);
		}
		else
		{
			__R30 |= ONE_MASK;
			__delay_cycles(DELAY);
			__R30 &= ZERO_MASK;
			__delay_cycles(DELAY);
		}
		byte >>= 1;
	}

	// send 01 as end bit
	__R30 &= ZERO_MASK;
        __delay_cycles(DELAY);
        __R30 |= ONE_MASK;
        __delay_cycles(DELAY);
}

void send_with_manchester(uint8_t* buff, uint8_t len)
{
	uint8_t i = 0;
	for (; i < len; ++i)
	{
		modulate_manchester(buff[i]);
	}
}



int main(void) {
  __R30 = 0; //  Clear the output pin.
  struct pru_rpmsg_transport transport;
  uint16_t src, dst, len, globe_len;
  volatile uint8_t *status;

	volatile uint32_t gpio;
	uint32_t send_timeout;
	uint32_t cpt;


	uint8_t tx_buff[100] = { 0xAA, 0xAA, 0xAA, 0xD5, 0x02};

  // Allow OCP master port access by the PRU so the PRU can read external
  // memories.
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

  // Clear the status of the PRU-ICSS system event that the ARM will use to
  // 'kick' us.
  CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

  // Make sure the Linux drivers are ready for RPMsg communication.
  status = &resourceTable.rpmsg_vdev.status;
  while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK))
    ;



  // Initialize the RPMsg transport structure.
  pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0,
                 &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);
  //  Block until the channel is established.
  while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC,
                           CHAN_PORT) != PRU_RPMSG_SUCCESS)
    ;
  //  The following code looks for a specific incoming message via RPMsg.
  //  This code blocks until the message is successfully received.
  //  If the correct message is received, the clock is allowed to begin.
  while (1) {
    /* Check bit 30 of register R31 to see if the ARM has kicked us */
    if (__R31 & HOST_INT) {
      /* Clear the event status */
      CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
      /* Receive all available messages, multiple messages can be sent per kick
       */
      if (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) ==
          PRU_RPMSG_SUCCESS) {
        if (payload[0] == 'g')
	{
	  for(int i=0; i < len - 2 ; i++)
          {
		tx_buff[i+5] = payload[i+1];
	  }
	  globe_len = len + 4;
	  tx_buff[len + 3] =  0x03;
	  packet_time = 2 * globe_len * 10 * DELAY;
	  packet_interval = packet_time + globe_len * 10 * DELAY; 

          break;
	}
      }
    }
  }

  //  The following is the code for the clock which is used to drive the SPI in
  //  PRU0.  The clock rate is approximately 8 kHz.

  //  The sample clock is located at shared memory address 0x00010000.
  //  Need a pointer for this address.  This is found in the linker file.
  //  The address 0x0001_000 is PRU_SHAREDMEM.
//  uint32_t *clockPointer = (uint32_t *)0x00010000;
//  *clockPointer = 0; //  Clear this memory location.


        cpt = 0;
	gpio = 0x00F0;
	//send_timeout = INTERVAL;	
	send_timeout = packet_interval;
	PRU1_CTRL.CTRL_bit.CTR_EN = 1;

	while (1)
	{
				
	       if (PRU1_CTRL.CYCLE_bit.CYCLECOUNT < 0xFFFFFFFF)
                {
			if (PRU1_CTRL.CYCLE_bit.CYCLECOUNT >= send_timeout)
			{
				send_with_manchester(tx_buff, globe_len);
				send_timeout = PRU1_CTRL.CYCLE_bit.CYCLECOUNT + packet_interval - packet_time;
				cpt = cpt+1;
			}
	       		if(cpt >= PACKET_CNT)
			{
				__halt();
			}	
               	}
		else // reset cycle counter if reached 0xFFFFFFFF (does not wrap around automatically)
		{
			PRU1_CTRL.CYCLE_bit.CYCLECOUNT = 0;
			PRU1_CTRL.CTRL_bit.CTR_EN = 1;
		}
		
	}




}
