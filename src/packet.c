#include "common.h"

struct PES_info *ARIBB24_parseTSpacket(uint8_t* data){
	uint8_t pos = 0;
	
	struct PES_info *pes_packet = initialize_PES_info();
	pes_packet->header->PID = (data[1] & 0x1F) << 8 | (data[2] & 0xff);
	pes_packet->header->PUSI = (data[1] & 0x40) >> 6;
	pes_packet->header->AFC = (data[3] & 0x30) >> 4;
	
	pos = pos + 4;
	
	if (pes_packet->header->AFC == 0x02 || pes_packet->header->AFC == 0x03){
		pos = pos + data[pos] + 1;
	}
	
	if (pes_packet->header->PUSI == 0x01 && (((data[pos] << 16) & 0xFF0000) | ((data[pos+1] << 8) & 0xFF00) | (data[pos+2] & 0xFF)) == 0x000001) {
		pes_packet->isPES = 1;
		uint32_t i;
		pes_packet->PPL = ((data[pos+4] & 0xFF00) << 8) | (data[pos+5] & 0xFF);
		pes_packet->SID = (data[pos+3]);
		uint16_t PPL = pes_packet->PPL;
		pos = pos + 6;
		
		pes_packet->PTSDTSF  = (data[pos+1] & 0xC0) >> 6;
		
		if (pes_packet->PTSDTSF == 0x2)
			pes_packet->DTS = cut_array(&(data[pos+3]),5);

		if (pes_packet->SID != 0xBD){			
			pes_packet->exit_code = ARIBB24_NO_PES_PACKET;
			return pes_packet;
		}

		uint8_t ESCRF    = (data[pos+1] & 0x20) >> 5;
		uint8_t ESRF     = (data[pos+1] & 0x10) >> 4;
		uint8_t DSMTMF   = (data[pos+1] & 0x08) >> 3;
		uint8_t ACIF     = (data[pos+1] & 0x04) >> 2;
		uint8_t PESCRCF  = (data[pos+1] & 0x02) >> 1;
		uint8_t PESEF    = (data[pos+1] & 0x01);
		uint8_t PESHDL  = data[pos+2];
		
		pos = pos + 3;
		PPL = PPL - 3;

		if (pes_packet->PTSDTSF == 0x2){
			pos = pos + 5; // 40 bit
			PPL = PPL - 5;
			PESHDL = PESHDL - 5;
		} 
		if (pes_packet->PTSDTSF == 0x3){
			pos = pos + 10; // 80 bit
			PPL = PPL - 10;
			PESHDL = PESHDL - 10;
		}
		if (ESCRF == 0x1){
			pos = pos + 6; // 48 bit
			PPL = PPL - 6;
			PESHDL = PESHDL - 6;
		}
		if (ESRF == 0x1){
			pos = pos + 3; // 24 bit
			PPL = PPL - 3;
			PESHDL = PESHDL - 3;
		}
		if (DSMTMF == 0x1){
			pos = pos + 1; // 8 bit
			PPL = PPL - 1;
			PESHDL = PESHDL - 1;
		}
		if (ACIF == 0x1){
			pos = pos + 1; // 8 bit
			PPL = PPL - 1;
			PESHDL = PESHDL - 1;
		}
		if (PESCRCF == 0x1){
			pos = pos + 2; // 16 bit
			PPL = PPL - 2;
			PESHDL = PESHDL - 2;
		}
		if (PESEF == 0x1){

			uint8_t PESPDF = (data[pos] & 0x80) >> 7;
			uint8_t PHFF = (data[pos] & 0x40) >> 6;
			uint8_t PPSCF = (data[pos] & 0x20) >> 5;
			uint8_t PSTDBF = (data[pos] & 0x10) >> 4;
			uint8_t PESEF2 = data[pos] & 0x01;
			pos++;
			
			PESHDL = PESHDL - 1;
			PPL = PPL - 1;
			
			if (PESPDF == 0x01){
				pos = pos + 16;
				PPL = PPL - 16;
				PESHDL = PESHDL - 16;
			}
			if (PHFF == 0x01){
				PPL = PPL - 1 - data[pos];
				PESHDL = PESHDL - 1 - data[pos];
				pos = pos + data[pos] + 1;
			}
			if (PPSCF == 0x01){
				pos = pos + 2;
				PPL = PPL - 2;
				PESHDL = PESHDL - 2;
			}
			if (PSTDBF == 0x01){
				pos = pos + 2;
				PPL = PPL - 2;
				PESHDL = PESHDL - 2;
			}
			if (PESEF2 == 0x01){
				uint8_t PEFL = data[pos] & 0x7F;
				uint8_t SIDEF = data[pos+1] & 0xF0 >> 7;
				
				pos = pos + 2;
				PPL = PPL - 2;
				PESHDL = PESHDL - 2;
				if (SIDEF == 0x0){
					pos = pos + PEFL;
					PPL = PPL - PEFL;
					PESHDL = PESHDL - PEFL;
				}
			}
		}

		for (i = 0; PESHDL>0; i++) {pos++; PPL--; PESHDL--; }
		
		pes_packet->datagroup->id = (data[pos] & 0xFC) >> 2;
		
		pos = pos + 3;
		PPL = PPL - 3;

		if (pes_packet->datagroup->id != 0x20){
			pes_packet->exit_code = ARIBB24_NO_ARIB_PACKET;
			return pes_packet;
		}
		
		pes_packet->isARIBB24 = 1;
		pes_packet->datagroup->id = (data[pos] & 0xFC) >> 2;

		if ( (pes_packet->datagroup->id > 0x08  && pes_packet->datagroup->id < 0x20) || (pes_packet->datagroup->id > 0x28) ){
			pes_packet->exit_code = ARIBB24_NO_VALID_DATA_GROUP;
			return pes_packet;
		}

		pes_packet->datagroup->size = ((data[pos+3] << 8) & 0xFF00) | (data[pos+4] & 0xFF);
		
		pos = pos + 5;
		PPL = PPL - 5;

		if (pes_packet->datagroup->size > PPL){
			pes_packet->exit_code = ARIBB24_INVALID_DATA_GROUP_LENGTH;
			return pes_packet;
		}

		#ifdef DEBUG
			printf("pes->dg->size: %u\n",pes_packet->datagroup->size);
		#endif

		
		if (pes_packet->datagroup->id == 0x0 || pes_packet->datagroup->id == 0x20) {
			pes_packet->datagroup->type=ARIBB24_DATAGROUP_CAPTION_MANAGEMENT;
		} else {
			pes_packet->datagroup->type=ARIBB24_DATAGROUP_CAPTION_STATEMENT;
		}
		uint8_t TMD = (data[pos] & 0xC0) >> 6;
		pos++;

		if (TMD == 0x1 || TMD == 0x02){
			pes_packet->datagroup->hasOffsetTime = 1;
			pes_packet->datagroup->offsetTime = cut_array(data,5);
			pos=pos+5;
		}
			
		if (pes_packet->datagroup->type == ARIBB24_DATAGROUP_CAPTION_MANAGEMENT){
			pes_packet->datagroup->languages_num = data[pos];
			pes_packet->datagroup->languages = malloc(pes_packet->datagroup->languages_num);
			pos++;

			for (i = 0; i < pes_packet->datagroup->languages_num; i++){
				pes_packet->datagroup->languages[i] = malloc(4*sizeof(char));
				uint8_t DMF = data[pos] & 0x0F;
				pos++;
				if (DMF == 0x0C || DMF == 0x0D || DMF == 0x0E){
					pos++;
				}
				snprintf(pes_packet->datagroup->languages[i],4,"%c%c%c",(uint8_t)data[pos],(uint8_t)data[pos+1],(uint8_t)data[pos+2]);
				pos = pos + 3;
				pos++;
			}
		} 
		
		uint32_t dull = ((data[pos] << 16) & 0xFF0000) | ((data[pos+1] << 8) & 0xFF00) | (data[pos+2] & 0xFF);
		uint32_t dullcounter = dull;
		
		pos = pos + 3;
		PPL = PPL - 3;
		
		while (dullcounter != 0){
			uint8_t oldpos = pos;
			struct ARIB_data_unit *DU = initialize_ARIB_data_unit();
			DU->size = (((data[pos+2] & 0x0000FF) << 16) | ((data[pos+3] & 0x00FF) << 8) | ((data[pos+4] & 0xFF) << 0));
			#ifdef DEBUG
				printf("du->size: %u\n",DU->size);
			#endif

			if (DU->size > 188 || DU->size > PPL){
				if (dullcounter < DU->size)
					dullcounter=0;
				else
					dullcounter = dullcounter - DU->size;

				continue;
			}

			DU->parameter = data[pos+1];
			pos = pos + 5;
			DU->payload = cut_array(&(data[pos]),DU->size);
			pos = pos + DU->size;
			pes_packet->datagroup->data_units = append_list(pes_packet->datagroup->data_units,DU);
			pes_packet->datagroup->data_units_num++;
			dullcounter = dullcounter - (pos - oldpos);
		}
		
	}
	return pes_packet;
}