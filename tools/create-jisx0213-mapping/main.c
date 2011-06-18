#include <stdio.h>
#include <string.h>
#include <iconv.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

int char2hex(char in){
	switch(in){
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'A':
			return 10;
		case 'B':
			return 11;
		case 'C':
			return 12;
		case 'D':
			return 13;
		case 'E':
			return 14;
		case 'F':
			return 15;
	}
}

char *convertUCtoUTF8(char *in, char *pos){	
	char *toConvert = malloc(5*sizeof(char));
	char *ob = malloc(40*sizeof(char));
	int i,j;
	size_t inlen, outlen;
	char *inptr;
	char *outptr;
	iconv_t iv = NULL;

	if (strlen(in) > 5) {
		toConvert[0] = '.';
		toConvert[1] = 0;
		printf("echo -n \"\\\".\\\", \"\n");
		return toConvert;
	}
	
	char *toConvertIn = malloc(9*sizeof(char));
	memset(toConvertIn,0,9);
	for (i = 0; i < 8-strlen(in); i++){
		toConvertIn[i] = '0';
	}
	for (j = i; j < 8; j++){
		toConvertIn[j] = in[j-i];
	}
	//sscanf(toConvert,"%x%x%x%x",&toConvert[0],&toConvert[1],&toConvert[2],&toConvert[3]);
	/*
	for (i = 0; i < 8; i++)
		printf("%X",char2hex(toConvertIn[i]));
	*/
	toConvert[0] = (char2hex(toConvertIn[0]) << 4) | char2hex(toConvertIn[1]);
	toConvert[1] = (char2hex(toConvertIn[2]) << 4) | char2hex(toConvertIn[3]);
	toConvert[2] = (char2hex(toConvertIn[4]) << 4) | char2hex(toConvertIn[5]);
	toConvert[3] = (char2hex(toConvertIn[6]) << 4) | char2hex(toConvertIn[7]);
	toConvert[4] = 0;
//	printf("in: %s | toConvertIn: %s | toConvert: %s\n",in,toConvertIn,toConvert);
	
	outptr = ob;
	inptr = toConvert;
	
	memset(ob,0,40);

	inlen = 4;
	outlen = 40;

	/*iv = iconv_open("UTF-32BE","UTF-8");
	if (iv == NULL){
		printf("ERROR opening iconv.\n");
		return NULL;
	}
	iconv(iv,&inptr,&inlen,&outptr,&outlen);
	if (errno == EILSEQ){
		printf("iconv> Input conversion stopped due to an input byte that does not belong to the input codeset.\n"); 
	} else if (errno == E2BIG){
		printf("iconv> Input conversion stopped due to lack of space in the output buffer.\n"); 
	} else if (errno == EINVAL){
		printf("iconv> Input conversion stopped due to an incomplete character or shift sequence at the end of the input buffer.\n"); 
	} else if (errno == EBADF){
		printf("iconv> The cd argument is not a valid open conversion descriptor.\n");
	}*/
	
	//printf("\\x%02X\\x%02X\\x%02X\\x%02X -> %s\n",(uint8_t)toConvert[0],(uint8_t)toConvert[1],(uint8_t)toConvert[2],(uint8_t)toConvert[3],ob);
	printf("echo \"\\\"`echo -n -e \"\\x%02X\\x%02X\\x%02X\\x%02X\" | iconv -f UTF-32BE -t UTF-8`\\\", // %s\" \n",(uint8_t)toConvert[0],(uint8_t)toConvert[1],(uint8_t)toConvert[2],(uint8_t)toConvert[3],pos);
	return outptr;
}

int main(int argc, char **argv){
	FILE *f;
	char inbuf[1024];
	char param1[5];
	uint32_t lastcounter = 0;
	
	f = fopen("mapping.txt","r");
	printf("echo \"static const char x0215_mapping[][4] = {\"\n");
	while (!feof(f)){
		int len = 0;
		uint32_t curcounter;
		uint32_t i;
		memset(inbuf,0,1024);
		memset(param1,0,5);
		fgets(&inbuf,1024,f);
		if (strcmp(inbuf,"") != 0){
			char *lpcnt = malloc(4*sizeof(char));
			char *ucode = malloc((strlen(inbuf)-8)*sizeof(char));
			memset(lpcnt,0,4);
			memset(ucode,0,(strlen(inbuf)-8));
			while (((10+len) < strlen(inbuf)) && (inbuf[9 + len + 1] != ' ') && (inbuf[9 + len + 1] != '\t'))
				len++;
			len++;
			strncpy(lpcnt, &(inbuf[2]),4);
			strncpy(ucode, &(inbuf[9]),len);
			curcounter = ((char2hex(lpcnt[0]) << (4*3)) & 0xF000) | ((char2hex(lpcnt[1]) << (4*2)) & 0xF00) | ((char2hex(lpcnt[2]) << (4*1)) & 0xF0) | (char2hex(lpcnt[3]) & 0xF);
			//printf("lc: %X cc: %X\n",lastcounter,curcounter);
			for (i = lastcounter; i < curcounter; i++){
				printf("echo -n \"\\\".\\\", \"\n");
			}
			lastcounter=curcounter+1;
			//printf("lc: %X cc: %X | mapping -> %s = %s\n",lastcounter,curcounter,lpcnt,ucode);
			
			
			
			convertUCtoUTF8(ucode,lpcnt);
		}
		fflush(stdout);
	}
	printf("echo \"}\"\n");
	fclose(f);
}