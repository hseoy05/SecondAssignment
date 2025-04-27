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

typedef struct File {
	char name[64];
	struct Node* parentNode;
	struct File* next;
	char text[200];
} File;

#endif 
#pragma once