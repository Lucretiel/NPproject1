/*
 * layers.h
 *
 *  Created on: Jan 31, 2014
 *      Author: nathan
 */

#ifndef LAYERS_H_
#define LAYERS_H_

struct _student;
typedef struct _student student;

int layer1_write(char b);
int layer1_read(char* b);

int layer2_write(char* chunk, int len);
int layer2_read(char* chunk, int max);

int layer3_write(char* msg, int len);
int layer3_read(char* msg, int max);

int layer4_write(char* msg, int len);
int layer4_read(char* msg, int max);

int layer5_write(student* stu);
int layer5_read(student* stu);

#endif /* LAYERS_H_ */
