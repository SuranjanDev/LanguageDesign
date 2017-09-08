
//	author:	Suranjan Jena
//	last modified: June 1, 2016
#ifndef STACK
#define STACK
#include<iostream>

struct Stack {			// structure to implement stack.
	int blockNo;
	Stack *prev;
};
/**
*	The class stack implements a stack using linked lists. This stack is used to implement the list of active blocks.
*/
class stack {
private:
	Stack *top;
public:
	/**
	* Constructor to initialize top of stack
	*/
	stack() {
		top = NULL;
	}
	/**
	* Copy constructor
	*/
	stack(const stack & other) {
		top = NULL;
		Stack *temp = other.top;
		while (temp != NULL) {
			push(temp->blockNo);
			temp = temp->prev;
		}
	}
	/**
	*	The function push creates a node and then puts it at top of the stack.
	*	Parameters:		The value to be pushed
	*/
	void push(int data) {
		Stack *temp = new Stack;
		temp->blockNo = data;
		temp->prev = top;
		top = temp;
	}
	/**
	*	The function pops out node pointed by top.
	*/
	void pop() {
		if (!isEmpty()) {
			Stack *temp = top;
			top = temp->prev;
			delete temp;
		}
	}
	/**
	*	The function checks if the stack is empty.
	*	Return Value:	True if the stack is empty else false.
	*/
	bool isEmpty() {
		if (top == NULL)
			return true;
		return false;
	}
	/**
	*	The function returns the current data that top points to.
	*	Return Value:	Current data that top points to.
	*/
	int currentBlock() {
		return top->blockNo;
	}
};
#endif