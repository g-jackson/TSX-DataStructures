#pragma once
typedef signed __int64      INT64, *PINT64;
#include <cstddef>
#include <iostream>                             // cout

#pragma once
class Node {
public:
	INT64 volatile key;
	Node* volatile left;
	Node* volatile right;
	Node() { key = 0; right = left = NULL; } // default constructor
};

class binTree
{
public:
	Node* volatile root; // initially NULL
	volatile long lock;
	INT64 contains(INT64);
	INT64 add(Node*);
	Node* remove(INT64); 
	INT64 isValidTree();
	INT64 maxVal(Node*);
	INT64 minVal(Node*);
	INT64 isValid(Node*);
	void halfBinTree();
	binTree(){
		root = NULL;
		lock = 0;
	}
}; // constructor