#ifndef DIRECTORY_STRUCT_H
#define DIRECTORY_STRUCT_H

typedef struct Node {
	char name[64];
	struct Node* parent;
	struct Node* child;
	struct Node* sibling;
	struct File* file;
} Node;

extern Node* root;

void directoryStart();

//file도 여기에 첨부를 해야할 것 같은데...
typedef struct File {
	char name[64];
	struct File* next;
} File;

#endif 
#pragma once