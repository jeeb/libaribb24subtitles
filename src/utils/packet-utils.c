#include "../common.h"

void dumpPacket(uint8_t *data, uint8_t len, FILE* medium){			
	uint8_t i=0;
	uint8_t width = 16;
	char* tmp = malloc(7);
	memset(tmp,0,7);
	sprintf(tmp,"\n");
	fwrite(tmp,2,1,medium);
	while (i < len){
		sprintf(tmp,"0x%04x ",i);
		fwrite(tmp,7,1,medium);
		int j = i;
		while (j < i+width && j < len){
			/*char *bitstr = bitstrd_ui(data[j],8);
			fwrite(bitstr,strlen(bitstr),1,stderr);
			fwrite(" ",2,1,stderr);
			free(bitstr);*/
			sprintf(tmp,"%02x ",data[j]);
			fwrite(tmp,4,1,medium);
			j++;
		}
		i = j;
		fwrite("\n",2,1,medium);
	} 
	free(tmp);
}
