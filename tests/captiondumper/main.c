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
	printf("  -p pid      - only prints captions from given pid\n");
	printf("  -m          - deactivate caption management output\n");
	printf("  -s          - deactivate caption statement output\n");
	printf("  -v          - verbose\n");
	printf("  -x          - XML mode\n");
	printf("\n");
}

int main(int argc, char** argv){
	char* filename = NULL;
	char* outfile = NULL;
	FILE *f;
	FILE *of;
	int c;
	int verbose = 0;
	int print_cm = 1;
	int print_cs = 1;
	int use_pid_select = 0;
	int pid_select = 0;
	int print_xml = 0;
	
	while ((c = getopt(argc,argv,"xvmso:i:p:")) != -1){
		switch (c){
			case 'v':
				verbose++;
				break;
			case 'x':
				print_xml = 1;
				break;
			case 'm':
				print_cm = 0;
				break;
			case 's':
				print_cs = 0;
				break;
			case 'p':
				pid_select = atoi(optarg);
				use_pid_select = 1;
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

	if (print_xml == 1){
		snprintf(outdata,2048,"<xml>\n\t<appdata>\n\t\t<name>libaribb24captions-caption-dump</name>\n\t\t<version>GIT</version>\n\t</appdata>\n\t<data>\n");
	} else {
		snprintf(outdata,2048,"libaribb24captions caption-dump\nversion: GIT\n\n");
	}
	fwrite(outdata,strlen(outdata),1,of);
	
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
				
			if (use_pid_select == 1) {
				if (pid_select != pi->header->PID){
					clean_PES_info(pi);
					continue;
				}
			}
			
			if (print_cm != 1 && pi->datagroup->type == ARIBB24_DATAGROUP_CAPTION_MANAGEMENT){
				clean_PES_info(pi);
				continue;
			}
			
			if (print_cs != 1 && pi->datagroup->type == ARIBB24_DATAGROUP_CAPTION_STATEMENT){
				clean_PES_info(pi);
				continue;
			}
			
			if (print_xml == 1){
				snprintf(outdata,2048,"\t\t<PTS><DTS>%s</DTS><PID>%d</PID><TYPE>%s</TYPE>",parsePTStoString(pi->DTS),pi->header->PID, (pi->datagroup->type == ARIBB24_DATAGROUP_CAPTION_MANAGEMENT)?"CM":"CS");
			} else {
				snprintf(outdata,2048,"@%s[%d][%s]->[",parsePTStoString(pi->DTS),pi->header->PID, (pi->datagroup->type == ARIBB24_DATAGROUP_CAPTION_MANAGEMENT)?"CM":"CS");
			}
			fwrite(outdata,strlen(outdata),1,of);
				
			if (pi->datagroup->data_units_num > 0){
				struct list_element* l = pi->datagroup->data_units;
				while (l != NULL) {
					struct ARIB_data_unit *j = l->element_data;
/*					int i;
					for (i = 0; i < j->size; i++) {
						printf("%02X ",j->payload[i]);
					}*/
					if (print_xml == 1)
						snprintf(outdata,2048,"<PAYLOAD>%s</PAYLOAD>",parseARIBB24subtitleToText(j->payload,j->size));
					else
						snprintf(outdata,2048,"%s",parseARIBB24subtitleToText(j->payload,j->size));
						
					fwrite(outdata,strlen(outdata),1,of);
					l=l->next;
					if (l != NULL && print_xml != 1){
						snprintf(outdata,2048,"],[");
						fwrite(outdata,strlen(outdata),1,of);
					}
				}
			}
			
			if (print_xml == 1){
				snprintf(outdata,2048,"</PTS>");
			}else{
				snprintf(outdata,2048,"]");
			}
			fwrite(outdata,strlen(outdata),1,of);
			snprintf(outdata,2048,"\n");
			fwrite(outdata,strlen(outdata),1,of);
			fflush(of);
		}
		clean_PES_info(pi);
	}
	if (print_xml == 1){
		snprintf(outdata,2048,"\t</data>\n</xml>\n");
	} else {
		snprintf(outdata,2048,"\ndone.\n");
	}
	fwrite(outdata,strlen(outdata),1,of);

	fflush(of);
	if (of != stdout)
		fclose(of);
	fclose(f);
	return 0;
}