#include "Skiplist.h"

using namespace std;

// max height for the skiplist should be scaled such that 2^(MAX_LEVEL) 
// is of the magnitude of the number of elements in the list
#define MAX_LEVEL 6

//constructor for sentinel nodes (head and tail).
Node::Node(int x) { 
	volatile bool marked = false;
	volatile bool fullyLinked = false;	
	lock = 0;
	key = x;
	marked = false;
	fullyLinked = false;
	item = x;
	topLevel = MAX_LEVEL+1;
	next = new Node*[topLevel];
}

//constructor for regular nodes
Node::Node(int x, int height) {
	lock = 0;
	marked = false;
	fullyLinked = false;
	item = x;
	key = x;
	topLevel = height;
	next = new Node*[topLevel];	
}

//lock node
void Node::locked() {
	do {
		while (lock == 1)_mm_pause();
	} while (InterlockedExchange(&lock, 1));
}

//unlock node
void Node::unlocked() {
	lock = 0;
}

//LazySkipList Constructor
LazySkipList::LazySkipList() {
	srand(time(NULL));
	max = MAX_LEVEL;
	head = new Node(INT_MIN);
	tail = new Node(INT_MAX);
	for (int i = 0; i < MAX_LEVEL+1; i++) {
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
int LazySkipList::find(int x, Node** preds, Node** succs){
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
int LazySkipList::randomLevel(int max){
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
Returns false if Node with value x is already present (if it is currently
		being addded on another threadthen this add waits until the other add 
		has been completed before returning
*/
bool LazySkipList::add(int x){
	int toplevel = randomLevel(MAX_LEVEL);
	Node** preds = new Node*[MAX_LEVEL+1];
	Node** succs = new Node*[MAX_LEVEL+1];
	while (true){
		int lfound = find(x, preds, succs);
		if (lfound != -1){
			Node* nodeFound = succs[lfound];
			if (!nodeFound->marked){
				while (!nodeFound->fullyLinked) {}
				return false;
			}
			else{
				continue;
			}
		}
		int highestLocked = -1;
		Node* pred;
		Node* succ;
		bool valid = true;
		for (int level = 0; valid && (level <= toplevel); level++){
			pred = preds[level];
			succ = succs[level];
			pred->locked();
			succ->locked();
			highestLocked = level;
			valid = (!pred->marked) && (!succ->marked) && (pred->next[level] == succ);
		}
		if (!valid){
			for (int level = 0; level <= highestLocked; level++){
				preds[level]->unlocked();	
			}
			continue;
		}
		
		Node* newNode = new Node(x, toplevel);
		for (int level = 0; level <= toplevel; level++){
			newNode->next[level] = succs[level];
		}
		for (int level = 0; level <= toplevel; level++){
				preds[level]->next[level] = newNode;
		}
		newNode->fullyLinked = true;
		
		return true;
	}
	
}

/*
Remove:
Returns true if node with key is removed
Returns false if Node with value x is not present in the list or is 
		currently being removed or added but not finished being added(not fully linked)
*/
bool LazySkipList::remove(int x){
	Node* victim = NULL;
	bool isMarked = false;
	int toplevel = -1;
	Node** preds = new Node*[MAX_LEVEL + 1];
	Node** succs = new Node*[MAX_LEVEL + 1];
	while (true){
		int lfound = find(x, preds, succs);
		if (lfound != -1){
			victim = succs[lfound];
		}
		if (isMarked | ((lfound != -1) && (victim->fullyLinked && (victim->topLevel == lfound) && !victim->marked))){
			if (!isMarked){
				toplevel = victim->topLevel;
				victim->locked();
				if (victim->marked){
					victim->unlocked();
					return false;
				}
				victim->marked = true;
				isMarked = true;
			}
			int highestLocked = -1;
			Node* pred;
			Node* succ;
			bool valid = true;
			for (int level = 0; valid && (level <= toplevel); level++) {
				pred = preds[level];
				pred->locked();
				highestLocked = level;
				valid = (!pred->marked) && (pred->next[level] == victim);
			}
			if (!valid){
				for (int level = 0; level <= highestLocked; level++) {
					preds[level]->unlocked();
				}
				continue;
			}
			for (int level = toplevel; level >= 0; level--) {
					preds[level]->next[level] = victim->next[level];
			}
			victim->unlocked();
			return true;
		}
		else{
			return false;
		}
		
	}

}

/*
Contains:
Returns true if node with key x is present, unmarked and fully linked
Returns false if node with key x is absent, marked or fully linked
Note: Contains is wait free
*/
bool LazySkipList::contains(int x){
	Node** preds = new Node*[MAX_LEVEL + 1];
	Node** succs = new Node*[MAX_LEVEL + 1];
	int lfound = find(x, preds, succs);
	return ((lfound != -1) && (succs[lfound]->fullyLinked) && (!succs[lfound]->fullyLinked));
}

/*
Tests for LazySkipList on single thread
*/
void LazySkipList::test(){
	Node** preds = new Node*[MAX_LEVEL + 1];
	Node** succs = new Node*[MAX_LEVEL + 1];
	cout << "== Tests for skiplist: ==" << endl;
	cout << "Creating List... " << endl;
	LazySkipList* list =  new LazySkipList();

	cout << "Head: ";
	cout << list->head->item << endl;
	cout << "Tail: ";
	cout << list->tail->item << endl;
	cout << "Test next: " << endl;
	cout << list->head->next[0]->item << endl;
	cout << list->head->next[1]->item << endl;
	cout << list->head->next[2]->item << endl;

	cout << endl << "== Rand check: ==" << endl;
	cout << list->randomLevel(5) << list->randomLevel(5) << list->randomLevel(5) << list->randomLevel(5) << list->randomLevel(5) << list->randomLevel(5) << endl;

	cout << endl << "== Adding Tests ==" << endl;
	cout << "adding 1 - result: " << list->add(1) << endl;
	cout << "adding 10 - result: " << list->add(10) << endl;
	cout << "adding 10 - result: " << list->add(10) << endl;

	cout << endl <<  "== Finding Tests ==" << endl;
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
	list = new LazySkipList();
	for (int i = 0; i < 50; i++){
		int newnumber = rand() % 20;
		if (newnumber % 2 == 0){
			cout << "removing " << newnumber/2 << " - result: " << list->remove(newnumber/2) << endl;
		}
		else{
			cout << "adding   " << newnumber/2 << " - result: " << list->add(newnumber/2) << endl;
		}
		
	}

}

/*
int main(){
	LazySkipList* list = new LazySkipList();
	list->test();
	system("PAUSE");
}
*/