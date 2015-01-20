typedef signed __int64      INT64, *PINT64;
#include <cstddef>

#pragma once
class Node {
public:
	INT64 key;
	Node *left;
	Node *right;
	Node() { key = 0; right = left = NULL; } // default constructor
};
class binTree
{
public:
	Node *root; // initially NULL
	volatile long lock;
	INT64 contains(INT64);
	INT64 add(Node*);
	Node* remove(INT64); 
	INT64 hleAdd(Node*);
	Node* hleRemove(INT64);
	INT64 isValidTree();
	INT64 maxVal(Node*);
	INT64 minVal(Node*);
	INT64 isValid(Node*);
	binTree(){
		root = NULL;
		lock = 0;
	}
}; // constructor