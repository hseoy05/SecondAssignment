#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "directory_struct.h"

Node* root = NULL;

void directoryStart() {
	root = malloc(sizeof(Node));
	strcpy_s(root->name, sizeof(root->name), "/");
	root->parent = NULL;
	root->child = NULL;
	root->sibling = NULL;

	Node* bin = malloc(sizeof(Node));
	strcpy_s(bin->name, sizeof(bin->name), "bin");
	File* lib = malloc(sizeof(File));
	strcpy_s(lib->name, sizeof(lib->name), "file1");
	bin->file = lib;
	bin->parent = root;
	bin->child = NULL;
	bin->sibling = NULL;

	Node* user = malloc(sizeof(Node));
	strcpy_s(user->name, sizeof(user->name), "user");
	user->parent = root;
	user->child = NULL;
	user->sibling = bin;

	Node* ect = malloc(sizeof(Node));
	strcpy_s(ect->name, sizeof(ect->name), "ect");
	ect->parent = root;
	ect->child = NULL;
	ect->sibling = user;

	Node* home = malloc(sizeof(Node));
	strcpy_s(home->name, sizeof(home->name), "home");
	home->parent = root;
	home->child = NULL;
	home->sibling = ect;

	root->child = home;

	Node* user1 = malloc(sizeof(Node));
	strcpy_s(user1->name, sizeof(user1->name), "oepickle");
	user1->parent = home;
	user1->child = NULL;
	user1->sibling = NULL;

	home->child = user1;
}