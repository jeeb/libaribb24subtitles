#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <types/pseudo-list.h>
#include <types/pesinfo.h>

void usage(char* binary_name){
	printf("usage: %s [-pcwv] -i filename\n",binary_name);
	printf("switches:\n");
	printf("  -i filename - opens \"filename\"\n");
	printf("  -o outfile  - writes to \"outfile\"\n");
	printf("  -v          - verbose\n");
	printf("\n");
}

int main(int argc, char** argv){
	char* filename = NULL;
	char* outfile = NULL;
	FILE *f;
	FILE *of;
	int c;
	int verbose = 0;
	
	while ((c = getopt(argc,argv,"xvmso:i:p:")) != -1){
		switch (c){
			case 'v':
				verbose++;
				break;
			case 'i':
				filename = optarg;
				if (verbose > 0)
					printf("set in filename to: %s\n",filename);
				break;
			case 'o':
				outfile = optarg;
				if (verbose > 0)
					printf("set out filename to: %s\n",filename);
				break;
			default:
				printf("\n");
				usage(argv[0]);
				abort();
		}
	}

	if (filename == NULL){
		usage(argv[0]);
		return 1;
	}
	if (outfile == NULL){
		of = stdout;
	} else {
		if (!(of = fopen(outfile,"w"))){
			printf("couldn't open outfile.\n");
			return 3;
		}
	}

	char *outdata = (char*)malloc(2048);
	
	char* data = (char*)malloc(188);
	int chk = 0;

	if (!(f = fopen(filename,"r"))){
		printf("couldn't open in file.\n");
		return 2;
	} 
	
	int oldsize = ftell(f);
	
	//sync to syncbyte
	while (!feof(f) && chk != 0x47){
		chk = fgetc(f);
	}
	
	int newsize = ftell(f);
	if (oldsize != newsize) { fseek(f,-1,SEEK_CUR); oldsize--; }

	if (feof(f)){
		fclose(f);
		printf("EOF before hitting mainloop.\n");
		return 3;
	}

	while (!feof(f)){
		uint8_t pos = 0;
		fread(data,188,1,f);
		struct PES_info *pi = ARIBB24_parseTSpacket(data);
		if (pi->exit_code == ARIBB24_NO_ERROR){
			if (pi->isARIBB24 != 1){
				clean_PES_info(pi);
				continue;
			}
			if (pi->isPES != 1){
				clean_PES_info(pi);
				continue;
			}
			fwrite(data,188,1,of);
			fflush(of);
		}
		clean_PES_info(pi);
	}

	fflush(of);
	if (of != stdout)
		fclose(of);
	fclose(f);
	return 0;
}  