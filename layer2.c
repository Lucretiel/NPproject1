/*
 * layer2.c
 *
 *  Created on: Jan 31, 2014
 *      Author: Nathan West
 *
 *  Protocol: [length][content]
 *            B       B...
 *
 *  NOTE: Originally, I had mechanisms in place to do some basic error handling-
 *  always write the same number of bytes in a chunk, send a checksum byte,
 *  etc. These things were in place to make sure that layer2 reads were
 *  REASONABLY reliable in the even of layer 1 byte corruption. I was told
 *  "don't bother" by the professor and everyone else I asked, so I'm not
 *  bothering. God help you if even 1 bit is corrupted.
 */

#include <string.h>
#include <stdlib.h>

#include "layers.h"

const static int LAYER = 2;

//If this many consecutive layer 1 reads/writes fail, fail forever.
const static unsigned MAX_LAYER1_RETRIES = 500;

int layer2_write(char* chunk, int len)
{
	INIT_ERROR();

	if(len < 0 || len > MAX_CHUNK_SIZE) return -1;

	//Create the bytes to be written
	char chunk_buffer[MAX_CHUNK_SIZE + 1];

	//Write length byte
	chunk_buffer[0] = len;

	//Write data bytes
	memcpy(chunk_buffer+1, chunk, len);

	//Write
	for(unsigned i = 0; i < len+1; ++i)
		CHECK_ERROR(layer1_write(chunk_buffer[i]));

	return len;
}

int layer2_read(char* chunk, int max)
{
	INIT_ERROR();

	char len_byte;
	CHECK_ERROR(layer1_read(&len_byte));

	const int len = len_byte;
	CHECK_ERROR(len < 0 || len > MAX_CHUNK_SIZE);

	char read_buf[MAX_CHUNK_SIZE];
	for(int i = 0; i < len; ++i)
		CHECK_ERROR(layer1_read(read_buf + i));

	if(len > max)
		return -1;

	memcpy(chunk, read_buf, len);
	return len;
}
