
//
#include "resource_table_0.h"
#include <am335x/pru_cfg.h>
#include <am335x/pru_intc.h>
#include <pru_rpmsg.h>
#include <rsc_types.h>
#include <stdint.h>
#include <stdio.h>


// Define remoteproc related variables.
#define HOST_INT ((uint32_t)1 << 30)

//  The PRU-ICSS system events used for RPMsg are defined in the Linux device
//  tree.
//  PRU0 uses system event 16 (to ARM) and 17 (from ARM)
//  PRU1 uses system event 18 (to ARM) and 19 (from ARM)
#define TO_ARM_HOST 16
#define FROM_ARM_HOST 17

//  Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
//  at linux-x.y.x/drivers/rpmsg_pru.c
#define CHAN_NAME "rpmsg-pru"
#define CHAN_DESC "Channel 30"
#define CHAN_PORT 30
#define PULSEWIDTH 300

//  Used to make sure the Linux drivers are ready for RPMsg communication
//  Found at linux-x.y.z/include/uapi/linux/virtio_config.h
#define VIRTIO_CONFIG_S_DRIVER_OK 4

//  Buffer used for PRU to ARM communication.
char payload[490];

#define PRU_SHAREDMEM 0x00010000

#define THRESHOLD 2000


volatile register uint32_t __R30; 
volatile register uint32_t __R31; 


uint32_t *clockPointer = (uint32_t *)0x00010000;


uint16_t dataCounter = 0;
long temp_reg = 0;
unsigned int detected_word = 0;


char frame_buffer[38];
int frame_index = -1;
int frame_size = -1;

enum receiver_state{
	IDLE,
	SYNC,
	START,
	DATA
};

enum receiver_state frame_state = IDLE;
#define ETX 0x03
#define STX 0x02
#define SYNC_SYMBOL 0xd5
#define START_SYMBOL 0x02
#define STOP_SYMBOL 0x01
#define START_STOP_MASK ((STOP_SYMBOL << 20 ) | (START_SYMBOL << 18) | STOP_SYMBOL)
#define SYNC_SYMBOL_MANCHESTER (0x6665)

int is_a_word(long * manchester_word, int number_of_symbols, unsigned int* temp_detected_word)
{
	if(number_of_symbols >= 20)
	{
		if(((*manchester_word) & START_STOP_MASK) == (START_STOP_MASK))
		{
			(*temp_detected_word) = ((*manchester_word)>> 2) & 0xFFFF;
			if((*temp_detected_word) == SYNC_SYMBOL_MANCHESTER) return 2;
			return 1;  
		}
	}
	return 0;
}

#define SAMPLE_PER_SYMBOL 4
int detect_symbol( long * manchester_word, int symbol, int symbol_duration, int * time_from_last_sync, unsigned int * temp_detected_word )
{
	int word = 0;
	int is_a_word_value = 0;
	int sync_word_detect  = 0;

	if(((*manchester_word)& 0x01) != symbol)
	{
 		if(symbol_duration > SAMPLE_PER_SYMBOL + 1)
		{
			unsigned char last_bit = (*manchester_word) & 0x01;
			(*manchester_word) = ((*manchester_word) << 1) | last_bit;
			(*time_from_last_sync) += 1;
			is_a_word_value = is_a_word(manchester_word, (*time_from_last_sync), temp_detected_word);
			if(is_a_word_value > 0 ){
				word = 1;
				(*time_from_last_sync) = 0;
				if(is_a_word_value > 1) sync_word_detect = 1; 
			}
		}
		//storing edge in word
		if(symbol < 0){
			(*manchester_word) = ((*manchester_word) << 1 ) | 0x00;
		}
		else{
			(*manchester_word) = ((*manchester_word) << 1 ) | 0x01;	
		}
		(*time_from_last_sync) += 1;
		is_a_word_value = is_a_word(manchester_word, (*time_from_last_sync), temp_detected_word);
		if(sync_word_detect == 0 && is_a_word_value > 0 )
		{
			word = 1;
			(*time_from_last_sync) = 0;
		}
	}
	else
	{
		word = -1;
	}

	return word;
} 



//Receive Buffer, size can not be larger then MAX_RECEIVE_BUFFER.

//void detect_preamble(int16_t * symbol_buffer, uint8_t len)
//{
	//here we decode every byte with 10 bits. Each bit was encoded 
	//with manchester coding for two symbols.
	//10 as bit ONE
	//01 as bit ZERO
	//first step; we convert the received samples to HIGH/LOW symbol
	//During this tep, we need to know how many smaples does each symbol
	//contain. This depends on the sampling rate and the symbol rate
	//transmitted by TX. Let's take one example, assume the symbol rate
	//from the TX is 1k symbols/second. In RX, we can use a sampling 
	//rate of 8k samples/second. That means, for each symbol there will
	//be 8 samples. For simplicity, we use direct sampling to make sure
	//each sample represents each symbol by using a sampling rate that 
	//equals to the symbol rate.	
//	for(int i =0; i < 10*len, i++ )
//	{
//		symbol_buffer[i] = manchester_decoding();
//	}
//}


uint32_t data = 0x00000000; //
uint32_t old_data = 0x00000000;

int read_adc(void){

    data = 0x00000000; //data initialization
    __R30 = __R30 & 0xFFFFFFDF; // CS signal goes to low
    for (int i = 0; i < 16; i++)
    {
        data = data << 1; //shift left;	
        __R30 = __R30 | 0x00000004; //  Clock goes high; 
        __R30 = __R30 | 0x00000004; //  Clock goes high; 
        __R30 = __R30 | 0x00000004; //  Clock goes high; 
        if (__R31 & (1 << 3)) //  Probe MISO data from ADC.
            data = data | 1;
        else
        {
            data = data & 0xFFFFFFFE;
	}    
        __R30 = __R30 & 0xFFFFFFFB; //  Clock to LOW   P9.30
        __R30 = __R30 & 0xFFFFFFFB; //  Clock to LOW   P9.30
    }
    //right shift two 0's , refer to datasheet.
    data = data >> 2;
    __R30 = __R30 | 1 << 5; //  Chip select to HIGH 
    return 0;
}

int packet_construct(char * frame_buffer, int * frame_index, int * frame_size, enum receiver_state * frame_state,  unsigned char rx_data)
{
	if(rx_data == SYNC_SYMBOL){
		(*frame_index) = 0;
		(*frame_size) = 0;
		(*frame_state) = SYNC;
		return 0;
	}
	if((*frame_state) != IDLE)
	{
		frame_buffer[*frame_index] = rx_data;
		(*frame_index) ++;
		if(rx_data == STX){
			(*frame_state) = START;
			return 0;
		}
		else if(rx_data == ETX){
			(*frame_size) = (*frame_index);
			(*frame_index) = -1;
			(*frame_state) = IDLE;
			return 1;
		}
		else{
			(*frame_state) = DATA;
		}
		return 0;
	}
	return -1;
}

int main(void) { 
  struct pru_rpmsg_transport transport;
  uint16_t src, dst, len;
  volatile uint8_t *status;

  //  1.  Enable OCP Master Port
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;
  //  Clear the status of PRU-ICSS system event that the ARM will use to 'kick'
  //  us.
  CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

  //  Make sure the drivers are ready for RPMsg communication:
  status = &resourceTable.rpmsg_vdev.status;
  while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK))
    ;

  //  Initialize pru_virtqueue corresponding to vring0 (PRU to ARM Host
  //  direction).
  pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0,
                 &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

  // Create the RPMsg channel between the PRU and the ARM user space using the
  // transport structure.
  while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC,
                           CHAN_PORT) != PRU_RPMSG_SUCCESS)
    ;
  //  The above code should cause an RPMsg character to device to appear in the
  //  directory /dev.
  //  The following is a test loop.  Comment this out for normal operation.

  //  This section of code blocks until a message is received from ARM.
  while (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) !=
         PRU_RPMSG_SUCCESS) {
  }

  __R30 = 0x00000000;         //  Clear the output pin.
  __R30 = __R30 & 0xFFFFFFFB; //  Clock to LOW   P9.30
  __delay_cycles(100); //  Allow chip to stabilize.
  int symbol_level = 0;
  int old_symbol_level = 0;
  int stable_samples= 0;
  int new_word = 0;
  int dist_last_sync = 0;
  int byte_added = 0;
  unsigned char received_data;
  char * test_msg = "hello world";
  while (1) {
    __delay_cycles(200);
    read_adc();
    if(data >= THRESHOLD)
	data = 1;
    else
	data = 0;

    if(data > old_data)
	symbol_level = 1;
    else if(old_data > data)
	symbol_level = -1;
    else
	symbol_level = 0;
    old_data = data;
	  
    //check whether there will be a falling or rising edge and save the edge. 
    if(symbol_level == 0 || symbol_level == old_symbol_level || (symbol_level != old_symbol_level && stable_samples < 2) )
    {
	if(stable_samples < (4 * 4)) // assume the samples per symbol is 4
	{
		stable_samples++;
	}
    }
    // if it is the rising or falling edge
    else
    {
         //we need to have a function to detect edge.
	 new_word = detect_symbol(&temp_reg, symbol_level, stable_samples, &(dist_last_sync), &detected_word);
	 if (dist_last_sync > (8 * 4)) //
	 {
		dist_last_sync = 32;
	 }
	 stable_samples = 0;	
    }
    old_symbol_level = symbol_level;

    if (new_word == 1){
	(*clockPointer) = 25;
	received_data = 0 ;
	for (int i =0; i < 16; i = i + 2){
		received_data = received_data << 1;
		if(((detected_word >> i) & 0x03) == 0x01){
			received_data |= 0x01;
		}
		else{
			received_data &= ~0x01;
		}
	}
	received_data = received_data & 0xff;
	new_word = 0;
	
	byte_added = packet_construct(frame_buffer, &frame_index, &frame_size, &frame_state, received_data);
	if(byte_added > 0)
	{
		frame_buffer[frame_size - 1] = '\0';
		pru_rpmsg_send(&transport, dst, src, &frame_buffer[1], 490);
	}

    }
  }
}
