/*
 * layer2.c
 *
 *  Created on: Jan 31, 2014
 *      Author: Nathan West
 *
 *  Protocol: [length][content][padding][xor_checksum]
 *            B       B...     B...     B
 *            MAX_CHUNK_SIZE + 2 bytes sent total
 */

#include <string.h> //For memcpy
#include <stdlib.h>
#include "layers.h"

//If this many consecutive layer 1 reads/writes fail, fail forever.
const static unsigned MAX_LAYER1_RETRIES = 500;

//constant array indexes
const static int TRUE_CHUNK_SIZE = MAX_CHUNK_SIZE + 2;
const static int LENGTH_BYTE = 0;
const static int CHECKSUM_BYTE = TRUE_CHUNK_SIZE - 1;
const static int BEGIN_DATA = 1;

char xor_checksum(const char* buffer, const char* end)
{
	char checksum = 0;
	for(; buffer < end; ++buffer)
		checksum ^= *buffer;
	return checksum;
}

#define LAYER1_ATTEMPT_WRITE(BYTE) \
	ATTEMPT_WITH_RETRIES(MAX_LAYER1_RETRIES, layer1_write, BYTE)

static inline int attempt_layer1_write(char byte)
{
	int attempts = MAX_LAYER1_RETRIES;
	while(layer1_write(byte) == -1)
		if(!(--attempts))
			return 1;
	return 0;
}

/*
 * Padding bytes are filled with random data. They aren't checksummed, so if the
 * length is wrong they'll ruin the checksum.
 */
static inline char padding_byte()
{
	return rand() & 0xFF;
}

int layer2_write(char* chunk, int len)
{
	INIT_ERROR();

	if(len < 0 || len > MAX_CHUNK_SIZE) return -1;

	//Create the bytes to be written
	char chunk_buffer[TRUE_CHUNK_SIZE];

	//Fill random padding
	for(unsigned i = 0; i < (TRUE_CHUNK_SIZE - 1); ++i)
		chunk_buffer[i] = padding_byte();

	//Write length byte
	chunk_buffer[LENGTH_BYTE] = len;

	//Write data bytes
	memcpy(chunk_buffer+BEGIN_DATA, chunk, len);

	//Compute checksum of length byte and data bytes
	chunk_buffer[CHECKSUM_BYTE] =
			xor_checksum(chunk_buffer, chunk_buffer + 1 + len);

	//Write
	for(unsigned i = 0; i < TRUE_CHUNK_SIZE; ++i)
		CHECK_ERROR(attempt_layer1_write(chunk_buffer[i]));

	return len;
}

#define LAYER1_ATTEMPT_READ(BYTE) \
	ATTEMPT_WITH_RETRIES(MAX_LAYER1_RETRIES, layer1_read, BYTE)

static inline int attempt_layer1_read(char* byte)
{
	int attempts = MAX_LAYER1_RETRIES;
	while(layer1_read(byte) == -1)
		if(!(--attempts))
			return 1;
	return 0;
}

int layer2_read(char* chunk, int max)
{
	INIT_ERROR();

	char read_buf[TRUE_CHUNK_SIZE];

	/*
	 * ALWAYS read MAX_CHUNK_SIZE+2 bytes, no matter what max is or whatever
	 */
	for(int i = 0; i < TRUE_CHUNK_SIZE; ++i)
		CHECK_ERROR(attempt_layer1_read(read_buf + i));

	int len = read_buf[LENGTH_BYTE];
	if(len < 0 || len > MAX_CHUNK_SIZE)
		return -1;

	if(len > max)
		return -1;

	if(xor_checksum(read_buf, read_buf + len + 1) != read_buf[CHECKSUM_BYTE])
		return -1;

	memcpy(chunk, read_buf+BEGIN_DATA, len);
	return len;
}
