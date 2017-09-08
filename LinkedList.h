//	author:	Suranjan Jena
//	last modified: April 6, 2016
#ifndef LINKEDLIST
#define LINKEDLIST

#include<iostream>
#include<string>
using namespace std;

struct identifier {		// structure to store the identifier name and the block number.
	string name;
	int scope;
	int type;
	int location;
	string pramList;
	int size;
	int id;
	identifier *next;
};
/**
*	The class LinkedList provides routines to insert, find and display elements of the list.
*	It is used by the HashTable class to implement chained hashing.
*/
class LinkedList {
private:
	identifier* head;
public:
	/**
	*	Constructor to initialize head to null
	*/
	LinkedList() {
		head = nullptr;
	}
	/**
	*	The function insertInList inserts the current identifier at the beginning of the list
	*	Parameters:		Name of the identifier, scope, type, location in stac, parameters, size, Identifier type(array or function or just a variable).
	*/
	void insertInList(string n, int sc, int typ, int loc, string pram, int siz, int idType) {
		identifier * temp = new identifier;
		temp->name = n;
		temp->scope = sc;
		temp->type = typ;
		temp->location = loc;
		temp->pramList = pram;
		temp->size = siz;
		temp->id = idType;
		temp->next = head;
		head = temp;
	}
	/**
	*	The function findInList looks for the identifier in the given list.
	*	Parameters:		Name of the identifier and the block numer.
	*	Return value:	Pointer to the identifier if found else a null pointer.
	*/
	identifier* findInList(string n, int sc, string pram, int idType) {
		identifier * temp = head;
		if (idType == 0 || idType == 1) {
			while (temp != nullptr) {
				if (temp->name == n && temp->scope == sc && temp->pramList == pram)	// check for both name and block number. 
					return temp;
				temp = temp->next;
			}
		}
		else if (idType == 2) {
			while (temp != nullptr) {
				if (temp->name == n && temp->scope == sc && temp->pramList == pram)	// check for both name and block number. 
					return temp;
				temp = temp->next;
			}

		}
		return nullptr;
	}
	/**
	*	The function displayList looks for identifiers of a perticular block and displays the list.
	*	Parameters:		The block numer.
	*/
	void displayList(int sc) {
		identifier *temp = head;
		while (temp != nullptr) {
			
			if (temp->scope == sc)
				cout << "Name: " << temp->name << " Type: " << temp->type << " Offset: " << temp->location << " Prameters: " << temp->pramList << " Size(only for array): " << temp->size << endl;
			temp = temp->next;
		}
	}
};

#endif