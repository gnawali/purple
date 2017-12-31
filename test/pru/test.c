#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define THRESHOLD 1800

long temp_reg = 0;
unsigned int detected_word = 0;

char frame_buffer[38];
int frame_index = -1;
int frame_size = -1;

enum receiver_state{
	IDLE, //waiting for sync
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

int8_t is_a_word(long * manchester_word, int number_of_symbols, unsigned int* temp_detected_word)
{
//	printf("verifying word...\n");
	if(number_of_symbols >= 20)
	{
//		printf("we are detecting a word....\n");
		if(((*manchester_word) & START_STOP_MASK) == (START_STOP_MASK))
		{
			(*temp_detected_word) = ((*manchester_word)>> 2) & 0xFFFF;
			printf("START_STOP_MASK:detected_word: %d\n", (*temp_detected_word));
			if((*temp_detected_word) == SYNC_SYMBOL_MANCHESTER) 
			{
				printf("detected sync symbol...\n");	
				return 2;
			}
			return 1;  
		}	
//		else if(number_of_symbols == 20)
//		{
//			(*temp_detected_word) = ((*manchester_word) >> 2 ) & 0xffff;	
////			printf("detected_word: %d\n", (*temp_detected_word));
//			return 1;
//		}
	}
	return 0;
}

void print_binary(long number)
{
    if (number) {
        print_binary(number >> 1);
        putc((number & 1) ? '1' : '0', stdout);
    }
}


#define SAMPLE_PER_SYMBOL 4
int8_t detect_symbol( long * manchester_word, int symbol, int symbol_duration, int * time_from_last_sync, unsigned int * temp_detected_word )
{
	int word = 0;
	int is_a_word_value = 0;
	int sync_word_detect  = 0;
//	printf("detecting symbol...\n");
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
//		printf("time_from_last_sync: %d\n", (*time_from_last_sync));
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
//	printf("manchester_word: \n");
	//print_binary((* manchester_word));
//	printf("\nmanchester_word: \n");
	return word;
} 

int packet_construct(char * frame_buffer, int * frame_index, int * frame_size, enum receiver_state * frame_state,  unsigned char data)
{
	if(data == SYNC_SYMBOL){
		(*frame_index) = 0;
		(*frame_size) = 0;
		(*frame_state) = SYNC;
		return 0;
	}
	if((*frame_state) != IDLE)
	{
		frame_buffer[*frame_index] = data;
		(*frame_index) ++;
		if(data == STX){
			(*frame_state) = START;
			return 0;
		}
		else if(data == ETX){
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


int main()
{
    FILE* file = fopen("test.txt", "r");
    char line[100];
    int data = 0;
    int old_data = 0;
    int symbol_level = 0;
    int old_symbol_level = 0;
    int dist_last_sync = 0;
    int stable_samples = 0;
    int new_word = 0;
    int byte_added = 0;
	unsigned char received_data;

    while(fgets(line, sizeof(line), file)){
	//printf("%s  ", line);
	data = atoi(line);
//	printf("%d\n", data); 
//        if((data - old_data) > THRESHOLD)
//	    symbol_level = 1;
//        else if((old_data - data) > THRESHOLD)
//	    symbol_level = -1; 
//        else
//	    symbol_level = 0;
        if(data > old_data) 
	    symbol_level = 1;
        else if(old_data > data) 
	    symbol_level = -1; 
        else
	    symbol_level = 0;
        old_data = data;	
          

        if(symbol_level == 0 || symbol_level == old_symbol_level || (symbol_level != old_symbol_level && stable_samples < 2) )
    	{
//		printf("comparing symbol...\n");
		if(stable_samples < (4 * 4)) // assume the samples per symbol is 4
		{
			stable_samples++;
//			printf("stable_samples...%d\n", stable_samples);
		}
    	}    
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

	if(new_word == 1){
		
//		printf("if we detected a word...\n");	
		received_data = 0 ;
//		printf("detected_word: \n");
//		print_binary(detected_word);
//		printf("\ndetected_word: \n");
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
		//print_binary(received_data);
		new_word = 0;
//		printf("\n %c \n", received_data);
		if((byte_added = packet_construct(frame_buffer, &frame_index, &frame_size, &frame_state, received_data)) > 0)
		{
			frame_buffer[frame_size-1] = '\0';
			printf("%s\n", &(frame_buffer[1]));
		}
		
	}
    }

    fclose(file);
	
    return 0;

}
