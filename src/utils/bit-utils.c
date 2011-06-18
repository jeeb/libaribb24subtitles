#include "../common.h"

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