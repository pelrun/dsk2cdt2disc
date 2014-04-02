#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[])
{
	unsigned int num_tracks, track_num, sect_num;
	char filename[20];
	char dib[0x100];

	FILE *fin, *fout;
	fin = fopen(argv[1],"rb");
	
	if (fin == NULL)
	{
		return 1;
	}

	fout = fopen("buildcdt.sh","w");

	fprintf(fout,"../2cdt -n -F 22 -r LOADER ../TAPE2DISC.TXT disk.cdt\n");
	fprintf(fout,"../2cdt -L 0x9000 -r RSX ../RSX.BIN disk.cdt\n");
	
	fread(dib, 0x100, 1, fin);
	dump("DIB",dib,0x100);
	fprintf(fout,"../2cdt -w -r DIB DIB.BIN disk.cdt\n");
	
	printf("Num sides: %d\n",dib[0x31]);
	num_tracks = dib[0x30];
	printf("Num tracks: %d\n",num_tracks);

	for (track_num = 0; track_num < num_tracks; track_num++)
	{
		unsigned int num_sect;
		char tib[0x100];
		char sect[0x2000];
		
		fread(tib, 0x100, 1, fin);
		snprintf(filename, 20, "TIB%02d", track_num);
		dump(filename, tib, 0x100);
		fprintf(fout,"../2cdt -w -r %s %s.BIN disk.cdt\n", filename, filename);

		num_sect = tib[0x15];
		printf("Track: %d Number of sectors: %d\n",track_num, num_sect);
		
		for (sect_num = 0; sect_num < num_sect; sect_num++)
		{
			unsigned char sect_id = tib[(8*sect_num)+0x1A];
			unsigned int sect_size = tib[(8*sect_num)+0x1E] + (tib[0x1F+(8*sect_num)] << 8);
			printf("Sector: %02X Size: %d\n",sect_id,sect_size);
			
			fread(sect, sect_size, 1, fin);
			snprintf(filename, 20, "SCT%02d-%02X", track_num, sect_id);
			dump(filename, sect, sect_size);
			fprintf(fout,"../2cdt -w -r %s %s.BIN disk.cdt\n", filename, filename);
		}
	}

	fprintf(fout,"rm *.BIN\n");
	
	fclose(fout);
	fclose(fin);
	
	return 0;
}