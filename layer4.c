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

#include "layers.h"

//convert an unsigned to a char[4].
static inline void value_to_char_array(unsigned value, char* buffer)
{
	for(int i = 0; i < sizeof(value); ++i)
		buffer[i] = ((value >> (i * 8)) & 0xFF);
}

//convert a char[4] to an unsigned.
static inline unsigned char_array_to_value(char* buffer)
{
	unsigned result = 0;
	for(int i = 0; i < sizeof(result); ++i)
		result |= (((unsigned)(buffer[i] & 0xFF)) << (i * 8));
	return result;
}

int layer4_write(char* msg, int len)
{
	INIT_ERROR();

	//Compute checksum
	unsigned checksum = 0;
	for(int i = 0; i < len; ++i)
		checksum += msg[i];

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
