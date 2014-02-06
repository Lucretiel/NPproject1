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

int layer2_write(char* chunk, int len)
{
	INIT_ERROR();

	//Enforce chunk size
	if(len < 0 || len > MAX_CHUNK_SIZE) return -1;

	//Write chunk length byte
	CHECK_LAYER_ERROR(layer1_write(len));

	//Write chunk data bytes
	for(unsigned i = 0; i < len; ++i)
		CHECK_LAYER_ERROR(layer1_write(chunk[i]));

	return len;
}

int layer2_read(char* chunk, int max)
{
	INIT_ERROR();

	//Read chunk length byte
	char len;
	CHECK_LAYER_ERROR(layer1_read(&len));

	//Check for length byte corruption
	CHECK_ERROR(len < 0 || len > MAX_CHUNK_SIZE);

	//Check that it'll fit
	CHECK_ERROR(len > max);

	/*
	 * Note: An old implementation read the whole chunk into a buffer, THEN
	 * checked len>max, so that even if len>max was false it wasn't a
	 * permanant fail. Decided not to bother, to keep the implementation simple.
	 */

	//Read chunk data bytes
	for(int i = 0; i < len; ++i)
		CHECK_LAYER_ERROR(layer1_read(chunk + i));

	return len;
}
