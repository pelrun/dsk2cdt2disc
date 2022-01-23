#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "tzxfile.h"
// not exported by tzxfile.h
void TZX_InsertBlockAfter(TZX_FILE *pFile,TZX_BLOCK *pBlock, TZX_BLOCK *pPrev);
void TZX_DetachBlock(TZX_FILE *pFile,TZX_BLOCK *pBlock);
/*  */
#include "cdt.h"

#include "membuf.h"
#include "crunch.h"

#include "incbin.h"

INCBIN(Loader, "loader.bin");
INCBIN(Rsx, "rsx.bin");

#define EDSK_DIB_NUMTRACKS  0x30
#define EDSK_DIB_NUMSIDES   0x31
#define EDSK_DIB_TRACKTBL   0x34

#define EDSK_TIB_SECTORLBPS 0x14
#define EDSK_TIB_SECTORCOUNT 0x15
#define EDSK_TIB_SECTORINFO 0x18

#define EDSK_SIB_LENGTH     0x08
#define EDSK_SIB_LOGBPS     0x3

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

// address in CPC memory that our track blocks are loaded to
#define LOAD_ADDRESS 0x2000
#define MAX_BLOCK_SIZE 0x4000

// min time required to format/write a track
#define PAUSE_PER_TRACK 4000

#define BAUDRATE_NORMAL 2000
#define BAUDRATE_HIGH 4000

void write_dib(TZX_FILE *pTZXFile, TZX_BLOCK *pPrevBlock, char dib[])
{
  TZX_BLOCK *pBlock;

  CDT_set_pause_length(3000);

  CDT_add_headerless_file(pTZXFile, dib, 0x100, BAUDRATE_HIGH);

  pBlock = CDT_get_last_block(pTZXFile);
  TZX_DetachBlock(pTZXFile, pBlock);
  TZX_InsertBlockAfter(pTZXFile, pBlock, pPrevBlock);
}

unsigned int write_track_block(TZX_FILE *pTZXfile, struct membuf tracks[], int first_track, int last_track)
{
  char blockbuf[MAX_BLOCK_SIZE];
  int track;
  int track_count = last_track - first_track + 1;
  int block_ptr = (track_count+1) * 2; // space for pointer table at start plus 0x0000 terminator
  int block_tbl_ptr = 0;

  printf("writing tracks %d-%d\n", first_track, last_track);

  memset(blockbuf, 0, MAX_BLOCK_SIZE);

  for (track = first_track; track <= last_track; track++)
  {
    int track_len = membuf_memlen(&tracks[track]);

    if (track_len > 0) // skip null tracks
    {
      memcpy(blockbuf+block_ptr, membuf_get(&tracks[track]), track_len);
      blockbuf[block_tbl_ptr++] = block_ptr & 0xff;
      blockbuf[block_tbl_ptr++] = block_ptr >> 8;

      membuf_free(&tracks[track]);

      block_ptr += track_len;
    }
  }

  CDT_set_pause_length(track_count * PAUSE_PER_TRACK + 4000);
  CDT_add_headerless_file(pTZXfile, blockbuf, block_ptr, 4000);

  return block_ptr;
}

int main(int argc, char *argv[])
{
  char filename[20];

  unsigned int num_tracks, num_sides, track_num, side_num;
  char dib[0x100];
  struct membuf tracks[MAX_HEADS][MAX_TRACKS];
  size_t read_len;

  if (argc != 2)
  {
    printf("Usage: dsk2cdt <image.dsk>\n");
    printf("     Creates CDTs that rebuild a disc image when run.\n");
    printf("     Outputs sideN.cdt for each disc side in the image.\n");
    exit(1);
  }

  FILE *fin;
  fin = fopen(argv[1],"rb");

  if (fin == NULL)
  {
    printf("Failed to open %s\n",argv[1]);
    return 1;
  }

  if (fread(dib, 0x100, 1, fin) != 1)
  {
    printf("Failed to read DIB from %s\n", argv[1]);
    exit(1);
  }

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

  // Compress all the tracks
  for (track_num = 0; track_num < num_tracks; track_num++)
  {
    char track[0x4000];
    for (side_num = 0; side_num < num_sides; side_num++)
    {
      unsigned int tracksize = dib[EDSK_DIB_TRACKTBL+(track_num*num_sides)+side_num]*0x100;

      if(tracksize > 0 && fread(track, tracksize, 1, fin) != 1)
      {
        printf("File error %d reading from %s\n", errno, argv[1]);
        exit(1);
      }

      // Check for and skip tracks with 0 sectors
      if (tracksize == 0 || track[EDSK_TIB_SECTORCOUNT] == 0)
      {
        tracks[side_num][track_num].len = 0;
        printf("Track %02d Head %02d: skipping, no content\n", track_num, side_num);
      }
      else
      {
        // detect and fix a TIB with an unset logbps field (looking at you Red Sunset)
        if (track[EDSK_TIB_SECTORLBPS] == 0)
        {
          uint8_t max_logbps = 0;
          for (int s = 0; s < track[EDSK_TIB_SECTORCOUNT]; s++)
          {
            uint8_t sector_logbps = track[EDSK_TIB_SECTORINFO + EDSK_SIB_LOGBPS + EDSK_SIB_LENGTH*s];
            max_logbps = sector_logbps > max_logbps ? sector_logbps : max_logbps;
          }
          track[EDSK_TIB_SECTORLBPS] = max_logbps;
          printf("Corrected track %d side %d logbps value to %d\n", track_num, side_num, max_logbps);
        }

        exo_crunch(&tracks[side_num][track_num], track, tracksize);
        printf("Track %02d Head %02d: Size: %d bytes Packed: %d bytes\n", track_num, side_num, tracksize, membuf_memlen(&tracks[side_num][track_num]));
      }
    }
  }

  for (side_num = 0; side_num < num_sides; side_num++)
  {
    TZX_FILE *pCDTFile;
    TZX_BLOCK *pRSXBlock;
    CPCHeader header;

    int block_start_track = 0;
    int block_size = 0;

    char *dib_blocktbl_ptr = dib+0x34;

    memset(dib_blocktbl_ptr, 0, 0x100-0x34);

    pCDTFile = TZX_CreateFile(TZX_VERSION_MAJOR,TZX_VERSION_MINOR);
    if (!pCDTFile)
    {
      printf("Failed to create TZX file.\n");
      exit(1);
    }

    CDT_add_pause(pCDTFile, 3000);
    CDT_set_pause_length(4000);

    // write out loader
    // TODO: insert disc name into it?
    header.filename = "LOADER";
    header.loadAddress = 0x1000;
    header.execAddress = 0x1000;
    header.type = 22;
    CDT_add_file(pCDTFile, gLoaderData, gLoaderSize, BAUDRATE_NORMAL, &header);

    header.filename = "RSX";
    header.loadAddress = 0x9000;
    header.execAddress = 0x1000;
    header.type = 2;
    CDT_add_file(pCDTFile, gRsxData, gRsxSize, BAUDRATE_NORMAL, &header);

    // save this point in the tape for inserting the DIB later
    pRSXBlock = CDT_get_last_block(pCDTFile);

    for (track_num = 0; track_num < num_tracks; track_num++)
    {
      int track_len = membuf_memlen(&tracks[side_num][track_num]);

      if ((block_size + track_len + 4) <= MAX_BLOCK_SIZE)
      {
        block_size += track_len + 2; // track data plus pointer
      }
      else
      {
        unsigned int block_len = write_track_block(pCDTFile, tracks[side_num], block_start_track, track_num-1);

        // put block length in the dib
        *dib_blocktbl_ptr++ = block_len & 0xFF;
        *dib_blocktbl_ptr++ = block_len >> 8;

        block_size = track_len + 2;
        block_start_track = track_num;
      }
    }

    {
      // write out final block
      unsigned int block_len = write_track_block(pCDTFile, tracks[side_num], block_start_track, track_num-1);

      // put block length in the dib
      *dib_blocktbl_ptr++ = block_len & 0xFF;
      *dib_blocktbl_ptr++ = block_len >> 8;
    }

    write_dib(pCDTFile, pRSXBlock, dib);

    {
      int file_name_buffer_length = strlen(argv[1])+10+1; // with the zero end-of-string marker
      char cdtname[file_name_buffer_length];
      snprintf(cdtname, file_name_buffer_length, "%s_side%d.cdt", argv[1], side_num+1);
      printf("output file name %s\n", cdtname);
      TZX_WriteFile(pCDTFile, cdtname);
      TZX_FreeFile(pCDTFile);
    }
  }

  fclose(fin);

  return 0;
}
