#include "simpleslist.h"

using namespace std;

// max height for the skiplist should be scaled such that 2^(MAX_LEVEL) 
// is of the magnitude of the number of elements in the list
//#define MAX_LEVEL 6

//constructor for sentinel nodes (head and tail).
Node::Node(int x) {
	key = x;
	item = x;
	topLevel = MAX_LEVEL + 1;
	next = new Node*[topLevel];
}

//constructor for regular nodes
Node::Node(int x, int height) {
	item = x;
	key = x;
	topLevel = height;
	next = new Node*[topLevel];
}

//LazySkipList Constructor
SimpleSList::SimpleSList() {
	lock = 0;
	srand(time(NULL));
	max = MAX_LEVEL;
	head = new Node(INT_MIN);
	tail = new Node(INT_MAX);
	for (int i = 0; i < MAX_LEVEL + 1; i++) {
		head->next[i] = tail;
	}
}

/*
Find:
Returns -1 if not found, returns toplevel of node if found.
preds[]: now points to all nodes before find at each level
succs[]: points to nodes after each level where find node isn't
present and at find node on levels that it is present.
*/
int SimpleSList::find(int x, Node** preds, Node** succs){
	int key = x;
	int lfound = -1;
	Node* pred = head;
	for (int level = MAX_LEVEL; level >= 0; level--){
		Node* curr = pred->next[level];
		while (key > curr->key){
			pred = curr;
			curr = pred->next[level];
		}
		if (lfound == -1 && key == curr->key){
			lfound = level;
		}
		preds[level] = pred;
		succs[level] = curr;
	}
	return lfound;
}

/*
randomLevel:
Returns an int between 0 and max such that:
50% of the time result is 0,
25% of the time result is 1,
12.5% of the time result is 2,
etc.
*/
int randomLevel(int max){
	int level = 0;
	while (level < max){
		int val = rand() % 2;
		if (val == 1){
			level++;
		}
		else{
			return level;
		}
	}
	return level;
}

/*
Add:
Returns true if Node with value x is successfully added
Returns false if Node with value x is already present 
*/
bool SimpleSList::add(int x){
	int toplevel = randomLevel(MAX_LEVEL);
	Node** preds = new Node*[MAX_LEVEL + 1];
	Node** succs = new Node*[MAX_LEVEL + 1];
	int lfound = find(x, preds, succs);
	if (lfound != -1){
		return false;
	}
	Node* newNode = new Node(x, toplevel);
	for (int level = 0; level <= toplevel; level++){
		newNode->next[level] = succs[level];
	}
	for (int level = 0; level <= toplevel; level++){
		preds[level]->next[level] = newNode;
	}
	return true;
	
}

//pre-allocated memory version
bool SimpleSList::add(int x, Node* newNode){
	int toplevel = randomLevel(MAX_LEVEL);
	Node** preds = new Node*[MAX_LEVEL + 1];
	Node** succs = new Node*[MAX_LEVEL + 1];
	int lfound = find(x, preds, succs);
	if (lfound != -1){
		return false;
	}
	for (int level = 0; level <= toplevel; level++){
		newNode->next[level] = succs[level];
	}
	for (int level = 0; level <= toplevel; level++){
		preds[level]->next[level] = newNode;
	}
	return true;

}

/*
Remove:
Returns true if node with key is removed
Returns false if Node with value x is not present in the list when called
*/
bool SimpleSList::remove(int x){
	Node* victim = NULL;
	int toplevel = -1;
	Node** preds = new Node*[MAX_LEVEL + 1];
	Node** succs = new Node*[MAX_LEVEL + 1];
	int lfound = find(x, preds, succs);
	if (lfound != -1){
		victim = succs[lfound];
		toplevel = victim->topLevel;
		for (int level = toplevel; level >= 0; level--) {
			preds[level]->next[level] = victim->next[level];
		}
		return true;
		}
	else{
		return false;
	}
}

//pre-allocated memory version
bool SimpleSList::remove(int x, Node* victim){
	int toplevel = -1;
	Node** preds = new Node*[MAX_LEVEL + 1];
	Node** succs = new Node*[MAX_LEVEL + 1];
	int lfound = find(x, preds, succs);
	if (lfound != -1){
		victim = succs[lfound];
		toplevel = victim->topLevel;
		for (int level = toplevel; level >= 0; level--) {
			preds[level]->next[level] = victim->next[level];
		}
		return true;
	}
	else{
		return false;
	}
}

/*
Contains:
Returns true if node with key x is present
Returns false if node with key x is absent
*/
bool SimpleSList::contains(int x){
	Node** preds = new Node*[MAX_LEVEL + 1];
	Node** succs = new Node*[MAX_LEVEL + 1];
	int lfound = find(x, preds, succs);
	return (lfound != -1);
}

/*Fill:
Half fills list 
*/
void SimpleSList::fill(int x){
	for (int i = 0; i < x; i += 2){
		this->add(i);
	}
}

/*
Tests for SimpleSList on single thread
*/
void SimpleSList::test(){
	Node** preds = new Node*[MAX_LEVEL + 1];
	Node** succs = new Node*[MAX_LEVEL + 1];
	cout << "== Tests for skiplist: ==" << endl;
	cout << "Creating List... " << endl;
	SimpleSList* list = new SimpleSList();

	cout << "Head: ";
	cout << list->head->item << endl;
	cout << "Tail: ";
	cout << list->tail->item << endl;
	cout << "Test next: " << endl;
	cout << list->head->next[0]->item << endl;
	cout << list->head->next[1]->item << endl;
	cout << list->head->next[2]->item << endl;

	cout << endl << "== Rand check: ==" << endl;
	cout << randomLevel(5) << randomLevel(5) << randomLevel(5) << randomLevel(5) << randomLevel(5) << randomLevel(5) << endl;

	cout << endl << "== Adding Tests ==" << endl;
	cout << "adding 1 - result: " << list->add(1) << endl;
	cout << "adding 10 - result: " << list->add(10) << endl;
	cout << "adding 10 - result: " << list->add(10) << endl;

	cout << endl << "== Finding Tests ==" << endl;
	cout << "finding 1 - result: " << list->find(1, preds, succs) << endl;
	cout << "finding 10 - result: " << list->find(10, preds, succs) << endl;
	cout << "finding 11 - result: " << list->find(11, preds, succs) << endl;

	cout << endl << "== Removing Tests ==" << endl;
	cout << "removing 1 - result: " << list->remove(1) << endl;
	cout << "finding 1 - result: " << list->find(1, preds, succs) << endl;
	cout << "removing 1 - result: " << list->remove(1) << endl;
	cout << "finding 1 - result: " << list->find(1, preds, succs) << endl;

	cout << endl << "== Random Tests ==" << endl;
	cout << "Reinitialising list..." << endl;
	list = new SimpleSList();
	for (int i = 0; i < 50; i++){
		int newnumber = rand() % 20;
		if (newnumber % 2 == 0){
			cout << "removing " << newnumber / 2 << " - result: " << list->remove(newnumber / 2) << endl;
		}
		else{
			cout << "adding   " << newnumber / 2 << " - result: " << list->add(newnumber / 2) << endl;
		}

	}

}

/*
int main(){
	SimpleSList* list = new SimpleSList();
	list->test();
	system("PAUSE");
}
*/