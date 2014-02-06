/*
 * layer4.c
 *
 *  Created on: Feb 5, 2014
 *      Author: nathan
 *
 *  PROTOCOL: add all the bytes in an int. Write this int then the msg
 */

#include <stdlib.h>
#include "layers.h"

int layer4_write(char* msg, int len)
{
	INIT_ERROR();

	int checksum = 0;
	const int checksum_size = sizeof(checksum);

	//Sadly, because layer3 is message oriented, we have to allocate
	const int write_len = len + checksum_size;
	char* const buffer = (char*)malloc(write_len);
	char* checksum_dest = buffer;
	char* buffer_seek = buffer + checksum_size;
	char* msg_seek = msg;

	//Could memcpy, but we have to loop anyway, so fuck it
	for(int i = 0; i < len; ++i)
	{
		*buffer_seek = *msg_seek;
		checksum += *msg_seek;
		++buffer_seek;
		++msg_seek;
	}

	VALUE_TO_CHAR_ARRAY(checksum, checksum_dest);
	CHECK_ERROR(layer3_write(buffer, write_len) == -1);
	return write_len;
}

int layer4_read(char* msg, int max)
{
	INIT_ERROR();

	int checksum = 0;
	const int checksum_size = sizeof(checksum);
	const int actual_max = max + checksum_size;
	char* read_buffer = (char*)malloc(actual_max);
	const int amount_read = layer3_read(read_buffer, actual_max);
	CHECK_ERROR(amount_read == -1);
	const int data_read = amount_read - checksum_size;

	char* buffer_seek = read_buffer + checksum_size;
	char* write_seek = msg;
	for(int i = 0; i < data_read; ++i)
	{
		*write_seek = *buffer_seek;
		checksum += *buffer_seek;
		++buffer_seek;
		++write_seek;
	}
	int read_checksum;
	CHAR_ARRAY_TO_VALUE(msg, read_checksum);
	if(read_checksum != checksum) return -1;
	return data_read;
}
