#include "common.h"

//stolen from libavformat's mpegts.c
static int64_t get_pts(const uint8_t *p)
{
    int64_t pts;
    int val;

    pts = (int64_t)((p[0] >> 1) & 0x07) << 30;
    val = (p[1] << 8) | p[2];
    pts |= (int64_t)(val >> 1) << 15;
    val = (p[3] << 8) | p[4];
    pts |= (int64_t)(val >> 1);
    return pts;
}

char* parsePTStoString(uint8_t *ptsin){
	//uint64_t ptsticks = (((ptsin[0] & 0x0E) >> 1) <<32)| ((ptsin[1]) << 24) | ((ptsin[2] & 0xFE) << 16) | ((ptsin[3]) << 8) | ((ptsin[4]) & 0xFE);
	uint64_t ptsticks = get_pts(ptsin);
	uint64_t ptstotalseconds = ptsticks / 90;
	uint32_t pts_ms = ptstotalseconds % 1000;
	uint32_t pts_s = ((ptstotalseconds - pts_ms) / 1000) % 60;
	uint32_t pts_m = ((ptstotalseconds - pts_ms - (pts_s*1000)) / 1000 / 60) % 60;
	uint32_t pts_h = ((ptstotalseconds - pts_ms - (pts_s*1000) - (pts_m*60*1000)) / 1000 / 60 / 60);
	if ((pts_ms % 10) > 4){
		pts_ms = pts_ms + (10-pts_ms%10);
	} else {
		pts_ms = pts_ms - (pts_ms%10);
	}
	pts_ms=pts_ms/10;
	
	char* outstr = malloc(100 * sizeof(char));
	snprintf(outstr,100,"%2d:%2d:%2d.%02d",pts_h,pts_m,pts_s,pts_ms);
	#ifdef DEBUG
		printf("debug: %X %X %X %X %X : %u : %d %d %d %d\n",ptsin[0],ptsin[1],ptsin[2],ptsin[3],ptsin[4],ptstotalseconds,pts_ms,pts_s,pts_m,pts_h);	
		printf("outstr: %s\n",outstr);
	#endif
	return outstr;	
}