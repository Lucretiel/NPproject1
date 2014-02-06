/*
 * layer5.c
 *
 *  Created on: Feb 5, 2014
 *      Author: nathan
 *
 *  PROTOCOL: write 3 messages: rin/gpa, firstname, lastname.
 *    - snprintf and sscanf are used to write and read the rin/gpa
 *    - No \0s are sent with the names or the snprintf'd values. The message
 *      lengths determine the string lengths.
 *    - We could hypothetically use a single message for all the data, but this
 *      way makes it easier to support arbitrary characters (especially
 *      whitespace) in the first and last name. It also simplifies the
 *      implementation (no need for local buffers or memcpy), at the cost of
 *      increased write overhead (more chunks written)
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "layers.h"

#define MAX_NAME_LENGTH (80)
#define STAT_BUF_LENGTH /*(RIN_LENGTH + GPA_LENGTH + 2)*/ (80)

int layer5_write( student * stu )
{
	INIT_ERROR();

	char stat_buf[STAT_BUF_LENGTH];

	//string representation of rin and gpa
	snprintf(stat_buf, STAT_BUF_LENGTH, "%d/%.3f", stu->rin, stu->gpa);

	//write rin/gpa
	CHECK_LAYER_ERROR(layer4_write(stat_buf, strlen(stat_buf)));

	//write first name
	CHECK_LAYER_ERROR(layer4_write(stu->firstname, strlen(stu->firstname)));

	//write last name
	CHECK_LAYER_ERROR(layer4_write(stu->lastname, strlen(stu->lastname)));

	return 1;
}

//Creates a dynamically allocated null-terminated string from buffer
static inline char* make_name(const char* buffer, int len)
{
	char* const name = (char*)malloc(len+1);
	memcpy(name, buffer, len);

	//Add null-terminate
	name[len] = 0;

	return name;
}

int layer5_read( student * stu )
{
	INIT_ERROR();

	char stat_buf[STAT_BUF_LENGTH];
	memset(stat_buf, 0, STAT_BUF_LENGTH);

	/*
	 * Do we really need 3 buffers? No, but if we use just one, we have to make
	 * sure everything gets freed in case of errors. With 2 buffers, everything
	 * is read into static memory, errors are checked, THEN mallocs happen
	 * after potential errors have caused returns.
	 */
	//No need to add an extra byte for \0: the layer 5 protocol doesn't send it
	char firstname_buffer[MAX_NAME_LENGTH];
	char lastname_buffer[MAX_NAME_LENGTH];
	int firstname_size;
	int lastname_size;

	//Read rin/gpa
	CHECK_LAYER_ERROR(layer4_read(stat_buf, STAT_BUF_LENGTH));

	//Read firstname
	CHECK_LAYER_ERROR(firstname_size = layer4_read(
		firstname_buffer, MAX_NAME_LENGTH));

	//Read lastname
	CHECK_LAYER_ERROR(lastname_size = layer4_read(
		lastname_buffer, MAX_NAME_LENGTH));

	//Extract rin and gpa
	sscanf(stat_buf, "%d/%lf", &stu->rin, &stu->gpa);

	//Add the first name
	stu->firstname = make_name(firstname_buffer, firstname_size);

	//Add the last name
	stu->lastname = make_name(lastname_buffer, lastname_size);
	return 1;
}
