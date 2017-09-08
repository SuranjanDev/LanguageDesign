
//	author:	Suranjan Jena
//	last modified: April 6, 2016
#ifndef HASHTABLE
#define HASHTABLE

#include<iostream>
#include<string>
#include "LinkedList.h"
#include "stack.h"
using namespace std;
/**
*	The class HashTable implements a chained hash table. It defines
*	routines to find(both current and all), insert and display
*	identifiers in the hash table.
*/
class HashTable {
private:
	LinkedList *hashTable;			// To create the array of linked lists			
	int length;						//Length of hashTable that needs to be initialized 
									/**
									*	The hashKeys function generates a key for an identifier based
									*	on its name.
									*	Parameters:		A pointer to the starting character of the string
									*	Return Value:	An integer between 0 and length of the hash table.
									*/
	int hashKeys(char * val) {
		int sum = 0;
		while (*val != '\0') {
			sum += int(*val);
			val++;
		}
		return sum % length;		//Returns the remainder obtained
	}
public:
	/**
	*	Constructor to initialize the length and the array of LinkedList.
	*	Parameters:		The length of the hash table.
	*/
	HashTable(int len) {
		length = len;
		/*Creating an array of linked lists that will be used as the starting nodes of the hashtable.
		In case of a clash the element will be appended to the first position of the node.
		*/
		hashTable = new LinkedList[length];
	}
	/**
	*	The insert function inserts the identifier into the hashtable.
	*	In case of collsion it inserts at the beginning of the chain.
	*	Parameters:		Name of the identifier and the block number.
	*/
	void insert(string name, int scope, int type, int location, string pram, int size, int id) {
		hashTable[hashKeys(&name[0])].insertInList(name, scope, type, location, pram, size, id);// Inserts at the beginning of the node.	
	}
	/**
	*	The findInCurrent function finds the identifier within the current block number.
	*	Parameters:		Name of the identifier and the block number.
	*	Return Value:	Pointer to the identifier if found else a null pointer.
	*/
	identifier* findInCurrent(string name, const int scope, string pram, int idTyp) {
		return hashTable[hashKeys(&name[0])].findInList(name, scope, pram, idTyp);	// Note: it calls findInList routine of the LinkedList class.
	}
	/**
	*	The findInAll function finds the identifier from the parent blocks.
	*	Parameters:		Name of the identifier and the stack of active blocks.
	*	Return Value:	Pointer to the identifier if found else a null pointer.
	*/
	identifier* findInAll(string name, const stack activeBlock, string pram, int idTyp) {
		stack temp = activeBlock;
		identifier *ident;
		while (!temp.isEmpty()) {								// Loop till stack is empty
			ident = findInCurrent(name, temp.currentBlock(), pram, idTyp);		// check for identifier in the current parent block
			if (ident != nullptr)
				return ident;
			temp.pop();
		}
		return nullptr;
	}
	/**
	*	The display function displays all the identifiers in a perticular scope.
	*	Parameters:		Highest value of block number.
	*/
	void display(int maxScope) {
		cout << endl << "Symbol Table displaying an identifier's Name, Type, Location, PramList(for function), Size(for array)" << endl;
		cout << "1: integer type 2: boolean type 3: double type" << endl;
		for (int i = 0; i <= maxScope; i++) {
			cout << "Scope : " << i + 1 << endl;
			for (int j = 0; j < length; j++)
				hashTable[j].displayList(i);					// Note: it calls displayList in LinkedList class.
			cout << endl;
		}
	}
};
#endif
