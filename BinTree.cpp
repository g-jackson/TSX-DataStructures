#include "BinTree.h"

INT64 binTree::minVal(Node* in){
	if (in == NULL){
		return -1;
	}
	else if (in->left != NULL){
		return minVal(in->left);
	}
	else{
		return in->key;
	}
}

INT64 binTree::maxVal(Node* in){
	if (in == NULL){
		return -1;
	}
	else if (in->right != NULL){
		return minVal(in->right);
	}
	else{
		return in->key;
	}
}

INT64 binTree::isValidTree(){
	return isValid(root);
}

INT64 binTree::isValid(Node* in){
	if (in == NULL)
		return(true);

	/* false if the max of the left is > than us */
	if (in->left != NULL && maxVal(in->left) > in->key)
		return(false);

	/* false if the min of the right is <= than us */
	if (in->right != NULL && minVal(in->right) < in->key)
		return(false);

	/* false if, recursively, the left or right is not a BinTree */
	if (!isValid(in->left) || !isValid(in->right))
		return(false);

	/* passing all that, it's a BinTree */
	return(true);
}

INT64 binTree::contains(INT64 n){
	Node* volatile *pp = &root;
	Node *p = root;
	while (p->key != n){
		if (p->key < n){
			if (p->right != NULL){
				p = p->right;
			}
			else{
				return 0;
			}
		}
		else{
			if (p->left != NULL){
				p = p->left;
			}
			else{
				return 0;
			}
		}
	}
	return 1;
}

INT64 binTree::add(Node *n){
	Node* volatile *pp = &root;
	Node *p = root;
	while (p) {
		if (n->key < p->key) {
			pp = &p->left;
		}
		else if (n->key > p->key) {
			pp = &p->right;
		}
		else {
			return 0;
		}
		p = *pp;
	}
	*pp = n;
	return 1;
}

Node* binTree::remove(INT64 key){
	Node **pp = &root;
	Node *p = root;
	while (p) {
		if (key < p->key) {
			pp = &p->left;
		}
		else if (key > p->key) {
			pp = &p->right;
		}
		else {
			break;
		}
		p = *pp;
	}
	if (p == NULL)	return NULL;
	if (p->left == NULL && p->right == NULL) {
		*pp = NULL; // NO children
	}
	else if (p->left == NULL) {
		*pp = p->right; // ONE child
	}
	else if (p->right == NULL) {
		*pp = p->left; // ONE child
	}
	else {
		Node *r = p->right; // TWO children
		Node **ppr = &p->right; // find min key in right sub tree
		while (r->left) {
			ppr = &r->left;
			r = r->left;
		}
		p->key = r->key; // could move...
		p = r; // node instead
		*ppr = r->right;
	}
	return p; // return removed node
}

INT64 binTree::hleAdd(Node *n){

}

Node* binTree::hleRemove(INT64 key){

}