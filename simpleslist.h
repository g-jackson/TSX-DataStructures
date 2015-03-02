#include <limits.h>
#include <iostream>
#include <time.h> 
#ifndef CONFIG_H
#include "config.h"
#endif

class Node {
public:
	int key;
	int item;
	Node** next;
	int topLevel;
	Node();
	Node(int x);
	Node(int x, int height);
};

class SimpleSList{
public:
	volatile long lock;
	Node* head;
	Node* tail;
	int max;
	SimpleSList();
	int find(int x, Node** preds, Node** succs);
	bool add(int x);
	bool add(int x, Node* newNode);
	bool remove(int x);
	bool remove(int x, Node* victim);
	bool contains(int x);
	void fill(int x);
	void test();
};

int randomLevel(int max);