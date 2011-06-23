#include "../common.h"

char* bitstrd_ui(void* a, uint8_t d){
	char* bitstr;
	char* addstr;
	bitstr = malloc(1);
	bitstr[0] = 0;
	addstr = malloc(2);
	addstr[0] = 0;
	addstr[1] = 0;	
	uint8_t i;
	
	
	for (i = 0; i < d; i++){
		if (d == 8){
			sprintf(addstr,"%d",((( uint8_t)a)>>((d-1)-i))&0x1);
		}else if (d == 16){
			sprintf(addstr,"%d",(((uint16_t)a)>>((d-1)-i))&0x1);
		}else if (d == 32){
			sprintf(addstr,"%d",(((uint32_t)a)>>((d-1)-i))&0x1);
		}else{
			sprintf(addstr,"%d",(((uint64_t)a)>>((d-1)-i))&0x1);		
		}
		bitstr = append_text_string(bitstr,addstr);
	}
	return bitstr;
}

void bitout_ui8(uint8_t a){
	int i;
	for (i = 0; i < 8; i++){
		printf("%d",(a>>(7-i))&0x1);
	}
}

void bitout_ui16(uint16_t a){
	int i;
	for (i = 0; i < 16; i++){
		printf("%d",(a>>(15-i))&0x1);
	}
}
void bitout_ui32(uint32_t a){
	int i;
	for (i = 0; i < 32; i++){
		printf("%d",(a>>(31-i))&0x1);
	}
}
uint16_t endian_fix16( uint16_t x )
{
    return (x<<8)|(x>>8);
}