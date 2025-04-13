#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "directory_struct.h"

void directoryStart();
void fileStart(Node* connectNode);

Node* root = NULL;

void directoryStart() {
	root = malloc(sizeof(Node));
	strcpy_s(root->name, sizeof(root->name), "/");
	root->parent = NULL;
	root->child = NULL;
	root->sibling = NULL;
	root->file = NULL;

	Node* bin = malloc(sizeof(Node));
	strcpy_s(bin->name, sizeof(bin->name), "bin");
	bin->parent = root;
	bin->child = NULL;
	bin->sibling = NULL;
	bin->file = NULL;
	fileStart(bin);

	Node* user = malloc(sizeof(Node));
	strcpy_s(user->name, sizeof(user->name), "user");
	user->parent = root;
	user->child = NULL;
	user->sibling = bin;
	user->file = NULL;

	Node* ect = malloc(sizeof(Node));
	strcpy_s(ect->name, sizeof(ect->name), "ect");
	ect->parent = root;
	ect->child = NULL;
	ect->sibling = user;
	ect->file = NULL;

	Node* home = malloc(sizeof(Node));
	strcpy_s(home->name, sizeof(home->name), "home");
	home->parent = root;
	home->child = NULL;
	home->sibling = ect;
	home->file = NULL;

	root->child = home;

	Node* user1 = malloc(sizeof(Node));
	strcpy_s(user1->name, sizeof(user1->name), "oepickle");
	user1->parent = home;
	user1->child = NULL;
	user1->sibling = NULL;
	user1->file = NULL;

	home->child = user1;
}

void fileStart(Node* connectNode){
	File* lib = malloc(sizeof(File));
	strcpy_s(lib->name, sizeof(lib->name), "file1");
	connectNode->file = lib;
}