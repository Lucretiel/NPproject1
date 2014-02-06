/*
 * layers.h
 *
 *  Created on: Feb 4, 2014
 *      Author: nathan
 *
 *  Function prototypes for each layer, and some other helpers. Do NOT #include
 *  this in proj1test.c, as both files define the student struct.
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
 * These helper macros are used to cause a given function to permanently fail
 * in the event of an unrecoverable error (for instance, layer2 chunk prefix
 * length is corrupted).
 */

//If condition is true, fail forever.
#define CHECK_ERROR(EXPR) if((EXPR)) return _error = -1

//If an expression (usually a layer read/write) returns -1, fail forever.
#define CHECK_LAYER_ERROR(EXPR) CHECK_ERROR((EXPR) == -1)

/*
 * Create a static error value, and check that it hasn't previously been set.
 * Put this at the top of every function.
 *
 * Remember that static variables in a function are global- changes made in one
 * call are reflected in the next. However, the name is local to that function,
 * so the _error in the layer_write is different than the one in the layer_read.
 */
#define INIT_ERROR() static int _error=0; CHECK_ERROR(_error == -1)

#endif /* LAYERS_H_ */
