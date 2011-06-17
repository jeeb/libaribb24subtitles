#include "../common.h"

char* append_text_char(char* textin, char appendum){
	int newlen = strlen(textin)+1+1;
	char *z = malloc(sizeof(char)*(newlen));
	memset(z,0,newlen);
	snprintf(z,newlen,"%s%c",textin,appendum);
	return z;
}

char* append_text_string(char* textin,const char* appendum){
	int newlen = strlen(textin)+strlen(appendum)+1;
	char *z = malloc(sizeof(char)*(newlen));
	memset(z,0,newlen);
	snprintf(z,newlen,"%s%s",textin,appendum);
	#ifdef DEBUG
		printf("size: %d, textin: %s, appendum: %s, z: %s\n",newlen,textin,appendum,z);
	#endif
	return z;
}
