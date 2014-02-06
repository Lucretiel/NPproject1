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

const static int LAYER = 5;

int layer5_write( student * stu )
{
	INIT_ERROR();

	int gpa = stu->gpa * 100;

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
	 * after they're going to work.
	 */
	char firstname_buffer[80];
	int firstname_size;
	char lastname_buffer[80];
	int lastname_size;

	CHECK_ERROR(layer4_read((char*)&stu->rin, sizeof(stu->rin)));
	CHECK_ERROR(layer4_read((char*)&gpa, sizeof(gpa)));
	CHECK_ERROR(firstname_size = layer4_read(firstname_buffer, 80));
	CHECK_ERROR(lastname_size = layer4_read(lastname_buffer, 80));

	stu->gpa = (gpa / 100.0);

	update_name(&stu->firstname, firstname_buffer, firstname_size);
	update_name(&stu->lastname, lastname_buffer, lastname_size);

	return 1;
}
