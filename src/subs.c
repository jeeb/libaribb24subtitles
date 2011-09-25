#include "common.h"
#include "utils/string-utils.h"
#include "tables/ARIB_tables.h"
#include "tables/x0215_table.h"

#define ARIB_USE_HIRAGANA_TABLE 0x30
#define ARIB_USE_KATAKANA_TABLE 0x31
#define ARIB_USE_ALPHANUMERIC_TABLE 0x4A

char* textout; // UGLY GLOBAL VARIABLE WORKAROUND

int extractParameter(uint8_t *data, int len){
	int i, p;
	p = 0;
	for (i = 0; i < len; i++){
		p = p + ((data[i] & 0x0F) * pow(10,len-i-1));
	}
	return p;
}

char* parseARIBB24subtitleToASS(uint8_t* rawsubtitles, int rawsubtitles_length){
	// array position
	int pos = 0;
	
	// Current Character Table in use
	int current_table = 0;

	// debug variables
	int dumped_already = 0;

	// Error output stuff
	#define ERRBUF_MAX 2048
	FILE *errout = stdout;
	char *errbuf = malloc(ERRBUF_MAX);
	
	// work-array to be able to move around
	uint8_t* arr = rawsubtitles;
	
	// holds the last G charset we used.
	uint8_t lastcharset = 0;
	
	// Default character set definitions for locks (SS, LS)
	uint8_t g0charset = 0;
	uint8_t g1charset = ARIB_USE_KATAKANA_TABLE;
	uint8_t g3charset = 0;

	// UGLY GLOBAL VARIABLE WORKAROUND
	textout = malloc(1);
	char *assout = malloc(1);
	
	textout[0] = 0;
	assout[0] = 0;
	
	// initializing
	pos = 0;

	// starting mainloop
	while (pos < rawsubtitles_length){

		#ifdef DEBUG
			snprintf(errbuf,ERRBUF_MAX,"pos: %03d/%03d - char: %02X | ",pos,rawsubtitles_length,arr[pos]);
			fwrite(errbuf,strlen(errbuf),1,errout);
			fflush(errout);
		#endif

		#ifdef VERBOSE
			printf("%02X | ",arr[pos]); // output current processing hex
		#endif

		if (arr[pos] == 0x9B){
		
			#ifdef VERBOSE
				printf("CSI | ");
			#endif
			
			pos++;
			
			// extracting parameter
			int argstart = pos;
			int arglen = 0;

			while (arr[pos] != 0x3B && arr[pos] != 0x20 && (arr[pos] <= 0x52 || arr[pos] >= 0x70)){
				arglen++;
				pos++;
			}

			int P1 = extractParameter(&(arr[argstart]),arglen);
			
			#ifdef VERBOSE
				printf("P1: %d | ",P1);
			#endif
			
			pos++;
			
			// check type of command
			if (arr[pos-1] == 0x20 && arr[pos] > 0x52 && arr[pos] < 0x70){
				switch(arr[pos])
				{
					case 0x53:
						#ifdef VERBOSE
							printf("SWF | Set Writing Format");
						#endif
						break;
					case 0x54:
						#ifdef VERBOSE
							printf("CCC | Composite Character Composition");
						#endif
						break;
					case 0x58:
						#ifdef VERBOSE
							printf("SHS | Set Horizontal Spacing");
						#endif
						break;
					case 0x59:
						#ifdef VERBOSE
							printf("SVS | Set Vertical Spacing");
						#endif
						break;
					case 0x5D:
						#ifdef VERBOSE
							printf("GAA");
						#endif
						break;
					case 0x64:
						#ifdef VERBOSE
							printf("MDF");
						#endif
						break;
					case 0x65:
						#ifdef VERBOSE
							printf("CFS");
						#endif
						break;
					case 0x66:
						#ifdef VERBOSE
							printf("XCS");
						#endif
						break;
					case 0x68:
						#ifdef VERBOSE
							printf("PRA");
						#endif
						break;
					case 0x69:
						#ifdef VERBOSE
							printf("ACS");
						#endif
						break;
					case 0x6E:
						#ifdef VERBOSE
							printf("RCS | Raster Color command");
						#endif						
						break;
					default:
						snprintf(errbuf,ERRBUF_MAX,"libaribb24subtitles error: unknown 1 parametered CSI command: CSI P1: 0x%02X CMD: 0x%02X [@%d]\n",P1,arr[pos],pos);
						fwrite(errbuf,strlen(errbuf),1,errout);
						fflush(errout);
						break;
				}

				#ifdef VERBOSE
					printf("\n");
				#endif

				pos++;					
			} else if (arr[pos-1] == 0x9B){
				if (arr[pos] == 0x5B){
					#ifdef VERBOSE
						printf("PLD");
					#endif
				} else if (arr[pos] == 0x5C){
					#ifdef VERBOSE
						printf("PLU");
					#endif
				}
				#ifdef VERBOSE
					printf("\n");
				#endif
				
				pos++;
			} else if (arr[pos] == 0x6F){
				#ifdef VERBOSE
					printf("SCS");
				#endif
				
				#ifdef VERBOSE
					printf("\n");
				#endif
				pos++;
			} else if (arr[pos-1] == 0x20 || arr[pos-1] == 0x3B){
				argstart = pos;
				arglen = 0;
				while (arr[pos] != 0x3B && arr[pos] != 0x20 && (arr[pos] <= 0x52 || arr[pos] >= 0x70)){
					arglen++;
					pos++;
				}
				int P2 = extractParameter(&(arr[argstart]),arglen);
				//printf("argstart: %d - arglen: %d - P2: %d\n",argstart,arglen,P2);

				#ifdef VERBOSE
					printf("P2: %d | ",P2);
				#endif
				
				pos++;
				if (arr[pos-1] == 0x20){
					switch(arr[pos]){
						case 0x61:
							{
							#ifdef VERBOSE
								printf("ACPS | Active Coordinate Position Set");
							#endif
							char* tmp = malloc(1024);
							snprintf(tmp,1024,"{\\pos(%d,%d)}",P1,P2);
							assout = append_text_string(assout,tmp);
							free(tmp);
							break;
							}
						case 0x56:
							#ifdef VERBOSE
								printf("SDF");
							#endif
							break;
						case 0x57:
							#ifdef VERBOSE
								printf("SSM | Character composition dot disgnation");
							#endif
							break;
						case 0x5F:
							#ifdef VERBOSE
								printf("SDP");
							#endif
							break;
						case 0x42:
							#ifdef VERBOSE
								printf("GSM");
							#endif
							break;
						case 0x5E:
							#ifdef VERBOSE
								printf("SRC");
							#endif
							break;
						case 0x63:
							#ifdef VERBOSE
								printf("ORN");
							#endif
							break;
						default:
							snprintf(errbuf,ERRBUF_MAX,"unknown 1 parametered CSI command: CSI P1: 0x%02X P2: 0x%02X CMD: 0x%02X [@%d]\n",P1,P2,arr[pos],pos);
							fwrite(errbuf,strlen(errbuf),1,errout);
							fflush(errout);
							break;
					}
					#ifdef VERBOSE
						printf("\n");
					#endif
					pos++;
				}
			}
		} else if (arr[pos] == 0x00){
			pos++;
			#ifdef VERBOSE
				printf("NUL | Null | Skipping\n");
			#endif
		} else if (arr[pos] == 0x0F){
			#ifdef VERBOSE
				printf("LS0 | Lock Shift G0 | Changing Character Set.\n");
			#endif
			pos++;
			current_table = g0charset;
		} else if (arr[pos] == 0x0E){
			#ifdef VERBOSE
				printf("LS1 | Lock Shift G1 | Changing Character Set.\n");
			#endif
			pos++;
			current_table = g1charset;
		} else if (arr[pos] == 0x1D){
			#ifdef VERBOSE
				printf("SS3 | single shift G3 @ 0x%X - 0x31\n",arr[pos+1]);
			#endif
			g3charset = arr[pos+1]-0x31;
			pos = pos + 2;
		} else if (arr[pos] == 0x89){
			#ifdef VERBOSE
				printf("MSZ | Middle Size\n");
			#endif
			assout = append_text_string(assout,"{\\fscx125\\fscy125}");
			pos++;
		} else if (arr[pos] == 0x88){
			#ifdef VERBOSE
				printf("SSZ | Small Size\n");
			#endif
			assout = append_text_string(assout,"{\\fscx75\\fscy75}");
			pos++;
		} else if (arr[pos] == 0x8A){
			#ifdef VERBOSE
				printf("NSZ | Normal Size\n");
			#endif
			assout = append_text_string(assout,"{\\fscx100\\fscy100}");
			pos++;
		} else if (arr[pos] == 0x87){
			#ifdef VERBOSE
				printf("WHF | White Foreground\n");
			#endif
			assout = append_text_string(assout,"{\\1c&HFFFFFF&}");
			pos++;
		} else if (arr[pos] == 0x86){
			#ifdef VERBOSE
				printf("CNF | Cyan Foreground\n");
			#endif
			assout = append_text_string(assout,"{\\1c&H00FFFF&}");
			pos++;
		} else if (arr[pos] == 0x83){
			#ifdef VERBOSE
				printf("YLF | Yellow Foreground\n");
			#endif
			assout = append_text_string(assout,"{\\1c&H00FF00&}");
			pos++;
		} else if (arr[pos] == 0x82){
			#ifdef VERBOSE
				printf("GRF | Green Foreground\n");
			#endif
			assout = append_text_string(assout,"{\\1c&HFFFF00&}");
			pos++;
		} else if (arr[pos] == 0x90){
			#ifdef VERBOSE
				printf("COL | Color Controls | ");
			#endif
			pos++;
			if (arr[pos] != 0x20){
				#ifdef VERBOSE
					printf("color control: 0x%02X | ",arr[pos]);
				#endif
				switch (arr[pos]){
					case 0x48:
						#ifdef VERBOSE
							printf("foreground color -> transparent.");
						#endif
						//assout = append_text_string(assout,"{\1a&HFF&}");
						break;
					case 0x49:
						#ifdef VERBOSE
							printf("foreground color -> half intensity red.");
						#endif
						break;
					case 0x4A:
						#ifdef VERBOSE
							printf("foreground color -> half intensity green.");
						#endif
						break;
					case 0x4B:
						#ifdef VERBOSE
							printf("foreground color -> half intensity yellow.");
						#endif
						break;
					case 0x4C:
						#ifdef VERBOSE
							printf("foreground color -> half intensity blue.");
						#endif
						break;
					case 0x4D:
						#ifdef VERBOSE
							printf("foreground color -> half intensity magenta.");
						#endif
						break;
					case 0x4E:
						#ifdef VERBOSE
							printf("foreground color -> half intensity cyan.");
						#endif
						break;
					case 0x4F:
						#ifdef VERBOSE
							printf("foreground color -> half intensity white.");
						#endif
						break;
					case 0x50:
						#ifdef VERBOSE
							printf("background color -> black.");
						#endif
						break;
					case 0x51:
						#ifdef VERBOSE
							printf("background color -> full intensity red.");
						#endif
						break;
					case 0x52:
						#ifdef VERBOSE
							printf("background color -> full intensity green.");
						#endif
						break;
					case 0x53:
						#ifdef VERBOSE
							printf("background color -> full intensity yellow.");
						#endif
						break;
					case 0x54:
						#ifdef VERBOSE
							printf("background color -> full intensity blue.");
						#endif
						break;
					case 0x55:
						#ifdef VERBOSE
							printf("background color -> full intensity magenta.");
						#endif
						break;
					case 0x56:
						#ifdef VERBOSE
							printf("background color -> full intensity cysn.");
						#endif
						break;
					case 0x57:
						#ifdef VERBOSE
							printf("background color -> full intensity white.");
						#endif
						break;
					case 0x58:
						#ifdef VERBOSE
							printf("background color -> transparent.");
						#endif
						break;
					case 0x59:
						#ifdef VERBOSE
							printf("background color -> half intensity red.");
						#endif
						break;
					case 0x5A:
						#ifdef VERBOSE
							printf("background color -> half intensity green.");
						#endif
						break;
					default:
						#ifdef VERBOSE
							printf("i don't care about the color, i can't put it in ass anyways.");
						#endif
						break;
				}
				pos++;
			}
			else {
				pos++;
				#ifdef VERBOSE
					printf("palette number: 0x%d",arr[pos]);
				#endif
				pos++;
			}			
			#ifdef VERBOSE
				printf("\n");
			#endif
		} else if (arr[pos] == 0x09){
			#ifdef VERBOSE
				printf("APF | Active Position Forward\n");
			#endif
			textout = append_text_string(textout," ");
			assout = append_text_string(assout," ");
			pos++;
		} else if (arr[pos] == 0x0C){
			#ifdef VERBOSE
				printf("CS | Clear Screen | Time to drink my own piss.\n");
			#endif
			//assout = append_text_string(assout,"(clear screen)");
			pos++;
		} else if (arr[pos] == 0x9D){
			#ifdef VERBOSE
				printf("TIME | time command\n");
			#endif
			pos++;
			if (arr[pos] == 0x20){
				pos++;
				
			} else if (arr[pos] == 0x28){
				pos++;
				
				pos++;
				pos++;
				pos++;
				pos++;
				pos++;
				pos++;
				pos++;
				pos++;
			}
			//assout = append_text_string(assout,"(timed command)");
			pos++;
		} else if (arr[pos] == 0x1B){
			#ifdef VERBOSE
				printf("ESC | Escape Sequence | ");
			#endif
			pos++;
			if (arr[pos] == 0x29){
				#ifdef VERBOSE
					printf("Switching to Charset | [%02X|%02X] ",arr[pos+1],arr[pos+2]);
				#endif
				pos++;
				if (arr[pos] == ARIB_USE_ALPHANUMERIC_TABLE){
					#ifdef VERBOSE
						printf("using alphanumeric table.");
					#endif
					current_table = ARIB_USE_ALPHANUMERIC_TABLE;
				} else if (arr[pos] == ARIB_USE_KATAKANA_TABLE) {
					#ifdef VERBOSE
						printf("using katakana table.");
					#endif
					current_table = ARIB_USE_KATAKANA_TABLE;
				} else {
					if (dumped_already == 0) { dumpPacket(arr,rawsubtitles_length); dumped_already = 1; }
					snprintf(errbuf,ERRBUF_MAX,"unknown ESC 0x29 command parameter: 0x%02X [@%d]\n",arr[pos],pos);
					fwrite(errbuf,strlen(errbuf),1,errout);
					fflush(errout);
				}
				pos++;
				
			} else if (arr[pos] == 0x6E){
				#ifdef VERBOSE
					printf("LS3 | Lock Shift to G3");
				#endif
				current_table = g3charset;
				//pos++;
				
			} else {
				if (dumped_already == 0) { dumpPacket(arr,rawsubtitles_length); dumped_already = 1; }
				snprintf(errbuf,ERRBUF_MAX,"unknown ESC command: 0x%02X [@%d]\n",arr[pos],pos);
				fwrite(errbuf,strlen(errbuf),1,errout);
				fflush(errout);				
				//pos++;
			}
			#ifdef VERBOSE
				printf("\n");
			#endif
			pos++;
		} else if (arr[pos] == 0x20){ 
			#ifdef VERBOSE
				printf("SP | Space");
			#endif

			textout = append_text_string(textout," ");
			assout = append_text_string(assout," ");
			#ifdef VERBOSE
				printf("\n");
			#endif
			pos++;
		} else if (arr[pos] == 0x00){
			#ifdef VERBOSE
				printf("NUL | NULL command");
			#endif

			pos++;
		} else {
			if ((arr[pos] >= 0x00 && arr[pos] < 0x1F )|| (arr[pos] > 0x7F && arr[pos] < 0xA0) || arr[pos] >= 0xFE) {
				if (dumped_already == 0) { dumpPacket(arr,rawsubtitles_length); dumped_already = 1; }
				snprintf(errbuf,ERRBUF_MAX,"unparsed command: 0x%04X/%d [@%d/%02X]\n",arr[pos],arr[pos],pos,pos);
				fwrite(errbuf,strlen(errbuf),1,errout);
				fflush(errout);	
				pos++;
			} else if (current_table != 0){
				if (current_table == ARIB_USE_KATAKANA_TABLE){
					uint32_t arrelm = 0x2500 | arr[pos];
					#ifdef DEBUG
						printf("(%X) ",arrelm);
						printf("%s",x0215_mapping[arrelm]);
					#endif
					#ifdef VERBOSE
						printf("CHAR | appended %X/%s",arrelm,x0215_mapping[arrelm]);
					#endif
					textout = append_text_string(textout,x0215_mapping[arrelm]);
					assout = append_text_string(assout,x0215_mapping[arrelm]);
					
					if (strcmp(x0215_mapping[arrelm],".") == 0){
						if (dumped_already == 0) { dumpPacket(arr,rawsubtitles_length); dumped_already = 1; }
						snprintf(errbuf,ERRBUF_MAX,"undefined KATAKANA char: 0x%04X/%d [@%d/%02X]\n",arrelm,arrelm,pos,pos);
						fwrite(errbuf,strlen(errbuf),1,errout);
						fflush(errout);		
					}
				}
				else if (current_table == ARIB_USE_ALPHANUMERIC_TABLE){
					#ifdef DEBUG
						printf("(%X) %c",arr[pos],alphanumeric_graphical_set[arr[pos]]);
					#endif
					#ifdef VERBOSE
						printf("CHAR | appended %X/%c",arr[pos],alphanumeric_graphical_set[arr[pos]]);
					#endif

					textout = append_text_char(textout,alphanumeric_graphical_set[arr[pos]]);
					assout = append_text_char(assout,alphanumeric_graphical_set[arr[pos]]);

					if (alphanumeric_graphical_set[arr[pos]] == '.'){
						if (dumped_already == 0) { dumpPacket(arr,rawsubtitles_length); dumped_already = 1; }
						snprintf(errbuf,ERRBUF_MAX,"undefined ALPHANUMERIC char: 0x%04X/%d [@%d/0x%02X]\n",arr[pos],arr[pos],pos,pos);
						fwrite(errbuf,strlen(errbuf),1,errout);
						fflush(errout);		
					}

				}
				else if (current_table == ARIB_USE_HIRAGANA_TABLE){
					uint32_t arrelm = 0x2400 | arr[pos];
					#ifdef DEBUG
						printf("(%X) ",arrelm);
						printf("%s",x0215_mapping[arrelm]);
					#endif
					#ifdef VERBOSE
						printf("CHAR | appended %X/%s",arrelm,x0215_mapping[arrelm]);
					#endif					
					textout = append_text_string(textout,x0215_mapping[arrelm]);
					assout = append_text_string(assout,x0215_mapping[arrelm]);
					
					if (strcmp(x0215_mapping[arrelm],".") == 0){
						if (dumped_already == 0) { dumpPacket(arr,rawsubtitles_length); dumped_already = 1; }
						snprintf(errbuf,ERRBUF_MAX,"undefined HIRAGANA char: 0x%04X/%d [@%d/%02X]\n",arrelm,arrelm,pos,pos);
						fwrite(errbuf,strlen(errbuf),1,errout);
						fflush(errout);		
					}

				}
			} else if ((arr[pos] > 0x1F && arr[pos] < 0x7F) || (arr[pos] > 0xA0 && arr[pos] < 0xFF)) {
				uint32_t arrelm = ((arr[pos] << 8) & 0xFF00) | arr[pos+1];
				#ifdef DEBUG
					printf("(%X) ",arrelm);
					printf("%s",x0215_mapping[arrelm]);
				#endif
				#ifdef VERBOSE
						printf("CHAR | appended %X/%s",arrelm,x0215_mapping[arrelm]);
				#endif				
				textout = append_text_string(textout,x0215_mapping[arrelm]);
				assout = append_text_string(assout,x0215_mapping[arrelm]);
				
				if (strcmp(x0215_mapping[arrelm],".") == 0){
					if (dumped_already == 0) { dumpPacket(arr,rawsubtitles_length); dumped_already = 1; }
					snprintf(errbuf,ERRBUF_MAX,"undefined KANJI char: 0x%04X/%d [@%d/%02X]\n",arrelm,arrelm,pos,pos);
					fwrite(errbuf,strlen(errbuf),1,errout);
					fflush(errout);		
				}


				pos++;
			} else {
				if (dumped_already == 0) { dumpPacket(arr,rawsubtitles_length); dumped_already = 1; }
				snprintf(errbuf,ERRBUF_MAX,"unhandled command: 0x%02X [@%d/%02X]\n",arr[pos],pos,pos);
				fwrite(errbuf,strlen(errbuf),1,errout);
				fflush(errout);
			}
			#ifdef VERBOSE
					printf("\n");
			#endif
			pos++;
		} 
	}
	return assout;
}

char* parseARIBB24subtitleToText(uint8_t* rawsubtitles, int rawsubtitles_length){
	parseARIBB24subtitleToASS(rawsubtitles, rawsubtitles_length);
	//char* assout = parseARIBB24subtitleToASS(rawsubtitles, rawsubtitles_length);
	//if (arr[pos] == '{' && arr[pos+1] == '\\') { delete until '}' }
	return textout;
}
