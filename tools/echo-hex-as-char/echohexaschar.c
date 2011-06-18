#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
	if (argc < 2){
		printf("usage: %s hex[ hex]*\n",argv[0]);
		return 1;
	}
	int argp = 1;
	while (argc != argp){
		int a;
		sscanf(argv[argp],"%x",&a);
		//char a = (argv[argp][0] << 4) | (argv[argp][1] & 0x0F);
		printf("%c",a,a);
		argp++;
	}
	return 0;
}
