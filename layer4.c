/*
 * layer4.c
 *
 *  Created on: Feb 5, 2014
 *      Author: nathan
 *
 *  PROTOCOL: Add all the bytes together into an unsigned int- this is the
 *  checksum. Write it as a message, then the message itself. 2 layer3 messages
 *  are sent.
 *
 *  the unsigned checksum is converted to a char[4] to transmit.
 */

#include <stdint.h>
#include "layers.h"

#define BYTE_MASK(THING) ((uint32_t)((THING) & (0xFF)))

//convert an unsigned to a char array.
static inline void value_to_char_array(uint32_t value, char* buffer)
{
	for(int i = 0; i < sizeof(value); ++i)
		buffer[i] = (value >> BYTE_MASK(i * 8));
}

//convert a char array to an unsigned.
static inline uint32_t char_array_to_value(char* buffer)
{
	uint32_t result = 0;
	for(int i = 0; i < sizeof(result); ++i)
		result |= (BYTE_MASK(buffer[i]) << (i * 8));
	return result;
}

int layer4_write(char* msg, int len)
{
	INIT_ERROR();

	//Compute checksum
	uint32_t checksum = 0;
	for(int i = 0; i < len; ++i)
		checksum += msg[i];
	//No need to mod anything, because unsigneds roll over anyway

	//Write checksum to char array
	char checksum_buf[sizeof(unsigned)];
	value_to_char_array(checksum, checksum_buf);

	//Send checksum
	CHECK_LAYER_ERROR(layer3_write(checksum_buf, sizeof(unsigned)));

	//Send message
	CHECK_LAYER_ERROR(layer3_write(msg, len));

	return len;
}

int layer4_read(char* msg, int max)
{
	INIT_ERROR();

	//Read the checksum from the stream
	char remote_checksum_buf[sizeof(unsigned)];
	CHECK_LAYER_ERROR(layer3_read(remote_checksum_buf, sizeof(unsigned)));
	unsigned remote_checksum = char_array_to_value(remote_checksum_buf);

	//Read the message from the stream
	int bytes_read;
	CHECK_LAYER_ERROR(bytes_read = layer3_read(msg, max));

	//Compute the message checksum
	int local_checksum = 0;
	for(int i = 0; i < bytes_read; ++i)
		local_checksum += msg[i];

	//Check the checksum
	if(local_checksum == remote_checksum) return bytes_read;
	else return -1;
}
