#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libaribb24captions.h>

uint8_t* append_to_array(uint8_t* oldarr, uint8_t newval, int len){
	uint8_t *newarr;
	int i;
	newarr = malloc(sizeof(uint8_t)*(len+1));
	for (i = 0; i < len; i++)
		newarr[i] = oldarr[i];
	newarr[len] = newval;
	return newarr;
}

int main(int argc, char** argv){
	int arrlen = 0;
	uint8_t *arr = NULL;
	char* textout;
	char* assout;
	int i;
	
	if (argc < 2){
		printf("usage: %s \"HEX-STRING\"\n",argv[0]);
		return 1;
	} else {
		unsigned int nv;
		while (sscanf(argv[1],"%X %*s",&nv) > 0){
			#ifdef DEBUG
				printf("read %x\n",nv);
			#endif
			arr = append_to_array(arr,nv,arrlen);
			arrlen++;
			argv[1] = &(*argv[1]++);
			argv[1] = &(*argv[1]++);
			argv[1] = &(*argv[1]++);
		}
	}
		
	#ifdef VERBOSE
		printf("Input from command line: ");
		for (i = 0; i < arrlen; i++){
			printf("%02X ",arr[i]);
		}
		printf("\n");
	#endif
	
	textout = parseARIBB24subtitleToText(arr,arrlen);
	assout = parseARIBB24subtitleToASS(arr,arrlen);
	printf("\n\nText:\n%s\n",textout);
	printf("\nASS-line:\n%s\n",assout);
	return 0;
}