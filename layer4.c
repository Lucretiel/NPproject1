/*
 * layer4.c
 *
 *  Created on: Feb 5, 2014
 *      Author: nathan
 *
 *  PROTOCOL: Add all the bytes together into an unsigned int- this is the
 *  checksum. Write it as a message, then the message itself. 2 layer-3 messages
 *  are sent.
 */

#include "layers.h"

int layer4_write(char* msg, int len)
{
	INIT_ERROR();

	unsigned checksum = 0;
	for(int i = 0; i < len; ++i)
		checksum += msg[i];

	CHECK_ERROR(layer3_write((char*)&checksum, sizeof(checksum)));
	CHECK_ERROR(layer3_write(msg, len));

	return len;
}

int layer4_read(char* msg, int max)
{
	INIT_ERROR();

	unsigned remote_checksum;
	unsigned local_checksum = 0;
	int bytes_read;

	CHECK_ERROR(layer3_read((char*)&remote_checksum, sizeof(remote_checksum)));
	CHECK_ERROR(bytes_read = layer3_read(msg, max));

	for(int i = 0; i < bytes_read; ++i)
		local_checksum += msg[i];

	if(local_checksum != remote_checksum) return -1;
	else return bytes_read;
}
