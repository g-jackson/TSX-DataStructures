#include <limits.h>
#include <iostream>
#include <time.h> 
#ifndef HELPER_H
#define HELPER_H
#include "helper.h" 
#endif

class Node {
	friend class LazySkipList;
private:
	int item;
	int key;
	Node** next;
	volatile long lock;
	volatile bool marked;
	volatile bool fullyLinked;
	int topLevel;
	void locked();
	void unlocked();
	Node();
	Node(int x);
	Node(int x, int height);
};

class LazySkipList{
public:
	Node* head;
	Node* tail;
	int max;
	LazySkipList();
	int find(int x, Node** preds, Node** succs);
	bool add(int x);
	bool remove(int x);
	bool contains(int x);
	int randomLevel(int max);
	void test();
};