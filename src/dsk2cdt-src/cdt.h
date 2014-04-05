
#ifndef __CDT_H_
#define __CDT_H_

typedef struct
{
	char *filename;
	unsigned int loadAddress;
	unsigned int execAddress;
	char type;
} CPCHeader;

void CDT_add_pause(TZX_FILE *pTZXFile, unsigned long milliseconds);
void CDT_add_headerless_file(TZX_FILE *pTZXFile, char *data, unsigned int length, unsigned int baudrate);
void CDT_add_file(TZX_FILE *pTZXFile, char *data, unsigned int length, unsigned int baudrate, CPCHeader *header);

#endif // __CDT_H_