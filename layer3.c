/*
 * layer3.c
 *
 *  Created on: Jan 31, 2014
 *      Author: nathan
 *
 *  PROTOCOL: [DATA CHUNKS]... [SHORT DATA CHUNK]
 *  Writes data as a series of length 16 chunks followed by a length 0-15 chunk
 *
 */

#include <string.h>
#include "layers.h"

static inline int min(int a, int b) { return a < b ? a : b; }

int layer3_write(char* msg, int len)
{
	INIT_ERROR();

	//Total number on unwritten bytes
	int bytes_remaining = len;

	//ptr to the next chunk
	char* msg_seek = msg;

	//Keep sending chunks until a short one is sent
	int bytes_to_write;
	do
	{
		//How many bytes in this chunk
		bytes_to_write = min(MAX_CHUNK_SIZE, bytes_remaining);

		//Write the chunk
		CHECK_LAYER_ERROR(layer2_write(msg_seek, bytes_to_write));

		//Update counters
		msg_seek += bytes_to_write;
		bytes_remaining -= bytes_to_write;
	} while(bytes_to_write == MAX_CHUNK_SIZE);

	return len;
}

int layer3_read(char* msg, int max)
{
	INIT_ERROR();

	//Total number of bytes before we go over max
	int bytes_remaining = max;

	//ptr to where the next chunk will be written
	char* msg_seek = msg;

	/*
	 * Note: an old implementation of layer3_read used buffering to read the
	 * whole message, even if max was too small. Decided to not bother.
	 */

	//Keep reading until a short chunk is read
	int bytes_read;
	do
	{
		//Read the chunk into the client array
		//If the size of the chunk is < bytes_remaining, permafail.
		CHECK_LAYER_ERROR(bytes_read = layer2_read(msg_seek, bytes_remaining));

		//Update counters
		msg_seek += bytes_read;
		bytes_remaining -= bytes_read;
	} while(bytes_read == MAX_CHUNK_SIZE);

	//Compute length based on how far msg_seek moved
	return msg_seek - msg;
}
