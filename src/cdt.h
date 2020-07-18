
#ifndef __CDT_H_
#define __CDT_H_

typedef struct
{
    char *filename;
    unsigned int loadAddress;
    unsigned int execAddress;
    char type;
} CPCHeader;

// set length of pause after a data block
void CDT_set_pause_length(unsigned int milliseconds);

void CDT_add_pause(TZX_FILE *pTZXFile, unsigned long milliseconds);
void CDT_add_headerless_file(TZX_FILE *pTZXFile, const void *data, unsigned int length, unsigned int baudrate);
void CDT_add_file(TZX_FILE *pTZXFile, const void *data, unsigned int length, unsigned int baudrate, CPCHeader *header);
TZX_BLOCK *CDT_get_last_block(TZX_FILE *pTZXFile);

#endif // __CDT_H_
