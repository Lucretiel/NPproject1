/*
 * layer3.c
 *
 *  Created on: Jan 31, 2014
 *      Author: nathan
 *
 *  Assumptions about layer 2 protocol:
 *    - Every chunk written correspond to either a successful or failed chunk
 *      read. There are no undetected lost chunks.
 *    - successful chunk reads are PROBABLY correct
 *
 *  The key here is staying in sync, when variable numbers of chunks are being
 *  written. If the reader thinks an error has occured- a malformed chunk has
 *  been set, etc- it needs to be able to find the end of the message, and the
 *  start of the next one. It needs to not accidentally consume the next
 *  message- There's no peeking the stream.
 *
 *  We should try to give the same guarantee as layer 2- no lost messages, only
 *  success or errors.
 *
 *  Implementation assumptions: sizeof(int) != 1. This seems pretty reasonable
 *  to me.
 *
 *  PROTOCOL:
 *  No header data. All chunks are written with 1 byte of metadata and X bytes
 *  of message data. The amount of message data per chunk is left unspecified
 *  except that it must be at least 1 byte, though in practice it'll always be
 *  the max until near the end. At least 1 (possibly more) of the last chunks
 *  sent WILL be empty, except for the metadata. The last N (where N is some
 *  large number < SCHAR_MAX) chunks will have a countdown byte as the metadata
 *  byte, which will decrease from N-1 to 0 each chunk. The last, empty chunk
 *  will always have a 0. When reading, these metadata bytes will be examined
 *  to make sure that ONLY this message is read, and not a future one, even in
 *  the event of lost chunks.
 *
 *  For instance:
 *  [(-1)DATA][(-1)DATA]...[(99)DATA][(98)DATA]...[(2)DATA][(1)DAT][(0)]
 *  C(MAX)    C(MAX)       C(MAX)    C(MAX)       C(MAX)   C(N)    C(1)
 *
 *  RATIONALLE:
 *  The chunks we receive are assumed to be good, and we assume every layer 2
 *  read will result in a chunk being pulled off the stream, good or bad. We
 *  can't predict which chunks will be bad; we assume that it probably won't
 *  be all of N consecutive ones.
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
