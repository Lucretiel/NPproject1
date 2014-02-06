/*
 * layer5.c
 *
 *  Created on: Feb 5, 2014
 *      Author: nathan
 *
 *  PROTOCOL: rin, gpa, firstname, lastname.
 */

#include <string.h>
#include <stdlib.h>
#include "layers.h"

const static int GPA_DECIMAL_PLACES = 3;
const static int GPA_MULTIPLIER = 10 * GPA_DECIMAL_PLACES;
const static int MAX_NAME_LENGTH = 80;

int layer5_write( student * stu )
{
	INIT_ERROR();

	int gpa = stu->gpa * GPA_MULTIPLIER;

	CHECK_ERROR(layer4_write((char*)&stu->rin, sizeof(stu->rin)));
	CHECK_ERROR(layer4_write((char*)&gpa, sizeof(gpa)));
	CHECK_ERROR(layer4_write(stu->firstname, strlen(stu->firstname)));
	CHECK_ERROR(layer4_write(stu->lastname, strlen(stu->lastname)));

	return 1;
}

static inline void update_name(char** student_name, const char* buffer, int length)
{
	//Remember: calloc zeroes the memory
	*student_name = calloc(length + 1, sizeof(int));
	memcpy(*student_name, buffer, length);
}

int layer5_read( student * stu )
{
	INIT_ERROR();
	int gpa = 0;

	/*
	 * Do we really need 2 buffers? No, but if we use just one, we have to make
	 * sure everything gets freed in case of errors. With 2 buffers, everything
	 * is read into static memory, errors are checked, THEN mallocs happen
	 * after potential errors have caused returns
	 */
	char firstname_buffer[MAX_NAME_LENGTH];
	int firstname_size;
	char lastname_buffer[MAX_NAME_LENGTH];
	int lastname_size;

	CHECK_ERROR(layer4_read((char*)&stu->rin, sizeof(stu->rin)));
	CHECK_ERROR(layer4_read((char*)&gpa, sizeof(gpa)));
	CHECK_ERROR(firstname_size = layer4_read(firstname_buffer, MAX_NAME_LENGTH));
	CHECK_ERROR(lastname_size = layer4_read(lastname_buffer, MAX_NAME_LENGTH));

	stu->gpa = (gpa / (float)GPA_MULTIPLIER);

	update_name(&stu->firstname, firstname_buffer, firstname_size);
	update_name(&stu->lastname, lastname_buffer, lastname_size);

	return 1;
}
