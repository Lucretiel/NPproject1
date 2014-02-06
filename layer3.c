/*
 * layer3.c
 *
 *  Created on: Jan 31, 2014
 *      Author: nathan
 *
 *  PROTOCOL: [DATA CHUNKS]... [0-length chunk]
 *
 */

#include <stdbool.h>
#include <string.h>
#include "layers.h"

static inline int min(int a, int b) { return a < b ? a : b; }

int layer3_write(char* msg, int len)
{
	INIT_ERROR();

	int bytes_remaining = len;
	char* msg_seek = msg;

	int bytes_to_write = -1;
	do
	{
		bytes_to_write = min(MAX_CHUNK_SIZE, bytes_remaining);
		CHECK_ERROR(layer2_write(msg_seek, bytes_to_write));
		msg_seek += bytes_to_write;
		bytes_remaining -= bytes_to_write;
	} while(bytes_to_write > 0);

	return len;
}

int layer3_read(char* msg, int max)
{
	INIT_ERROR();

	int total_bytes_read = 0;
	int total_bytes_remaining = max;
	char* msg_seek = msg;
	char buffer[MAX_CHUNK_SIZE];
	bool too_long = false;

	int bytes_read;
	do
	{
		CHECK_ERROR(bytes_read = layer2_read(buffer, MAX_CHUNK_SIZE));
		if(bytes_read <= total_bytes_remaining)
		{
			memcpy(msg_seek, buffer, bytes_read);
			total_bytes_read += bytes_read;
			total_bytes_remaining -= bytes_read;
			msg_seek += bytes_read;
		}
		else
		{
			too_long = true;
		}
	} while(bytes_read > 0);

	if(too_long) return -1;
	else return total_bytes_read;
}
