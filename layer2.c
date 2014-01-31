/*
 * layer2.c
 *
 *  Created on: Jan 31, 2014
 *      Author: nathan
 *
 *  Protocol description:
 *  [length][xor-checksum of content][CONTENT][\0 pads out to 16 bytes]
 *
 */

#include <string.h> //For memcpy
#include "layers.h"

#define MAX_CHUNK_SIZE 16

static inline char xor_checksum(char* chunk, int len)
{
	char base = 0;
	for(; len != 0; ++chunk, --len)
		base ^= *chunk;
	return base;
}

#define LAYER1_WRITE_SET_ERROR(byte) if(layer1_write(byte) == -1) retcode = -1

int layer2_write(char* chunk, int len)
{
	//bounds check
	if(len > MAX_CHUNK_SIZE || len < 0) return -1;

	//convert length to byte
	char len_byte = len;

	//Get checksum
	char checksum = xor_checksum(chunk, len);

	int retcode = len;

	//Write 18 bytes no matter what
	//write length
	LAYER1_WRITE_SET_ERROR(len_byte);

	//write checksum
	LAYER1_WRITE_SET_ERROR(checksum);

	//write 16 bytes
	for(int i = 0; i < MAX_CHUNK_SIZE; ++i)
		LAYER1_WRITE_SET_ERROR(i < len ? chunk[i] : 0);

	return retcode;
}

#define LAYER1_READ_SET_ERROR(byte) if(layer1_read(byte) == -1) retcode = -1;

int layer2_read(char* chunk, int max)
{
	int retcode = 0;
	char read_len;
	char checksum;
	char read_buf[MAX_CHUNK_SIZE];

	//Read 18 bytes no matter what
	LAYER1_READ_SET_ERROR(&read_len);
	LAYER1_READ_SET_ERROR(&checksum);
	for(int i = 0; i < MAX_CHUNK_SIZE; ++i)
		LAYER1_READ_SET_ERROR(read_buf+i);

	//Read error?
	if(retcode == -1) return -1;

	//bad length?
	if(read_len > MAX_CHUNK_SIZE || read_len < 0) return -1;

	//max too low?
	if(read_len > max) return -1;

	//bad checksum?
	if(xor_checksum(read_buf, read_len) != checksum) return -1;

	memcpy(chunk, read_buf, read_len);
	return read_len;
}
