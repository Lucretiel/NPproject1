/*
 * layer3.c
 *
 *  Created on: Jan 31, 2014
 *      Author: nathan
 *
 *  Protocol: write len as sizeof(int) bytes. They are written from least to
 *  most significant, by repeatedly getting a byte from lower 8 bits then
 *  shifting. Then write the bytes in clumps
 */

#include "layers.h"

const static int int_size = sizeof(int);

//with int_size = 4: i=0->24, i=1->16, i=2->8, i=3->0
static inline int shift_amount(int i) { return (int_size - (i + 1)) * 8; }
int layer3_write(char* msg, int len)
{
	char len_bytes[int_size];
	for(int i = 0; i < int_size; ++i)
		len_bytes[i] = ((len >> (i * 8)) & 0xFF);
}

int layer3_read(char* msg, int max)
{

}
