/*
 * layers.h
 *
 *  Created on: Feb 4, 2014
 *      Author: nathan
 */

#ifndef LAYERS_H_
#define LAYERS_H_

#define MAX_CHUNK_SIZE 16

typedef struct {
  char * firstname;
  char * lastname;
  int rin;
  double gpa;
} student;

int layer1_write(char b);
int layer1_read(char* b);

int layer2_write(char* chunk, int len);
int layer2_read(char* chunk, int max);

int layer3_write(char* msg, int len);
int layer3_read(char* msg, int max);

int layer4_write(char* msg, int len);
int layer4_read(char* msg, int max);

int layer5_write( student * stu );
int layer5_read( student * stu );


/*
 * PERMAFAIL MACROS
 *
 * Used to check for unrecoverable errors and make the layer dead
 */

//If condition is true, fail unrecoverably.
#define CHECK_ERROR(EXPR) if((EXPR) == -1) return error = -1

//Initialize static error; check it
#define INIT_ERROR() static int error=0; CHECK_ERROR(error)

#endif /* LAYERS_H_ */
