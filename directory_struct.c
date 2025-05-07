#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "directory_struct.h"

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

	Node* etc = malloc(sizeof(Node));
	strcpy(etc->name, "etc");
	etc->parent = root;
	etc->child = NULL;
	etc->sibling = user;
	etc->file = NULL;

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

	Node* user2 = malloc(sizeof(Node));
	strcpy(user2->name, "sexyboyKim");
	user2->parent = home;
	user2->child = NULL;
	user2->sibling = NULL;
	user2->file = NULL;

	user1->sibling = user2;
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

	File* commandCat = malloc(sizeof(File));
	strcpy(commandCat->name, "cat");
	commandCat->parentNode = bin;
	commandCat->next = NULL;
	strcpy(commandCat->text, "command cat");

	File* commandEcho = malloc(sizeof(File));
	strcpy(commandEcho->name, "echo");
	commandEcho->parentNode = bin;
	commandEcho->next = NULL;
	strcpy(commandEcho->text, "command echo");

	commandCat->next = commandEcho;
	bin->file = commandCat;

	lib1->next = lib2;
	user1->file = lib1;
}