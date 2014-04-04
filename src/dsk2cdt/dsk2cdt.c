#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "tzxfile.h"
#include "cdt.h"

#include "membuf.h"
#include "crunch.h"

#define EDSK_DIB_NUMTRACKS  0x30
#define EDSK_DIB_NUMSIDES   0x31
#define EDSK_DIB_TRACKTBL   0x34

extern char binary___dsk2cdt_loader_txt_start[];
extern char binary___dsk2cdt_loader_txt_end[];
extern char binary___dsk2cdt_loader_txt_size[];
extern char binary___dsk2cdt_rsx_bin_start[];
extern size_t binary___dsk2cdt_rsx_bin_end[];
extern size_t binary___dsk2cdt_rsx_bin_size[];

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

#define MAX_HEADS 2
#define MAX_TRACKS 80

int main(int argc, char *argv[])
{
	char filename[20];

	unsigned int num_tracks, num_sides, track_num, side_num;
	char dib[0x100];
	struct membuf tracks[MAX_TRACKS][MAX_HEADS];

	TZX_FILE *pCDTFile[MAX_HEADS];
	CPCHeader header;

	if (argc != 2)
	{
		printf("Usage: dsk2cdt <image.dsk>\n");
		printf("       Creates CDTs that rebuild a disc image when run.\n");
		printf("       Outputs sideN.cdt for each disc side in the image.\n");
		exit(1);
	}

	FILE *fin;
	fin = fopen(argv[1],"rb");

	if (fin == NULL)
	{
		printf("Failed to open %s\n",argv[1]);
		return 1;
	}

	fread(dib, 0x100, 1, fin);

	if (strncmp(dib,"EXTENDED CPC DSK File", 21) != 0)
	{
		printf("dsk2cdt requires an image in extended dsk format.\n");
		printf("This doesn't appear to be one.\n");
		return 1;
	}

	num_tracks = dib[EDSK_DIB_NUMTRACKS];
	num_sides = dib[EDSK_DIB_NUMSIDES];
	printf("Num sides: %d\n",dib[EDSK_DIB_NUMSIDES]);
	printf("Num tracks: %d\n",num_tracks);

	for (side_num = 0; side_num < num_sides; side_num++)
	{
		pCDTFile[side_num] = TZX_CreateFile(TZX_VERSION_MAJOR,TZX_VERSION_MINOR);
		if (!pCDTFile[side_num])
		{
			printf("Failed to create TZX file.\n");
			exit(1);
		}

		CDT_add_pause(pCDTFile[side_num], 3000);

		// write out loader
		// TODO: insert disc name into it
		header.filename = "LOADER";
		header.loadAddress = 0x1000;
		header.execAddress = 0x1000;
		header.type = 22;
		CDT_add_file(pCDTFile[side_num], binary___dsk2cdt_loader_txt_start, (size_t)binary___dsk2cdt_loader_txt_size, 2000, &header);

		header.filename = "RSX";
		header.loadAddress = 0x9000;
		header.execAddress = 0x1000;
		header.type = 2;
		CDT_add_file(pCDTFile[side_num], binary___dsk2cdt_rsx_bin_start, (size_t)binary___dsk2cdt_rsx_bin_size, 2000, &header);
	}

	// Compress all the tracks
	for (track_num = 0; track_num < num_tracks; track_num++)
	{
		char track[0x4000];
		for (side_num = 0; side_num < num_sides; side_num++)
		{
			unsigned int tracksize = dib[EDSK_DIB_TRACKTBL+(track_num*num_sides)+side_num]*0x100;

			fread(track, tracksize, 1, fin);

			exo_crunch(&tracks[track_num][side_num], track, tracksize);
			printf("Track %02d Head %02d: Size: %d bytes Packed: %d bytes\n", track_num, side_num, tracksize, tracks[track_num][side_num].len);
		}
	}

	for (side_num = 0; side_num < num_sides; side_num++)
	{
		// rewrite the track length data in the dib for each side
		memset(dib+0x34, 0, 0x100-0x34);
		for (track_num = 0; track_num < num_tracks; track_num++)
		{
			dib[0x34+(track_num*2)]   = tracks[track_num][side_num].len & 0xFF;
			dib[0x34+(track_num*2)+1] = tracks[track_num][side_num].len >> 8;
		}

		CDT_add_headerless_file(pCDTFile[side_num], dib, 0x100, 4000);

		for (track_num = 0; track_num < num_tracks; track_num++)
		{
			CDT_add_headerless_file(pCDTFile[side_num], tracks[track_num][side_num].buf, tracks[track_num][side_num].len, 4000);
			membuf_free(&tracks[track_num][side_num]);
		}
	}

	for (side_num = 0; side_num < num_sides; side_num++)
	{
		char cdtname[21];
		snprintf(cdtname, 20, "side%d.cdt", side_num+1);
		TZX_WriteFile(pCDTFile[side_num], cdtname);
		TZX_FreeFile(pCDTFile[side_num]);
	}

	fclose(fin);

	return 0;
}