#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "directory_struct.h"
#include "childFunction.h"

void directoryStart();

Node* root = NULL;

void directoryStart() {
	root = malloc(sizeof(Node));
	strcpy(root->name, "/");
	root->parent = NULL;
	root->child = NULL;
	root->sibling = NULL;
	root->file = NULL;

	Node* bin = malloc(sizeof(Node));
	strcpy(bin->name, "bin");
	bin->parent = root;
	bin->child = NULL;
	bin->sibling = NULL;
	bin->file = NULL;

	Node* user = malloc(sizeof(Node));
	strcpy(user->name, "user");
	user->parent = root;
	user->child = NULL;
	user->sibling = bin;
	user->file = NULL;

	Node* ect = malloc(sizeof(Node));
	strcpy(ect->name, "ect");
	ect->parent = root;
	ect->child = NULL;
	ect->sibling = user;
	ect->file = NULL;

	Node* home = malloc(sizeof(Node));
	strcpy(home->name, "home");
	home->parent = root;
	home->child = NULL;
	home->sibling = ect;
	home->file = NULL;

	root->child = home;

	Node* user1 = malloc(sizeof(Node));
	strcpy(user1->name,"oepickle");
	user1->parent = home;
	user1->child = NULL;
	user1->sibling = NULL;
	user1->file = NULL;

	home->child = user1;

	//-----------file make---------------
	File* lib1 = malloc(sizeof(File));
	strcpy(lib1->name, "file1");
	lib1->parentNode = user1;
	lib1->next = NULL;
	strcpy(lib1->text, "Hello, World1");

	File* lib2 = malloc(sizeof(File));
	strcpy(lib2->name, "file2");
	lib2->parentNode = user1;
	lib2->next = NULL;
	strcpy(lib2->text, "My name is SeoYeon.");

	lib1->next = lib2;
	user1->file = lib1;
}