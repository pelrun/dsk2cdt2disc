
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tzxfile.h"
#include "cdt.h"

// Can't just link in 2cdt.c
// because of the static BaudRate variable
// I'm crying.
#include "2cdt.c"

#define CPC_DATA_SYNC 0x16
#define CPC_HEADER_SYNC 0x2C

void CDT_add_pause(TZX_FILE *pTZXFile, unsigned long milliseconds)
{
	TZX_BLOCK *pBlock;

	/* correct pause */
	pBlock = TZX_CreateBlock(TZX_PAUSE_BLOCK);

	if (pBlock!=NULL)
	{
		TZX_SetupPauseBlock(pBlock, milliseconds);
		TZX_AddBlockToEndOfFile(pTZXFile, pBlock);
	}		
}

void CDT_add_headerless_file(TZX_FILE *pTZXFile, char *data, unsigned int length, unsigned int baudrate)
{
	BaudRate = baudrate; // ugh
	CPC_WriteTurboLoadingDataBlock(pTZXFile, CPC_DATA_SYNC, data, length, CPC_PAUSE_AFTER_BLOCK_IN_MS);
}

void CDT_add_file(TZX_FILE *pTZXFile, char *data, unsigned int length, unsigned int baudrate, CPCHeader header)
{
	unsigned char tapeHeader[CPC_TAPE_HEADER_SIZE];
	int lastBlock;
	int blockIndex;
	
	tapeHeader[CPC_TAPE_HEADER_FILE_TYPE] = header.type;

	/* set execution address */
	tapeHeader[CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_LOW] = header.execAddress&0xFF;
	tapeHeader[CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_HIGH] = (header.execAddress>>8)&0xFF;

	/* set load address */
	tapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_LOW] = header.loadAddress&0xFF;
	tapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_HIGH] = (header.loadAddress>>8)&0xFF;

	BaudRate = baudrate; // ugh

	if (header.filename!=NULL)
	{
		int i;
		int nLength = strlen(header.filename);
		if (nLength>16) nLength = 16;
		for (i=0; i<nLength; i++)
		{
			tapeHeader[i] = toupper(header.filename[i]);
		}
	}

	tapeHeader[CPC_TAPE_HEADER_FILE_TYPE] = Type;

	lastBlock = (length-1)/CPC_DATA_BLOCK_SIZE;
	
	for (blockIndex = 0; blockIndex <= lastBlock; blockIndex++)
	{
		unsigned int blockStart = blockIndex * CPC_DATA_BLOCK_SIZE;
		unsigned int blockLength = length - blockStart;

		if (blockLength > CPC_DATA_BLOCK_SIZE)
		{
			blockLength = CPC_DATA_BLOCK_SIZE;
		}
		
		tapeHeader[CPC_TAPE_HEADER_FIRST_BLOCK_FLAG] = (blockIndex==0) ? 0xFF : 0x00;
		tapeHeader[CPC_TAPE_HEADER_LAST_BLOCK_FLAG] = (blockIndex==lastBlock) ? 0xFF : 0x00;

		tapeHeader[CPC_TAPE_HEADER_DATA_LENGTH_LOW] = blockLength & 0xFF;
		tapeHeader[CPC_TAPE_HEADER_DATA_LENGTH_HIGH] = (blockLength >> 8) & 0xFF;

		tapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_LOW] = blockStart;
		tapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_HIGH] = (blockStart >> 8) & 0xFF;
		
		tapeHeader[CPC_TAPE_HEADER_BLOCK_NUMBER] = blockIndex+1;
		
		CPC_WriteTurboLoadingDataBlock(pTZXFile, CPC_HEADER_SYNC, tapeHeader, CPC_TAPE_HEADER_SIZE, CPC_PAUSE_AFTER_HEADER_IN_MS);
		CPC_WriteTurboLoadingDataBlock(pTZXFile, CPC_DATA_SYNC, data+blockStart, blockLength, CPC_PAUSE_AFTER_BLOCK_IN_MS);
	}

}
