/*
 * layer2.c
 *
 *  Created on: Jan 31, 2014
 *      Author: Nathan West
 *
 *  Protocol: [length][content][padding]
 *            B       B...
 *            MAX_CHUNK_SIZE + 1 bytes sent total
 *
 *  All errors are assumed to be catastrophic. As soon as an error happens, all
 *  future calls will return an error as well.
 */

#include <string.h> //For memcpy

int layer1_write(char b);
int layer1_read(char* b);

#define MAX_CHUNK_SIZE 16

//If condition is true, fail unrecoverably.
#define CHECK_ERROR(condition) if(condition) return error = -1
#define INIT_ERROR() static int error=0; CHECK_ERROR(error == -1)

int layer2_write(char* chunk, int len)
{

	INIT_ERROR();

	CHECK_ERROR(len < 0 || len > MAX_CHUNK_SIZE);

	char len_byte = len;
	CHECK_ERROR(layer1_write(len_byte) == -1);

	int i;
	for(i = 0; i < len; ++i)
		CHECK_ERROR(layer1_write(chunk[i]) == -1);
	for(; i < MAX_CHUNK_SIZE; ++i)
		CHECK_ERROR(layer1_write(0) == -1);

	return len;
}

int layer2_read(char* chunk, int max)
{
	INIT_ERROR();

	char read_len;
	char read_buf[MAX_CHUNK_SIZE];

	CHECK_ERROR(layer1_read(&read_len) == -1);
	for(int i = 0; i < MAX_CHUNK_SIZE; ++i)
		CHECK_ERROR(layer1_read(read_buf+i) == -1);

	int len = read_len;
	CHECK_ERROR(len < 0 || len > MAX_CHUNK_SIZE);

	CHECK_ERROR(len > max);

	memcpy(chunk, read_buf, read_len);
	return read_len;
}
