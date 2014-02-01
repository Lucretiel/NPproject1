/*
 * layer3.c
 *
 *  Created on: Jan 31, 2014
 *      Author: nathan
 *
 *  PROTOCOL
 */

#include <limits.h>
#include "layers.h"

const static int countdown_length = 100;

inline static char countdown_byte(int chunk)
{
	return chunk < countdown_length ? chunk : -1;
}

inline static int min(int a, int b)
{
	return a < b ? a : b;
}

/*
 * Normally we wouldn't need this, because this implementation of layer 2 will
 */
inline static void attempt_layer2_write(char* chunk, int len)
{
	int remaining = max_consecutive_fails;
	while(layer2_write(chunk, len) == -1)
		if(!(remaining--))
			exit(-1);
}

//TODO: more in-place math
int layer3_write(char* msg, int len)
{
	int num_chunks = (len / (MAX_CHUNK_SIZE - 1)) + 2;
	int length_remaining = len;

	for(; num_chunks > 0; --num_chunks)
	{
		char chunk[MAX_CHUNK_SIZE];
		chunk[0] = countdown_byte(num_chunks);
		int amount_to_write = min(length_remaining, MAX_CHUNK_SIZE-1);
		memcpy(chunk + 1, msg, amount_to_write);
		length_remaining -= amount_to_write;

		//TODO: check for error here.
		layer2_write(chunk, amount_to_write + 1);
	}

	return len;
}

int layer3_read(char* msg, int max)
{

}
