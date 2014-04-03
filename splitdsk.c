#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void dump(char *basename, char *data, unsigned long length)
{
	FILE *fp;
	char filename[20];

	snprintf(filename, 20, "%s.BIN", basename);

	fp = fopen(filename,"wb");
	if (fp != NULL)
	{
		fwrite(data,length,1,fp);
		fclose(fp);
	}
}

#define EDSK_DIB_NUMTRACKS  0x30
#define EDSK_DIB_NUMSIDES   0x31
#define EDSK_DIB_TRACKTBL   0x34

int main(int argc, char *argv[])
{
	unsigned int num_tracks, num_sides, track_num, side_num;
	char filename[20];
	char cdtname[2][20];
	char dib[0x100];
	uint16_t *tracksizetable;

	FILE *fin, *fout;
	fin = fopen(argv[1],"rb");

	if (fin == NULL)
	{
		return 1;
	}

	fread(dib, 0x100, 1, fin);

	num_tracks = dib[EDSK_DIB_NUMTRACKS];
	num_sides = dib[EDSK_DIB_NUMSIDES];
	printf("Num sides: %d\n",dib[EDSK_DIB_NUMSIDES]);
	printf("Num tracks: %d\n",num_tracks);

	tracksizetable = (uint16_t *)calloc(2,num_tracks*num_sides);
	if (tracksizetable == NULL)
	{
		fprintf(stderr,"Memory full\nReady\n");
		exit(1);
	}

	fout = fopen("buildcdt.sh","w");

	for (side_num = 0; side_num < num_sides; side_num++)
	{
		snprintf(cdtname[side_num], 20, "side%d.cdt", side_num+1);
		fprintf(fout,"../2cdt -n -F 22 -r LOADER ../TAPE2DISC.TXT %s\n", cdtname[side_num]);
		fprintf(fout,"../2cdt -L 0x9000 -r RSX ../RSX.BIN %s\n", cdtname[side_num]);
		fprintf(fout,"../2cdt -w -b 4000 -r DIB DIB%d.BIN %s\n", side_num, cdtname[side_num]);
	}

	for (track_num = 0; track_num < num_tracks; track_num++)
	{
		char track[0x4000];
		for (side_num = 0; side_num < num_sides; side_num++)
		{
			unsigned int tracksize = dib[EDSK_DIB_TRACKTBL+(track_num*num_sides)+side_num]*0x100;
			fread(track, tracksize, 1, fin);
			snprintf(filename, 20, "TRACK%d%02d", side_num, track_num);
			dump(filename, track, tracksize);
			// TODO: compress track with exomizer here
			tracksizetable[side_num*num_tracks+track_num] = tracksize;
			fprintf(fout,"../2cdt -b 4000 -m 1 -w -r %s %s.BIN %s\n", filename, filename, cdtname[side_num]);
		}
	}

	// rewrite the track length data in the dib for each side
	for (side_num = 0; side_num < num_sides; side_num++)
	{
		memset(dib+0x34, 0, 0x100-0x34);
		for (track_num = 0; track_num < num_tracks; track_num++)
		{
			dib[0x34+(track_num*2)]   = tracksizetable[side_num*num_tracks+track_num] & 0xFF;
			dib[0x34+(track_num*2)+1] = tracksizetable[side_num*num_tracks+track_num] >> 8;
		}
		snprintf(filename, 20, "DIB%d", side_num);
		dump(filename,dib,0x100);
	}

	fprintf(fout,"rm *.BIN\n");

	fclose(fout);
	fclose(fin);

	return 0;
}